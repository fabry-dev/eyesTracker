#ifndef INTERFACEWINDOW_H
#define INTERFACEWINDOW_H

#include <QObject>
#include <QWidget>
#include "qdebug.h"
#include "targetSeeker.h"
#include "qevent.h"


class interfaceWindow : public QWidget
{
    Q_OBJECT
public:
    explicit interfaceWindow(QWidget *parent = nullptr, QString PATH="", std::vector<int> parameters=std::vector<int>());

private:
    QString PATH;
    targetSeeker *ts;


protected:
    void resizeEvent(QResizeEvent* event);
signals:

public slots:
};

#endif // INTERFACEWINDOW_H
