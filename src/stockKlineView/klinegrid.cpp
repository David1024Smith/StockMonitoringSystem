#include "klinegrid.h"

#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QVector>
#include <QDockWidget>
#include <QWidget>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>
#include <QResizeEvent>
#include <exception>

#include "mainwindow.h"

KLineGrid::KLineGrid(QWidget *parent) : AutoGrid(parent)
{
    //开启鼠标追踪
    setMouseTracking(true);

    m_mutex = new QMutex();

    initial();
}
bool KLineGrid::readData(QString strFile)
{
    QMutexLocker locker(m_mutex);
    if (mDataFile.readData(strFile))
        return true;
    else
        return false;
}
bool KLineGrid::readData(std::vector<KLine> datas)
{
    if (datas.empty()) {
        qDebug() << "KLineGrid::readData - 数据为空";
        return false;
    }
    
    QMutexLocker locker(m_mutex);
    
    try {
        if (!mDataFile.readData(datas)) {
            qDebug() << "KLineGrid::readData - DataFile.readData失败";
            return false;
        }
        
        // 修复索引设置，确保正确的边界
        totalDay = mDataFile.kline.size();
        beginDay = 0;
        endDay = totalDay; // endDay作为上界，不包含在绘制范围内
        currentDay = totalDay / 2;
        

        
        // 重置价格范围
        highestBid = 0;
        lowestBid = 100000;
        maxVolume = 0;
        
        // 确保组件尺寸正确初始化
        if (getWidgetWidth() == 0 || getWidgetHeight() == 0) {
            QResizeEvent resizeEvent(size(), size());
            this->resizeEvent(&resizeEvent);
        }
        
        update();
        
        qDebug() << "KLineGrid数据加载成功，共" << totalDay << "条K线数据";
        
        return true;
    } catch (const std::exception& e) {
        qDebug() << "KLineGrid::readData异常:" << e.what();
        return false;
    } catch (...) {
        qDebug() << "KLineGrid::readData未知异常";
        return false;
    }
}

KLineGrid::~KLineGrid()
{
    if (m_mutex) {
        delete m_mutex;
        m_mutex = nullptr;
    }
    if (mShowDrtail) {
        delete mShowDrtail;
        mShowDrtail = nullptr;
    }
}

void KLineGrid::initial()
{

//    //读取数据
//    QString file = tr("dataKLine.txt");
//    if( !mDataFile.readData(file) )
//    {
//        QMessageBox::about(this,tr("数据文件读取失败"),tr("确定"));
//        return ;
//    }
//    //开启鼠标追踪
//    setMouseTracking(true);
//    //初始化一些成员变量

//    endDay = mDataFile.kline.size() - 1;
//    totalDay = 200;
//    beginDay  = endDay - totalDay;
//    currentDay = beginDay + totalDay /2;
//    if( beginDay < 0)
//    {
//        beginDay = 0;
//        totalDay = mDataFile.kline.size();
//    }
//    highestBid = 0;
//    lowestBid = 1000;
//    maxVolume = 0;


    //构造详情展示页面

    /*
        mShowDrtail = new ShowDetail(this);
        //mShowDrtail->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
        mShowDrtail->setFeatures(QDockWidget::QDockWidget::DockWidgetVerticalTitleBar);
        QWidget* main = this->parentWidget() ;
        static_cast<MainWindow*>(main)->addDockWidget(Qt::AllDockWidgetAreas,mShowDrtail);
        QWidget* titleWidget = new QWidget(this);
        //mShowDrtail->setTitleBarWidget( titleWidget );
        mShowDrtail->resize(50,100);
        //mShowDrtail->setGeometry(20,20,100,300);
        //mShowDrtail->move(20,20);
    */


    //构造详细数据展示页面
//    mShowDrtail = new ShowDetail(this);
//    mShowDrtail->setModal(false);
//    mShowDrtail->setFixedSize(140,700);
//    mShowDrtail->show();

    //mShowDrtail->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
//    QWidget* main = this->parentWidget() ;
    //static_cast<MainWindow*>(main)->addDockWidget(Qt::LeftDockWidgetArea,mShowDrtail);
    //delete main;

}


