// -------------------------------------------------------------------------------------------------------------------
//
//  File: RTLSControl.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "RTLSControl.h"

#include "RTLSControllerApplication.h"
#include "RTLSCLEConnection.h"
#include "DataAnchor.h"
#include "DataModel.h"
#include "DataLink.h"
#include "ChannelSettingsWidget.h"
#include "anchorlistwidget.h"
#include <QThread>
#include <QXmlStreamWriter>

#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

RTLSControl::RTLSControl(QObject *parent) :
    QObject(parent),
    connection(nullptr),
    _running(0),
    _autoPosition(false),
    _userFeedback(true),
    _rxRangingResult(0),
    _wirelessSync(true)
{

    /*
    Filter Types
    0 - No Filtering
    1 - Moving Average
    2 - Moving Average excluding max and min
    3 - Kalman Filter
    */
    motionFilterTypes << "No Filtering" << "Moving Average" << "Moving Avg. ex" << "Kalman";

    /*
    Logging Types
    0 - No Logging
    1 - Level 1
    2 - Level 2
    3 - Log Diagnostics
    4 - Log Diagnostics + ALL CS_RX
    */
    loggingTypes << "No Logging" << "Level 1" << "Level 2" << "Diagnostics" << "Diagnostics ALL CCP";

    RTLSControllerApplication::connectReady(this, "onReady()");
}

//这种onready 需要mate操作
void RTLSControl::onReady()
{
    //get pointer to RTLS Controller's CLE connection
    connection = RTLSControllerApplication::cleConnection();

    //configure two SIGNAL-SLOT pairs to handle:
    //1. change of RTLSCLEConnection::ConnectionState state
    //2. messages received from the CLE
    QObject::connect(connection, SIGNAL(connectionStateChanged(RTLSCLEConnection::ConnectionState)),
                     this, SLOT(connectionStateChanged(RTLSCLEConnection::ConnectionState)));
    QObject::connect(connection, SIGNAL(messageReceived(QByteArray)),
                     this, SLOT(onMessageReceived(QByteArray)));

    QObject::connect(this, SIGNAL(positionAnchors()),
                     RTLSControllerApplication::instance(), SLOT(positionAnchorsFromRange()));
}

void RTLSControl::connectionStateChanged(RTLSCLEConnection::ConnectionState state)
{
    qDebug() << "RTLSControl::connectionStateChanged " << state;

    //Don't send anything on the connection.... the CLE will send anchor list
    //after processing the anchor list the controller will send configuration message
}

//
void RTLSControl::onMessageReceived(const QByteArray &line)
{
    QXmlStreamReader stream(line);

    //qDebug() << "RTLSControl::onMessageReceived" ;

    //ind 表示由CLE 返回的内容
    if (stream.readNextStartElement() && stream.name() == "ind")
    {
        QStringRef type = stream.attributes().value("type");

        qDebug() << stream.name() << " value = " << type;

        if (type == "comm test")
        {
            handleCommResult(stream);
        }
        else if(type == "cle cfg")
        {
            handleCLECfgResult(stream);
        }
        else if(type == "rf cfg")
        {
            if(_userFeedback)
                handleRFCfgResult(stream);
        }
        else if(type == "range test")
        {
            handleRangeResult(stream);
        }
        else if(type == "system status")
        {
            handleSystemStatus(stream);
        }
        else if(type == "status")
        {
            handleStatusIndication(stream);
        }
        else if(type == "anchor list")
        {
            handleAnchorList(stream);
        }
        else if(type == "anchor cfg")
        {
            handleAnchorCfg(stream);
        }
        else if(type == "rtls start")
        {
            //change the Start/Stop button to "Stop"
            emit rtlsStateChanged(RTLSCLEConnection::Running);
            updateRTLSState(1);
        }
        else if(type == "rtls stop")
        {
            //change the Stop/Start button to "Start"
            emit rtlsStateChanged(RTLSCLEConnection::Stopped);
            updateRTLSState(0);
        }
        else if(type == "rtls reset_complete")
        {
            //response
            qDebug() << "rtls reset_complete received";
        }
    }
}

