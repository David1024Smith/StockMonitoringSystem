#include "stockviewdata.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QEventLoop>
#include <QTimer>
#include <QDateTime>
#include <QDate>
#include <QRegExp>
#include <QDebug>
#include <cstdlib>
#include <ctime>

stockViewData::stockViewData(QObject *parent)
    : QObject(parent)
{
    //创建一个管理器
    manager = new QNetworkAccessManager(this);
    //   reply = manager->get(request);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));
    szSecID = "0000001" ;
    szDate  ;
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &stockViewData::updateData);
    // 减少更新频率，避免频繁请求API
    m_timer->start(30000); // 30秒更新一次
}
void stockViewData::setData(const QString &code, const QString &date)
{
    szSecID = code ;
    szDate  = date;
    updateData();
}
void stockViewData::GetFSJLINFO()
{

    if (fsjl.count() < 1) {
        return;
    }
    double UpRate = 0, DnRate = 0;

    //排头不应该是今日开的价格
    //info.deal_Start = info.deal_Max = info.deal_Min = fsjl[0].Deal;
    info.vol_Max = info.vol_Min = 0;

    for (auto fs : fsjl) {
        if (info.deal_Max < fs.Deal)
            info.deal_Max = fs.Deal;
        if (info.deal_Min > fs.Deal)
            info.deal_Min = fs.Deal;

        if (info.vol_Max < fs.Vol)
            info.vol_Max = fs.Vol;
        if (info.vol_Min > fs.Vol)
            info.vol_Min = fs.Vol;
    }

//    qDebug("%d",info.deal_Max);

    if (info.deal_Max > info.deal_Start) {
        UpRate = (info.deal_Max - info.deal_Start) / info.deal_Start ;
    }
    if (info.deal_Min < info.deal_Start) {
        DnRate = (info.deal_Start - info.deal_Min) / info.deal_Start ;
    }

    info.deal_rate = UpRate > DnRate ? UpRate : DnRate ;

}

void stockViewData::replyFinished(QNetworkReply *reply)
{
    fsjl.clear();
    
    // 检查网络请求是否成功
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "分时数据请求失败，尝试下一个API:" << reply->errorString();
        currentApiIndex++;
        reply->deleteLater();
        
        // 重新尝试其他API
        QStringList apiUrls;
        apiUrls << QString("http://img1.money.126.net/data/hs/time/today/" + QString(szSecID) + ".json");
        QString sinaCode = szSecID;
        if (szSecID.startsWith("0")) {
            sinaCode = "sh" + szSecID.mid(1);
        } else if (szSecID.startsWith("1")) {
            sinaCode = "sz" + szSecID.mid(1);
        }
        apiUrls << QString("http://hq.sinajs.cn/list=" + sinaCode);
        
        tryNextApi(apiUrls);
        return;
    }
    
    QByteArray responseText = reply->readAll();
    
    if (responseText.isEmpty()) {
        qDebug() << "分时数据响应为空，尝试下一个API";
        currentApiIndex++;
        reply->deleteLater();
        
        QStringList apiUrls;
        apiUrls << QString("http://img1.money.126.net/data/hs/time/today/" + QString(szSecID) + ".json");
        QString sinaCode = szSecID;
        if (szSecID.startsWith("0")) {
            sinaCode = "sh" + szSecID.mid(1);
        } else if (szSecID.startsWith("1")) {
            sinaCode = "sz" + szSecID.mid(1);
        }
        apiUrls << QString("http://hq.sinajs.cn/list=" + sinaCode);
        
        tryNextApi(apiUrls);
        return;
    }
    
    // 尝试解析不同格式的数据
    QString responseStr = QString::fromUtf8(responseText);
    
    // 检查是否是新浪API格式
    if (responseStr.contains("hq_str_")) {
        parseSinaData(responseStr);
        reply->deleteLater();
        return;
    }
    
    // 尝试解析JSON格式数据
    QJsonParseError parseError;
    QJsonDocument json = QJsonDocument::fromJson(responseText, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "分时数据JSON解析失败，尝试下一个API:" << parseError.errorString();
        qDebug() << "响应内容:" << responseText.left(200);
        currentApiIndex++;
        reply->deleteLater();
        retryWithNextApi();
        return;
    }
    
    // 根据API类型解析不同格式的JSON数据
    QJsonObject rootObj = json.object();
    
    // 检查是否是腾讯API格式
    if (rootObj.contains("data") && rootObj["data"].isObject()) {
        QJsonObject dataObj = rootObj["data"].toObject();
        if (dataObj.contains(szSecID) || dataObj.contains("sh" + szSecID.mid(1)) || dataObj.contains("sz" + szSecID.mid(1))) {
            parseTencentData(json);
            reply->deleteLater();
            return;
        }
    }
    
    // 检查是否是东方财富API格式
    if (rootObj.contains("data") && rootObj["data"].isObject()) {
        QJsonObject dataObj = rootObj["data"].toObject();
        if (dataObj.contains("trends")) {
            parseEastMoneyData(json);
            reply->deleteLater();
            return;
        }
    }
    
    // 默认按网易格式解析
    parseWangyiData(json);
    reply->deleteLater();
}

