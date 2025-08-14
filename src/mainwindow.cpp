#include "mainwindow.h"
#include "application.h"
#include "addmoneywidget.h"
#include "signalm.h"
#include "stackstock.h"
#include "updatemygpdialog.h"
#include "stockView/stockcanvas.h"

#include <QString>
#include <QSystemTrayIcon>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextCodec>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QColor>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QFileInfo>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi();


    //    m_trayIcon=new QSystemTrayIcon();
    //    m_trayIcon->setToolTip("test");
    //    m_trayIcon->show();

    //    normalFrame->hide();
    miniTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    myTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(signalM::instance(), &signalM::sendDataGPsChange, this, &MainWindow::slotDataGPsChange);
    connect(signalM::instance(), &signalM::sendDataHaveGPsChange, this, &MainWindow::slotDataHaveGPsChange);
    connect(signalM::instance(), &signalM::sendDataAllDPChange, this, &MainWindow::slotDataAllDPChange);
#ifdef Q_OS_WINDOWS
//    this->setWindowFlag(Qt::Tool,true);
#endif
    m_shStock = new stackStock();
    m_shStock->setData("sh000001");
    m_shStock->setMinimumHeight(300);
//    m_shStock->setStatus(ViewStatus::NOLINETIP);

    m_szStock = new stackStock();
    m_szStock->setData("sz399001");
    m_szStock->setMinimumHeight(300);
//    m_szStock->setStatus(ViewStatus::NOLINETIP);

    m_cyStock = new stackStock();
    m_cyStock->setData("sz399006");
    m_cyStock->setMinimumHeight(300);
//    m_cyStock->setStatus(ViewStatus::NOLINETIP);

    m_shLabel = new QLabel("上证指数");
    m_szLabel = new QLabel("深圳成指");
    m_cyLabel = new QLabel("创业板指");

    shlayout->addWidget(m_shLabel);
    szlayout->addWidget(m_szLabel);
    cylayout->addWidget(m_cyLabel);

    shlayout->addWidget(m_shStock);
    szlayout->addWidget(m_szStock);
    cylayout->addWidget(m_cyStock);

    initLeftMenu();
    setWindowTitle("股票监控系统");

}

MainWindow::~MainWindow()
{
    m_shStock->deleteLater();
    m_shStock = nullptr;
    m_szStock->deleteLater();
    m_szStock = nullptr;
    m_cyStock->deleteLater();
    m_cyStock = nullptr;
    m_szLabel->deleteLater();
    m_szLabel = nullptr;
    m_shLabel->deleteLater();
    m_shLabel = nullptr;
    m_cyLabel->deleteLater();
    m_cyLabel = nullptr;
}

void MainWindow::initLeftMenu()
{
    tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    miniTable->setContextMenuPolicy(Qt::CustomContextMenu);
    myTable->setContextMenuPolicy(Qt::CustomContextMenu);
    m_normalLeftMenu = new QMenu(this);
    m_myLeftMenu = new QMenu(this);

    QAction *removeNoraml = new QAction("删除", m_normalLeftMenu);
    connect(removeNoraml, &QAction::triggered, this, [ = ] {
        if (miniTable->currentRow() >= 0)
        {
            QString code = miniTable->item(miniTable->currentRow(), 0)->data(1).toString();
            if (!code.isEmpty()) {
                signalM::instance()->sendremoveGP(code);
            }
        }
        if (tableWidget->currentRow() >= 0)
        {
            QString code = tableWidget->item(tableWidget->currentRow(), 1)->data(0).toString();
            if (!code.isEmpty()) {
                signalM::instance()->sendremoveGP(code);
            }
        }
    });
    m_normalLeftMenu->addAction(removeNoraml);

    QAction *editMy = new QAction("配置", m_myLeftMenu);
    connect(editMy, &QAction::triggered, this, [ = ] {
        if (myTable->currentRow() >= 0)
        {
            QTableWidgetItem *item = myTable->item(myTable->currentRow(), 0);
            QString key = item->data(1).toString();
            DataHaveGP haveGp = m_mMyGp.value(key);
            if (!haveGp.codec.isEmpty()) {
                updateMyGpDialog dialog(haveGp);
                dialog.exec();
            }
        }
    });

    QAction *removeMy = new QAction("删除", m_myLeftMenu);
    connect(removeMy, &QAction::triggered, this, [ = ] {
        if (myTable->currentRow() >= 0)
        {
            QString code = myTable->item(myTable->currentRow(), 0)->data(1).toString();
            if (!code.isEmpty()) {
                signalM::instance()->sendremoveMyGP(code);
            }
        }
    });


    m_myLeftMenu->addAction(removeMy);

}


