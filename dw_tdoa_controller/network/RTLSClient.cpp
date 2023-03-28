// -------------------------------------------------------------------------------------------------------------------
//
//  File: RTLSClient.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "RTLSClient.h"

#include "RTLSControllerApplication.h"
#include "RTLSCLEConnection.h"
#include "DataModel.h"
#include "DataLink.h"

#include <QDateTime>
#include <QThread>
#include <QXmlStreamWriter>
#include <QFile>
#include <QMessageBox>

#include <math.h>
#include <stdint.h>
#include "rtls_interface.h"


RTLSClient::RTLSClient(QObject *parent) :
    QObject(parent),
    socket(nullptr),
    file(nullptr)
{
    _graphicsWidgetReady = false ;
    tag_list.clear();

    sizeOfAncRep = sizeof(pos_report_t);
    sizeOfPosRep = sizeof(pos_report_t);
    sizeOfPosExtRep = sizeof(pos_report_ext_t);
    sizeOfStsRep = sizeof(stats_report_t);
    sizeOfImuRep = sizeof(imu_report_t);

    RTLSControllerApplication::connectReady(this, "onReady()");
}

RTLSClient::~RTLSClient()
{
    if(file != nullptr)
        file->close();
}

void RTLSClient::onReady()
{
    QObject::connect(RTLSControllerApplication::cleConnection(), SIGNAL(dataSocketConnected()),
                         this, SLOT(onConnected()));


}

void RTLSClient::onConnected()
{
    QDateTime now = QDateTime::currentDateTime();

    QString filename("../Logs/"+now.toString("yyyyMMdd_hhmmss")+"CCtag_log.txt");
    file = new QFile(filename);

    if (!file->open(QFile::ReadWrite | QFile::Text))
    {
        qDebug(qPrintable(QString("Error: Cannot read file %1 %2").arg(filename).arg(file->errorString())));
        QMessageBox::critical(nullptr, tr("Log File Error"), QString("Cannot create file %1 %2\nPlease make sure ../Logs/ folder exists.").arg(filename).arg(file->errorString()));
    }
    else
    {
        //write the header
        QString nowstr = now.toString("T:hhmmsszzz:");
        QString s = nowstr + QString("DecaWaveScalableRTLSSW:LogFile\n");
        QTextStream ts( file );
        ts << s;
    }
    //get pointer to RTLS Controller's CLE connection data socket
    socket = RTLSControllerApplication::cleConnection()->dataSocketPtr();

    QObject::connect(this->socket, SIGNAL(connected()), this, SLOT(connected()));
    QObject::connect(this->socket, SIGNAL(readyRead()), this, SLOT(newData()));
    QObject::connect(this->socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));

}

void RTLSClient::connectToHost(const QString & address, quint16 port)
{
    socket->connectToHost(address, port);
}

void RTLSClient::connected()
{
    //request anchor positions.. so we can display them
    char c = 0x41;
    this->socket->write(&c, 1);
    emit connectionSucceeded();
}

//initialise the tag reports strusture and add to the list
void RTLSClient::initialiseTagList(quint64 id)
{
    tag_reports_t r;
    memset(&r, 0, sizeof(tag_reports_t));
    r.id = id;
    r.ready = false;
    tag_list.append(r);
}

