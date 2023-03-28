// -------------------------------------------------------------------------------------------------------------------
//
//  File: RTLSControllerApplication.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "RTLSControllerApplication.h"

#include "DataModel.h"
#include "mainwindow.h"
#include "RTLSCLEConnection.h"
#include "RTLSControl.h"
#include "RTLSClient.h"
#include "ViewSettings.h"
#include "DataAnchor.h"
#include "GraphicsWidget.h"
#include "ChannelSettingsWidget.h"
#include "DataLink.h"
#include "anchorlistwidget.h"

#include <QDomDocument>
#include <QItemSelectionModel>
#include <QMetaProperty>
#include <QFile>
#include <QtCore>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QPolygonF>
#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>

RTLSControllerApplication::RTLSControllerApplication(int &argc, char **argv) : QApplication(argc, argv)
{
    _ready = false;

    this->setOrganizationName("DecaWave");
    this->setOrganizationDomain("decawave.com");
    this->setApplicationName("RTLSController");

    //new DataModel - we have a tree structure of tags and anchors
    _model = new DataModel(this, 1);

    _anchorSelectionMode = QAbstractItemView::ExtendedSelection;
    _anchorSelectionModel = new QItemSelectionModel(_model, this);

    _viewSettings = new ViewSettings(this);

    //net work init
    _cleConnection = new RTLSCLEConnection(this); //只是初始化window
    _control = new RTLSControl(this); // Data port
    _client = new RTLSClient(this); // Controller port

    //debug file one for each time start the project;
    QDateTime now = QDateTime::currentDateTime();
    QString filename("../Logs/"+now.toString("yyyyMMdd_hhmmss")+"Debug_Data_log.txt");
    _debugfile = new QFile(filename);

    _mainWindow = new MainWindow(); //window

    QDesktopWidget desktop;

    int desktopHeight=desktop.geometry().height();
    int desktopWidth=desktop.geometry().width();
    _mainWindow->resize(desktopWidth/2,desktopHeight/2);

    _ready = true;

    //Connect the RTLSClient with GraphicsWidget (Main Window)
    QObject::connect(_client, SIGNAL(tagPos(quint64,double,double,double)), graphicsWidget(), SLOT(tagPos(quint64,double,double,double)));
    QObject::connect(_client, SIGNAL(tagStats(quint64,double,double,double,double)), graphicsWidget(), SLOT(tagStats(quint64,double,double,double,double)));
    QObject::connect(_client, SIGNAL(tagCLEStats(quint64,double,double)), graphicsWidget(), SLOT(tagCLEStats(quint64,double,double)));

    QObject::connect(_control, SIGNAL(rangingProgress(QString)), _mainWindow, SLOT(rangingProgress(QString)));
    QObject::connect(_control, SIGNAL(commsTestProgress(QString)), _mainWindow, SLOT(commsTestProgress(QString)));
    QObject::connect(_control, SIGNAL(statusBarMessage(QString)), _mainWindow, SLOT(statusBarMessage(QString)));
    QObject::connect(_control, SIGNAL(disableReSync(bool)), _mainWindow, SLOT(disableReSync(bool)));

    QObject::connect(_control, SIGNAL(clearTags()), graphicsWidget(), SLOT(clearTags()));

    QObject::connect(_mainWindow, SIGNAL(reSyncAnchors()), _control, SLOT(reSyncAnchors()));

    QObject::connect(_mainWindow, SIGNAL(chanConfAction()), _control, SLOT(chanConfAction()));

    QObject::connect(this, SIGNAL(positionError()), _mainWindow, SLOT(positionError()));

    emit ready();
}

RTLSControllerApplication::~RTLSControllerApplication()
{
    // Delete the objects manually, because we want to control the order
    delete _mainWindow;

    delete _client;
    delete _control;
    delete _cleConnection;

    delete _viewSettings;

    delete _anchorSelectionModel;
    delete _model;
    delete _debugfile;
}

RTLSControllerApplication *RTLSControllerApplication::instance()
{
    return qobject_cast<RTLSControllerApplication *>(QCoreApplication::instance());
}

QFile * RTLSControllerApplication::debugFile(){
    return instance()->_debugfile;
}

DataModel *RTLSControllerApplication::model()
{
    return instance()->_model;
}


