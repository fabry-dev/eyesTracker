#include "interfacewindow.h"

interfaceWindow::interfaceWindow(QWidget *parent,QString(PATH),std::vector<int>parameters) : QWidget(parent),PATH(PATH)
{


    ts = new targetSeeker(this,PATH,parameters);




}
void interfaceWindow::resizeEvent(QResizeEvent* event)
{

}