void KLineGrid::paintEvent(QPaintEvent *event)
{
    QMutexLocker locker(m_mutex);
    AutoGrid::paintEvent(event);
    //画k线
    drawLine();
}


void KLineGrid::drawLine()
{
    //获取y轴指标
    getIndicator();

    //显示y轴价格
    drawYtick();

    //画k线
    drawKline();

    //画十字线
    if (!isKeyDown && bCross) {
        drawCross2();
    }

    if (isKeyDown && bCross) {
        drawCross();
    }

    //画均线
    if (isDrawAverageLine) {
        drawAverageLine(5);
        drawAverageLine(10);
        drawAverageLine(20);
        drawAverageLine(30);
        drawAverageLine(60);
    }
}


void KLineGrid::getIndicator()
{
    highestBid = 0;
    lowestBid = 100000;
    maxVolume = 0;

    if (mDataFile.kline.empty()) {
        return;
    }

    // 确保索引范围有效
    int actualBeginDay = qMax(0, beginDay);
    int actualEndDay = qMin(endDay, (int)mDataFile.kline.size());
    
    if (actualBeginDay >= actualEndDay) {
        return;
    }

    for (int i = actualBeginDay; i < actualEndDay; ++i) {
        if (mDataFile.kline[i].highestBid > highestBid)
            highestBid = mDataFile.kline[i].highestBid;
        if (mDataFile.kline[i].lowestBid < lowestBid)
            lowestBid = mDataFile.kline[i].lowestBid;
    }
    
    // 如果最高价和最低价相同，需要调整范围以避免除零错误
    if (highestBid == lowestBid) {
        double mid = highestBid;
        highestBid = mid + mid * 0.01; // 增加1%
        lowestBid = mid - mid * 0.01;  // 减少1%
        if (lowestBid < 0) lowestBid = 0;
    }
}

void KLineGrid::drawYtick()
{
    QPainter painter(this);
    QPen     pen;
    pen.setColor(Qt::red);
    painter.setPen(pen);

    double ystep = (highestBid - lowestBid) / getHGridNum();
    QString str;


    if (0 == getHGridNum()) {
        str.sprintf("%.2f", lowestBid);
        painter.drawText(QPoint(getWidgetWidth() - getMarginLeft() + 10,
                                getWidgetHeight() - getMarginBottom()),
                         str);
        str.sprintf("%.2f", highestBid);
        painter.drawText(QPoint(getWidgetWidth() - getMarginLeft() + 10,
                                getMarginTop()),
                         str);
        return;
    }

    for (int i = 0; i <= getHGridNum(); ++i) {
        str.sprintf("%.2f", lowestBid + i * ystep);
        painter.drawText(QPoint(getWidgetWidth() - getMarginLeft() + 10,
                                getWidgetHeight() - getMarginBottom() - i * getAtomGridHeight()),
                         str);
    }
}