void RTLSClient::updateTagStatistics(int i, double x, double y, double z)
//update the history array and the average
{
    QDateTime now = QDateTime::currentDateTime();
    QString nowstr = now.toString("T:hhmmsszzz:");
    int j = 0;
    int idx = tag_list.at(i).arr_idx;
    uint64_t id = tag_list.at(i).id;
    tag_reports_t rp = tag_list.at(i);
    double avDistanceXY = 0;
    double sum_std = 0;
    double DistanceXY[HIS_LENGTH];
    double DstCentreXY[HIS_LENGTH];
    double stdevXY = 0;

    rp.av_x = 0;
    rp.av_y = 0;
    rp.av_z = 0;


    for(j=0; j<HIS_LENGTH; j++)
    {
       rp.av_x += rp.x_arr[j];
       rp.av_y += rp.y_arr[j];
       rp.av_z += rp.z_arr[j];
    }

    rp.av_x /= HIS_LENGTH;
    rp.av_y /= HIS_LENGTH;
    rp.av_z /= HIS_LENGTH;

    for(j=0; j<HIS_LENGTH; j++)
    {
        DistanceXY[j] = sqrt((rp.x_arr[j] - rp.av_x)*(rp.x_arr[j] - rp.av_x) + (rp.y_arr[j] - rp.av_y)*(rp.y_arr[j] - rp.av_y));
    }

    for (j=0; j<HIS_LENGTH; j++)
    {
        avDistanceXY += DistanceXY[j]/HIS_LENGTH;
    }

    for(j=0; j<HIS_LENGTH; j++)
    {
        sum_std += (DistanceXY[j]-avDistanceXY)*(DistanceXY[j]-avDistanceXY);

    }

    stdevXY = sqrt(sum_std/HIS_LENGTH);

    vec2d sum_tempXY = {0, 0};
    vec2d CentrerXY = {0, 0};

    int counterXY = 0;

    for(j=0; j<HIS_LENGTH; j++)
    {
        if (DistanceXY[j] < stdevXY*2)
        {
            sum_tempXY.x += rp.x_arr[j];
            sum_tempXY.y += rp.y_arr[j];
            counterXY++;
        }

    }

    CentrerXY.x  = sum_tempXY.x/counterXY;
    CentrerXY.y  = sum_tempXY.y/counterXY;

    for(j=0; j<HIS_LENGTH; j++)
    {
        DstCentreXY[j] = sqrt((rp.x_arr[j] - CentrerXY.x)*(rp.x_arr[j] - CentrerXY.x) + (rp.y_arr[j] - CentrerXY.y)*(rp.y_arr[j] - CentrerXY.y));
    }

    R95sort(DstCentreXY,0,HIS_LENGTH-1);

    rp.r95 = DstCentreXY[int(0.95*HIS_LENGTH)];

    //R95 = SQRT(meanErrx*meanErrx + meanErry*meanErry) + 2*SQRT(stdx*stdx+stdy*stdy)
    //rp.r95 = sqrt((rp.averr_x*rp.averr_x) + (rp.averr_y*rp.averr_y)) +
    //        2.0 * sqrt((rp.std_x*rp.std_x) + (rp.std_y*rp.std_y)) ;


    //update the value in the array
    rp.x_arr[idx] = x;
    rp.y_arr[idx] = y;
    rp.z_arr[idx] = z;

    rp.arr_idx++;
    //wrap the index
    if(rp.arr_idx >= HIS_LENGTH)
    {
        rp.arr_idx = 0;
        rp.ready = true;
    }

    rp.count++;

    //update the list entry
    tag_list.replace(i, rp);

    if(rp.ready)
    {
        if(_graphicsWidgetReady)
        {
            //emit tagStats(id, rp.av_x, rp.av_y, rp.av_z, rp.r95);
            emit tagStats(id, CentrerXY.x, CentrerXY.y, rp.av_z, rp.r95);
        }
        rp.ready = false;
    }

    //log data to file
    QString s = nowstr + QString("TS:%1 avx:%2 avy:%3 avz:%4 r95:%5\n").arg(QString::number(id,16)).arg(rp.av_x).arg(rp.av_y).arg(rp.av_z).arg(rp.r95);
    QTextStream ts( file );
    ts << s;
}

