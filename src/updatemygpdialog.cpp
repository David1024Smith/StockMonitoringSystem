#include "updatemygpdialog.h"
#include "signalm.h"
#include "stockView/stockcanvas.h"

updateMyGpDialog::updateMyGpDialog(DataHaveGP map, QWidget *parent) :
    QDialog(parent),
    m_map(map)
{
    setupUi();
    codecLabel->setText(m_map.codec);
    nameLabel->setText(m_map.name);
    totalEdit->setText(QString::number(m_map.payallPrice));
    numEdit->setText(QString::number(m_map.haveNum));
    double eveNum = m_map.payallPrice / m_map.haveNum;
    eveEdit->setText(QString::number(eveNum));
    setWindowTitle("购买股票配置");

    QString codec = m_map.codec.replace("sz", "1");
    codec == codec.replace("sh", "0");
    char  *chSecID;
    QByteArray baSecID = codec.toLatin1(); // must
    chSecID = baSecID.data();
    m_stockWidget = new StockCanvas(codec);
    m_stockWidget->setMinimumSize(400, 300);
    stockView->addWidget(m_stockWidget);
}


updateMyGpDialog::~updateMyGpDialog()
{
    m_stockWidget->deleteLater();
    m_stockWidget = nullptr;
}

void updateMyGpDialog::on_okBtn_clicked()
{
    //    if(m_db.isValid()){

    //        double dnum=inum;
    //        double addMoney =dnum*chasePrice;
    //        QSqlQuery query(m_db);
    //        //占位符 : + 自定义名字
    //        //        query.prepare("insert into myData(code) values(:code)");
    //        QString insert_sql = "insert into haveData values (?, ?, ?)";
    //        query.prepare(insert_sql);
    //        query.addBindValue(codec);
    //        query.addBindValue(QString::number(addMoney));
    //        query.addBindValue(inum);
    //        //        query.bindValue(":code", str);
    //        query.setForwardOnly(true);
    //        query.exec();
    //        DataHaveGP gp;
    //        gp.codec=codec;
    //        gp.payallPrice=addMoney;
    //        gp.haveNum=inum;
    //        m_mMyGp.insert(codec,gp);
    //    }
    double money = totalEdit->text().toDouble();
    int number = numEdit->text().toInt();
    QString str = QString("UPDATE  haveData SET money = %1 , number= %2  WHERE code ='%3'").arg(money).arg(number).arg(m_map.codec);
    emit signalM::instance()->sendExecDb(str);
    emit signalM::instance()->refreashHaveData();
    close();
}

void updateMyGpDialog::on_cancelBtn_clicked()
{
    close();
}

void updateMyGpDialog::on_totalEdit_textChanged(const QString &arg1)
{
    double eveNum = totalEdit->text().toDouble() / numEdit->text().toDouble();
    QString eveStr = QString::number(eveNum);
    if (eveStr != eveEdit->text()) {
        eveEdit->setText(eveStr);
    }

}

void updateMyGpDialog::on_numEdit_textChanged(const QString &arg1)
{
    double eveNum = totalEdit->text().toDouble() / numEdit->text().toDouble();
    QString eveStr = QString::number(eveNum);
    if (eveStr != eveEdit->text()) {
        eveEdit->setText(eveStr);
    }
}

void updateMyGpDialog::on_eveEdit_textChanged(const QString &arg1)
{
    double totalNum = eveEdit->text().toDouble() * numEdit->text().toInt();
    QString totalStr = QString::number(totalNum);
    if (totalStr != totalEdit->text()) {
        totalEdit->setText(totalStr);
    }
}

void updateMyGpDialog::setupUi()
{
    if (objectName().isEmpty())
        setObjectName(QString::fromUtf8("updateMyGpDialog"));
    resize(348, 282);
    setWindowTitle("Dialog");
    
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    
    stockView = new QVBoxLayout();
    stockView->setObjectName(QString::fromUtf8("stockView"));
    verticalLayout->addLayout(stockView);
    
    horizontalLayout_5 = new QHBoxLayout();
    horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
    
    label = new QLabel(this);
    label->setObjectName(QString::fromUtf8("label"));
    label->setText("股票代码");
    horizontalLayout_5->addWidget(label);
    
    codecLabel = new QLabel(this);
    codecLabel->setObjectName(QString::fromUtf8("codecLabel"));
    codecLabel->setText("TextLabel");
    horizontalLayout_5->addWidget(codecLabel);
    
    verticalLayout->addLayout(horizontalLayout_5);
    
    horizontalLayout_4 = new QHBoxLayout();
    horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
    
    label_2 = new QLabel(this);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setText("股票名称");
    horizontalLayout_4->addWidget(label_2);
    
    nameLabel = new QLabel(this);
    nameLabel->setObjectName(QString::fromUtf8("nameLabel"));
    nameLabel->setText("TextLabel");
    horizontalLayout_4->addWidget(nameLabel);
    
    verticalLayout->addLayout(horizontalLayout_4);
    
    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    
    label_3 = new QLabel(this);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setText("成本总价");
    horizontalLayout_2->addWidget(label_3);
    
    totalEdit = new QLineEdit(this);
    totalEdit->setObjectName(QString::fromUtf8("totalEdit"));
    horizontalLayout_2->addWidget(totalEdit);
    
    verticalLayout->addLayout(horizontalLayout_2);
    
    horizontalLayout_6 = new QHBoxLayout();
    horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
    
    label_5 = new QLabel(this);
    label_5->setObjectName(QString::fromUtf8("label_5"));
    label_5->setText("平均股价");
    horizontalLayout_6->addWidget(label_5);
    
    eveEdit = new QLineEdit(this);
    eveEdit->setObjectName(QString::fromUtf8("eveEdit"));
    horizontalLayout_6->addWidget(eveEdit);
    
    verticalLayout->addLayout(horizontalLayout_6);
    
    horizontalLayout_3 = new QHBoxLayout();
    horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
    
    label_4 = new QLabel(this);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    label_4->setText("购买股数");
    horizontalLayout_3->addWidget(label_4);
    
    numEdit = new QLineEdit(this);
    numEdit->setObjectName(QString::fromUtf8("numEdit"));
    horizontalLayout_3->addWidget(numEdit);
    
    verticalLayout->addLayout(horizontalLayout_3);
    
    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    
    okBtn = new QPushButton(this);
    okBtn->setObjectName(QString::fromUtf8("okBtn"));
    okBtn->setText("确定");
    horizontalLayout->addWidget(okBtn);
    
    cancelBtn = new QPushButton(this);
    cancelBtn->setObjectName(QString::fromUtf8("cancelBtn"));
    cancelBtn->setText("取消");
    horizontalLayout->addWidget(cancelBtn);
    
    verticalLayout->addLayout(horizontalLayout);
    
    // Connect signals
    connect(okBtn, &QPushButton::clicked, this, &updateMyGpDialog::on_okBtn_clicked);
    connect(cancelBtn, &QPushButton::clicked, this, &updateMyGpDialog::on_cancelBtn_clicked);
    connect(totalEdit, &QLineEdit::textChanged, this, &updateMyGpDialog::on_totalEdit_textChanged);
    connect(numEdit, &QLineEdit::textChanged, this, &updateMyGpDialog::on_numEdit_textChanged);
    connect(eveEdit, &QLineEdit::textChanged, this, &updateMyGpDialog::on_eveEdit_textChanged);
}
