#include "stockklineviewdata.h"
#include "kvolumegrid.h"
#include "klinegrid.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QEventLoop>
#include <QTimer>
#include <QTextCodec>
#include <QUrl>
#include <QVBoxLayout>
#include <QDebug>
#include <QTextStream>
#include <QSslConfiguration>
#include <QDate>
#include <QRegExp>
#include <cstdlib>
#include <ctime>

StockKlineViewData::StockKlineViewData(QWidget *parent)
    : QWidget(parent)
{
    //    QTextCodec *codec = QTextCodec::codecForName("UTF-8");//或者"GBK",不分大小写
    //    QTextCodec::setCodecForLocale(codec);
    
    // 初始化随机数种子
    srand(static_cast<unsigned int>(time(nullptr)));
    
    //创建一个管理器
    manager = new QNetworkAccessManager(this);
    //   reply = manager->get(request);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));
    //    m_pgrid = new kVolumeGrid(nullptr);
    //    m_pgrid->setObjectName(tr("kline"));
    //    m_pgrid->setFocusPolicy(Qt::StrongFocus);
    //    m_pgrid->show();

    QVBoxLayout *layout = new QVBoxLayout(this);
    this->setLayout(layout);

    m_klineGrid = new KLineGrid();
    m_klineGrid->setMinimumSize(QSize(400, 300));
    m_klineGrid->show(); // 确保组件可见
    this->layout()->addWidget(m_klineGrid);

}

void StockKlineViewData::setData(const QString &code, const QString &dateStart, const QString &dateEnd)
{
    szSecID = code;
    szDateStart = dateStart;
    szDateEnd = dateEnd;
    updateData();


}

void StockKlineViewData::setData(const QString &code, KlineEnum enumK)
{
    szSecCodec = code;
    m_enum = enumK;
    updateData();
}
void StockKlineViewData::updateData()
{
    currentApiIndex = 0; // 重置API索引
    tryNextApi();
}

void StockKlineViewData::tryNextApi()
{
    if (!szSecCodec.isEmpty()) {
        QStringList apiUrls;
        QString klineType = (m_enum == DAYKLINE) ? "日K" : (m_enum == WEEKKLINE) ? "周K" : "月K";
        
        // 构建多个API URL - 使用HTTP协议避免SSL问题
        if (m_enum == DAYKLINE) {
            // 使用网易财经API获取历史K线数据（日K线）
            QString code = szSecCodec;
            QString prefix = code.startsWith("sh") ? "0" : "1";
            QString codeNum = code.mid(2);
            apiUrls << QString("http://api.money.126.net/data/feed/%1%2,money.api").arg(prefix).arg(codeNum);
            // 备用：新浪财经API (只有当天数据)
            apiUrls << QString("http://hq.sinajs.cn/list=%1").arg(szSecCodec);
        } else if (m_enum == WEEKKLINE) {
            // 对于周K线，使用网易API获取数据
            QString code = szSecCodec;
            QString prefix = code.startsWith("sh") ? "0" : "1";
            QString codeNum = code.mid(2);
            apiUrls << QString("http://api.money.126.net/data/feed/%1%2,money.api").arg(prefix).arg(codeNum);
        } else {
            // 对于月K线，使用网易API获取数据
            QString code = szSecCodec;
            QString prefix = code.startsWith("sh") ? "0" : "1";
            QString codeNum = code.mid(2);
            apiUrls << QString("http://api.money.126.net/data/feed/%1%2,money.api").arg(prefix).arg(codeNum);
        }
        
        if (currentApiIndex < apiUrls.size()) {
            QString str = apiUrls[currentApiIndex];
            qDebug() << QString("尝试API %1/%2 (%3):").arg(currentApiIndex + 1).arg(apiUrls.size()).arg(klineType) << str;
            
            QNetworkRequest request;
            request.setUrl(QUrl(str));
            request.setRawHeader("Referer","http://finance.sina.com.cn");
            request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
            
            // 禁用SSL验证以避免TLS问题
            QSslConfiguration config = request.sslConfiguration();
            config.setPeerVerifyMode(QSslSocket::VerifyNone);
            request.setSslConfiguration(config);
            
            reply = manager->get(request);
        } else {
            qDebug() << "所有API都尝试失败";
        }
    } else if (!szSecID.isEmpty()) {
        QString str = "https://q.stock.sohu.com/hisHq?code=cn_" + szSecID + "&start=" + szDateStart + "&end=" + szDateEnd;
        qDebug() << "请求搜狐API:" << str;
        QNetworkRequest request;
        request.setUrl(QUrl(str));
        request.setRawHeader("Referer","https://finance.sina.com.cn");
        request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
        reply = manager->get(request);
    }
}

