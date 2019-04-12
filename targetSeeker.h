#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "qobject.h"
#include "qlabel.h"
#include "qdebug.h"
#include "qtimer.h"
#include "qthread.h"



#include "opencv2/opencv.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"



#include <OpenNI.h>


#include "qmutex.h"

using namespace std;
using namespace cv;
class videoWorker;
using namespace openni;




class targetSeeker : public QObject
{
    Q_OBJECT
public:
    targetSeeker(QWidget *parent = 0, QString PATH="", std::vector<int> parameters=std::vector<int>(),bool DEBUG=false);
private:
    QString PATH;
    bool DEBUG;
    QThread *videoThread;
    videoWorker *worker;



signals:
    void initWorker();
    void nuPos(double x,double y);

};




class videoWorker : public QObject
{
    Q_OBJECT
public:
    videoWorker(QWidget *parent = 0, QString PATH="", std::vector<int> parameters=std::vector<int>(),bool DEBUG = false);

private:
    QString PATH;
    std::vector<int> parameters;
    bool DEBUG;

    int initOpenNI(const char* deviceUri);
    openni::VideoFrameRef *frame ;
    openni::Device device;
    openni::VideoStream *depth;


    void findLargestRegion(Mat in, Mat *out, Mat *buf, Point *target);

    Point lastTarget;
signals:
        void nuPos(double x,double y);

private slots:
    void init();
    void processNextFrame();


};


#endif // VIDEOPLAYER_H