void KLineGrid::drawKline()
{
    if (mDataFile.kline.empty()) {
        return;
    }
    
    QPainter painter(this);
    QPen     pen;
    pen.setColor(Qt::red);
    painter.setPen(pen);

    // 确保索引范围有效
    int actualBeginDay = qMax(0, beginDay);
    int actualEndDay = qMin(endDay, (int)mDataFile.kline.size());
    
    if (actualBeginDay >= actualEndDay) {
        return;
    }

    //y轴缩放
    if (highestBid == lowestBid) {
        yscale = 1.0;
    } else {
        yscale = getGridHeight() / (highestBid - lowestBid);
    }

    //画线连接的两个点
    QPoint p1, p2, p3, p4;

    if (getGridWidth() <= 0 || getGridHeight() <= 0) {
        return;
    }

    // 修复X轴步长计算，使用实际显示的数据量
    int displayDataCount = actualEndDay - actualBeginDay;
    double xstep = (displayDataCount > 1) ? (getGridWidth() / displayDataCount) : getGridWidth();

    for (int i = actualBeginDay; i < actualEndDay; ++i) {
        // 设置K线颜色
        if (mDataFile.kline[i].openingPrice > mDataFile.kline[i].closeingPrice)
            pen.setColor(QColor(85, 252, 252)); // 阴线：青色
        else
            pen.setColor(Qt::red); // 阳线：红色

        // 计算K线宽度
        lineWidth = (displayDataCount > 1) ? (getGridWidth() / displayDataCount) : 20;
        lineWidth = lineWidth - 0.2 * lineWidth; // 设置间隔
        if (lineWidth < 3) lineWidth = 3; // 最小线宽

        // 计算当前K线的X位置
        double xPos = getMarginLeft() + xstep * (i - actualBeginDay);

        //阴线 (开盘价 > 收盘价)
        if (mDataFile.kline[i].openingPrice > mDataFile.kline[i].closeingPrice) {
            //画开盘与收盘之间的粗实线
            pen.setWidth(lineWidth);
            painter.setPen(pen);
            p1.setX(xPos + 0.5 * lineWidth);
            p1.setY(getWidgetHeight() - (mDataFile.kline[i].openingPrice - lowestBid) * yscale - getMarginBottom());
            p2.setX(xPos + 0.5 * lineWidth);
            p2.setY(getWidgetHeight() - (mDataFile.kline[i].closeingPrice - lowestBid) * yscale - getMarginBottom());
            painter.drawLine(p1, p2);

            //画最高价与最低价之间的细线
            pen.setWidth(1);
            painter.setPen(pen);
            p1.setX(xPos + 0.5 * lineWidth);
            p1.setY(getWidgetHeight() - (mDataFile.kline[i].highestBid - lowestBid) * yscale - getMarginBottom());
            p2.setX(xPos + 0.5 * lineWidth);
            p2.setY(getWidgetHeight() - (mDataFile.kline[i].lowestBid - lowestBid) * yscale - getMarginBottom());
            painter.drawLine(p1, p2);
        } else {
            //阳线画成空心的
            pen.setWidth(1);
            painter.setPen(pen);

            // 画矩形框
            p1.setX(xPos);
            p1.setY(getWidgetHeight() - (mDataFile.kline[i].openingPrice - lowestBid) * yscale - getMarginBottom());
            p2.setX(xPos + lineWidth);
            p2.setY(getWidgetHeight() - (mDataFile.kline[i].openingPrice - lowestBid) * yscale - getMarginBottom());
            p3.setX(xPos);
            p3.setY(getWidgetHeight() - (mDataFile.kline[i].closeingPrice - lowestBid) * yscale - getMarginBottom());
            p4.setX(xPos + lineWidth);
            p4.setY(getWidgetHeight() - (mDataFile.kline[i].closeingPrice - lowestBid) * yscale - getMarginBottom());

            painter.drawLine(p1, p2);
            painter.drawLine(p1, p3);
            painter.drawLine(p2, p4);
            painter.drawLine(p3, p4);

            //画最高价与最低价之间的细线
            double y1 = qMax(mDataFile.kline[i].openingPrice, mDataFile.kline[i].closeingPrice);
            double y2 = qMin(mDataFile.kline[i].openingPrice, mDataFile.kline[i].closeingPrice);

            p1.setX(xPos + 0.5 * lineWidth);
            p1.setY(getWidgetHeight() - (mDataFile.kline[i].highestBid - lowestBid) * yscale - getMarginBottom());
            p2.setX(xPos + 0.5 * lineWidth);
            p2.setY(getWidgetHeight() - (y1 - lowestBid) * yscale - getMarginBottom());
            p3.setX(xPos + 0.5 * lineWidth);
            p3.setY(getWidgetHeight() - (y2 - lowestBid) * yscale - getMarginBottom());
            p4.setX(xPos + 0.5 * lineWidth);
            p4.setY(getWidgetHeight() - (mDataFile.kline[i].lowestBid - lowestBid) * yscale - getMarginBottom());

            painter.drawLine(p1, p2);
            painter.drawLine(p3, p4);
        }
    }
}