QItemSelectionModel *RTLSControllerApplication::anchorSelectionModel()
{
    return instance()->_anchorSelectionModel;
}

QAbstractItemView::SelectionMode RTLSControllerApplication::anchorSelectionMode()
{
    return instance()->_anchorSelectionMode;
}

ViewSettings *RTLSControllerApplication::viewSettings()
{
    return instance()->_viewSettings;
}

RTLSClient *RTLSControllerApplication::client()
{
    return instance()->_client;
}

RTLSCLEConnection *RTLSControllerApplication::cleConnection()
{
    return instance()->_cleConnection;
}

RTLSControl *RTLSControllerApplication::control()
{
    return instance()->_control;
}

MainWindow *RTLSControllerApplication::mainWindow()
{
    return instance()->_mainWindow;
}

AnchorListWidget *RTLSControllerApplication::anchorListWidget()
{
    return mainWindow()->anchorListWidget();
}

GraphicsWidget *RTLSControllerApplication::graphicsWidget()
{
    return mainWindow()->graphicsWidget();
}

GraphicsView *RTLSControllerApplication::graphicsView()
{
    return mainWindow()->graphicsWidget()->graphicsView();
}

ChannelSettingsWidget *RTLSControllerApplication::channelSettings()
{
    return mainWindow()->channelSettings();
}

void RTLSControllerApplication::setAnchorSelectionMode(QAbstractItemView::SelectionMode mode)
{
    instance()->_anchorSelectionMode = mode;
    emit instance()->selectionModeChanged(mode);
}

void RTLSControllerApplication::connectReady(QObject *receiver, const char *member, Qt::ConnectionType type)
{
    QMetaMethod method = receiver->metaObject()->method(receiver->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(member)));
    // Either call the method or connect it to the ready signal

    if (instance()->_ready && method.isValid())
        method.invoke(receiver, type);
    else
        QObject::connect(instance(), QMetaMethod::fromSignal(&RTLSControllerApplication::ready), receiver, method, type);
}