void RTLSControl::handleSystemStatus(QXmlStreamReader &stream)
{
    // <ind type=“system status” msg=”connecting” addr="1" /></ind>
    // <ind type=“system status” msg=”comm tx” addr="1" /></ind>
    // <ind type=“system status” msg=”ranging” initiator="1" responder =“2”/></ind>
     QStringRef msg = stream.attributes().value("msg");
     QString statusMsg;
     if (msg == "connecting")
     {
         statusMsg += "Connecting to anchor " + stream.attributes().value("addr").toString() + " ......";
         emit commsTestProgress(statusMsg);
     }
     else if(msg == "comm tx")
     {
         statusMsg += "Communcation test :  transmit by anchor " + stream.attributes().value("addr").toString() + " ......";
         emit commsTestProgress(statusMsg);
     }
     else if(msg == "ranging")
     {
         statusMsg += "Ranging anchors: " + stream.attributes().value("initiator").toString() + " and " + stream.attributes().value("responder").toString() + " ......";
         //emit rangingProgress(statusMsg);
         emit statusBarMessage(statusMsg);
     }
     else
     {
        qDebug() << "handleSystemStatus msg: " << msg.toString();
     }

}

void RTLSControl::handleAnchorCfg(QXmlStreamReader &stream)
{
    bool ok;
    uint64_t id;
    QString status;

    while (stream.readNextStartElement())
    {
        if (stream.name() == "anchor")
        {
            id = stream.attributes().value("addr").toULongLong(&ok, 16);

            if (ok) //check its status
            {
                _connected_e connected = NotConnected;
                DataAnchor *a = RTLSControllerApplication::model()->anchor(id);
                int idx = RTLSControllerApplication::model()->anchorIdx(id);

                status = stream.attributes().value(("status")).toString();

                if(idx != -1)
                {
                    if(status.contains("opened", Qt::CaseInsensitive)) //socket opened - CLE conented to this anchor
                    {
                        connected = EthConnected;
                    }

                    //UWB backhaul connections
                    if(status.contains("configured via", Qt::CaseInsensitive)) //Configured via coordinator
                    {
                       connected = UwbBhConnected;
                    }
                }

                {
                    a->setConnected(connected);
                    RTLSControllerApplication::anchorListWidget()->updateAnchorConnected(idx, connected);

                    qDebug() << "ind cfg SHOW/HIDE " << QString::number(id, 16);
                }

                //qDebug() << stream.name() << "cfg " << id << status;
            }

            stream.skipCurrentElement();

        }
        else
        {
            stream.skipCurrentElement();
        }
    }
}

void RTLSControl::handleAnchorList(QXmlStreamReader &stream)
{
    bool ok;
    uint64_t id;
    while (stream.readNextStartElement())
    {
        if (stream.name() == "anchor")
        {
            //DataAnchor *anchor = nullptr;

            id = stream.attributes().value("addr").toULongLong(&ok, 16);

            if (ok) //this will add the new anchor if it is not in the list already
            {
                RTLSControllerApplication::model()->anchor(id, true);

                //show all anchors after update
                RTLSControllerApplication::anchorListWidget()->updateAnchorConnected(RTLSControllerApplication::model()->anchorIdx(id), true);

                qDebug() << "ind list SHOW " << QString::number(id, 16);
            }
            //qDebug() << stream.name() << "Anchor " << id;

            stream.skipCurrentElement();

        }
        else
            stream.skipCurrentElement();
    }
}

void RTLSControl::updateRTLSState(int state)
{
    //update the button text and other GUI CLE's status indicators
    switch(state)
    {
    case 0:
        _running = state;
        emit rtlsStateChanged(RTLSCLEConnection::Stopped);
        emit statusBarMessage(tr("RTLS stopped"));
        break;
    case 1:
        _running = state;
        emit rtlsStateChanged(RTLSCLEConnection::Running);
        emit statusBarMessage(tr("RTLS running"));
        break;
    default:
        _running = state;
        emit rtlsStateChanged(RTLSCLEConnection::Unknown);
        emit statusBarMessage(tr("RTLS status unknown"));
        break;
    }
}

void RTLSControl::handleStatusIndication(QXmlStreamReader &stream)
{
    bool ok;
    int state;

    while (stream.readNextStartElement()) {
        if (stream.name() == "state") {

            state = stream.attributes().value("").toInt(&ok);

            if(ok)
            {
                updateRTLSState(state);
            }
        }
    }
}