void KLineGrid::keyPressEvent(QKeyEvent *event)
{
    currentDay = (double)(mousePoint.x() - getMarginLeft()) / (getGridWidth()) * totalDay + beginDay;

    isKeyDown = true;
    switch (event->key()) {
    case Qt::Key_Left: {
        double xstep = getGridWidth() / totalDay ;

        if (mousePoint.x() - xstep < getMarginLeft()) {
            if (beginDay - 1 < 0)
                return;
            endDay -= 1;
            beginDay -= 1;
        } else
            mousePoint.setX(mousePoint.x() - xstep);

        update();
        break;
    }

    case Qt::Key_Right: {
        double xstep = getGridWidth() / totalDay ;

        if (mousePoint.x() + xstep > getWidgetWidth() - getMarginRight()) {
            if (endDay >= mDataFile.kline.size())
                return;
            endDay += 1;
            beginDay += 1;
        } else
            mousePoint.setX(mousePoint.x() + xstep);

        update();
        break;
    }

    case Qt::Key_Up: {
        totalDay = totalDay / 2;

        //最少显示10个
        if (totalDay < 10) {
            totalDay *= 2;
            return;
        }

        endDay = currentDay + totalDay / 2;
        beginDay = currentDay - totalDay / 2;

        if (endDay > mDataFile.kline.size()) {
            endDay = mDataFile.kline.size();
            beginDay = endDay - totalDay;
        }

        if (beginDay < 0) {
            beginDay = 0;
            endDay = beginDay + totalDay;
            if (endDay > mDataFile.kline.size()) {
                endDay = mDataFile.kline.size();
            }
        }

        update();
        break;
    }

    case Qt::Key_Down: {
        if (totalDay >= mDataFile.kline.size())
            return;

        totalDay = totalDay * 2;
        if (totalDay > mDataFile.kline.size()) {
            totalDay = mDataFile.kline.size();
        }

        endDay = currentDay + totalDay / 2;
        if (endDay > mDataFile.kline.size()) {
            endDay = mDataFile.kline.size();
        }

        beginDay = currentDay - totalDay / 2;
        if (beginDay < 0)
            beginDay = 0;

        totalDay = endDay - beginDay;

        update();
        break;
    }
    default:
        break;
    }
}

void KLineGrid::mouseMoveEvent(QMouseEvent *event)
{
    mousePoint = event->pos();
    isKeyDown = false;
    update();
}


void KLineGrid::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        bCross = !bCross;
        update();
    }
}


void KLineGrid::resizeEvent(QResizeEvent *event)
{

    AutoGrid::resizeEvent(event);
    bCross = false;

}

void KLineGrid::drawCross()
{

    drawCrossVerLine();
    drawCrossHorLine();
    drawTips();
}

void KLineGrid::drawCrossVerLine()
{
    QPainter painter(this);
    QPen     pen;
    pen.setColor(QColor("#FFFFFF"));
    pen.setWidth(1);
    painter.setPen(pen);


    double xstep = getGridWidth() / totalDay ;
    double xPos = getMarginLeft() ;
    while (mousePoint.x() - xPos > xstep) {
        xPos += xstep;
    }
    xPos += 0.5 * lineWidth;
    QLine horline(xPos, getMarginTop(), xPos, getWidgetHeight() - getMarginBottom());
    painter.drawLine(horline);

}

void KLineGrid::drawCrossHorLine()
{
    QPainter painter(this);
    QPen     pen;
    pen.setColor(QColor("#FFFFFF"));
    pen.setWidth(1);
    painter.setPen(pen);


    double yPos;
    currentDay = (mousePoint.x() - getMarginLeft()) * totalDay / getGridWidth() + beginDay;


    if (mDataFile.kline[currentDay].openingPrice < mDataFile.kline[currentDay].closeingPrice)
        yPos = (mDataFile.kline[currentDay].closeingPrice - lowestBid) * yscale ;
    else
        yPos = (mDataFile.kline[currentDay].closeingPrice - lowestBid) * yscale ;

    QLine verline(getMarginLeft(), getWidgetHeight() - getMarginBottom() - yPos,
                  getWidgetWidth() - getMarginRight(), getWidgetHeight() - getMarginBottom() - yPos);
    painter.drawLine(verline);

}




