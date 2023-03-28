// -------------------------------------------------------------------------------------------------------------------
//
//  File: RTLSControl.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef RTLSCONTROL_H
#define RTLSCONTROL_H

#include <QObject>

#include "RTLSCLEConnection.h"

class DataAnchor;
class QXmlStreamReader;

class RTLSControl : public QObject
{
    Q_OBJECT
public:
    explicit RTLSControl(QObject *parent = 0);

    void sendMotionFilterCmd(int filter);
    void sendLoggingCmd(int level);
    QStringList getMotionFilter(void);
    QStringList getLogging(void);

    void setAutoPosition(bool set) {_autoPosition = set ;}

    void sendRFConfiguration(bool userFeedback);

signals:
    void rtlsStateChanged(RTLSCLEConnection::RTLSState state);
    void rfCfgApplied(void);
    void rangingProgress(QString status);
    void commsTestProgress(QString status);
    void statusBarMessage(QString status);
    void disableReSync(bool);
    void positionAnchors();
    void setLogging(int);
    void setFilter(int);
    void clearTags();

    void toSwitchonTag();

public slots:
    void reSyncAnchors();
    void reSyncAnchors(const QList<DataAnchor *> &anchors);
    void range(const DataAnchor *initiator, const DataAnchor *responder);
    void range(const QList<DataAnchor *> &anchors);
    void range(int calibration);
    void accumLog(int ccp_blink , const QList<DataAnchor *> &anchors);
    void communicationTest(DataAnchor *transmitter);
    void communicationTest(const QList<DataAnchor *> &transmitters);
    void powerTest(DataAnchor *anchor);
    void powerTest(const QList<DataAnchor *> &anchors);
    void sendConfiguration(void);
    void sendConfiguration(DataAnchor *anchor);
    void sendConfiguration(const QList<DataAnchor *> &anchors);

    void sendResetCmd(void);
    void sendStartStopCmd(void);
    void sendAddTextToLog(QString string);

    void sendRFConfigurationClick(void);

    void sendCLECfgReq(void);
    void sendAnchorListReq(void);
    void stop();
    void start();
    void logging(int);

    //void calibrateAntennas(void);

protected slots:
    void onReady();
    void connectionStateChanged(RTLSCLEConnection::ConnectionState state);
    void onMessageReceived(const QByteArray &line);

protected:
    void handleCommResult(QXmlStreamReader &stream);
    void handleRangeResult(QXmlStreamReader &stream);
    void handleStatusIndication(QXmlStreamReader &stream);
    void handleAnchorList(QXmlStreamReader &stream);
    void handleAnchorCfg(QXmlStreamReader &stream);
    void handleRFCfgResult(QXmlStreamReader &stream);
    void handleCLECfgResult(QXmlStreamReader &stream);
    void handleSystemStatus(QXmlStreamReader &stream);

    void updateRTLSState(int state);
private:
    RTLSCLEConnection *connection;
    int _running; //RTLS system status (stopped/running) ... maybe make it an enum with particular states...
    bool _autoPosition;
    bool _userFeedback;
    int _rxRangingResult;
    bool _wirelessSync;

    QStringList motionFilterTypes ;
    QStringList loggingTypes ;
};

#endif // RTLSCONTROL_H
