#include "targetSeeker.h"


#define VIDEO_FPS 24
#define DMIN parameters[0]
#define DMAX parameters[1]
#define BLUR_BAND ((DMAX-DMIN)/10)

#define OFFSET_X parameters[2]
#define OFFSET_Y parameters[3]

targetSeeker::targetSeeker(QWidget *parent, QString PATH,std::vector<int>parameters):QObject(parent),PATH(PATH)
{

    videoThread = new QThread;
    worker = new videoWorker(NULL,PATH,parameters);
    worker->moveToThread(videoThread);
    connect(videoThread,SIGNAL(started()),worker,SLOT(init()),Qt::QueuedConnection);
    videoThread->start();

    connect(worker,SIGNAL(nuPos(double,double)),this,SIGNAL(nuPos(double,double)));
}







videoWorker::videoWorker(QWidget *parent, QString PATH, std::vector<int> parameters):QObject(parent),PATH(PATH),parameters(parameters)
{


}


void videoWorker::init()
{

    frame = new VideoFrameRef();
    initOpenNI(openni::ANY_DEVICE);


    while(true)
        processNextFrame();
}


int videoWorker::initOpenNI(const char* deviceUri) {



    Status rc = OpenNI::initialize();

    depth = new VideoStream();

    if (rc != STATUS_OK)
    {
        qDebug()<<"Initialize failed"<< OpenNI::getExtendedError();
        return 1;
    }

    rc = device.open(deviceUri);
    if (rc != STATUS_OK)
    {
        qDebug()<<"Couldn't open device"<< OpenNI::getExtendedError();
        return 2;
    }

    if (device.getSensorInfo(SENSOR_DEPTH) != NULL)
    {
        rc = depth->create(device, SENSOR_DEPTH);
        if (rc != STATUS_OK)
        {
            qDebug()<<"Couldn't create depth stream"<<OpenNI::getExtendedError();
            return 3;
        }
    }

    // Get the device vendor and name, if Asus Xtion or Primesense set its resolution to 640x480.
    openni::Array<openni::DeviceInfo> deviceInfoList;
    OpenNI::enumerateDevices(&deviceInfoList);
    for (int i = 0; i < deviceInfoList.getSize(); i++)
    {
        qDebug()<<i<<deviceInfoList[i].getUri()<<deviceInfoList[i].getVendor()<<deviceInfoList[i].getName();

        const openni::SensorInfo* sinfo = device.getSensorInfo(openni::SENSOR_DEPTH);
        const openni::Array< openni::VideoMode>& modesDepth = sinfo->getSupportedVideoModes();
        bool set = false;
        for (int i = 0; i < modesDepth.getSize(); i++) {

            //qDebug()<<i<<modesDepth[i].getResolutionX()<<"x"<<modesDepth[i].getResolutionY()<<" - "<<modesDepth[i].getFps()<<"FPS"<<" - "<<modesDepth[i].getPixelFormat()<<" pxl format";


            if (!set && modesDepth[i].getResolutionX() == 640 && modesDepth[i].getResolutionY() == 480) {
                rc = depth->setVideoMode(modesDepth[i]); // 640x480
                set = true;
                qDebug()<<"mode "<<i;
                if (openni::STATUS_OK != rc)
                    qDebug() << "error: depth format not supprted...";

            }
        }
    }


    rc = depth->start();
    if (rc != STATUS_OK)
    {
        qDebug()<<"Couldn't start the depth stream\n%s\n"<< OpenNI::getExtendedError();
        return 4;
    }
}






#define SAMPLE_READ_WAIT_TIMEOUT 1000
#define DETECT_BAND 0.2
#define HEAD_X_POSITION 0.5
#define HEAD_Y_POSITION 0.2



void videoWorker::findLargestRegion(Mat in, Mat *out,Mat *buf, Point *target)
{



    uint16_t min = 65535;
    uint16_t max = 0;



    for(int x = 0;x<in.cols;x++)
        for(int y = 0;y<in.rows;y++)
        {


            if(in.at<uint16_t>(y,x)>max)
                max = in.at<uint16_t>(y,x);
            if((in.at<uint16_t>(y,x)>0)&&(in.at<uint16_t>(y,x)<min))
                min = in.at<uint16_t>(y,x);
        }

    double v;



    for(int x = 0;x<in.cols;x++)
        for(int y = 0;y<in.rows;y++)
        {
            v = (double)  (in.at<uint16_t>(y,x) - min)/(max-min);
            uint b = 255*v;
            out->at<Vec3b>(y,x)=Vec3b(b,b,b);
            if(v<0)v=0;
            if((v>0)&&(v<DETECT_BAND))
            {
                buf->at<uchar>(y,x) = 255;
            }
            else
                buf->at<uchar>(y,x) = 0;
        }

    RNG rng(12345);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours( *buf, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );


    if(contours.size()==0)//no region found
        return;

    int largestContour;
    double aMax=0;
    for( size_t i = 0; i< contours.size(); i++ )
    {
        double a=contourArea( contours[i],false);
        if(a>aMax)
        {
            aMax = a;
            largestContour = i;
        }

    }

    drawContours( *out, contours, largestContour, Scalar(0,0,255), 2, 8, hierarchy, 0, Point() );


    Rect bRect=boundingRect(contours[largestContour]); // Find the bounding recta





    target->x=bRect.x+bRect.width*HEAD_X_POSITION;
    target->y=bRect.y+bRect.height*HEAD_Y_POSITION;




}




void videoWorker::processNextFrame()
{
    int changedStreamDummy;
    int rc = OpenNI::waitForAnyStream(&depth, 1, &changedStreamDummy,SAMPLE_READ_WAIT_TIMEOUT );
    if (rc != STATUS_OK)
    {
        qDebug()<<"Wait failed! "<<OpenNI::getExtendedError();
        return;
    }

    //openni::VideoFrameRef *frame = new VideoFrameRef();
    rc = depth->readFrame(frame);
    if (rc != STATUS_OK)
    {
        qDebug()<<"Read failed!"<<OpenNI::getExtendedError();
        return ;
    }

    if (frame->getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_1_MM && frame->getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_100_UM)
    {
        qDebug()<<"Unexpected frame format";
        return ;
    }


    openni::DepthPixel* dData = (openni::DepthPixel*)frame->getData();
    cv::Mat depthImage(frame->getHeight(),frame->getWidth(), CV_16UC1, dData);



    resize(depthImage,depthImage,Size(depthImage.cols,depthImage.rows));

    Mat display(depthImage.rows, depthImage.cols, CV_8UC3);
    Mat buf(depthImage.rows, depthImage.cols, CV_8UC1);
    Point target = Point(0,0);


    findLargestRegion(depthImage,&display,&buf,&target);


    if(target!=Point(0,0))
    {
        circle(display,target,20,cv::Scalar(0,255,0));
        emit nuPos((double)100*target.x/depthImage.cols,(double)100*target.y/depthImage.rows);
    }


  //  cvtColor(display,display, COLOR_BGR2RGB);
    imshow("test",display);



}


















