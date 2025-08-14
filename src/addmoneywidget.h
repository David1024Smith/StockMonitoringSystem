#ifndef ADDMONEYWIDGET_H
#define ADDMONEYWIDGET_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include "data.h"
class QNetworkAccessManager;
class QNetworkReply;

class addMoneyWidget : public QDialog
{
    Q_OBJECT

public:
    explicit addMoneyWidget(QWidget *parent = nullptr);
    ~addMoneyWidget();
    void setCodecData(QMap <QString,DataGP> gpMap);

    void addGP();

    void sendData();
private slots:
    void on_minecodecBtn_clicked();

    void on_noMineSearchBtn_clicked();

    void on_cancelBtn_clicked();

    void on_setBtn_clicked();

    void replyFinished(QNetworkReply *reply);
private:
    void setupUi();
    
    // UI elements
    QVBoxLayout *verticalLayout;
    QVBoxLayout *stockView;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_6;
    QComboBox *comboxCodec;
    QPushButton *minecodecBtn;
    QHBoxLayout *horizontalLayout_5;
    QComboBox *comboxJYS;
    QLineEdit *editCodec;
    QPushButton *noMineSearchBtn;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_4;
    QLabel *codecLabel;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLabel *currentPrice;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLineEdit *editPurchasePrice;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_3;
    QLineEdit *editNum;
    QHBoxLayout *horizontalLayout_7;
    QPushButton *setBtn;
    QPushButton *cancelBtn;

    QNetworkAccessManager *manager {nullptr};
    QNetworkReply *reply {nullptr};
    DataGP currentData;

};

#endif // ADDMONEYWIDGET_H