void RTLSControllerApplication::loadConfigFile(QString filename, int connection)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug(qPrintable(QString("Error: Cannot read file %1 %2").arg(filename).arg(file.errorString())));
        return;
    }

    QDomDocument doc;
    QString error;
    int errorLine;
    int errorColumn;

    if(doc.setContent(&file, false, &error, &errorLine, &errorColumn))
    {
        qDebug() << "file error !!!" << error << errorLine << errorColumn;
    }

    //QDomElement root = doc.documentElement();
    //QDomElement anchlist = root.firstChild().toElement();
    QDomElement anchlist = doc.documentElement();

    //qDebug() << root.tagName() ;
    qDebug() << anchlist.tagName() ;

    if( anchlist.tagName() == "anchor_list" )
    {
        //there are acnhors saved in the config file
        //populate the _model anchor list

        QDomNode n = anchlist.firstChild();
        while( !n.isNull() )
        {
            QDomElement e = n.toElement();
            if( !e.isNull() )
            {
                if( e.tagName() == "anchor" )
                {
                    bool ok;
                    bool master = false;
                    quint64 id = (e.attribute( "addr", "" )).toULongLong(&ok, 16);

                    DataAnchor *a = _model->anchor(id, true);

                    a->setCoordinates((e.attribute( "x", "" )).toDouble(), (e.attribute( "y", "" )).toDouble(), (e.attribute( "z", "" )).toDouble());
                    a->setNumber((e.attribute( "id", "" )).toInt());
                    a->setSelected(((e.attribute( "selected", "" )).toInt() == 1) ? true : false);
                    master = (((e.attribute( "master", "" )).toInt() == 1) ? true : false);
                    a->setMasterId((e.attribute( "master_id", "" )).toULongLong(&ok, 16));
                    a->setLagDelayUs((e.attribute( "master_lag_delay", "" )).toInt());
                    a->setAntennaDlyRx((e.attribute( "ant_delay_rx", "" )).toDouble());
                    a->setAntennaDlyTx((e.attribute( "ant_delay_tx", "" )).toDouble());

                    if(master && ((a->masterId() == 0) || (a->masterId() == id)) )
                    {
                        a->setMaster(PRIMARY_MASTER); //set this anchor as primary master
                        a->setMasterId(0);
                    }
                    else if(master)
                    {
                        a->setMaster(SECONDARY_MASTER); //set this anchor as secondary master
                    }
                    else
                    {
                        a->setMaster(SLAVE);
                    }

                    a->setConnected(NotConnected);
                    anchorListWidget()->updateAnchorConnected(_model->anchorIdx(id), false);
                    qDebug() << "load file anc (HIDE)" << QString::number(id, 16);

                    //load a master list and associated rf distance
                    {
                        DataAnchor *master = nullptr;
                        DataUndirectedLink *undirectedLink = nullptr;
                        QDomNode m = e.firstChild();
                        while( !m.isNull() )
                        {
                            QDomElement me = m.toElement();
                            if( !me.isNull())
                            {
                                if(me.hasAttribute("offset"))
                                {//config for wired clock sync anchors
                                    uint64_t refID = (me.attribute( "addr", "" )).toULongLong(&ok, 16);

                                    //qDebug() << "refID" << QString::number(refID, 16);
                                    if (ok)
                                    {
                                        uint64_t tagID = (me.attribute( "tag", "" )).toULongLong(&ok, 16);

                                        if(ok)
                                        {
                                            double x = (me.attribute( "x", "" )).toDouble();
                                            double y = (me.attribute( "y", "" )).toDouble();
                                            double z = (me.attribute( "z", "" )).toDouble();
                                            int offset = (me.attribute( "offset", "" )).toInt();
                                            int blinks =  (me.attribute( "blinks", "" )).toInt();
                                            a->addReferencePair(refID, tagID, x, y, z, offset, blinks);
                                        }
                                    }
                                }
                                else
                                {

                                    uint64_t masterID = (me.attribute( "addr", "" )).toULongLong(&ok, 16);

                                    if (ok)
                                    {
                                        master = RTLSControllerApplication::model()->anchor(masterID, true);

                                        a->addMasterAnchor(masterID);

                                        qDebug() << "load file add master list anc " << QString::number(masterID, 16);
                                    }
                                    // Get the undirected link, and then get the receiver side of the link.
                                    // Add a new undirected link if it does not exist already (Anchor A, Anchor B, add the link to the model)

                                    if(masterID == id)
                                    {
                                        m = m.nextSibling();
                                        continue;
                                    }

                                    if (master)
                                    {
                                        if(masterID < id)
                                            undirectedLink = RTLSControllerApplication::model()->link(master, a, true);
                                        else
                                            undirectedLink = RTLSControllerApplication::model()->link(a, master, true);
                                    }
                                    if (undirectedLink)
                                    {
                                        undirectedLink->setRFDistance((me.attribute( "rfdistance", "" )).toDouble());

                                        undirectedLink->setUseRFDistance((me.attribute( "rfUseDistance", "" )).toInt());
                                    }
                                }
                            }

                            m = m.nextSibling();
                        }
                    } //end of loading of master list

                }
            }

            n = n.nextSibling();
        }

        //send the configuration to CLE
        if(connection != 0)
        {
            RTLSControllerApplication::control()->sendConfiguration();
        }
    }

    file.close();
}