void KLineGrid::drawTips()
{
    QPainter painter(this);
    QPen     pen;
    QBrush brush(QColor(64, 0, 128));
    painter.setBrush(brush);
    pen.setColor(QColor("#FFFFFF"));
    pen.setWidth(1);
    painter.setPen(pen);


    int currentDay = (mousePoint.x() - getMarginLeft()) * totalDay / getGridWidth() + beginDay;
    double yval = mDataFile.kline[currentDay].closeingPrice;


    double yPos;
    if (mDataFile.kline[currentDay].openingPrice < mDataFile.kline[currentDay].closeingPrice)
        yPos = (mDataFile.kline[currentDay].closeingPrice - lowestBid) * yscale ;
    else
        yPos = (mDataFile.kline[currentDay].closeingPrice - lowestBid) * yscale ;


    yPos = getWidgetHeight() - getMarginBottom() - yPos;

    int iTipsWidth = 60;
    int iTipsHeight = 30;

    QString str;

    QRect rect(getWidgetWidth() - getMarginRight(),
               yPos - iTipsHeight / 2, iTipsWidth, iTipsHeight);
    painter.drawRect(rect);


    QRect rectText(getWidgetWidth() - getMarginRight() + iTipsWidth / 4,
                   yPos - iTipsHeight / 4, iTipsWidth, iTipsHeight);
    painter.drawText(rectText, str.sprintf("%.2f", yval));



    if (currentDay == 0)
        return;


    QColor openingColor = mDataFile.kline[currentDay].openingPrice > mDataFile.kline[currentDay - 1].openingPrice ?
                          QColor("#FF0000") : QColor("#00FF00");

    QColor highestColor = mDataFile.kline[currentDay].highestBid > mDataFile.kline[currentDay - 1].closeingPrice ?
                          QColor("#FF0000") : QColor("#00FF00");

    QColor lowestColor = mDataFile.kline[currentDay].lowestBid > mDataFile.kline[currentDay - 1].closeingPrice ?
                         QColor("#FF0000") : QColor("#00FF00");


    QColor closeingColor = mDataFile.kline[currentDay].closeingPrice > mDataFile.kline[currentDay ].openingPrice ?
                           QColor("#FF0000") : QColor("#00FF00");


    QColor amountOfIncreaseColor = mDataFile.kline[currentDay].amountOfIncrease > 0 ?
                                   QColor("#FF0000") : QColor("#00FF00");

    mShowDrtail->receiveParams(mDataFile.kline[currentDay].time, QColor("#FFFFFF"),
                               mDataFile.kline[currentDay].closeingPrice, QColor("#FF0000"),
                               mDataFile.kline[currentDay].openingPrice, openingColor,
                               mDataFile.kline[currentDay].highestBid, highestColor,
                               mDataFile.kline[currentDay].lowestBid, lowestColor,
                               mDataFile.kline[currentDay].closeingPrice, closeingColor,
                               mDataFile.kline[currentDay].amountOfIncrease, amountOfIncreaseColor,
                               mDataFile.kline[currentDay].amountOfAmplitude, QColor("#02E2F4"),
                               mDataFile.kline[currentDay].totalVolume, QColor("#02E2F4"),
                               mDataFile.kline[currentDay].totalAmount, QColor("#02E2F4"),
                               mDataFile.kline[currentDay].turnoverRate, QColor("#02E2F4")
                              );
}


void KLineGrid::drawMouseMoveCrossVerLine()
{

    if (mousePoint.x() < getMarginLeft() || mousePoint.x() > getWidgetWidth() - getMarginRight())
        return;

    if (mousePoint.y() < getMarginTop() || mousePoint.y() > getWidgetHeight() - getMarginBottom())
        return;

    QPainter painter(this);
    QPen     pen;
    pen.setColor(QColor("#FFFFFF"));
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawLine(mousePoint.x(), getMarginTop(),
                     mousePoint.x(), getWidgetHeight() - getMarginBottom());

}


void KLineGrid::drawMouseMoveCrossHorLine()
{

    if (mousePoint.x() < getMarginLeft() || mousePoint.x() > getWidgetWidth() - getMarginRight())
        return;

    if (mousePoint.y() < getMarginTop() || mousePoint.y() > getWidgetHeight() - getMarginBottom())
        return;

    QPainter painter(this);
    QPen     pen;
    pen.setColor(QColor("#FFFFFF"));
    pen.setWidth(1);
    painter.setPen(pen);

    painter.drawLine(getMarginLeft(), mousePoint.y(),
                     getWidgetWidth() - getMarginRight(), mousePoint.y());

}