void RTLSControl::handleCLECfgResult(QXmlStreamReader &stream)
{
    bool ok;
    int  ll;
    int  mf;
    int  rtls, wireless;
    while (stream.readNextStartElement())
    {
        if (stream.name() == "cle")
        {
            ll = stream.attributes().value("loglevel").toInt(&ok, 10);

            if (ok) //set the long level
            {
                emit setLogging(ll);
            }

            mf = stream.attributes().value("motionfilter").toInt(&ok, 10);

            if (ok) //set the long level
            {
                emit setFilter(mf);
            }

            rtls = stream.attributes().value("state").toInt(&ok, 10);

            if (ok) //set the long level
            {
                updateRTLSState(rtls);
            }

            wireless = stream.attributes().value("sync").toInt(&ok, 10);

            if (ok) //set the long level
            {
                if(wireless == 0)
                {
                    _wirelessSync = false;
                    emit statusBarMessage(tr("Wired sync support."));
                    emit disableReSync(false);
                }
                else
                {
                    _wirelessSync = true;
                    emit statusBarMessage(tr("Wireless sync support."));
                    emit disableReSync(true);
                }

            }

            stream.skipCurrentElement();

        }
        else
            stream.skipCurrentElement();
    }
}

void RTLSControl::handleRFCfgResult(QXmlStreamReader &stream)
{
    Q_UNUSED(stream);
    emit rfCfgApplied();
}


void RTLSControl::handleCommResult(QXmlStreamReader &stream)
{
    bool ok;
    uint64_t id;
    while (stream.readNextStartElement()) {
        if (stream.name() == "transmitter") {

            DataAnchor *transmitter = nullptr;

            id = stream.attributes().value("addr").toULongLong(&ok, 16);

            if (ok)
                transmitter = RTLSControllerApplication::model()->anchor(id);

            qDebug() << stream.name() << "TX ID " << id;

            //if we have an anchor with this ID
            if (transmitter)
            {
                while (stream.readNextStartElement()) {

                    DataAnchor *receiver = nullptr;
                    DataUndirectedLink *undirectedLink = nullptr;
                    //DataLink *link = nullptr;

                    id = stream.attributes().value("addr").toULongLong(&ok, 16);

                    if (ok)
                        receiver = RTLSControllerApplication::model()->anchor(id);

                    qDebug() << stream.name() << "RX ID " << id;

                    // Get the undirected link, and then get the receiver side of the link.
                    // Add a new undirected link if it does not exist already (Anchor A, Anchor B, add the link to the model)
                    if (receiver)
                        undirectedLink = RTLSControllerApplication::model()->link(transmitter, receiver, true);

                    //if (undirectedLink) //we have the undirected link between A and B
                      //  link = (undirectedLink->anchorA()->id() == receiver->id()) ? undirectedLink->linkA() : undirectedLink->linkB();

                    qDebug() << "anchA " << undirectedLink->anchorA()->id() << "anchB " << undirectedLink->anchorB()->id();

                    // Set the RX Ratio
                    // TODO: The number of TX frames is fixed, the CLE should send it. (assume it is 1000)
                    //if (link)
                    //{
                      //  qDebug() << "link parent " << link->parent()->id();
                      //  qDebug() << "link target " << link->target()->id();
                      //  link->setRXRatio(stream.attributes().value("rx_data").toDouble() / 1000.);
                    //}
                    if (undirectedLink)
                    {
                        undirectedLink->linkA()->setRXRatio(stream.attributes().value("rx_data").toDouble() / 1000.);
                        undirectedLink->linkB()->setRXRatio(stream.attributes().value("rx_data").toDouble() / 1000.);
                    }
                    stream.skipCurrentElement();
                }
            }
            else
                stream.skipCurrentElement();

        }
        else
            stream.skipCurrentElement();
    }

    emit commsTestProgress(tr("Received Communication Test Result"));
}