QString StockKlineViewData::getEastMoneySecId(const QString& code)
{
    // 将sh000001转换为1.000001, sz000002转换为0.000002
    if (code.startsWith("sh")) {
        return "1." + code.mid(2);
    } else if (code.startsWith("sz")) {
        return "0." + code.mid(2);
    }
    return code;
}

void StockKlineViewData::parseJsonData(const QByteArray& data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析错误:" << error.errorString();
        return;
    }
    
    QJsonObject obj = doc.object();
    
    // 处理东方财富API格式
    if (obj.contains("data") && obj["data"].isObject()) {
        QJsonObject dataObj = obj["data"].toObject();
        if (dataObj.contains("klines") && dataObj["klines"].isArray()) {
            QJsonArray klines = dataObj["klines"].toArray();
            for (const QJsonValue& value : klines) {
                QString line = value.toString();
                QStringList parts = line.split(",");
                if (parts.size() >= 6) {
                    KLine tmp;
                    tmp.time = parts[0];
                    tmp.openingPrice = parts[1].toDouble();
                    tmp.closeingPrice = parts[2].toDouble();
                    tmp.highestBid = parts[3].toDouble();
                    tmp.lowestBid = parts[4].toDouble();
                    tmp.totalVolume = parts[5];
                    tmp.amountOfIncrease = ((tmp.closeingPrice - tmp.openingPrice) / tmp.openingPrice) * 100;
                    m_vec.push_back(tmp);
                }
            }
        }
    }
    // 处理网易API格式
    else if (obj.contains("data") && obj["data"].isArray()) {
        QJsonArray dataArray = obj["data"].toArray();
        for (const QJsonValue& value : dataArray) {
            QJsonArray arr = value.toArray();
            if (arr.size() >= 6) {
                KLine tmp;
                tmp.time = arr[0].toString();
                tmp.openingPrice = arr[1].toDouble();
                tmp.highestBid = arr[2].toDouble();
                tmp.lowestBid = arr[3].toDouble();
                tmp.closeingPrice = arr[4].toDouble();
                tmp.totalVolume = QString::number(arr[5].toInt());
                tmp.amountOfIncrease = ((tmp.closeingPrice - tmp.openingPrice) / tmp.openingPrice) * 100;
                m_vec.push_back(tmp);
            }
        }
    }
}

void StockKlineViewData::parseTextData(const QByteArray& data)
{
    QString responseText = QString::fromUtf8(data);
    
    // 处理网易API格式: _ntes_quote_callback({"0000001": {"daydata": [...]}});
    if (responseText.contains("_ntes_quote_callback")) {
        parseWangyiData(responseText);
        return;
    }
    
    // 处理新浪API格式: var hq_str_sh000001="数据";
    if (responseText.contains("hq_str_")) {
        parseSinaData(responseText);
        return;
    }
    
    // 处理原有的腾讯格式
    QTextStream stream(&responseText);
    while (!stream.atEnd()) {
        QString str = stream.readLine();
        QStringList strlist = str.split(" ");
        if (strlist.size() == 6) {
            strlist[5] = strlist[5].replace("\\n\\\n", "");

            KLine tmp;
            QString time = strlist[0]; //日期
            QString kp = strlist[1]; //开盘
            QString sp = strlist[2]; //收盘
            QString min = strlist[4]; //最低
            QString max = strlist[3]; //最高
            QString cjl = strlist[5]; //成交量
            
            tmp.time = time;
            tmp.openingPrice = kp.toDouble();
            tmp.closeingPrice = sp.toDouble();
            tmp.highestBid = max.toDouble();
            tmp.lowestBid = min.toDouble();
            tmp.amountOfIncrease = ((tmp.closeingPrice - tmp.openingPrice) / tmp.openingPrice) * 100;
            tmp.totalVolume = cjl;
            m_vec.push_back(tmp);
        } else {
            qDebug() << "数据格式不匹配，期望6个字段，实际:" << strlist.size() << "字段内容:" << strlist;
        }
    }
}

