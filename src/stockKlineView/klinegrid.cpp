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
        qDebug() << "KLineGrid::readData - 开始处理数据，条数:" << datas.size();
        
        if (!mDataFile.readData(datas)) {
            qDebug() << "KLineGrid::readData - DataFile.readData失败";
            return false;
        }
        
        qDebug() << "KLineGrid::readData - DataFile.readData成功，kline大小:" << mDataFile.kline.size();
        
        endDay = mDataFile.kline.size();  // 改为不减1，让endDay作为上界
        totalDay = mDataFile.kline.size();
        beginDay = 0;  // 从0开始
        currentDay = totalDay / 2;
        
        qDebug() << "KLineGrid::readData - 设置索引: beginDay=" << beginDay << " endDay=" << endDay << " totalDay=" << totalDay;
        
        highestBid = 0;
        lowestBid = 100000;
        maxVolume = 0;
        
        qDebug() << "KLineGrid::readData - 准备调用update()";
        qDebug() << "组件可见性:" << isVisible() << " 尺寸:" << size() << " 位置:" << pos();
        qDebug() << "网格尺寸: width=" << getWidgetWidth() << " height=" << getWidgetHeight();
        qDebug() << "绘制区域: gridWidth=" << getGridWidth() << " gridHeight=" << getGridHeight();
        
        // 确保组件尺寸正确初始化
        if (getWidgetWidth() == 0 || getWidgetHeight() == 0) {
            qDebug() << "组件尺寸为0，强制触发resizeEvent";
            QResizeEvent resizeEvent(size(), size());
            this->resizeEvent(&resizeEvent);
        }
        
        update();
        repaint(); // 强制立即重绘
        
        qDebug() << "KLineGrid::readData - update()和repaint()调用完成";
        
        return true;
    } catch (const std::exception& e) {
        qDebug() << "KLineGrid::readData - 捕获异常:" << e.what();
        return false;
    } catch (...) {
        qDebug() << "KLineGrid::readData - 捕获未知异常";
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
    qDebug() << "KLineGrid::paintEvent - 开始绘制事件";
    AutoGrid::paintEvent(event);
    //画k线
    drawLine();
    qDebug() << "KLineGrid::paintEvent - 绘制事件完成";
}


void KLineGrid::drawLine()
{
    qDebug() << "KLineGrid::drawLine - 开始绘制流程";
    
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


    //画5日均线
    drawAverageLine(5);
    //画5日均线
    drawAverageLine(10);
    //画5日均线
    drawAverageLine(20);
    //画5日均线
    drawAverageLine(30);
    //画5日均线
    drawAverageLine(60);

}