void RTLSControl::handleRangeResult(QXmlStreamReader &stream)
{
    bool ok;
    uint64_t idi;
    uint64_t idr;
    double distance = 0;
    while (stream.readNextStartElement())
    {
        if (stream.name() == "initiator")
        {
            DataAnchor *initiator = nullptr;

            idi = stream.attributes().value("addr").toULongLong(&ok, 16);

            if (ok)
            {
                initiator = RTLSControllerApplication::model()->anchor(idi);
            }
            if (initiator)
            {
                while (stream.readNextStartElement()) {

                    DataAnchor *responder = nullptr;
                    DataUndirectedLink *undirectedLink = nullptr;
                    //DataLink *link = nullptr;

                    idr = stream.attributes().value("addr").toULongLong(&ok, 16);

                    if (ok)
                    {
                        responder = RTLSControllerApplication::model()->anchor(idr);
                    }
                    // Get the undirected link, and then get the receiver side of the link.
                    // Add a new undirected link if it does not exist already (Anchor A, Anchor B, add the link to the model)
                    if (responder)
                    {
                        undirectedLink = RTLSControllerApplication::model()->link(initiator, responder, true);
                    }
                    if (undirectedLink)
                    {
                        distance = stream.attributes().value("distance").toDouble();
                        undirectedLink->setRFDistance(distance);
                        undirectedLink->setUseRFDistance(0); //don't use RF for master-slave sync unless user configures it
                        qDebug() << "RF distance " << distance << "anchs " << idi << idr << " #" << _rxRangingResult;
                    }
                    else
                    {
                        qDebug() << "handleRangeResult: undirectedLink!";
                    }
                    stream.skipCurrentElement();
                }
            }
            else
            {
                stream.skipCurrentElement();
            }

        }
        else
        {
            stream.skipCurrentElement();
        }
    }

    //emit rangingProgress(tr("Received ranging result: ") + QString::number(distance, 'f', 3));
    emit statusBarMessage(tr("Received ranging results: ") + QString::number(distance, 'f', 3));

    _rxRangingResult++;

    //auto - position anchors when ranging finished
    //TODO: autopositioning should work only once after all planned anchors ranges,
    //      not like here
    if(_autoPosition)
    {
        if(_rxRangingResult >= 6)
        {
            emit positionAnchors();
            emit rangingProgress("Finished with auto-positioning.");
        }
    }
}

/*void RTLSControl::calibrateAntennas(void)
{
    range(1); //calibration test
}*/

void RTLSControl::accumLog(int ccp_blink , const QList<DataAnchor *> &anchors)
{
    int nLogs = 10;
    QIODevice *device;
    if (!connection || !(device = connection->controlDevice()))
        return;

    if (anchors.size() < 1)
    {
        qDebug() << "Please select anchors from the list" ;
        return;
    }
    bool ok;

    QString idStr = QInputDialog::getText(nullptr, "Num Logs", "#", QLineEdit::Normal, "1", &ok);

    if (ok && !idStr.isEmpty())
    {
        nLogs = idStr.toUShort(&ok , 10);
    }

    if (ok && (nLogs>0))
    {
        QXmlStreamWriter stream(device);
        stream.writeStartDocument();

        stream.writeStartElement("req");
        stream.writeAttribute("type", "log accumulator");
        stream.writeAttribute("num_accum", QString::number(nLogs));
        stream.writeAttribute("ccp_blink", QString::number(ccp_blink));

        foreach (DataAnchor *tx, anchors) {
            stream.writeStartElement("anchor");
            stream.writeAttribute("addr",  QString::number(tx->id(), 16));
            stream.writeEndElement();
        }

        stream.writeEndElement();

        stream.writeEndDocument();
    }
}

/*
Standard TWR
<req type="range test"  calibration="0"></req>
Or
<req type="range test"></req>

Antenna Calibration Test
<req type="range test"  calibration="1"></req>
*/
void RTLSControl::range(int calibration)
{
    QIODevice *device;
    if (!connection || !(device = connection->controlDevice()))
        return;

    Q_UNUSED(calibration)

    QXmlStreamWriter stream(device);

    _rxRangingResult = 0;

    stream.writeStartDocument();

    stream.writeStartElement("req");
    stream.writeAttribute("type", "range test");
    stream.writeAttribute("calibration", QString::number(1));
    stream.writeEndElement();

    stream.writeEndDocument();

    emit rangingProgress(tr("Antenna Calibration Test in Progress"));
}

