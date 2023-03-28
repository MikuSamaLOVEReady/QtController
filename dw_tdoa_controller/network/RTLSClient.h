// -------------------------------------------------------------------------------------------------------------------
//
//  File: RTLSClient.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef RTLSCLIENT_H
#define RTLSCLIENT_H

#include <QObject>
#include <QTcpSocket>

#include "RTLSCLEConnection.h"
#include <stdint.h>
#include "rtls_interface.h"

class QFile;
class DataAnchor;
class DataTag;
class QXmlStreamReader;

#define HIS_LENGTH 100

typedef struct
{
    double x_arr[HIS_LENGTH];
    double y_arr[HIS_LENGTH];
    double z_arr[HIS_LENGTH];
    double av_x, av_y, av_z; //average
    double sqx_arr[HIS_LENGTH]; //square x
    double sqy_arr[HIS_LENGTH];
    double sqz_arr[HIS_LENGTH];
    double avsq_x, avsq_y, avsq_z; //average of squares
    double errx_arr[HIS_LENGTH]; //error x (x-av_x)
    double erry_arr[HIS_LENGTH];
    double errz_arr[HIS_LENGTH];
    double averr_x, averr_y, averr_z; //avearge error
    double variancex, variancey, variancez;
    double std_x, std_y, std_z;
    double r95;
    uint64_t id;
    int arr_idx;
    int count;
    bool ready;
} tag_reports_t;

typedef struct
{
    double x, y, z;
    uint64_t id;
} pos_report_t;


typedef struct
{
    double x, y, z;
    uint64_t	id;
    uint8_t    len;
    uint16_t	idx[16]; //hardcoded, -> should correspond to definition of MAX_RX_BLINKS
} pos_report_ext_t;

typedef struct
{
    double blinkrx, multirate;
    uint64_t id;
} stats_report_t;

typedef struct {
    uint8_t mask;
    uint16_t mx, my, mz;
    uint16_t ax, ay, az;
    uint16_t gx, gy, gz;
    uint32_t pressure;
    uint16_t temp;
} imu_sens_t;

typedef struct
{
    uint8_t data[DWT_SIZEOFIMUDATA];
    uint64_t id;
    double time;
} imu_report_t;

typedef struct
{
  double x;
  double y;
} vec2d;


class RTLSClient : public QObject
{
    Q_OBJECT
public:
    explicit RTLSClient(QObject *parent = 0);
    virtual ~RTLSClient();

    void emitblink (quint64 tagId, double x, double y, double z);

    void updateTagStatistics(int i, double x, double y, double z);
    void initialiseTagList(quint64 id);

    void setGWReady(bool set);

signals:
    void anchor(quint64 anchorId, double x, double y, double z);
    void tagPos(quint64 tagId, double x, double y, double z);
    void tagStats(quint64 tagId, double x, double y, double z, double r95);
    void tagCLEStats(quint64 tagId, double blinkrx, double multirate);
    //void tagIMUData(quint64 tagId, double time, const uint8_t *imudata);
    void connectionSucceeded();
    void connectionError(QAbstractSocket::SocketError socketError);

public slots:
    void connectToHost(const QString & address, quint16 port);

protected slots:
    void onReady();
    void onConnected();

private slots:
    void connected();
    void newData();
    void error(QAbstractSocket::SocketError socketError);

private:
    bool _graphicsWidgetReady;
    QTcpSocket *socket;

    QList <tag_reports_t> tag_list;

    QFile *file;

    int sizeOfAncRep ;
    int sizeOfPosRep ;
    int sizeOfPosExtRep ;
    int sizeOfStsRep ;
    int sizeOfImuRep ;
};

void R95sort(double s[], int l, int r);

#endif // RTLSCLIENT_H