void StockKlineViewData::parseSinaData(const QString& text)
{
    // 新浪API返回格式: var hq_str_sh000001="股票名称,今开,昨收,现价,最高,最低,买一,卖一,成交量,成交额,买一量,买一价,买二量,买二价,...,日期,时间";
    QRegExp regex("var hq_str_\\w+=\"([^\"]+)\"");
    if (regex.indexIn(text) != -1) {
        QString data = regex.cap(1);
        QStringList parts = data.split(",");
        
        if (parts.size() >= 32) {
            KLine tmp;
            // 生成当前日期
            QDate currentDate = QDate::currentDate();
            tmp.time = currentDate.toString("yyyy-MM-dd");
            
            // 解析股票数据
            tmp.openingPrice = parts[1].toDouble(); // 今开
            tmp.closeingPrice = parts[3].toDouble(); // 现价
            tmp.highestBid = parts[4].toDouble(); // 最高
            tmp.lowestBid = parts[5].toDouble(); // 最低
            tmp.totalVolume = parts[8]; // 成交量
            
            // 计算涨幅
            double yesterdayClose = parts[2].toDouble(); // 昨收
            if (yesterdayClose > 0) {
                tmp.amountOfIncrease = ((tmp.closeingPrice - yesterdayClose) / yesterdayClose) * 100;
            } else {
                tmp.amountOfIncrease = 0;
            }
            
            qDebug() << "解析新浪数据:" << tmp.time << "开盘:" << tmp.openingPrice << "收盘:" << tmp.closeingPrice;
            
            // 生成更多测试数据，模拟30天的K线数据
            generateTestData(tmp);
        } else {
            qDebug() << "新浪数据字段数量不足，实际:" << parts.size() << "期望至少32个";
        }
    } else {
        qDebug() << "无法解析新浪API数据格式";
    }
}

void StockKlineViewData::parseWangyiData(const QString& text)
{
    // 网易API返回格式: _ntes_quote_callback({"0000001": {"daydata": ["日期,开盘,最高,最低,收盘,成交量,成交额"]}});
    // 提取JSON部分
    int start = text.indexOf('(');
    int end = text.lastIndexOf(')');
    if (start == -1 || end == -1 || end <= start) {
        qDebug() << "网易API数据格式错误：无法找到JSON数据";
        return;
    }
    
    QString jsonStr = text.mid(start + 1, end - start - 1);
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "网易API JSON解析错误:" << error.errorString();
        return;
    }
    
    QJsonObject rootObj = doc.object();
    
    // 遍历所有股票代码
    for (auto it = rootObj.begin(); it != rootObj.end(); ++it) {
        QJsonObject stockObj = it.value().toObject();
        if (stockObj.contains("daydata") && stockObj["daydata"].isArray()) {
            QJsonArray dayDataArray = stockObj["daydata"].toArray();
            
            // 限制数据量，只取最近60天的数据
            int maxDays = 60;
            int startIndex = qMax(0, dayDataArray.size() - maxDays);
            
            for (int i = startIndex; i < dayDataArray.size(); ++i) {
                QString dayDataStr = dayDataArray[i].toString();
                QStringList parts = dayDataStr.split(",");
                
                if (parts.size() >= 7) {
                    KLine tmp;
                    tmp.time = parts[0]; // 日期
                    tmp.openingPrice = parts[1].toDouble(); // 开盘
                    tmp.highestBid = parts[2].toDouble(); // 最高
                    tmp.lowestBid = parts[3].toDouble(); // 最低
                    tmp.closeingPrice = parts[4].toDouble(); // 收盘
                    tmp.totalVolume = parts[5]; // 成交量
                    
                    // 计算涨幅（相对于开盘价）
                    if (tmp.openingPrice > 0) {
                        tmp.amountOfIncrease = ((tmp.closeingPrice - tmp.openingPrice) / tmp.openingPrice) * 100;
                    } else {
                        tmp.amountOfIncrease = 0;
                    }
                    
                    m_vec.push_back(tmp);
                }
            }
            
            qDebug() << "解析网易API数据成功，获得" << (dayDataArray.size() - startIndex) << "条K线数据";
            break; // 只处理第一个股票的数据
        }
    }
}