void RTLSControl::range(const QList<DataAnchor *> &anchors)
{
    int numRanges = 200;
    QIODevice *device;
    if (!connection || !(device = connection->controlDevice()))
        return;

    _rxRangingResult = 0;
    if (anchors.size() < 2)
    {
        qDebug() << " minimum number of anchors is two - can't do raning test! " << anchors.size() ;
        return;
    }
    bool ok;

    QString idStr = QInputDialog::getText(nullptr, "Number of Ranges", "#", QLineEdit::Normal, "", &ok, Qt::MSWindowsFixedSizeDialogHint); // TODO: add custom dialog with format checking ...

    if (ok)
    {
        int nRanges = idStr.toInt(&ok);

        if(ok &&(nRanges > 0))
        {
            numRanges = nRanges;
        }
    }
    else
    {
        return; //"x" - cancel
    }

    QXmlStreamWriter stream(device);
    stream.writeStartDocument();

    stream.writeStartElement("req");
    stream.writeAttribute("type", "range test");
    stream.writeAttribute("num_ranges", QString::number(numRanges));

    foreach (DataAnchor *a, anchors) {
        stream.writeStartElement("anchor");
        stream.writeAttribute("addr",  QString::number(a->id(), 16));
        stream.writeEndElement();

        a->setPosition(true);
    }

    stream.writeEndElement();

    stream.writeEndDocument();

    if(!_autoPosition)
        emit rangingProgress(tr("Requested Range Test"));
}

void RTLSControl::range(const DataAnchor *initiator, const DataAnchor *responder)
{
    Q_UNUSED(initiator)
    Q_UNUSED(responder)

    qDebug() << "Ranging test" ;

    _rxRangingResult = 0;
}

void RTLSControl::powerTest(DataAnchor *anchor)
{
    QList<DataAnchor *> l;
    l.append(anchor);
    powerTest(l);
}

void RTLSControl::powerTest(const QList<DataAnchor *> &anchors)
{
    QIODevice *device;
    if (!connection || !(device = connection->controlDevice()))
        return;

    if (anchors.size() != 1)
    {
        qDebug() << "only one anchor can be selected for power test! " << anchors.size() ;
        return;
    }

    QXmlStreamWriter stream(device);
    stream.writeStartDocument();

    stream.writeStartElement("req");
    stream.writeAttribute("type", "power test");

    foreach (DataAnchor *tx, anchors) {
        stream.writeStartElement("anchor");
        stream.writeAttribute("addr",  QString::number(tx->id(), 16));
        stream.writeEndElement();
    }

    stream.writeEndElement();

    stream.writeEndDocument();

    emit commsTestProgress(tr("Requested Power Test"));
}

void RTLSControl::communicationTest(DataAnchor *transmitter)
{
    QList<DataAnchor *> l;
    l.append(transmitter);
    communicationTest(l);
}

void RTLSControl::communicationTest(const QList<DataAnchor *> &transmitters)
{
    /*if (transmitters.size() == 1)
    {
        qDebug() << "only one anchor selected - can't do comms test! " << transmitters.size() ;
        return;
    }*/

    QIODevice *device;
    if (!connection || !(device = connection->controlDevice()))
        return;

    QXmlStreamWriter stream(device);
    stream.writeStartDocument();

    stream.writeStartElement("req");
    stream.writeAttribute("type", "comm test");
    stream.writeAttribute("anchors", QString::number(transmitters.size()));

    foreach (DataAnchor *tx, transmitters) {
        stream.writeStartElement("anchor");
        stream.writeAttribute("addr",  QString::number(tx->id(), 16));
        stream.writeEndElement();
    }

    stream.writeEndElement();

    stream.writeEndDocument();

    emit commsTestProgress(tr("Requested Communication Test"));
}

/*
Filter Types
0 - No Filtering
1 - Moving Average
2 - Moving Average excluding max and min
3 - Kalman Filter
*/
//<req type="motion filter" filter_type="1" filter_length="10"></req>
void RTLSControl::sendMotionFilterCmd(int filter)
{
    QIODevice *device;
    if (!connection || !(device = connection->controlDevice()))
        return;

    QXmlStreamWriter stream(device);
    stream.writeStartDocument();

    stream.writeStartElement("req");
    stream.writeAttribute("type", "motion filter");

    stream.writeAttribute("filter_type",  QString::number(filter, 10));

    stream.writeAttribute("filter_length",  QString::number(10, 10));

    stream.writeEndElement();

    stream.writeEndDocument();
}

