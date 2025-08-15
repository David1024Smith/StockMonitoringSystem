#include "stackstock.h"
#include "stockView/stockcanvas.h"
#include "stockKlineView/stockklineviewdata.h"
#include "application.h"
#include "QDebug"

stackStock::stackStock(QWidget *parent) :
    QWidget(parent)
{
    setupUi();
}

stackStock::~stackStock()
{
}

void stackStock::setData(QString code)
{
    if (!code.isEmpty()) {
        QString codeNum = code;
        QString codec = code.replace("sz", "1");
        codec = codec.replace("sh", "0");
        m_codec = codec;
        m_codeNum = codeNum;
//        codeNum=codeNum.replace("sz","");
//        m_codeNum=codeNum.replace("sh","");
        char  *chSecID;
        QByteArray baSecID = codec.toLatin1(); // must
        chSecID = baSecID.data();
        if (!m_Stock) {
            m_Stock = new StockCanvas(codec);
            m_Stock->setMinimumSize(400, 300);
        } else {
            m_Stock->setIDandTime(chSecID);
        }
        
        // 调试输出
        qDebug() << "日线数据设置 - 原始代码:" << code << "转换后代码:" << codec;
        m_Stock->setWindowTitle(codec);
//        m_Stock->show();


        if (!m_KlineDay) {
            m_KlineDay = new StockKlineViewData();
//            m_KlineMonth->setData(m_codeNum,App->getLastMonthTime(),App->getCurrentTime());
            m_KlineDay->setData(m_codeNum, DAYKLINE);
        } else {
//            m_KlineMonth->setData(m_codeNum,App->getLastMonthTime(),App->getCurrentTime());
            m_KlineDay->setData(m_codeNum, DAYKLINE);

        }
        m_KlineDay->setWindowTitle(codec);

        if (!m_KlineWeek) {
            m_KlineWeek = new StockKlineViewData();
//            m_KlineYear->setData(m_codeNum,App->getLastYearTime(),App->getCurrentTime());
            m_KlineWeek->setData(m_codeNum, WEEKKLINE);

        } else {
//            m_KlineYear->setData(m_codeNum,App->getLastYearTime(),App->getCurrentTime());
            m_KlineWeek->setData(m_codeNum, WEEKKLINE);
        }
        m_KlineWeek->setWindowTitle(codec);

        if (!m_KlineMonth) {
            m_KlineMonth = new StockKlineViewData();
//            m_KlineYear->setData(m_codeNum,App->getLastYearTime(),App->getCurrentTime());
            m_KlineMonth->setData(m_codeNum, MONTHKLINE);

        } else {
//            m_KlineYear->setData(m_codeNum,App->getLastYearTime(),App->getCurrentTime());
            m_KlineMonth->setData(m_codeNum, MONTHKLINE);
        }
        m_KlineMonth->setWindowTitle(codec);
//        m_KlineYear->show();
    }
    vday->addWidget(m_Stock);

    dayK->addWidget(m_KlineDay);
    weekK->addWidget(m_KlineWeek);
    monthK->addWidget(m_KlineMonth);

    this->show();
    this->setWindowTitle(m_codec);
//    m_Stock=new StockCanvas();
//    m_Stock->setIDandTime("0000001");
//    m_Stock->setMinimumHeight(300);
//    m_Stock->setStatus(ViewStatus::NOLINETIP);
}

void stackStock::setupUi()
{
    if (objectName().isEmpty())
        setObjectName(QString::fromUtf8("stackStock"));
    resize(429, 300);
    setWindowTitle("Form");
    
    gridLayout = new QGridLayout(this);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setSpacing(0);
    
    tabWidget = new QTabWidget(this);
    tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
    tabWidget->setCurrentIndex(0);
    
    // Day tab
    day = new QWidget();
    day->setObjectName(QString::fromUtf8("day"));
    
    gridLayout_2 = new QGridLayout(day);
    gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
    gridLayout_2->setContentsMargins(0, 0, 0, 0);
    gridLayout_2->setSpacing(0);
    
    vday = new QVBoxLayout();
    vday->setObjectName(QString::fromUtf8("vday"));
    vday->setSpacing(0);
    
    gridLayout_2->addLayout(vday, 0, 0, 1, 1);
    
    tabWidget->addTab(day, "日线");
    
    // Day K tab
    tab = new QWidget();
    tab->setObjectName(QString::fromUtf8("tab"));
    
    gridLayout_5 = new QGridLayout(tab);
    gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
    gridLayout_5->setContentsMargins(0, 0, 0, 0);
    gridLayout_5->setSpacing(0);
    
    dayK = new QVBoxLayout();
    dayK->setObjectName(QString::fromUtf8("dayK"));
    
    gridLayout_5->addLayout(dayK, 0, 0, 1, 1);
    
    tabWidget->addTab(tab, "日K线");
    
    // Week K tab
    week = new QWidget();
    week->setObjectName(QString::fromUtf8("week"));
    
    gridLayout_3 = new QGridLayout(week);
    gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
    gridLayout_3->setContentsMargins(0, 0, 0, 0);
    gridLayout_3->setSpacing(0);
    
    weekK = new QVBoxLayout();
    weekK->setObjectName(QString::fromUtf8("weekK"));
    
    gridLayout_3->addLayout(weekK, 0, 0, 1, 1);
    
    tabWidget->addTab(week, "周K线");
    
    // Month K tab
    month = new QWidget();
    month->setObjectName(QString::fromUtf8("month"));
    
    gridLayout_4 = new QGridLayout(month);
    gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
    gridLayout_4->setContentsMargins(0, 0, 0, 0);
    gridLayout_4->setSpacing(0);
    
    monthK = new QVBoxLayout();
    monthK->setObjectName(QString::fromUtf8("monthK"));
    monthK->setSpacing(0);
    
    gridLayout_4->addLayout(monthK, 0, 0, 1, 1);
    
    tabWidget->addTab(month, "月K线");
    
    gridLayout->addWidget(tabWidget, 0, 0, 1, 1);
}