QDomElement AnchorToNode( QDomDocument &d, DataAnchor *a )
{
    QDomElement cn = d.createElement( "anchor" );
    quint64 id = a->id();

    cn.setAttribute("addr", QString::number(id, 16));
    cn.setAttribute("id", QString::number(a->number()));
    cn.setAttribute("x", QString::number(a->x()));
    cn.setAttribute("y", QString::number(a->y()));
    cn.setAttribute("z", QString::number(a->z()));
    cn.setAttribute("master", (a->master() > 0)? "1" : "0");
    cn.setAttribute("selected", a->selected() ? "1" : "0");
    cn.setAttribute("ant_delay_rx", QString::number(a->antennaDlyRx()));
    cn.setAttribute("ant_delay_tx", QString::number(a->antennaDlyTx()));
    cn.setAttribute("master_id", (a->master() > 0)? QString::number(a->masterId(), 16) : "0");
    cn.setAttribute("master_lag_delay", QString::number(a->lagDelayUs()));

    //add a list of master anchors
    for(int m=0; m<a->masterAnchorListSize(); m++)
    {
       QDomElement cm = d.createElement( "master" );
       //get the link if it exists...
       uint64_t masterID = a->getMasterAnchorAddrAt(m);
       DataAnchor *master = RTLSControllerApplication::model()->anchor(masterID);
       DataUndirectedLink *undirectedLink = nullptr;
       double rfdist = 0;
       int rfdistUse = 0;

       // Get the undirected link, and then get the receiver side of the link.
       // Add a new undirected link if it does not exist already (Anchor A, Anchor B)
       if (master)
       {
           undirectedLink = RTLSControllerApplication::model()->link(master, a, false);
       }
       if (undirectedLink)
       {
           rfdist = undirectedLink->rfDistance();
           rfdistUse = undirectedLink->rfUseDistance();
       }
       cm.setAttribute("addr", QString::number(masterID, 16));
       cm.setAttribute("rfdistance", QString::number(rfdist));
       cm.setAttribute("rfUseDistance", QString::number(rfdistUse));
       cn.appendChild(cm);
    }

    //add a list of reference pairs
    for(int m=0; m<a->referencePairsListSize(); m++)
    {
       QDomElement cm = d.createElement( "reference" );

       ReferencePair rp = a->getReferencePairAt(m);

       cm.setAttribute("addr", QString::number(rp.anchorid, 16));
       cm.setAttribute("offset", QString::number(rp.offset));
       cm.setAttribute("blinks", QString::number(rp.blinks));
       cm.setAttribute("tag", QString::number(rp.tagid, 16));
       cm.setAttribute("x", QString::number(rp.tagx));
       cm.setAttribute("y", QString::number(rp.tagy));
       cm.setAttribute("z", QString::number(rp.tagz));
       cn.appendChild(cm);
    }


    return cn;
}

void RTLSControllerApplication::saveConfigFile(QString filename)
{
    QFile file( filename );

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug(qPrintable(QString("Error: Cannot read file %1 %2").arg("./CCconfig.xml").arg(file.errorString())));
        return;
    }

    QDomDocument doc;

    /*if(doc.setContent(&file, false))
    {
        qDebug("Failed to set content");
    }*/

    // Adding more elements - a list of anchors to the root element
    QDomElement anchlist = doc.createElement("anchor_list");
    doc.appendChild(anchlist);

    //save the anchor configuration
    //for each anchor in the anchors list
    foreach(DataAnchor *a, _model->anchors())
    {
        anchlist.appendChild( AnchorToNode(doc, a) );
    }

    QTextStream ts( &file );
    ts << doc.toString();

    file.close();

    qDebug() << doc.toString();

}