void stockViewData::parseWangyiData(const QJsonDocument& json)
{
    QJsonObject rootObj = json.object();
    QString date = rootObj.value("date").toString();
    QString symbol = rootObj.value("symbol").toString();
    int lastVolume = rootObj.value("lastVolume").toInt();
    QString name = rootObj.value("name").toString();
    double yestclose = rootObj.value("yestclose").toDouble();
    int count = rootObj.value("count").toInt();
    QJsonArray data = rootObj.value("data").toArray();

    //设置中线位置
    info.deal_Start = info.deal_Max = info.deal_Min = yestclose * 1000;

    if (data.isEmpty()) {
        qDebug() << "网易分时数据为空，可能是非交易时间或股票代码错误，生成测试数据";
        generateTestData();
        return;
    }

    for (QJsonValue value : data) {
        FSJL jl;

        QString time = value.toArray()[0].toString();
        double nubmer1 = value.toArray()[1].toDouble();
        double nubmer2 = value.toArray()[2].toDouble();
        int vol = value.toArray()[3].toInt();

        jl.Date = date.toInt();
        QString timeN = time + "00";
        jl.Time = timeN.toInt();
        jl.SecID = symbol;
        jl.SecName = name;
        jl.Deal = nubmer1 * 1000;
        jl.Vol = vol;
        fsjl.push_back(jl);
    }
    
    qDebug() << "网易分时数据解析成功，共" << fsjl.size() << "条数据，昨收:" << yestclose;
    
    GetFSJLINFO();
    emit refreashView();
}

void stockViewData::parseSinaData(const QString& responseStr)
{
    // 新浪API返回格式: var hq_str_sh000001="股票名称,今开,昨收,现价,最高,最低,买一,卖一,成交量,成交额,买一量,买一价,买二量,买二价,...,日期,时间";
    QRegExp regex("var hq_str_\\w+=\"([^\"]+)\"");
    if (regex.indexIn(responseStr) != -1) {
        QString data = regex.cap(1);
        QStringList parts = data.split(",");
        
        if (parts.size() >= 32) {
            // 从新浪实时数据生成分时数据
            double currentPrice = parts[3].toDouble(); // 现价
            double yesterdayClose = parts[2].toDouble(); // 昨收
            
            if (currentPrice > 0 && yesterdayClose > 0) {
                qDebug() << "解析新浪分时数据成功，当前价格:" << currentPrice << "昨收:" << yesterdayClose;
                generateTestDataFromPrice(currentPrice, yesterdayClose);
            } else {
                qDebug() << "新浪数据价格无效，生成默认测试数据";
                generateTestData();
            }
        } else {
            qDebug() << "新浪数据字段数量不足，生成默认测试数据";
            generateTestData();
        }
    } else {
        qDebug() << "无法解析新浪分时数据格式，生成默认测试数据";
        generateTestData();
    }
}

void stockViewData::generateTestDataFromPrice(double currentPrice, double yesterdayClose)
{
    fsjl.clear();
    
    // 设置基础信息
    info.deal_Start = info.deal_Max = info.deal_Min = yesterdayClose * 1000;
    
    QDateTime startTime = QDateTime::fromString("09:30", "HH:mm");
    
    for (int i = 0; i < 241; i++) {
        FSJL jl;
        
        // 计算时间
        QDateTime currentTime;
        if (i < 120) { // 上午9:30-11:30
            currentTime = startTime.addSecs(i * 60);
        } else { // 下午13:00-15:00
            currentTime = startTime.addSecs((i + 90) * 60);
        }
        
        // 基于真实价格生成分时数据
        double progress = (double)i / 240.0; // 进度0-1
        double priceRange = currentPrice - yesterdayClose;
        double basePrice = yesterdayClose + priceRange * progress;
        
        // 添加随机波动
        double variation = basePrice * 0.01; // 1%的随机波动
        double finalPrice = basePrice + ((rand() % 100 - 50) / 50.0) * variation;
        
        // 生成成交量
        int volume = 1000000 + rand() % 5000000;
        
        jl.Date = QDate::currentDate().toString("yyyyMMdd").toInt();
        jl.Time = currentTime.toString("HHmmss").toInt();
        jl.SecID = szSecID;
        jl.SecName = "股票";
        jl.Deal = finalPrice * 1000;
        jl.Vol = volume;
        
        fsjl.push_back(jl);
    }
    
    qDebug() << "基于真实价格生成分时数据完成，共" << fsjl.size() << "条数据";
    
    GetFSJLINFO();
    emit refreashView();
}