void MainWindow::on_pushButton_clicked()
{
    signalM::instance()->sendposthttpGp(m_currentZQ + searchEdit->text());

}


void MainWindow::on_comboBox_activated(const QString &arg1)
{
    if (arg1.contains("上")) {
        m_currentZQ = "sh";
    } else if (arg1.contains("深")) {
        m_currentZQ = "sz";
    }
}

void MainWindow::on_miniTable_doubleClicked(const QModelIndex &index)
{
    //    QMutexLocker locker(App->m_mutex);
    //    if(miniTable->currentRow()>=0){
    //        QString code=miniTable->item(miniTable->currentRow(),0)->data(1).toString();
    //        if(!code.isEmpty()){
    //            removeGP(code);
    //        }
    //    }
}


void MainWindow::on_normalBtn_clicked()
{
    //    if(ui->normalFrame->isVisible()){
    //        ui->normalFrame->setVisible(false);
    //    }
    //    else {
    //        ui->normalFrame->setVisible(true);
    //    }
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_detailedBtn_clicked()
{
    if (tableWidget->isVisible()) {
        tableWidget->setVisible(false);
    } else {
        tableWidget->setVisible(true);
    }
}

void MainWindow::on_myBtn_clicked()
{
    //    if(myTable->isVisible()){
    //        myTable->setVisible(false);
    //    }
    //    else {
    //        myTable->setVisible(true);
    //    }
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_addMyBtn_clicked()
{
    addMoneyWidget addDialog;
    addDialog.setCodecData(m_mGp);
    addDialog.exec();
}

void MainWindow::slotDataGPsChange(MapdataGP map)
{

    m_mGp = map.map;
    refreshNormalWidget();
    refreshLabel();
}

void MainWindow::slotDataHaveGPsChange(MapdataHaveGP map)
{
    m_mMyGp = map.map;
    refreshMyhaveWidget();
}

void MainWindow::slotDataAllDPChange(DataAllDP data)
{
    m_myAllDP = data;
}
void MainWindow::refreshNormalWidget()
{
    tableWidget->setRowCount(m_mGp.count());
    miniTable->setRowCount(m_mGp.count());
    if (m_mGp.count() > 0) {
        //        sendData();
        //        sendData2();

        int index = 0;
        for (auto gp : m_mGp) {
            tableWidget->setItem(index, 0, new QTableWidgetItem(gp.name));
            tableWidget->setItem(index, 1, new QTableWidgetItem(gp.codec));
            tableWidget->setItem(index, 2, new QTableWidgetItem(gp.TodayOpeningPrice));
            tableWidget->setItem(index, 3, new QTableWidgetItem(gp.YesterdayClosingPrice));
            tableWidget->setItem(index, 4, new QTableWidgetItem(gp.currentPrice));
            tableWidget->setItem(index, 5, new QTableWidgetItem(gp.todayMax));
            tableWidget->setItem(index, 6, new QTableWidgetItem(gp.todayMin));
            tableWidget->setItem(index, 7, new QTableWidgetItem(gp.Date));
            tableWidget->setItem(index, 8, new QTableWidgetItem(gp.Time));
            QString name1 = gp.name;
            QString codec = gp.codec;
            QString cuurent = gp.currentPrice;
            double zd = gp.currentPrice.toDouble() - gp.YesterdayClosingPrice.toDouble();
            QString zdStr = QString::number(zd);
            double zf = zd * 100.0 / gp.YesterdayClosingPrice.toDouble();
            QString zfStr = QString::number(zf, 'g', 3) + "%";
            QTableWidgetItem *nameitem = new QTableWidgetItem(name1);
            nameitem->setData(1, gp.codec);
            QTableWidgetItem *currentItem = new QTableWidgetItem(cuurent);
            QTableWidgetItem *zdStrItem = new QTableWidgetItem(zdStr);
            QTableWidgetItem *zfStrItem = new QTableWidgetItem(zfStr);
            if (zf > 0) {
                nameitem->setTextColor(QColor("white"));
                nameitem->setBackground(QColor("red"));
                currentItem->setTextColor(QColor("white"));
                currentItem->setBackground(QColor("red"));
                zdStrItem->setTextColor(QColor("white"));
                zdStrItem->setBackground(QColor("red"));
                zfStrItem->setTextColor(QColor("white"));
                zfStrItem->setBackground(QColor("red"));
            } else {
                nameitem->setTextColor(QColor("white"));
                nameitem->setBackground(QColor("green"));
                currentItem->setTextColor(QColor("white"));
                currentItem->setBackground(QColor("green"));
                zdStrItem->setTextColor(QColor("white"));
                zdStrItem->setBackground(QColor("green"));
                zfStrItem->setTextColor(QColor("white"));
                zfStrItem->setBackground(QColor("green"));
            }
            miniTable->setItem(index, 0, nameitem);
            miniTable->setItem(index, 1, currentItem);
            miniTable->setItem(index, 2, zdStrItem);
            miniTable->setItem(index, 3, zfStrItem);

            index++;
        }
    }
}


void MainWindow::refreshMyhaveWidget()
{
    m_myAllDP = DataAllDP();
    myTable->setRowCount(m_mMyGp.count() + 1);
    int index1 = 0;
    DataHaveGP allgp;
    for (auto gp : m_mMyGp) {
        allgp.haveNum += gp.haveNum;
        allgp.todaySY += gp.todaySY;
        allgp.historySY += gp.historySY;
        allgp.payallPrice += gp.payallPrice;
        allgp.currentPrice += gp.currentPrice;
        allgp.yesterDayPrice += gp.yesterDayPrice;
        allgp.currentallPrice += gp.currentallPrice;
        myTable->setItem(index1, 0, new QTableWidgetItem(gp.name));
        myTable->item(index1, 0)->setData(1, gp.codec);;
        myTable->setItem(index1, 1, new QTableWidgetItem(QString::number(gp.haveNum)));
        myTable->setItem(index1, 2, new QTableWidgetItem(QString::number(gp.payallPrice)));
        myTable->setItem(index1, 3, new QTableWidgetItem(QString::number(gp.currentallPrice)));
        myTable->setItem(index1, 4, new QTableWidgetItem(QString::number(gp.currentPrice)));
        myTable->setItem(index1, 5, new QTableWidgetItem(QString::number(gp.historySY)));
        double historySyl = 100 * gp.historySY / gp.payallPrice;


        QString historyL = QString::number(historySyl, 'g', 3) + "%";
        myTable->setItem(index1, 6, new QTableWidgetItem(historyL));
        if (historySyl > 0) {
            myTable->item(index1, 5)->setTextColor(QColor("white"));
            myTable->item(index1, 6)->setTextColor(QColor("white"));
            myTable->item(index1, 5)->setBackground(QColor("red"));
            myTable->item(index1, 6)->setBackground(QColor("red"));
        } else {
            myTable->item(index1, 5)->setTextColor(QColor("white"));
            myTable->item(index1, 6)->setTextColor(QColor("white"));
            myTable->item(index1, 5)->setBackground(QColor("green"));
            myTable->item(index1, 6)->setBackground(QColor("green"));
        }
        myTable->setItem(index1, 7, new QTableWidgetItem(QString::number(gp.todaySY)));
        double todayyl = 100 * (gp.currentPrice - gp.yesterDayPrice) / gp.yesterDayPrice;
        QString todayL = QString::number(todayyl, 'g', 3) + "%";
        myTable->setItem(index1, 8, new QTableWidgetItem(todayL));
        if (todayyl > 0) {
            myTable->item(index1, 7)->setTextColor(QColor("white"));
            myTable->item(index1, 8)->setTextColor(QColor("white"));
            myTable->item(index1, 7)->setBackground(QColor("red"));
            myTable->item(index1, 8)->setBackground(QColor("red"));
        } else {
            myTable->item(index1, 7)->setTextColor(QColor("white"));
            myTable->item(index1, 8)->setTextColor(QColor("white"));
            myTable->item(index1, 7)->setBackground(QColor("green"));
            myTable->item(index1, 8)->setBackground(QColor("green"));
        }
        m_myAllDP.payallPrice += gp.payallPrice;
        m_myAllDP.currentallPrice += gp.currentallPrice;
        m_myAllDP.todaySY += gp.todaySY;
        index1++;
    }
    myTable->setItem(index1, 0, new QTableWidgetItem("总收益"));
    myTable->item(index1, 0)->setData(1, "总收益");
    myTable->setItem(index1, 1, new QTableWidgetItem(QString::number(allgp.haveNum)));
    myTable->setItem(index1, 2, new QTableWidgetItem(QString::number(allgp.payallPrice)));
    myTable->setItem(index1, 3, new QTableWidgetItem(QString::number(allgp.currentallPrice)));
    myTable->setItem(index1, 4, new QTableWidgetItem(QString::number(allgp.currentPrice)));
    myTable->setItem(index1, 5, new QTableWidgetItem(QString::number(allgp.historySY)));
    double historySyl = 100 * allgp.historySY / allgp.payallPrice;


    QString historyL = QString::number(historySyl, 'g', 3) + "%";
    myTable->setItem(index1, 6, new QTableWidgetItem(historyL));
    if (historySyl > 0) {
        myTable->item(index1, 5)->setTextColor(QColor("white"));
        myTable->item(index1, 6)->setTextColor(QColor("white"));
        myTable->item(index1, 5)->setBackground(QColor("red"));
        myTable->item(index1, 6)->setBackground(QColor("red"));
    } else {
        myTable->item(index1, 5)->setTextColor(QColor("white"));
        myTable->item(index1, 6)->setTextColor(QColor("white"));
        myTable->item(index1, 5)->setBackground(QColor("green"));
        myTable->item(index1, 6)->setBackground(QColor("green"));
    }
    myTable->setItem(index1, 7, new QTableWidgetItem(QString::number(allgp.todaySY)));
    double todayyl = 100 * (allgp.todaySY) / allgp.currentallPrice;
    QString todayL = QString::number(todayyl, 'g', 3) + "%";
    myTable->setItem(index1, 8, new QTableWidgetItem(todayL));
    if (todayyl > 0) {
        myTable->item(index1, 7)->setTextColor(QColor("white"));
        myTable->item(index1, 8)->setTextColor(QColor("white"));
        myTable->item(index1, 7)->setBackground(QColor("red"));
        myTable->item(index1, 8)->setBackground(QColor("red"));
    } else {
        myTable->item(index1, 7)->setTextColor(QColor("white"));
        myTable->item(index1, 8)->setTextColor(QColor("white"));
        myTable->item(index1, 7)->setBackground(QColor("green"));
        myTable->item(index1, 8)->setBackground(QColor("green"));
    }
    cureentInfo = "今日总收益:" + QString::number(allgp.todaySY) + " \n" + " 今日收益率" + todayL;
    //    m_trayIcon->setToolTip(cureentInfo);
}

void MainWindow::refreshLabel()
{
    if (m_shLabel) {
        DataGP gp = m_mGp.value("sh000001");

        double zd = gp.currentPrice.toDouble() - gp.YesterdayClosingPrice.toDouble();
        QString zdStr = QString::number(zd);
        double zf = zd * 100.0 / gp.YesterdayClosingPrice.toDouble();
        QString zfStr = QString::number(zf, 'g', 3) + "%";
        QString sh = "上证指数";
        sh += "\n";
        sh += gp.currentPrice;
        sh += "\n";
        sh += zdStr;
        sh += "\n";
        sh += zfStr;
        m_shLabel->setText(sh);
        if (zd > 0) {
            m_shLabel->setStyleSheet("QLabel { color:red; padding:2px; border:1px solid red; border-radius:15px; }");
        } else {
            m_shLabel->setStyleSheet("QLabel { color:green; padding:2px; border:1px solid green; border-radius:15px; }");
        }
    }
    if (m_szLabel) {
        DataGP gp = m_mGp.value("sz399001");

        double zd = gp.currentPrice.toDouble() - gp.YesterdayClosingPrice.toDouble();
        QString zdStr = QString::number(zd);
        double zf = zd * 100.0 / gp.YesterdayClosingPrice.toDouble();
        QString zfStr = QString::number(zf, 'g', 3) + "%";
        QString sz = "深圳成指";
        sz += "\n";
        sz += gp.currentPrice;
        sz += "\n";
        sz += zdStr;
        sz += "\n";
        sz += zfStr;
        m_szLabel->setText(sz);
        if (zd > 0) {
            m_szLabel->setStyleSheet("QLabel { color:red; padding:2px; border:1px solid red; border-radius:15px; }");
        } else {
            m_szLabel->setStyleSheet("QLabel { color:green; padding:2px; border:1px solid green; border-radius:15px; }");
        }
    }
    if (m_cyLabel) {
        DataGP gp = m_mGp.value("sz399006");

        double zd = gp.currentPrice.toDouble() - gp.YesterdayClosingPrice.toDouble();
        QString zdStr = QString::number(zd);
        double zf = zd * 100.0 / gp.YesterdayClosingPrice.toDouble();
        QString zfStr = QString::number(zf, 'g', 3) + "%";
        QString cy = "创业板指";
        cy += "\n";
        cy += gp.currentPrice;
        cy += "\n";
        cy += zdStr;
        cy += "\n";
        cy += zfStr;
        m_cyLabel->setText(cy);
        if (zd > 0) {
            m_cyLabel->setStyleSheet("QLabel { color:red; padding:2px; border:1px solid red; border-radius:15px; }");
        } else {
            m_cyLabel->setStyleSheet("QLabel { color:green; padding:2px; border:1px solid green; border-radius:15px; }");
        }
    }
};

void MainWindow::on_miniTable_customContextMenuRequested(const QPoint &pos)
{
    m_normalLeftMenu->exec(QCursor::pos());
}

void MainWindow::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    m_normalLeftMenu->exec(QCursor::pos());
}


void MainWindow::on_myTable_customContextMenuRequested(const QPoint &pos)
{
    m_myLeftMenu->exec(QCursor::pos());
}

void MainWindow::on_myTable_cellDoubleClicked(int row, int column)
{
    qDebug() << row << column;
    QTableWidgetItem *item = myTable->item(row, 0);
    QString key = item->data(1).toString();
    DataHaveGP haveGp = m_mMyGp.value(key);
    if (!haveGp.codec.isEmpty()) {
        updateMyGpDialog dialog(haveGp);
        dialog.exec();
    }
}

void MainWindow::on_miniTable_cellDoubleClicked(int row, int column)
{
    if (miniTable->currentRow() >= 0) {
        QString code = miniTable->item(miniTable->currentRow(), 0)->data(1).toString();
        if (!code.isEmpty()) {
            QString codecS = code;
            QString codec = code.replace("sz", "1");
            codec == codec.replace("sh", "0");
            char  *chSecID;
            QByteArray baSecID = codec.toLatin1(); // must
            chSecID = baSecID.data();
            if (!m_stockWidget) {
//                m_stockWidget =new StockCanvas(codec);
                m_stockWidget = new stackStock();
                m_stockWidget->setMinimumSize(400, 300);
                m_stockWidget->setData(codecS);
            } else {
//                m_stockWidget->setIDandTime(chSecID);
                m_stockWidget->setData(codecS);
            }
            m_stockWidget->setWindowTitle(miniTable->item(miniTable->currentRow(), 0)->text());
            m_stockWidget->show();
        }
    }

}

void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    if (tableWidget->currentRow() >= 0) {
        QString code = tableWidget->item(tableWidget->currentRow(), 1)->data(0).toString();
        if (!code.isEmpty()) {
            QString codecS = code;
            QString codec = code.replace("sz", "1");
            codec == codec.replace("sh", "0");
            char  *chSecID;
            QByteArray baSecID = codec.toLatin1(); // must
            chSecID = baSecID.data();
            if (!m_stockWidget) {
//                m_stockWidget =new StockCanvas(codec);
                m_stockWidget = new stackStock();
                m_stockWidget->setMinimumSize(400, 300);
                m_stockWidget->setData(codecS);
            } else {
//                m_stockWidget->setIDandTime(chSecID);
                m_stockWidget->setData(codecS);
            }
            m_stockWidget->setWindowTitle(tableWidget->item(tableWidget->currentRow(), 0)->text());
            m_stockWidget->show();
        }
    }
}