//
//   FUNCTION: calculateDistance(double, double, double, double, double, double)
//
//   PURPOSE: Calculate the X,Y co-ordinates based on the DecaRanging results
//
//   COMMENTS:
//
//        In this function, the 6 distances between each pair of anchors are used and calculated
//        for the co-ordinates of all the anchors.
//
bool RTLSControllerApplication::calculateDistance(double r12, double r13, double r14, double r23, double r24, double r34)
{
	bool error = false;
    double d12, d13, d14, d23, d24, d34;

    //get anchors with numbers 1,2,3,4 from the model...
    DataAnchor *a1 = _model->anchor(1);
    DataAnchor *a2 = _model->anchor(2);
    DataAnchor *a3 = _model->anchor(3);
    DataAnchor *a4 = _model->anchor(4);

    // Considering the height (z) differences of the anchors
    d12 = qSqrt(r12*r12 - (a1->z() - a2->z())*(a1->z() - a2->z()) );
    d13 = qSqrt(r13*r13 - (a1->z() - a3->z())*(a1->z() - a3->z()) );
    d14 = qSqrt(r14*r14 - (a1->z() - a4->z())*(a1->z() - a4->z()) );
    d23 = qSqrt(r23*r23 - (a2->z() - a3->z())*(a2->z() - a3->z()) );
    d24 = qSqrt(r24*r24 - (a2->z() - a4->z())*(a2->z() - a4->z()) );
    d34 = qSqrt(r34*r34 - (a3->z() - a4->z())*(a3->z() - a4->z()) );

    if(qIsNaN(d12) || qIsNaN(d13) || qIsNaN(d14) || qIsNaN(d23) || qIsNaN(d24) || qIsNaN(d34)
            || qIsInf(d12) || qIsInf(d13) ||qIsInf(d14) || qIsInf(d23) || qIsInf(d24) || qIsInf(d34))
    {
        error = true;
    }

    /*NOTES:
     * this is the layout we assume:
     *
     * ^ y (+)ve
     * ------------------------
     * |A3                  A4|
     * |                      |
     * |                      |
     * |                      |
     * |                      |
     * |A2                  A1|
     * ------------------------> x (+ve)
     *
    */
    else
    {
        double a3x, a4x, a3y, a4y;
        a3x = (((d23*d23)-(d13*d13)+(d12*d12))/(2*d12));
        a4x = (((d24*d24)-(d14*d14)+(d12*d12))/(2*d12));

        a3y = qSqrt((d23*d23)-(a3x*a3x));
        a4y = qSqrt((d24*d24)-(a4x*a4x));

        if(qIsNaN(a3x) || qIsNaN(a4x) || qIsNaN(a3y) || qIsNaN(a4y)
                || qIsInf(a3x) || qIsInf(a4x) || qIsInf(a3y) || qIsInf(a4y))
        {
            error = true;
        }
        else
        {
            a1->setX(d12); //distance a1 to a2 (assume a2 x = 0, then a1 x is at d12)
            a1->setY(0);   //assume a1, and a2 y are the same (e.g. 0)
            a2->setY(0);
            a2->setX(0);   //assume a2 x is 0

            a3->setX(a3x);
            a4->setX(a4x);

            a3->setY(a3y);
            a4->setY(a4y);

            qDebug("Anchor %llx %d : %f %f %f\n", a1->id(), 1, a1->x(), a1->y(), a1->z());
            qDebug("Anchor %llx %d : %f %f %f\n", a2->id(), 2, a2->x(), a2->y(), a2->z());
            qDebug("Anchor %llx %d : %f %f %f\n", a3->id(), 3, a3->x(), a3->y(), a3->z());
            qDebug("Anchor %llx %d : %f %f %f\n", a4->id(), 4, a4->x(), a4->y(), a4->z());
        }

    }
	return error;
}


//get each selected anchor and rotate it 90 degrees clockwise (around the origin)
void RTLSControllerApplication::rotate90Anchors()
{
    int i = 0;
    int angle = 90;
    QPolygonF p1 = QPolygonF(); // << QPoint(0, 1) << QPoint(4, 1) << QPoint(4, 2) << QPoint(0, 2);
    QPointF pt;

    const QList<DataAnchor *> &anchors = RTLSControllerApplication::model()->anchors();
    //first check if at least two anchors
    if (anchors.size() < 2)
        return;

    /*bool ok;

    QString idStr = QInputDialog::getText(NULL, "Enter the angle", "counter-clockwise", QLineEdit::Normal, "", &ok); // TODO: add custom dialog with format checking ...

    if (ok)
    {
        angle = idStr.toInt(&ok);
    }*/

    foreach(DataAnchor *a, anchors)
    {
        if(a->selected()) //if this anchor has been selected to be used in this RTLS/this CLE
        {
            if(i==0) pt = QPoint(a->x(), a->y());
            //add anchors to a polygon
            p1 << QPoint(a->x()*1000.0, a->y()*1000.0);
            //p1.putPoints(i, 1, a->x(), a->y());
            qDebug() << i << a->number() << a->x() << a->y();

            i++;
        }
    }

    //seems the translate/rotate rounds to integers so we turn into mm and then back to meters.
    QTransform t;
    t.translate(pt.x(), pt.y());
    t.rotate(angle);
    t.translate(-pt.x(), -pt.y());

    QPolygonF p2 = t.map(p1);

    i = 0;

    foreach(DataAnchor *a, anchors)
    {
        if(a->selected()) //if this anchor has been selected to be used in this RTLS/this CLE
        {
            //QPoint p = p2.point(i);

            QPointF p = p2.at(i);

            a->setX(p.x()*0.001);
            a->setY(p.y()*0.001);

            qDebug() << i << a->number() << a->x() << a->y();
            i++;
        }
    }

}

void RTLSControllerApplication::flipXYAnchors(int X)
{
    const QList<DataAnchor *> &anchors = RTLSControllerApplication::model()->anchors();

    foreach(DataAnchor *a, anchors)
    {
        if(a->selected()) //if this anchor has been selected to be used in this RTLS/this CLE
        {
            //flip
            if(X) //new Y = - old Y
            {
                a->setY(-a->y());
            }
            else //new X = - old X
            {
                a->setX(-a->x());
            }
        }
    }
}