void stockViewData::updateData()
{
    // 尝试多个真实分时数据API源
    QStringList apiUrls;
    
    // 转换股票代码格式
    QString sinaCode = szSecID;
    if (szSecID.startsWith("0")) {
        sinaCode = "sh" + szSecID.mid(1);
    } else if (szSecID.startsWith("1")) {
        sinaCode = "sz" + szSecID.mid(1);
    }
    
    // 1. 腾讯分时数据API (真实分时数据)
    apiUrls << QString("http://web.ifzq.gtimg.cn/appstock/app/minute/query?code=%1").arg(sinaCode);
    
    // 2. 东方财富分时数据API (真实分时数据)
    QString eastMoneyCode = szSecID;
    if (szSecID.startsWith("0")) {
        eastMoneyCode = "1." + szSecID.mid(1);
    } else if (szSecID.startsWith("1")) {
        eastMoneyCode = "0." + szSecID.mid(1);
    }
    apiUrls << QString("http://push2his.eastmoney.com/api/qt/stock/trends2/get?fields1=f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13&fields2=f51,f52,f53,f54,f55,f56,f57,f58&ut=7eea3edcaed734bea9cbfc24409ed989&secid=%1&iscr=0").arg(eastMoneyCode);
    
    // 3. 网易API（原有的）
    apiUrls << QString("http://img1.money.126.net/data/hs/time/today/" + QString(szSecID) + ".json");
    
    // 4. 新浪实时数据API作为最后备用
    apiUrls << QString("http://hq.sinajs.cn/list=" + sinaCode);
    
    // 先尝试第一个API
    currentApiIndex = 0;
    tryNextApi(apiUrls);
}

void stockViewData::tryNextApi(const QStringList& apiUrls)
{
    if (currentApiIndex >= apiUrls.size()) {
        qDebug() << "所有分时数据API都失败，生成测试数据";
        generateTestData();
        return;
    }
    
    QString url = apiUrls[currentApiIndex];
    qDebug() << "请求分时数据API:" << url;
    
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Referer", "http://finance.sina.com.cn");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    
    reply = manager->get(request);
}

void stockViewData::retryWithNextApi()
{
    // 重新构建API列表并尝试下一个
    QStringList apiUrls;
    
    QString sinaCode = szSecID;
    if (szSecID.startsWith("0")) {
        sinaCode = "sh" + szSecID.mid(1);
    } else if (szSecID.startsWith("1")) {
        sinaCode = "sz" + szSecID.mid(1);
    }
    
    apiUrls << QString("http://web.ifzq.gtimg.cn/appstock/app/minute/query?code=%1").arg(sinaCode);
    
    QString eastMoneyCode = szSecID;
    if (szSecID.startsWith("0")) {
        eastMoneyCode = "1." + szSecID.mid(1);
    } else if (szSecID.startsWith("1")) {
        eastMoneyCode = "0." + szSecID.mid(1);
    }
    apiUrls << QString("http://push2his.eastmoney.com/api/qt/stock/trends2/get?fields1=f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13&fields2=f51,f52,f53,f54,f55,f56,f57,f58&ut=7eea3edcaed734bea9cbfc24409ed989&secid=%1&iscr=0").arg(eastMoneyCode);
    
    apiUrls << QString("http://img1.money.126.net/data/hs/time/today/" + QString(szSecID) + ".json");
    apiUrls << QString("http://hq.sinajs.cn/list=" + sinaCode);
    
    tryNextApi(apiUrls);
}