void MainWindow::setupUi()
{
    if (objectName().isEmpty())
        setObjectName(QString::fromUtf8("MainWindow"));
    resize(1449, 1066);
    
    centralWidget = new QWidget(this);
    centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
    setCentralWidget(centralWidget);
    
    verticalLayout_2 = new QVBoxLayout(centralWidget);
    verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
    
    viewLayout = new QHBoxLayout();
    viewLayout->setObjectName(QString::fromUtf8("viewLayout"));
    
    shlayout = new QVBoxLayout();
    shlayout->setObjectName(QString::fromUtf8("shlayout"));
    viewLayout->addLayout(shlayout);
    
    szlayout = new QVBoxLayout();
    szlayout->setObjectName(QString::fromUtf8("szlayout"));
    viewLayout->addLayout(szlayout);
    
    cylayout = new QVBoxLayout();
    cylayout->setObjectName(QString::fromUtf8("cylayout"));
    viewLayout->addLayout(cylayout);
    
    verticalLayout_2->addLayout(viewLayout);
    
    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    
    normalBtn = new QPushButton(centralWidget);
    normalBtn->setObjectName(QString::fromUtf8("normalBtn"));
    normalBtn->setText("自选");
    horizontalLayout_2->addWidget(normalBtn);
    
    myBtn = new QPushButton(centralWidget);
    myBtn->setObjectName(QString::fromUtf8("myBtn"));
    myBtn->setText("持有");
    horizontalLayout_2->addWidget(myBtn);
    
    verticalLayout_2->addLayout(horizontalLayout_2);
    
    stackedWidget = new QStackedWidget(centralWidget);
    stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
    stackedWidget->setCurrentIndex(0);
    
    // Page 1
    page = new QWidget();
    page->setObjectName(QString::fromUtf8("page"));
    
    gridLayout = new QGridLayout(page);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    
    normalFrame = new QFrame(page);
    normalFrame->setObjectName(QString::fromUtf8("normalFrame"));
    normalFrame->setFrameShape(QFrame::StyledPanel);
    normalFrame->setFrameShadow(QFrame::Raised);
    
    verticalLayout = new QVBoxLayout(normalFrame);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    
    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    
    comboBox = new QComboBox(normalFrame);
    comboBox->setObjectName(QString::fromUtf8("comboBox"));
    comboBox->addItem("上证");
    comboBox->addItem("深圳成指");
    horizontalLayout->addWidget(comboBox);
    
    searchEdit = new QLineEdit(normalFrame);
    searchEdit->setObjectName(QString::fromUtf8("searchEdit"));
    horizontalLayout->addWidget(searchEdit);
    
    pushButton = new QPushButton(normalFrame);
    pushButton->setObjectName(QString::fromUtf8("pushButton"));
    pushButton->setText("添加");
    horizontalLayout->addWidget(pushButton);
    
    verticalLayout->addLayout(horizontalLayout);
    
    miniTable = new QTableWidget(normalFrame);
    miniTable->setObjectName(QString::fromUtf8("miniTable"));
    miniTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    miniTable->horizontalHeader()->setDefaultSectionSize(150);
    miniTable->horizontalHeader()->setMinimumSectionSize(50);
    
    miniTable->setColumnCount(4);
    QStringList miniHeaders;
    miniHeaders << "名称" << "最新" << "涨跌" << "涨幅";
    miniTable->setHorizontalHeaderLabels(miniHeaders);
    
    verticalLayout->addWidget(miniTable);
    
    tableWidget = new QTableWidget(normalFrame);
    tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
    tableWidget->setRowCount(1);
    tableWidget->setColumnCount(9);
    
    QStringList headers;
    headers << "名称" << "代码" << "今日开" << "昨日收" << "当前价格" 
            << "今日最高" << "今日最低" << "日期" << "时间";
    tableWidget->setHorizontalHeaderLabels(headers);
    
    verticalLayout->addWidget(tableWidget);
    
    gridLayout->addWidget(normalFrame, 0, 0, 1, 1);
    
    stackedWidget->addWidget(page);
    
    // Page 2
    page_2 = new QWidget();
    page_2->setObjectName(QString::fromUtf8("page_2"));
    
    gridLayout_2 = new QGridLayout(page_2);
    gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
    
    addMyBtn = new QPushButton(page_2);
    addMyBtn->setObjectName(QString::fromUtf8("addMyBtn"));
    addMyBtn->setText("添加我的持股");
    gridLayout_2->addWidget(addMyBtn, 0, 0, 1, 1);
    
    myTable = new QTableWidget(page_2);
    myTable->setObjectName(QString::fromUtf8("myTable"));
    myTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    myTable->setColumnCount(9);
    
    QStringList myHeaders;
    myHeaders << "名称" << "持有股数" << "成本总价" << "当前总价" << "当前单价"
              << "历史营收" << "历史收益率" << "今日营收" << "今日收益率";
    myTable->setHorizontalHeaderLabels(myHeaders);
    
    gridLayout_2->addWidget(myTable, 1, 0, 1, 1);
    
    stackedWidget->addWidget(page_2);
    
    verticalLayout_2->addWidget(stackedWidget);
    
    // Menu, toolbar and status bar
    menuBar = new QMenuBar(this);
    menuBar->setObjectName(QString::fromUtf8("menuBar"));
    menuBar->setGeometry(QRect(0, 0, 1449, 36));
    setMenuBar(menuBar);
    
    mainToolBar = new QToolBar(this);
    mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
    addToolBar(Qt::TopToolBarArea, mainToolBar);
    
    statusBar = new QStatusBar(this);
    statusBar->setObjectName(QString::fromUtf8("statusBar"));
    setStatusBar(statusBar);
    
    setWindowTitle("MainWindow");
    
    // Connect signals
    connect(normalBtn, &QPushButton::clicked, this, &MainWindow::on_normalBtn_clicked);
    connect(myBtn, &QPushButton::clicked, this, &MainWindow::on_myBtn_clicked);
    connect(pushButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_clicked);
    connect(addMyBtn, &QPushButton::clicked, this, &MainWindow::on_addMyBtn_clicked);
    connect(comboBox, QOverload<const QString &>::of(&QComboBox::activated), this, &MainWindow::on_comboBox_activated);
    connect(miniTable, &QTableWidget::doubleClicked, this, &MainWindow::on_miniTable_doubleClicked);
    connect(tableWidget, &QTableWidget::cellDoubleClicked, this, &MainWindow::on_tableWidget_cellDoubleClicked);
    connect(myTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::on_myTable_cellDoubleClicked);
    connect(miniTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::on_miniTable_cellDoubleClicked);
    connect(miniTable, &QTableWidget::customContextMenuRequested, this, &MainWindow::on_miniTable_customContextMenuRequested);
    connect(tableWidget, &QTableWidget::customContextMenuRequested, this, &MainWindow::on_tableWidget_customContextMenuRequested);
    connect(myTable, &QTableWidget::customContextMenuRequested, this, &MainWindow::on_myTable_customContextMenuRequested);
}