double getDistance(int a, int b)
{
    DataUndirectedLink *undirectedLink = NULL;

    //range array = 1-2, 1-3, 1-4, 2-3, 2-4, 3-4
    DataAnchor *initiator = RTLSControllerApplication::model()->anchor(a);
    DataAnchor *responder = RTLSControllerApplication::model()->anchor(b);

    if((initiator == NULL) ||(responder == NULL))
        return 0;

    undirectedLink = RTLSControllerApplication::model()->link(initiator, responder, false);

    if(undirectedLink != NULL)
        return undirectedLink->rfDistance();
    else
        return 0;
}


void RTLSControllerApplication::autoPositionAnchors()
{
    int selected = 0;
    const QList<DataAnchor *> &anchors = RTLSControllerApplication::model()->anchors();

    anchorsRngList.clear();
    //Step 1 - check if have at least 4 anchors
    if (anchors.size() < 4)
    {
        QMessageBox::critical(nullptr, tr("Error"), "Need to have at least 4 anchors connected!");
        return;
    }

    //Step 2 - select 4 anchors if not already selected
    foreach(DataAnchor *a, anchors)
    {
        if(a->selected()) //if this anchor has been selected to be used in this RTLS/this CLE
        {
            a->setPosition(true);
            selected++;
            anchorsRngList << a;
        }
    }

    if((selected < 4) && (anchors.size() >= 4)) //select 4 anchors if less than 4 selected
    {
        foreach(DataAnchor *a, anchors)
        {
            if(!a->selected()) //if this anchor has been selected to be used in this RTLS/this CLE
            {
                a->setPosition(true);
                a->setSelected(true);
                selected++;
                anchorsRngList << a;
                if(selected == 4)
                    break;
            }
        }
    }

    //Step 3 - We have 4 selected anchors, perform TWR
    _control->setAutoPosition(true);
    _control->range(anchorsRngList);
    QMessageBox::warning(nullptr, tr("Ranging started"), "Ranging started, please wait to finish.");

}

void RTLSControllerApplication::positionAnchorsFromRange()
{
    QList<DataAnchor *> anchors = RTLSControllerApplication::model()->anchors();
    double rangeArray[6];
    bool error = false ;

    //range array = 1-2, 1-3, 1-4, 2-3, 2-4, 3-4
    rangeArray[0] = getDistance(1,2);
    rangeArray[1] = getDistance(1,3);
    rangeArray[2] = getDistance(1,4);
    rangeArray[3] = getDistance(2,3);
    rangeArray[4] = getDistance(2,4);
    rangeArray[5] = getDistance(3,4);


    if(rangeArray[0] == 0) {error = true;} //error
    if(rangeArray[1] == 0) {error = true;} //error
    if(rangeArray[2] == 0) {error = true;} //error
    if(rangeArray[3] == 0) {error = true;} //error
    if(rangeArray[4] == 0) {error = true;} //error
    if(rangeArray[5] == 0) {error = true;} //error

    if(error)
    {
        emit positionError();
        return;
    }


    //save old anchor coordinates
    foreach(DataAnchor *a, anchors)
    {
        a->saveCoords();
    }

    //get selected anchors and distances between them
    error = calculateDistance(rangeArray[0], rangeArray[1], rangeArray[2], rangeArray[3], rangeArray[4], rangeArray[5]);

    if(error)
    {
        emit positionError();
        return;
    }

    //QMessageBox::about(NULL, tr("Ranging finished"), "Ranging finished, the anchor positions have been updated.");
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(NULL, tr("Ranging finished"),
                                      "Ranging finished, the anchor positions have been updated.<br>Would you like to save it?",
                                      QMessageBox::Yes|QMessageBox::No);

        if(reply == QMessageBox::Yes)
        {
            //we need to send the new configuration to the CLE
            RTLSControllerApplication::control()->sendConfiguration();
        }
        else //user does not want to apply the new x,y,z coordinates...
        {
            //revert to old anchor coordinates
            foreach(DataAnchor *a, anchors)
            {
                if(a->position())
                {
                    a->setPosition(false);
                    a->restoreCoords();
                }
            }
        }
    }
}