/*
Logging Types
0 - No Logging
1 - Level 1
2 - Level 2
3 - Diagnostics
4 - Log Diagnostics + ALL CS_RX
*/
//<req type=“log start” loglevel=3></req >
//<req type=“log stop”></req>

void RTLSControl::sendLoggingCmd(int level)
{
    QIODevice *device;
    if (!connection || !(device = connection->controlDevice()))
        return;

    QXmlStreamWriter stream(device);
    stream.writeStartDocument();

    stream.writeStartElement("req");

    if(level > 0)
    {
        stream.writeAttribute("type", "log start");

        stream.writeAttribute("loglevel",  QString::number(level, 10));
    }
    else
    {
        stream.writeAttribute("type", "log stop");
    }

    stream.writeEndElement();
    stream.writeEndDocument();
}

QStringList RTLSControl::getMotionFilter(void)
{
    return motionFilterTypes;
}

QStringList RTLSControl::getLogging(void)
{
    return loggingTypes;
}

void RTLSControl::sendAddTextToLog(QString string)
{
    QIODevice *device;
    if (!connection || !(device = connection->controlDevice()))
        return;

    QXmlStreamWriter stream(device);
    stream.writeStartDocument();

    stream.writeStartElement("req");
    stream.writeAttribute("type", "log text");

    stream.writeAttribute("text",  string);

    stream.writeEndElement();
    stream.writeEndDocument();
}


void RTLSControl::sendResetCmd(void)
{
    const QList<DataAnchor *> &anchors = RTLSControllerApplication::model()->anchors();
    //first check if at least one master anchor (for REK there should only be one)
    if (!anchors.size())
        return;

    QIODevice *device;

    //if no connection to CLE return
    if (!connection || !(device = connection->controlDevice()))
        return;

    emit rtlsStateChanged(RTLSCLEConnection::Stopped);
    updateRTLSState(0);

    QXmlStreamWriter stream(device);
    stream.writeStartDocument();
    stream.writeStartElement("req");
    stream.writeAttribute("type", "rtls anc_reset");
    stream.writeEndElement();
    stream.writeEndDocument();
}

void RTLSControl::sendStartStopCmd(void)
{
    if(_running)
    {
        stop();
        _running = 0;
    }
    else
    {
        emit toSwitchonTag();
        QMessageBox::StandardButton reply ;
        int master = 0;

        const QList<DataAnchor *> &anchors = RTLSControllerApplication::model()->anchors();
        //first check if at least one master anchor (for REK there should only be one)
        if (!anchors.size())
            return;

        foreach(DataAnchor *a, anchors)
        {
            if(a->selected()) //if this anchor has been selected to be used in this RTLS/this CLE
            {
                if(a->master() > 0)
                    master++;
            }
        }
#if (REK_PRODUCT == 1)
        if(master != 1)
        {
            reply = QMessageBox::critical(nullptr, tr("Error"), "One anchor needs to be set as a master.");
        }
#else
        if((master < 1) && (_wirelessSync == true)) //for wired sync master is not needed
        {
            reply = QMessageBox::critical(nullptr, tr("Error"), "At least one anchor needs to be set as a master.");
        }
#endif
        else
        {
            reply = QMessageBox::No;
            start();
            //clear the screen - remove any tags
            emit clearTags();
            _running = 1;
            reply = QMessageBox::Cancel;
        }

        if(reply == QMessageBox::Ok)
        {
            emit rtlsStateChanged(RTLSCLEConnection::Stopped);
        }

    }
}

//<req type=”rf cfg”>
//<rf chan = “5” prf=“64” rate=“850” code=“3” plen=“1024” pac=“32” nsfd=“0” ant_delay=“16454”/>
//</req>
void RTLSControl::sendRFConfigurationClick(void)
{
    sendRFConfiguration(true);
}

