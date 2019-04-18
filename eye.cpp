#include "eye.h"

#define eyeSize 600
#define border 20
#define maxStep 12

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}


eye::eye(QWidget *parent, QString PATH, QPixmap eyePix):QLabel(parent),PATH(PATH),eyePix(eyePix)
{
    resize(1080,1920);
    setStyleSheet("QLabel { background-color : white; }");
    eyePix = eyePix.scaled(eyeSize,eyeSize,Qt::KeepAspectRatio,Qt::SmoothTransformation);

    eyeLbl.setParent(this);
    eyeLbl.resize(eyePix.size());
    eyeLbl.setPixmap(eyePix);
    qDebug()<<eyePix.size();

    eyeLbl.move(100,100);
    eyeLbl.show();

    xmax = width()-eyeSize-border;
    ymax = height()-eyeSize-border;
    xmin = border;
    ymin = border;

    updTimer = new QTimer(this);
    updTimer->setInterval(1000/24);
    connect(updTimer,SIGNAL(timeout()),this,SLOT(updEye()));
    updTimer->start();

}


void eye::moveEye(double x_pct,double y_pct)
{
    if(x_pct<0) return;
    if(y_pct<0) return;
    if(x_pct>100) return;
    if(y_pct>100) return;

    x0  = xmin+(xmax-xmin)*x_pct/100;
    y0  = ymin+(ymax-ymin)*y_pct/100;
}


void eye::updEye()
{
    if((x0==eyeLbl.x())&&(y0==eyeLbl.y()))
        return;

    int x1,y1;

    x1 = eyeLbl.x();
    y1 = eyeLbl.y();

    if(abs(x1-x0)<maxStep) x1 = x0;
    if(abs(y1-y0)<maxStep) y1 = y0;

    x1+=sgn(x0-x1)*maxStep;
    y1+=sgn(y0-y1)*maxStep;

    eyeLbl.move(x1,y1);
}
