#ifndef EYE_H
#define EYE_H

#include "qobject.h"
#include "qlabel.h"
#include "qdebug.h"
#include "qtimer.h"

#define RIGHT 1
#define LEFT 2

class eye: public QLabel
{
    Q_OBJECT
public:
    eye(QWidget *parent, QString PATH, QPixmap eyePix);
private:
    QString PATH;
    QPixmap eyePix;

    QLabel eyeLbl;
    int xmax,xmin,ymax,ymin;
    QTimer *updTimer;
    int x0,y0;
private slots:
    void moveEye(double x_pct,double y_pct);
    void updEye();
};

#endif // EYE_H
