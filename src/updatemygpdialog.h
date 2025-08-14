#ifndef UPDATEMYGPDIALOG_H
#define UPDATEMYGPDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "data.h"
class StockCanvas;

class updateMyGpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit updateMyGpDialog(DataHaveGP map,QWidget *parent = nullptr);
    ~updateMyGpDialog();

private slots:
    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

    void on_totalEdit_textChanged(const QString &arg1);

    void on_numEdit_textChanged(const QString &arg1);

    void on_eveEdit_textChanged(const QString &arg1);


private:
    void setupUi();
    
    // UI elements
    QVBoxLayout *verticalLayout;
    QVBoxLayout *stockView;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label;
    QLabel *codecLabel;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_2;
    QLabel *nameLabel;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QLineEdit *totalEdit;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_5;
    QLineEdit *eveEdit;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_4;
    QLineEdit *numEdit;
    QHBoxLayout *horizontalLayout;
    QPushButton *okBtn;
    QPushButton *cancelBtn;
    
    DataHaveGP m_map;
    StockCanvas *m_stockWidget{nullptr};
};

#endif // UPDATEMYGPDIALOG_H