void KLineGrid::drawCross2()
{

    drawMouseMoveCrossHorLine();
    drawMouseMoveCrossVerLine();
    drawTips2();
}




void KLineGrid::drawTips2()
{

    if (mousePoint.x() < getMarginLeft() || mousePoint.x() > getWidgetWidth() - getMarginRight())
        return;

    if (mousePoint.y() < getMarginTop() || mousePoint.y() > getWidgetHeight() - getMarginBottom())
        return;

    QPainter painter(this);
    QPen     pen;
    QBrush brush(QColor(64, 0, 128));
    painter.setBrush(brush);
    pen.setColor(QColor("#FFFFFF"));
    pen.setWidth(1);
    painter.setPen(pen);

    double yval =  highestBid - (mousePoint.y() - getMarginTop()) / yscale;
    double yPos = mousePoint.y();

    int iTipsWidth = 60;
    int iTipsHeight = 30;

    QString str;

    QRect rect(getWidgetWidth() - getMarginRight(),
               yPos - iTipsHeight / 2, iTipsWidth, iTipsHeight);
    painter.drawRect(rect);


    QRect rectText(getWidgetWidth() - getMarginRight() + iTipsWidth / 4,
                   yPos - iTipsHeight / 4, iTipsWidth, iTipsHeight);
    painter.drawText(rectText, str.sprintf("%.2f", yval));
}



void KLineGrid::drawAverageLine(int day)
{
    if (mDataFile.kline.empty() || highestBid == lowestBid) {
        return;
    }

    // 确保索引范围有效
    int actualBeginDay = qMax(0, beginDay);
    int actualEndDay = qMin(endDay, (int)mDataFile.kline.size());
    
    if (actualBeginDay >= actualEndDay) {
        return;
    }

    //y轴缩放
    yscale = getGridHeight() / (highestBid - lowestBid);
    //画线要连接的点
    QVector<QPoint> point;
    //临时点
    QPoint temp;

    // 修复X轴步长计算
    int displayDataCount = actualEndDay - actualBeginDay;
    double xstep = (displayDataCount > 1) ? (getGridWidth() / displayDataCount) : getGridWidth();

    // 根据均线天数选择对应的数据
    for (int i = actualBeginDay; i < actualEndDay; ++i) {
        double averageValue = 0;
        
        switch (day) {
        case 5:
            averageValue = mDataFile.kline[i].averageLine5;
            break;
        case 10:
            averageValue = mDataFile.kline[i].averageLine10;
            break;
        case 20:
            averageValue = mDataFile.kline[i].averageLine20;
            break;
        case 30:
            averageValue = mDataFile.kline[i].averageLine30;
            break;
        case 60:
            averageValue = mDataFile.kline[i].averageLine60;
            break;
        default:
            continue;
        }
        
        if (averageValue <= 0) {
            continue;
        }
        
        temp.setX(getMarginLeft() + xstep * (i - actualBeginDay) + 0.5 * lineWidth);
        temp.setY(getWidgetHeight() - (averageValue - lowestBid) * yscale - getMarginBottom());
        point.push_back(temp);
    }

    if (point.size() < 2) {
        return; // 需要至少2个点才能画线
    }

    QPainter painter(this);
    QPen pen;

    // 设置均线颜色
    switch (day) {
    case 5:
        pen.setColor(Qt::white);
        break;
    case 10:
        pen.setColor(Qt::yellow);
        break;
    case 20:
        pen.setColor(Qt::magenta);
        break;
    case 30:
        pen.setColor(Qt::green);
        break;
    case 60:
        pen.setColor(Qt::cyan);
        break;
    default:
        pen.setColor(Qt::white);
        break;
    }
    
    pen.setWidth(1);
    painter.setPen(pen);
    QPolygon polykline(point);
    painter.drawPolyline(polykline);
}



