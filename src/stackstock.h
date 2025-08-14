#ifndef STACKSTOCK_H
#define STACKSTOCK_H

#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QTabWidget>
class StockCanvas;
class StockKlineViewData;

class stackStock : public QWidget
{
    Q_OBJECT

public:
    explicit stackStock(QWidget *parent = nullptr);
    ~stackStock();
    void setData(QString code);

private:
    void setupUi();
    
    // UI elements
    QGridLayout *gridLayout;
    QTabWidget *tabWidget;
    QWidget *day;
    QGridLayout *gridLayout_2;
    QVBoxLayout *vday;
    QWidget *tab;
    QGridLayout *gridLayout_5;
    QVBoxLayout *dayK;
    QWidget *week;
    QGridLayout *gridLayout_3;
    QVBoxLayout *weekK;
    QWidget *month;
    QGridLayout *gridLayout_4;
    QVBoxLayout *monthK;

    StockCanvas *m_Stock{nullptr};

    StockKlineViewData *m_KlineDay{nullptr};
    StockKlineViewData *m_KlineWeek{nullptr};
    StockKlineViewData *m_KlineMonth{nullptr};

    QString m_codec;
    QString m_codeNum;
};

#endif // STACKSTOCK_H