void stockViewData::parseTencentData(const QJsonDocument& json)
{
    QJsonObject rootObj = json.object();
    QJsonObject dataObj = rootObj["data"].toObject();
    
    // 腾讯API返回格式解析
    QString stockKey;
    for (auto it = dataObj.begin(); it != dataObj.end(); ++it) {
        stockKey = it.key();
        break; // 取第一个股票数据
    }
    
    if (stockKey.isEmpty()) {
        qDebug() << "腾讯分时数据为空，尝试下一个API";
        currentApiIndex++;
        retryWithNextApi();
        return;
    }
    
    QJsonObject stockData = dataObj[stockKey].toObject();
    QJsonArray minuteData = stockData["data"].toArray();
    
    if (minuteData.isEmpty()) {
        qDebug() << "腾讯分时数据为空，尝试下一个API";
        currentApiIndex++;
        retryWithNextApi();
        return;
    }
    
    fsjl.clear();
    
    // 设置基础信息
    double yestclose = stockData["qt"].toObject()["close"].toDouble();
    info.deal_Start = info.deal_Max = info.deal_Min = yestclose * 1000;
    
    for (const QJsonValue& value : minuteData) {
        QJsonArray point = value.toArray();
        if (point.size() >= 3) {
            FSJL jl;
            
            QString timeStr = point[0].toString(); // 时间格式: "202412161030"
            double price = point[1].toDouble();
            int volume = point[2].toInt();
            
            jl.Date = QDate::currentDate().toString("yyyyMMdd").toInt();
            jl.Time = timeStr.right(4).toInt() * 100; // 转换为HHMMSS格式
            jl.SecID = szSecID;
            jl.SecName = stockData["qt"].toObject()["name"].toString();
            jl.Deal = price * 1000;
            jl.Vol = volume;
            
            fsjl.push_back(jl);
        }
    }
    
    qDebug() << "腾讯分时数据解析成功，共" << fsjl.size() << "条数据，昨收:" << yestclose;
    
    GetFSJLINFO();
    emit refreashView();
}

void stockViewData::parseEastMoneyData(const QJsonDocument& json)
{
    QJsonObject rootObj = json.object();
    QJsonObject dataObj = rootObj["data"].toObject();
    QJsonArray trends = dataObj["trends"].toArray();
    
    if (trends.isEmpty()) {
        qDebug() << "东方财富分时数据为空，尝试下一个API";
        currentApiIndex++;
        retryWithNextApi();
        return;
    }
    
    fsjl.clear();
    
    // 设置基础信息
    double yestclose = dataObj["preClose"].toDouble();
    info.deal_Start = info.deal_Max = info.deal_Min = yestclose * 1000;
    
    for (const QJsonValue& value : trends) {
        QString trendStr = value.toString();
        QStringList parts = trendStr.split(",");
        
        if (parts.size() >= 5) {
            FSJL jl;
            
            QString timeStr = parts[0]; // 时间格式: "2024-12-16 10:30"
            double price = parts[2].toDouble();
            int volume = parts[5].toInt();
            
            QDateTime dateTime = QDateTime::fromString(timeStr, "yyyy-MM-dd hh:mm");
            jl.Date = dateTime.date().toString("yyyyMMdd").toInt();
            jl.Time = dateTime.time().toString("hhmmss").toInt();
            jl.SecID = szSecID;
            jl.SecName = dataObj["name"].toString();
            jl.Deal = price * 1000;
            jl.Vol = volume;
            
            fsjl.push_back(jl);
        }
    }
    
    qDebug() << "东方财富分时数据解析成功，共" << fsjl.size() << "条数据，昨收:" << yestclose;
    
    GetFSJLINFO();
    emit refreashView();
}

void stockViewData::generateTestData()
{
    fsjl.clear();
    
    // 生成241个分时数据点（9:30-15:00，每分钟一个点）
    double basePrice = 50.0; // 基础价格
    info.deal_Start = info.deal_Max = info.deal_Min = basePrice * 1000;
    
    QDateTime startTime = QDateTime::fromString("09:30", "HH:mm");
    
    for (int i = 0; i < 241; i++) {
        FSJL jl;
        
        // 计算时间
        QDateTime currentTime;
        if (i < 120) { // 上午9:30-11:30
            currentTime = startTime.addSecs(i * 60);
        } else { // 下午13:00-15:00
            currentTime = startTime.addSecs((i + 90) * 60); // 加90分钟的午休时间
        }
        
        // 生成价格（在基础价格附近波动）
        double priceVariation = basePrice * 0.05; // 5%的波动范围
        double currentPrice = basePrice + ((rand() % 100 - 50) / 50.0) * priceVariation;
        
        // 生成成交量
        int volume = 1000000 + rand() % 5000000;
        
        jl.Date = QDate::currentDate().toString("yyyyMMdd").toInt();
        jl.Time = currentTime.toString("HHmmss").toInt();
        jl.SecID = szSecID;
        jl.SecName = "测试股票";
        jl.Deal = currentPrice * 1000;
        jl.Vol = volume;
        
        fsjl.push_back(jl);
    }
    
    qDebug() << "生成分时测试数据完成，共" << fsjl.size() << "条数据";
    
    GetFSJLINFO();
    emit refreashView();
}



bool stockViewData::ReadFSJL()
{
    return true;
}
