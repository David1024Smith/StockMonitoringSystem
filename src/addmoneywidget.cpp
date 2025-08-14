#include "addmoneywidget.h"
#include "application.h"
#include "signalm.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextCodec>

addMoneyWidget::addMoneyWidget(QWidget *parent) :
    QDialog(parent)
{
    setupUi();
    //创建一个管理器
    manager = new QNetworkAccessManager(this);
    //   reply = manager->get(request);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));
    setWindowTitle("添加自选");
}

addMoneyWidget::~addMoneyWidget()
{
}

void addMoneyWidget::setCodecData(QMap<QString, DataGP> gpMap)
{
    for (QString str : gpMap.keys()) {
        comboxCodec->addItem(str);
    }
}


void addMoneyWidget::on_minecodecBtn_clicked()
{
    //    App->m_db;
    if (!comboxCodec->currentText().isEmpty()) {
        QString bumStr = "http://hq.sinajs.cn/list=" + comboxCodec->currentText();
        QNetworkRequest request;
        request.setUrl(QUrl(bumStr));
        request.setRawHeader("Referer","https://finance.sina.com.cn");
        reply = manager->get(request);
    }
}

void addMoneyWidget::addGP()
{
    emit signalM::instance()->sendaddMyGP(currentData.codec, editNum->text().toInt(), editPurchasePrice->text().toDouble());
}

void addMoneyWidget::on_noMineSearchBtn_clicked()
{
    QString ZQ;
    if (comboxJYS->currentText().contains("上")) {
        ZQ = "sh";
    } else if (comboxJYS->currentText().contains("深")) {
        ZQ = "sz";
    }
    if (!editCodec->text().isEmpty()) {
        QString num = ZQ + editCodec->text();
        QString bumStr = "http://hq.sinajs.cn/list=" + num;
        QNetworkRequest request;
        request.setUrl(QUrl(bumStr));
        request.setRawHeader("Referer","https://finance.sina.com.cn");
        reply = manager->get(request);
    }
}
void addMoneyWidget::replyFinished(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();
    QTextCodec *tc = QTextCodec::codecForName("GBK");

    QString str = tc->toUnicode(data);//str如果是中文则是中文字符
    str.remove("\n");

    QList<QString> numList = str.split(";");
    for (QString numStr : numList) {
        if (!numStr.isEmpty()) {
            QList<QString> strList = numStr.split(",");
            if (strList.count() > 31) {
                QStringList str1 = strList.at(0).split("str_");
                if (str1.count() == 2) {
                    QString num = str1.at(1).mid(0, 8);
                    currentData.codec = num;
                    QStringList str2 = str1.at(1).split('\"');
                    if (str2.count() == 2) {
                        QString name = str2.at(1);
                        currentData.name = name;
                    }
                }
                currentData.TodayOpeningPrice = strList.at(1);
                currentData.YesterdayClosingPrice = strList.at(2);
                currentData.currentPrice = strList.at(3);
                currentData.todayMax = strList.at(4);
                currentData.todayMin = strList.at(5);
                currentData.Date = strList.at(30);
                currentData.Time = strList.at(31);
                if (!currentData.name.isEmpty() && !currentData.codec.isEmpty()) {
                    codecLabel->setText(currentData.name);
                    currentPrice->setText(currentData.currentPrice);
                }
            }
        }
    }
    reply->deleteLater();
}

void addMoneyWidget::on_cancelBtn_clicked()
{
    close();
}

void addMoneyWidget::on_setBtn_clicked()
{
    //    QMutexLocker locker(App->m_mutex);
    addGP();
    close();
}

