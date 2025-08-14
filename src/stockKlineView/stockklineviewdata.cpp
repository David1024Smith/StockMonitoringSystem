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

StockKlineViewData::StockKlineViewData(QWidget *parent)
    : QWidget(parent)
{
    //    QTextCodec *codec = QTextCodec::codecForName("UTF-8");//或者"GBK",不分大小写
    //    QTextCodec::setCodecForLocale(codec);
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
    this->layout()->addWidget(m_klineGrid);
    //    m_klineGrid->setMinimumSize(QSize(400,300));

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
            // 使用新浪财经API (简单文本格式)
            apiUrls << QString("http://hq.sinajs.cn/list=%1").arg(szSecCodec);
            // 腾讯API（改为HTTP）
            apiUrls << QString("http://qt.gtimg.cn/q=%1").arg(szSecCodec);
        } else if (m_enum == WEEKKLINE) {
            // 对于周K和月K，暂时使用日K数据进行模拟
            apiUrls << QString("http://hq.sinajs.cn/list=%1").arg(szSecCodec);
        } else {
            // 对于月K，也使用日K数据
            apiUrls << QString("http://hq.sinajs.cn/list=%1").arg(szSecCodec);
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
            m_vec.push_back(tmp);
        } else {
            qDebug() << "新浪数据字段数量不足，实际:" << parts.size() << "期望至少32个";
        }
    } else {
        qDebug() << "无法解析新浪API数据格式";
    }
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