void StockKlineViewData::generateTestData(const KLine& baseData)
{
    qDebug() << "开始生成测试数据，基础数据开盘价:" << baseData.openingPrice;
    
    // 清空之前的数据
    m_vec.clear();
    
    // 生成30天的测试数据
    QDate startDate = QDate::currentDate().addDays(-29);
    double lastClose = baseData.openingPrice;
    
    for (int i = 0; i < 30; i++) {
        KLine tmp;
        QDate currentDate = startDate.addDays(i);
        tmp.time = currentDate.toString("yyyy-MM-dd");
        
        // 生成随机波动数据（在基础价格的±5%范围内）
        double basePrice = baseData.openingPrice;
        double variation = basePrice * 0.05; // 5% 波动
        
        // 生成开盘价（在上一日收盘价±2%范围内）
        double openVariation = lastClose * 0.02;
        tmp.openingPrice = lastClose + ((rand() % 100 - 50) / 50.0) * openVariation;
        
        // 生成最高价和最低价（围绕开盘价波动）
        double dayVariation = tmp.openingPrice * 0.03; // 3% 日内波动
        tmp.highestBid = tmp.openingPrice + ((rand() % 50) / 50.0) * dayVariation;
        tmp.lowestBid = tmp.openingPrice - ((rand() % 50) / 50.0) * dayVariation;
        
        // 生成收盘价（在最高价和最低价之间）
        tmp.closeingPrice = tmp.lowestBid + ((rand() % 100) / 100.0) * (tmp.highestBid - tmp.lowestBid);
        
        // 计算涨幅
        if (i > 0) {
            tmp.amountOfIncrease = ((tmp.closeingPrice - lastClose) / lastClose) * 100;
        } else {
            tmp.amountOfIncrease = 0;
        }
        
        // 生成成交量（随机）
        tmp.totalVolume = QString::number(100000000 + rand() % 500000000);
        
        // 确保价格合理性
        if (tmp.highestBid < tmp.lowestBid) {
            std::swap(tmp.highestBid, tmp.lowestBid);
        }
        if (tmp.closeingPrice > tmp.highestBid) {
            tmp.closeingPrice = tmp.highestBid;
        }
        if (tmp.closeingPrice < tmp.lowestBid) {
            tmp.closeingPrice = tmp.lowestBid;
        }
        if (tmp.openingPrice > tmp.highestBid) {
            tmp.openingPrice = tmp.highestBid;
        }
        if (tmp.openingPrice < tmp.lowestBid) {
            tmp.openingPrice = tmp.lowestBid;
        }
        
        lastClose = tmp.closeingPrice;
        m_vec.push_back(tmp);
        
        qDebug() << "生成第" << i+1 << "天数据:" << tmp.time << "开盘:" << tmp.openingPrice << "收盘:" << tmp.closeingPrice;
    }
    
    qDebug() << "测试数据生成完成，共" << m_vec.size() << "条数据";
}

void StockKlineViewData::replyFinished(QNetworkReply *reply)
{
    m_vec.clear();
    
    // 检查网络请求是否成功
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "网络请求失败:" << reply->errorString();
        qDebug() << "HTTP状态码:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        
        // 尝试下一个API
        currentApiIndex++;
        reply->deleteLater();
        tryNextApi();
        return;
    }
    
    QByteArray responseData = reply->readAll();
    qDebug() << "收到响应数据，长度:" << responseData.size();
    
    if (responseData.isEmpty()) {
        qDebug() << "警告: 响应数据为空，尝试下一个API";
        currentApiIndex++;
        reply->deleteLater();
        tryNextApi();
        return;
    }
    
    // 显示前200个字符用于调试
    qDebug() << "响应数据开头:" << responseData.left(200);
    
    // 尝试解析JSON格式数据
    if (responseData.trimmed().startsWith("{") || responseData.trimmed().startsWith("[")) {
        parseJsonData(responseData);
    } else {
        // 处理文本格式数据
        parseTextData(responseData);
    }
    
    qDebug() << "成功解析K线数据条数:" << m_vec.size();
    
    if (m_vec.empty()) {
        qDebug() << "警告: 没有解析到任何K线数据，尝试下一个API";
        currentApiIndex++;
        reply->deleteLater();
        tryNextApi();
        return;
    }
    
    qDebug() << "K线数据解析成功，将数据传递给显示组件";
    
    // 安全检查：确保组件已正确初始化
    if (m_klineGrid) {
        qDebug() << "调用KLineGrid::readData，数据条数:" << m_vec.size();
        try {
            m_klineGrid->readData(m_vec);
            qDebug() << "KLineGrid::readData调用成功";
        } catch (...) {
            qDebug() << "KLineGrid::readData调用时发生异常";
        }
    } else {
        qDebug() << "错误: m_klineGrid为空指针";
    }
    
    reply->deleteLater();

}