void RTLSControl::sendRFConfiguration(bool userFeedback)
{
    QIODevice *device;
    if (!connection || !(device = connection->controlDevice()))
        return;

    QXmlStreamWriter stream(device);
    stream.writeStartDocument();

    stream.writeStartElement("req");
    stream.writeAttribute("type", "rf cfg");

    stream.writeStartElement("rf");
    stream.writeAttribute("chan",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentChannel(), 10));
    stream.writeAttribute("prf",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentPRFN(), 10));
    stream.writeAttribute("rate",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentDataRateN(), 10));
    stream.writeAttribute("code",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentPreambleCodeN(), 10));
    stream.writeAttribute("plen",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentPreambleLengthN(), 10));
    stream.writeAttribute("pac",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentPACN(), 10));
    stream.writeAttribute("nsfd",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentNSFD(), 10));
    //don't send antenna delay as it is configured with anchor config command
    //stream.writeAttribute("ant_delay",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentAntennaDly(), 10));

    stream.writeEndElement();

    stream.writeEndElement();
    stream.writeEndDocument();

    _userFeedback = userFeedback;
}

//<req type=“CLE cfg”></req>
void RTLSControl::sendCLECfgReq(void)
{
    QIODevice *device;
    if (!connection || !(device = connection->controlDevice()))
        return;

    QXmlStreamWriter stream(device);
    stream.writeStartDocument();

    stream.writeStartElement("req");
    stream.writeAttribute("type", "cle cfg");
    stream.writeEndElement();
    stream.writeEndDocument();
}

//<req type=“anchor list”></req>
void RTLSControl::sendAnchorListReq(void)
{
    QIODevice *device;
    if (!connection || !(device = connection->controlDevice()))
        return;

    QXmlStreamWriter stream(device);
    stream.writeStartDocument();

    stream.writeStartElement("req");
    stream.writeAttribute("type", "anchor list");
    stream.writeEndElement();
    stream.writeEndDocument();
}

void RTLSControl::sendConfiguration()
{
    sendConfiguration(RTLSControllerApplication::model()->anchors());
}

void RTLSControl::sendConfiguration(DataAnchor *anchor)
{
    QList<DataAnchor *> l;
    l.append(anchor);
    sendConfiguration(l);
}

//第二轮，返回user想使用anchors
void RTLSControl::sendConfiguration(const QList<DataAnchor *> &anchors)
{
    if (!anchors.size())
        return;

    QIODevice *device;
    if (!connection || !(device = connection->controlDevice()))
    {
        QMessageBox::critical(nullptr, tr("Error"), "No connection to CLE.");
        return;
    }
    QXmlStreamWriter stream(device);
    stream.writeStartDocument();

    stream.writeStartElement("req");
    stream.writeAttribute("type", "anchor cfg");
    //stream.writeAttribute("anchors", QString::number(anchors.size()));

    foreach(DataAnchor *a, anchors)
    {
        double antDelayRx,antDelayTx;
        int delayRx,delayTx;
        antDelayRx = (a->antennaDlyRx()*1e-9)/15.65e-12; delayRx = antDelayRx;
        antDelayTx = (a->antennaDlyTx()*1e-9)/15.65e-12; delayTx = antDelayTx;

        if(a->selected()) //if this anchor has been selected to be used in this RTLS/this CLE
        {
            quint64 id = a->id();
            bool master = (a->master() > 0)? true : false;

            stream.writeStartElement("anchor");
            stream.writeAttribute("addr", QString::number(id, 16));
            stream.writeAttribute("id", QString::number(a->number()));
            stream.writeAttribute("x", QString::number(a->x()));
            stream.writeAttribute("y", QString::number(a->y()));
            stream.writeAttribute("z", QString::number(a->z()));
            stream.writeAttribute("master", master ? "1" : "0");
            stream.writeAttribute("master_addr", master ? QString::number(a->masterId(), 16) : "0"); //master_id in the XML config
            stream.writeAttribute("master_lag_delay", master ? QString::number(a->lagDelayUs(), 10) : "0");
            stream.writeAttribute("ant_delay_rx", QString::number(delayRx,10));
            stream.writeAttribute("ant_delay_tx", QString::number(delayTx,10));

            for(int m=0; m<a->masterAnchorListSize(); m++)
            {
                //get the link if it exists...
                uint64_t masterID = a->getMasterAnchorAddrAt(m);
                DataAnchor *master = RTLSControllerApplication::model()->anchor(masterID);
                DataUndirectedLink *undirectedLink = nullptr;
                double rfdist = 0;

                // Get the undirected link, and then get the receiver side of the link.
                // Add a new undirected link if it does not exist already (Anchor A, Anchor B)
                if (master)
                {
                    undirectedLink = RTLSControllerApplication::model()->link(master, a, false);
                }
                if (undirectedLink)
                {
                    if(undirectedLink->rfUseDistance()==1)
                    {
                        rfdist = undirectedLink->rfDistance();
                    }
                }
                stream.writeStartElement("masteranchor");
                stream.writeAttribute("addr", QString::number(a->getMasterAnchorAddrAt(m), 16));
                stream.writeAttribute("rfdistance", QString::number(rfdist));
                stream.writeEndElement();
            }

            stream.writeEndElement();
        }
        else
        {
            //hide it from the anchor list widget
            quint64 id = a->id();
            int idx = RTLSControllerApplication::model()->anchorIdx(id);
            a->setConnected(NotConnected);
            RTLSControllerApplication::anchorListWidget()->updateAnchorConnected(idx, false);
            qDebug() << "send CFG HIDE " << QString::number(id, 16);
        }
    }

    stream.writeEndElement();

    stream.writeEndDocument();
}