void RTLSClient::newData()
{
    char header[2];
    char data[100];
    QDateTime now = QDateTime::currentDateTime();
    QString nowstr = now.toString("T:hhmmsszzz:");
    //static int counter = 0;

    //qDebug() << "newData " << this->socket->bytesAvailable() << counter++;

    while(this->socket->bytesAvailable())
    {
        qint64 dataLen = this->socket->peek(data, sizeof(data));

        //Debug Header
        QString string = QString::fromLocal8Bit(data);
        qDebug() << dataLen << string;

        //TODO: Debug write Any info into Logfiles
        /*if(!RTLSControllerApplication::instance()->debugFile()->open(QFile::Append|QFile::Text))
        {
            qDebug()<<"Can not open file";
        }
        else
        {
           QTextStream stream(RTLSControllerApplication::instance()->debugFile());
           stream << data<<"\n";
           RTLSControllerApplication::instance()->debugFile()->close();
        }*/

        //look at the header

        if((data[0] == RTLS_DATA_ANCHOR)|| //this is an anchor list header
           (data[0] == RTLS_DATA_BLINK) || //this is a blink coordinates header
           (data[0] == RTLS_DATA_BLINK_EXT) || //this is a blink coordinates header + anchor positions
           (data[0] == RTLS_DATA_STATS) || //this is a blink status header
           (data[0] == RTLS_DATA_IMU) ) //this is an imu data header
        {

            //this gives a list of anchors and their positions
            if ((data[0] == RTLS_DATA_ANCHOR) && (dataLen > 1))
            {
                if (this->socket->bytesAvailable() >= ((sizeOfAncRep * data[1]) + 2))
                {
                    this->socket->read(&header[0], 2); //read the header
                    for (int i = 0; i < header[1]; i++)
                    {
                        pos_report_t report;
                        this->socket->read((char*)&report, sizeof(report));

                        if(_graphicsWidgetReady)
                        {
                            emit anchor(report.id, report.x, report.y, report.z);
                        }
                        QString s = nowstr + QString("AP:%1 x:%2 y:%3 z:%4").arg(QString::number(report.id,16)).arg(report.x).arg(report.y).arg(report.z);
                        QTextStream ts( file );
                        ts << s;

                    }
                    //continue;
                }
                else
                {
                    break; //wait for more data
                }
            }
            //this gives a tag ID with its position (x, y, z)
            else if (data[0] == RTLS_DATA_BLINK)
            {

                if (this->socket->bytesAvailable() >= (sizeOfPosRep + 1))
                {
                    int i = 0;
                    //this->socket->read(&header[0], 1); //read the header
                    pos_report_t report;
                    this->socket->getChar(0); // Discard header
                    this->socket->read((char*)&report, sizeof(report)); //Read the report

                    //TODO: a Tag data RTLS_DATA_BLINK 这个
                    if(!RTLSControllerApplication::instance()->debugFile()->open(QFile::Append|QFile::Text))
                    {
                        qDebug()<<"Can not open  datafile";
                    }
                    else
                    {
                       QTextStream stream(RTLSControllerApplication::instance()->debugFile());
                       QString s =  QString("RTLS_DATA_BLINK ID:%1 x:%2 y:%3 z:%4 : ")
                                    .arg(QString::number(report.id,16)).arg(report.x).arg(report.y).arg(report.z);
                       stream <<s<<"\n";
                       RTLSControllerApplication::instance()->debugFile()->close();
                    }

                    if(_graphicsWidgetReady)
                    {
                        emit tagPos(report.id, report.x, report.y, report.z); //send the update to graphic
                    }
                    //log data to file
                    QString s = nowstr + QString("TP:%1 x:%2 y:%3 z:%4\n").arg(QString::number(report.id,16)).arg(report.x).arg(report.y).arg(report.z);
                    QTextStream ts( file );
                    ts << s;

                    //find the tag in the list
                    for(i=0; i<tag_list.size(); i++)
                    {
                        //find this tag in the list
                        if(tag_list.at(i).id == report.id)
                            break;
                    }

                    //if we don't have this tag in the list add it
                    if(i == tag_list.size())
                    {
                        initialiseTagList(report.id);
                    }

                    updateTagStatistics(i, report.x, report.y, report.z);

                    //finished with this tag position report
                    //continue;
                }
                else
                {
                    break; //wait for more data
                }
            }
            //this gives a tag ID with its position (x, y, z) and Anchors in multilateration
            else if (data[0] == RTLS_DATA_BLINK_EXT)
            {

                if (this->socket->bytesAvailable() >= (sizeOfPosExtRep + 1))
                {

                    int i = 0;
                    //this->socket->read(&header[0], 1); //read the header
                    pos_report_ext_t report;
                    this->socket->getChar(0); // Discard header
                    this->socket->read((char*)&report, sizeof(report)); //Read the report

                    //log data to file

                    QString s = nowstr + QString("TP:%1 x:%2 y:%3 z:%4 : A: %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15 %16 %17 %18 %19 %20 \n")
                            .arg(QString::number(report.id,16)).arg(report.x).arg(report.y).arg(report.z)
                            .arg(report.idx[0]).arg(report.idx[1]).arg(report.idx[2]).arg(report.idx[3])
                            .arg(report.idx[4]).arg(report.idx[5]).arg(report.idx[6]).arg(report.idx[7])
                            .arg(report.idx[8]).arg(report.idx[9]).arg(report.idx[10]).arg(report.idx[11])
                            .arg(report.idx[12]).arg(report.idx[13]).arg(report.idx[14]).arg(report.idx[15]);

                    QTextStream ts( file );
                    ts << s;

                    //TODO: update data form
                    if(!RTLSControllerApplication::instance()->debugFile()->open(QFile::Append|QFile::Text))
                    {
                        qDebug()<<"Can not open  datafile";
                    }
                    else
                    {
                       QTextStream stream(RTLSControllerApplication::instance()->debugFile());
                       QString s =  QString("RTLS_DATA_BLINK_EXT ID:%1 x:%2 y:%3 z:%4 : A: %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15 %16 %17 %18 %19 %20 length: %21 \n")
                                    .arg(QString::number(report.id,16)).arg(report.x).arg(report.y).arg(report.z)
                                    .arg(report.idx[0],0,16).arg(report.idx[1],0,16).arg(report.idx[2],0,16).arg(report.idx[3],0,16)
                                    .arg(report.idx[4],0,16).arg(report.idx[5],0,16).arg(report.idx[6],0,16).arg(report.idx[7],0,16)
                                    .arg(report.idx[8],0,16).arg(report.idx[9],0,16).arg(report.idx[10],0,16).arg(report.idx[11],0,16)
                                    .arg(report.idx[12],0,16).arg(report.idx[13],0,16).arg(report.idx[14],0,16).arg(report.idx[15],0,16).arg(report.len);
                       stream <<s<<"\n";
                       RTLSControllerApplication::instance()->debugFile()->close();
                    }

                    if(_graphicsWidgetReady)
                    {
                        emit tagPos(report.id, report.x, report.y, report.z); //send the update to graphic
                    }

                    //find the tag in the list
                    for(i=0; i<tag_list.size(); i++)
                    {
                        //find this tag in the list
                        if(tag_list.at(i).id == report.id)
                            break;
                    }

                    //if we don't have this tag in the list add it
                    if(i == tag_list.size())
                    {
                        initialiseTagList(report.id);
                    }

                    updateTagStatistics(i, report.x, report.y, report.z);

                    //finished with this tag position report
                    //continue;
                }
                else
                {
                    break; //wait for more data
                }
            }
            //this gives a tag ID with its statistics (x, y)
            else if (data[0] == RTLS_DATA_STATS)
            {
                if (this->socket->bytesAvailable() >= (sizeOfStsRep + 1))
                {
                    stats_report_t report;
                    this->socket->getChar(0); // Discard header
                    this->socket->read((char*)&report, sizeof(report));

                    //log data to file
                    QString s = nowstr + QString("CS:%1 blinkrx:%2 multirate:%3\n").arg(QString::number(report.id,16)).arg(report.blinkrx).arg(report.multirate);
                    QTextStream ts( file );
                    ts << s;
                    if(_graphicsWidgetReady)
                    {
                        emit tagCLEStats(report.id, report.blinkrx, report.multirate);
                    }
                }
                else
                {
                    break; //wait for more data
                }
            }
            //this gives a tag ID with its imu data
            else if (data[0] == RTLS_DATA_IMU)
            {
                if (this->socket->bytesAvailable() >= (sizeOfImuRep + 1))
                {
                    imu_report_t report;

                    this->socket->read(&header[0], 1); //read the header

                    //this->socket->getChar(0); // Discard header
                    this->socket->read((char*)&report, sizeof(report));
                    if(_graphicsWidgetReady)
                    {
                        //NOT SUPPORTED - emit tagIMUData(report.id, report.time, &report.data[0]);
                    }
                }
                else
                {
                    break; //wait for more data
                }
            }
            else
            {
                break; //wait for more data
            }
        } //end of if (header)
        else //if the string does not start with the header dump it...
        //not starting with header - dump the bytes
        //if(this->socket->bytesAvailable())
        {
            char array[100];
            qDebug() << "dump bytes: " << this->socket->bytesAvailable();

            qint64 len = this->socket->bytesAvailable();
            if (len > sizeof(array)) len = sizeof(array);
            this->socket->read(array, len);
        }
	}
}

void RTLSClient::emitblink(quint64 id, double x, double y, double z)
{
    emit tagPos(id, x, y, z);
}

void RTLSClient::error(QAbstractSocket::SocketError socketError)
{
    emit connectionError(socketError);
}

void RTLSClient::setGWReady(bool set)
{
    _graphicsWidgetReady = set;
}

void R95sort (double s[], int l, int r)
{
    int i,j;
    double x;
    if(l<r)
    {
        i = l;
        j = r;
        x = s[i];
        while(i<j)
        {
            while (i<j&&s[j]>x) j--;
            if (i<j) s[i++] = s[j];
            while (i<j&&s[i]<x) i++;
            if (i < j) s[j--] = s[i];
        }
        s[i] = x;
        R95sort(s, l, i-1);
        R95sort(s, i+1, r);
    }

}
