// -------------------------------------------------------------------------------------------------------------------
//
//  File: ConnectionWidget.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>
#include "RTLSCLEConnection.h"


namespace Ui {
class ConnectionWidget;
}

class ConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionWidget(QWidget *parent = nullptr);
    ~ConnectionWidget();

public slots:
    void connectionStateChanged(RTLSCLEConnection::ConnectionState state);
    void rtlsStateChanged(RTLSCLEConnection::RTLSState state);
    void updateMotionFiler(int index);
    void updateLogging(int index);
    void setLogging(int index);
    void setFilter(int index);
    void configureCLEConnection(void);

    void toSwitchonTag(void);

protected slots:
    void onReady();
    void connectButtonClicked();
    void runstopButtonClicked();
    void addToCLELogButtonClicked();
    void diagnosticsClicked();
    void loadSettings();
    void saveSettings();
    void resetButtonClicked();

private:
    RTLSCLEConnection::ConnectionState _state;
    RTLSCLEConnection::RTLSState _rtlsstate;
    Ui::ConnectionWidget *ui;
    bool _diagnostics;
    QString _hostName;
    quint16 _control;
    quint16 _data;
};

#endif // CONNECTIONWIDGET_H
