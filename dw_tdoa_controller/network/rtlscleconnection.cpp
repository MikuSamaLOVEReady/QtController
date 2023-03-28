// -------------------------------------------------------------------------------------------------------------------
//
//  File: RTLSCLEConnection.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "RTLSCLEConnection.h"

#include <QDebug>
#include <QTcpSocket>
#include <QHostAddress>
#include <QXmlStreamWriter>

RTLSCLEConnection::RTLSCLEConnection(QObject *parent) :
    QObject(parent)
{
    controlSocket = new QTcpSocket(this);
    dataSocket = new QTcpSocket(this);

    QObject::connect(controlSocket, SIGNAL(connected()), SLOT(controlConnected()));
    QObject::connect(controlSocket, SIGNAL(disconnected()), SLOT(controlDisconnected()));
    QObject::connect(controlSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(controlError(QAbstractSocket::SocketError)));
    QObject::connect(controlSocket, SIGNAL(readyRead()), this, SLOT(controlReadyRead()));

    QObject::connect(dataSocket, SIGNAL(connected()), SLOT(dataConnected()));
    QObject::connect(dataSocket, SIGNAL(disconnected()), SLOT(dataDisconnected()));
    QObject::connect(dataSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(dataError(QAbstractSocket::SocketError)));
    //QObject::connect(dataSocket, SIGNAL(readyRead()), this, SLOT(dataReadyRead()));
}

QIODevice *RTLSCLEConnection::controlDevice()
{
    if (controlSocket->state() == QAbstractSocket::ConnectedState)
        return controlSocket;
    else
        return nullptr;
}

QIODevice *RTLSCLEConnection::dataDevice()
{
    if (dataSocket->state() == QAbstractSocket::ConnectedState)
        return dataSocket;
    else
        return nullptr;
}

QTcpSocket *RTLSCLEConnection::dataSocketPtr()
{
    if (dataSocket->state() == QAbstractSocket::ConnectedState)
        return dataSocket;
    else
        return nullptr;
}

void RTLSCLEConnection::connectToCLE(const QString &hostname, quint16 controlPort, quint16 dataPort)
{
    controlSocket->abort();
    dataSocket->abort();

    qDebug() << "Connecting :" << hostname << controlPort << dataPort;
    controlSocket->connectToHost(hostname, controlPort); // 这是在不断监听有没有CLE传输的 XML数据流 ？？？？？
    dataSocket->connectToHost(hostname, dataPort); // TODO: Debug USE

    emit connectionStateChanged(RTLSCLEConnection::Connecting);
}
void RTLSCLEConnection::closeConnection()
{
    controlSocket->close();
    dataSocket->close();
}
void RTLSCLEConnection::cancelConnection()
{
    controlSocket->abort();
    dataSocket->abort();
    emit connectionStateChanged(ConnectionFailed);
}

void RTLSCLEConnection::controlConnected()
{
    emit connectionStateChanged(Connected);
}
void RTLSCLEConnection::controlDisconnected()
{
    emit connectionStateChanged(Disconnected);
}
void RTLSCLEConnection::controlError(QAbstractSocket::SocketError e)
{
    Q_UNUSED(e)
    emit connectionStateChanged(ConnectionFailed);
}

void RTLSCLEConnection::controlReadyRead()
{
    while (controlSocket->canReadLine())
    {
        QByteArray line = controlSocket->readLine();
        emit messageReceived(line);
    }
}

void RTLSCLEConnection::dataReadyRead()
{
    while (dataSocket->canReadLine())
    {
        QByteArray line = dataSocket->readLine();
        emit dataMessageReceived(line);
    }
}

void RTLSCLEConnection::dataConnected()
{
    qDebug() << "data socket connected" ;

    emit dataSocketConnected();
}

void RTLSCLEConnection::dataDisconnected() { }
void RTLSCLEConnection::dataError(QAbstractSocket::SocketError e) { Q_UNUSED(e) }