void RTLSControl::reSyncAnchors()
{
    reSyncAnchors(RTLSControllerApplication::model()->anchors());
}

void RTLSControl::reSyncAnchors(const QList<DataAnchor *> &anchors)
{
    if (!anchors.size())
        return;

    QIODevice *device;
    if (!connection || !(device = connection->controlDevice()))
    {
        QMessageBox::critical(nullptr, tr("Error"), "No connection to CLE.");
        return;
    }
    QXmlStreamWriter stream(device);
    stream.writeStartDocument();

    stream.writeStartElement("req");
    stream.writeAttribute("type", "tag training");

    foreach(DataAnchor *a, anchors)
    {
        if(a->selected() && a->referencePairsListSize()) //if this anchor has been selected to be used in this RTLS/this CLE
        {                                                //and if it has reference pairs
            quint64 id = a->id();

            stream.writeStartElement("anchor");
            stream.writeAttribute("addr", QString::number(id, 16));

            //add each reference anchor - tag pair
            for(int m=0; m<a->referencePairsListSize(); m++)
            {
                ReferencePair rp = a->getReferencePairAt(m);

                stream.writeStartElement("reference");
                stream.writeAttribute("blinks", QString::number(rp.blinks));
                stream.writeAttribute("offset", QString::number(rp.offset));
                stream.writeAttribute("tag", QString::number(rp.tagid, 16));
                stream.writeAttribute("x", QString::number(rp.tagx));
                stream.writeAttribute("y", QString::number(rp.tagy));
                stream.writeAttribute("z", QString::number(rp.tagz));
                stream.writeAttribute("addr", QString::number(rp.anchorid, 16));
                stream.writeEndElement();
            }
            stream.writeEndElement();
        }

    }
    stream.writeEndElement();

    stream.writeEndDocument();
}

void RTLSControl::stop()
{
    QIODevice *device;

    //if no connection to CLE return
    if (!connection || !(device = connection->controlDevice()))
        return;

    QXmlStreamWriter stream(device);
    stream.writeStartDocument();
    stream.writeStartElement("req");
    stream.writeAttribute("type", "rtls stop");
    stream.writeEndElement();
    stream.writeEndDocument();
}

void RTLSControl::start()
{
    QIODevice *device;

    //if no connection to CLE return
    if (!connection || !(device = connection->controlDevice()))
        return;

    QXmlStreamWriter stream(device);
    stream.writeStartDocument();
    stream.writeStartElement("req");
    stream.writeAttribute("type", "rtls start");
    stream.writeEndElement();
    stream.writeEndDocument();
}

/*
Logging - Enable/Disable Diagnostics from Controller

diagnostics=0 for UWB Backhaul.

<req type="log" diagnostics="1"></req>
*/

void RTLSControl::logging(int diagnostics)
{
    QIODevice *device;

    //if no connection to CLE return
    if (!connection || !(device = connection->controlDevice()))
        return;

    QXmlStreamWriter stream(device);
    stream.writeStartDocument();

    stream.writeStartElement("req");
    stream.writeAttribute("type", "log");
    stream.writeAttribute("dignostics", QString::number(diagnostics)); //on/off
    stream.writeEndElement();
    stream.writeEndDocument();
}