void KLineGrid::getIndicator()
{
    qDebug() << "KLineGrid::getIndicator - 开始计算指标，数据范围:" << beginDay << "到" << endDay;
    
    highestBid = 0;
    lowestBid = 100000;
    maxVolume = 0;

    if (endDay <= mDataFile.kline.size() && beginDay < endDay) {
        for (int i = beginDay; i < endDay; ++i) {
            if (mDataFile.kline[i].highestBid > highestBid)
                highestBid = mDataFile.kline[i].highestBid;
            if (mDataFile.kline[i].lowestBid < lowestBid)
                lowestBid = mDataFile.kline[i].lowestBid;
            //        if( mDataFile.kline[i].totalVolume.toFloat() > maxVolume )
            //            maxVolume = mDataFile.kline[i].totalVolume.toFloat();
        }
        
        // 如果最高价和最低价相同，需要调整范围以避免除零错误
        if (highestBid == lowestBid) {
            double mid = highestBid;
            highestBid = mid + mid * 0.01; // 增加1%
            lowestBid = mid - mid * 0.01;  // 减少1%
            if (lowestBid < 0) lowestBid = 0;
        }
        
        qDebug() << "KLineGrid::getIndicator - 计算结果: 最高价=" << highestBid << " 最低价=" << lowestBid;
    } else {
        qDebug() << "KLineGrid::getIndicator - 无效的数据范围或数据为空";
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
    qDebug() << "KLineGrid::drawKline - 开始绘制K线";
    qDebug() << "绘制参数: beginDay=" << beginDay << " endDay=" << endDay << " totalDay=" << totalDay;
    qDebug() << "价格范围: 最高=" << highestBid << " 最低=" << lowestBid;
    
    QPainter painter(this);
    QPen     pen;
    pen.setColor(Qt::red);
    painter.setPen(pen);

    if (beginDay < 0) {
        qDebug() << "KLineGrid::drawKline - beginDay < 0，退出绘制";
        return;
    }
    
    if (endDay <= beginDay) {
        qDebug() << "KLineGrid::drawKline - endDay <= beginDay，退出绘制";
        return;
    }

    //y轴缩放
    if (highestBid == lowestBid) {
        qDebug() << "KLineGrid::drawKline - 最高价等于最低价，使用默认缩放";
        yscale = 1.0;
    } else {
        yscale = getGridHeight() / (highestBid - lowestBid);
        qDebug() << "KLineGrid::drawKline - Y轴缩放比例:" << yscale;
    }

    //画线连接的两个点
    QPoint p1;
    QPoint p2;
    QPoint p3;
    QPoint p4;

    if (getGridWidth() <= 0 || getGridHeight() <= 0) {
        qDebug() << "KLineGrid::drawKline - 绘制区域尺寸无效: gridWidth=" << getGridWidth() << " gridHeight=" << getGridHeight();
        return;
    }

    double xstep = getGridWidth() / totalDay;
    qDebug() << "KLineGrid::drawKline - X轴步长:" << xstep;

    for (int i = beginDay; i < endDay; ++i) {
        qDebug() << "绘制第" << i << "个K线，开盘价:" << mDataFile.kline[i].openingPrice << " 收盘价:" << mDataFile.kline[i].closeingPrice;
        if (mDataFile.kline[i].openingPrice > mDataFile.kline[i].closeingPrice)
            pen.setColor(QColor(85, 252, 252));
        else
            pen.setColor(Qt::red);


        lineWidth = getGridWidth() / totalDay;

        //为了各个k线之间不贴在一起，设置一个间隔
        lineWidth = lineWidth - 0.2 * lineWidth;

        //最小线宽为3
        if (lineWidth < 3)
            lineWidth = 3;

        //阴线 (开盘价 > 收盘价)
        if (mDataFile.kline[i].openingPrice > mDataFile.kline[i].closeingPrice) {
            //画开盘与收盘之间的粗实线
            pen.setWidth(lineWidth);
            painter.setPen(pen);
            p1.setX(getMarginLeft() + xstep * (i - beginDay) + 0.5 * lineWidth);
            p1.setY(getWidgetHeight() - (mDataFile.kline[i].openingPrice - lowestBid) *yscale - getMarginBottom());
            p2.setX(getMarginLeft() + xstep * (i - beginDay) + 0.5 * lineWidth);
            p2.setY(getWidgetHeight() - (mDataFile.kline[i].closeingPrice - lowestBid) *yscale - getMarginBottom() - 0.5 * lineWidth);
            painter.drawLine(p1, p2);


            //画最高价与最低价之间的细线
            pen.setWidth(1);
            painter.setPen(pen);
            p1.setX(getMarginLeft() + xstep * (i - beginDay) + 0.5 * lineWidth);
            p1.setY(getWidgetHeight() - (mDataFile.kline[i].highestBid - lowestBid) *yscale - getMarginBottom());
            p2.setX(getMarginLeft() + xstep * (i - beginDay) + 0.5 * lineWidth);
            p2.setY(getWidgetHeight() - (mDataFile.kline[i].lowestBid - lowestBid) *yscale - getMarginBottom());
            painter.drawLine(p1, p2);


        } else {
            //像同花顺一样阳线画成空心的

            pen.setWidth(1);
            painter.setPen(pen);


            p1.setX(getMarginLeft() + xstep * (i - beginDay));
            p1.setY(getWidgetHeight() - (mDataFile.kline[i].openingPrice - lowestBid) *yscale - getMarginBottom());

            p2.setX(getMarginLeft() + xstep * (i - beginDay) + lineWidth);
            p2.setY(getWidgetHeight() - (mDataFile.kline[i].openingPrice - lowestBid) *yscale - getMarginBottom());


            p3.setX(getMarginLeft() + xstep * (i - beginDay));
            p3.setY(getWidgetHeight() - (mDataFile.kline[i].closeingPrice - lowestBid) *yscale - getMarginBottom());

            p4.setX(getMarginLeft() + xstep * (i - beginDay) + lineWidth);
            p4.setY(getWidgetHeight() - (mDataFile.kline[i].closeingPrice - lowestBid) *yscale - getMarginBottom());

            painter.drawLine(p1, p2);
            painter.drawLine(p1, p3);
            painter.drawLine(p2, p4);
            painter.drawLine(p3, p4);


            //画最高价与最低价之间的细线
            pen.setWidth(1);
            painter.setPen(pen);
            p1.setX(getMarginLeft() + xstep * (i - beginDay) + 0.5 * lineWidth);
            p1.setY(getWidgetHeight() - (mDataFile.kline[i].highestBid - lowestBid) *yscale - getMarginBottom());


            double y1, y2;
            if (mDataFile.kline[i].openingPrice > mDataFile.kline[i].closeingPrice) {
                y1 = mDataFile.kline[i].openingPrice;
                y2 = mDataFile.kline[i].closeingPrice;
            } else {
                y1 = mDataFile.kline[i].closeingPrice;
                y2 = mDataFile.kline[i].openingPrice;
            }

            p2.setX(getMarginLeft() + xstep * (i - beginDay) + 0.5 * lineWidth);
            p2.setY(getWidgetHeight() - (y1 - lowestBid) *yscale - getMarginBottom());
            p3.setX(getMarginLeft() + xstep * (i - beginDay) + 0.5 * lineWidth);
            p3.setY(getWidgetHeight() - (y2 - lowestBid) *yscale - getMarginBottom());
            p4.setX(getMarginLeft() + xstep * (i - beginDay) + 0.5 * lineWidth);
            p4.setY(getWidgetHeight() - (mDataFile.kline[i].lowestBid - lowestBid) *yscale - getMarginBottom());

            painter.drawLine(p1, p2);
            painter.drawLine(p3, p4);
        }
    }
    
    qDebug() << "KLineGrid::drawKline - K线绘制完成";
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
            if (endDay + 1 > mDataFile.kline.size() - 1)
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

        if (endDay > mDataFile.kline.size() - 10) {
            endDay = mDataFile.kline.size() - 10;
            beginDay = endDay - totalDay;
        }

        if (beginDay < 0) {
            beginDay = 0;
            endDay = beginDay + totalDay;
        }

        update();


        break;
    }

    case Qt::Key_Down: {
        if (totalDay == mDataFile.kline.size() - 1)
            return;

        totalDay = totalDay * 2;
        if (totalDay > mDataFile.kline.size() - 1) {
            totalDay = mDataFile.kline.size() - 1;
        }


        endDay = currentDay + totalDay / 2;
        if (endDay > mDataFile.kline.size() - 10) {
            endDay = mDataFile.kline.size() - 10;
        }



        beginDay = currentDay - totalDay / 2;
        if (beginDay < 0)
            beginDay = 0;



        totalDay = endDay - beginDay;

        update();

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

    //y轴缩放
    yscale = getGridHeight() / (highestBid - lowestBid) ;
    //画笔的线宽
    lineWidth;
    //画线要连接的点
    QVector<QPoint> point;

    //临时点
    QPoint temp;

    //x轴步进
    double xstep = getGridWidth() / totalDay;



    if (beginDay < 0)
        return;


    switch (day) {
    case 5:
        for (int i = beginDay; i < endDay; ++i) {
            if (mDataFile.kline[i].averageLine5 == 0)
                continue;
            temp.setX(getMarginLeft() + xstep * (i - beginDay) + 0.5 * lineWidth);
            temp.setY(getWidgetHeight() - (mDataFile.kline[i].averageLine5 - lowestBid) *yscale - getMarginBottom());
            point.push_back(temp);
        }
        break;
    case 10:
        for (int i = beginDay; i < endDay; ++i) {
            if (mDataFile.kline[i].averageLine10 == 0)
                continue;
            temp.setX(getMarginLeft() + xstep * (i - beginDay) + 0.5 * lineWidth);
            temp.setY(getWidgetHeight() - (mDataFile.kline[i].averageLine10 - lowestBid) *yscale - getMarginBottom());
            point.push_back(temp);
        }
        break;
    case 20:
        for (int i = beginDay; i < endDay; ++i) {
            if (mDataFile.kline[i].averageLine20 == 0)
                continue;
            temp.setX(getMarginLeft() + xstep * (i - beginDay) + 0.5 * lineWidth);
            temp.setY(getWidgetHeight() - (mDataFile.kline[i].averageLine20 - lowestBid) *yscale - getMarginBottom());
            point.push_back(temp);
        }
        break;
    case 30:
        for (int i = beginDay; i < endDay; ++i) {
            if (mDataFile.kline[i].averageLine30 == 0)
                continue;
            temp.setX(getMarginLeft() + xstep * (i - beginDay) + 0.5 * lineWidth);
            temp.setY(getWidgetHeight() - (mDataFile.kline[i].averageLine30 - lowestBid) *yscale - getMarginBottom());
            point.push_back(temp);
        }
        break;
    case 60:
        for (int i = beginDay; i < endDay; ++i) {
            if (mDataFile.kline[i].averageLine60 == 0)
                continue;
            temp.setX(getMarginLeft() + xstep * (i - beginDay) + 0.5 * lineWidth);
            temp.setY(getWidgetHeight() - (mDataFile.kline[i].averageLine60 - lowestBid) *yscale - getMarginBottom());
            point.push_back(temp);
        }
        break;
    default:
        break;
    }


    QPainter painter(this);
    QPen     pen;

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
    painter.setPen(pen);
    QPolygon polykline(point);
    painter.drawPolyline(polykline);
}



