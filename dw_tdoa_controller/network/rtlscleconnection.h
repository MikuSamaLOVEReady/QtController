// -------------------------------------------------------------------------------------------------------------------
//
//  File: RTLSCLEConnection.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef RTLSCLECONNECTION_H
#define RTLSCLECONNECTION_H

#include <QObject>
#include <QAbstractSocket>

class QTcpSocket;
class QIODevice;

class RTLSCLEConnection : public QObject
{
    Q_OBJECT
public:
    explicit RTLSCLEConnection(QObject *parent = 0);

    enum ConnectionState
    {
        Disconnected = 0,
        Connecting,
        Connected,
        ConnectionFailed
    };

    enum RTLSState
    {
        Unknown = 0,
        Running,
        Stopped
    };

    QIODevice *controlDevice();
    QIODevice *dataDevice();
    QTcpSocket *dataSocketPtr();
signals:
    void connectionStateChanged(RTLSCLEConnection::ConnectionState);
    void messageReceived(const QByteArray &data); //由connection 发现并告知control
    void dataMessageReceived(const QByteArray &data);
    void dataSocketConnected(void);

public slots:
    void connectToCLE(const QString &hostname, quint16 controlPort, quint16 dataPort);
    void closeConnection();
    void cancelConnection();

protected slots:
    //control port slots
    void controlConnected();
    void controlDisconnected();
    void controlError(QAbstractSocket::SocketError);
    void controlReadyRead();
    //data port slots
    void dataReadyRead();
    void dataConnected();
    void dataDisconnected();
    void dataError(QAbstractSocket::SocketError);


private:
    QTcpSocket *controlSocket;
    QTcpSocket *dataSocket;
};

#endif // RTLSCLECONNECTION_H