void addMoneyWidget::setupUi()
{
    if (objectName().isEmpty())
        setObjectName(QString::fromUtf8("addMoneyWidget"));
    resize(421, 326);
    setWindowTitle("Form");
    
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    
    stockView = new QVBoxLayout();
    stockView->setObjectName(QString::fromUtf8("stockView"));
    verticalLayout->addLayout(stockView);
    
    verticalLayout_2 = new QVBoxLayout();
    verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
    
    horizontalLayout_6 = new QHBoxLayout();
    horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
    
    comboxCodec = new QComboBox(this);
    comboxCodec->setObjectName(QString::fromUtf8("comboxCodec"));
    horizontalLayout_6->addWidget(comboxCodec);
    
    minecodecBtn = new QPushButton(this);
    minecodecBtn->setObjectName(QString::fromUtf8("minecodecBtn"));
    minecodecBtn->setText("已添加股票查询");
    horizontalLayout_6->addWidget(minecodecBtn);
    
    verticalLayout_2->addLayout(horizontalLayout_6);
    
    horizontalLayout_5 = new QHBoxLayout();
    horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
    
    comboxJYS = new QComboBox(this);
    comboxJYS->setObjectName(QString::fromUtf8("comboxJYS"));
    comboxJYS->addItem("上证");
    comboxJYS->addItem("深圳成指");
    horizontalLayout_5->addWidget(comboxJYS);
    
    editCodec = new QLineEdit(this);
    editCodec->setObjectName(QString::fromUtf8("editCodec"));
    horizontalLayout_5->addWidget(editCodec);
    
    noMineSearchBtn = new QPushButton(this);
    noMineSearchBtn->setObjectName(QString::fromUtf8("noMineSearchBtn"));
    noMineSearchBtn->setText("自选查询查询");
    horizontalLayout_5->addWidget(noMineSearchBtn);
    
    verticalLayout_2->addLayout(horizontalLayout_5);
    
    horizontalLayout_4 = new QHBoxLayout();
    horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
    
    label_4 = new QLabel(this);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    label_4->setText("股票");
    horizontalLayout_4->addWidget(label_4);
    
    codecLabel = new QLabel(this);
    codecLabel->setObjectName(QString::fromUtf8("codecLabel"));
    codecLabel->setText("无");
    horizontalLayout_4->addWidget(codecLabel);
    
    verticalLayout_2->addLayout(horizontalLayout_4);
    
    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    
    label = new QLabel(this);
    label->setObjectName(QString::fromUtf8("label"));
    label->setText("当前价格");
    horizontalLayout->addWidget(label);
    
    currentPrice = new QLabel(this);
    currentPrice->setObjectName(QString::fromUtf8("currentPrice"));
    currentPrice->setText("无");
    horizontalLayout->addWidget(currentPrice);
    
    verticalLayout_2->addLayout(horizontalLayout);
    
    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    
    label_2 = new QLabel(this);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setText("购买时价格");
    horizontalLayout_2->addWidget(label_2);
    
    editPurchasePrice = new QLineEdit(this);
    editPurchasePrice->setObjectName(QString::fromUtf8("editPurchasePrice"));
    horizontalLayout_2->addWidget(editPurchasePrice);
    
    verticalLayout_2->addLayout(horizontalLayout_2);
    
    horizontalLayout_3 = new QHBoxLayout();
    horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
    
    label_3 = new QLabel(this);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setText("购买时股数");
    horizontalLayout_3->addWidget(label_3);
    
    editNum = new QLineEdit(this);
    editNum->setObjectName(QString::fromUtf8("editNum"));
    horizontalLayout_3->addWidget(editNum);
    
    verticalLayout_2->addLayout(horizontalLayout_3);
    
    horizontalLayout_7 = new QHBoxLayout();
    horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
    
    setBtn = new QPushButton(this);
    setBtn->setObjectName(QString::fromUtf8("setBtn"));
    setBtn->setText("配置");
    horizontalLayout_7->addWidget(setBtn);
    
    cancelBtn = new QPushButton(this);
    cancelBtn->setObjectName(QString::fromUtf8("cancelBtn"));
    cancelBtn->setText("取消");
    horizontalLayout_7->addWidget(cancelBtn);
    
    verticalLayout_2->addLayout(horizontalLayout_7);
    
    verticalLayout->addLayout(verticalLayout_2);
    
    // Connect signals
    connect(minecodecBtn, &QPushButton::clicked, this, &addMoneyWidget::on_minecodecBtn_clicked);
    connect(noMineSearchBtn, &QPushButton::clicked, this, &addMoneyWidget::on_noMineSearchBtn_clicked);
    connect(cancelBtn, &QPushButton::clicked, this, &addMoneyWidget::on_cancelBtn_clicked);
    connect(setBtn, &QPushButton::clicked, this, &addMoneyWidget::on_setBtn_clicked);
}
