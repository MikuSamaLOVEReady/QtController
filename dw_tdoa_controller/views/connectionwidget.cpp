// -------------------------------------------------------------------------------------------------------------------
//
//  File: ConnectionWidget.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "connectionwidget.h"
#include "ui_connectionwidget.h"

#include "RTLSControllerApplication.h"
#include "RTLSControl.h"
#include "RTLSClient.h"
#include "mainwindow.h"

#include <QInputDialog>
#include <QSettings>
#include <QMessageBox>

ConnectionWidget::ConnectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionWidget),
    _diagnostics(false),
    _hostName("localhost"), //set default values
    _control(3334),
    _data(3335)
{
    ui->setupUi(this);

    QObject::connect(ui->connect_pb, SIGNAL(clicked()), SLOT(connectButtonClicked()));  //add a function for the Connect button click
    QObject::connect(ui->runstop_pb, SIGNAL(clicked()), SLOT(runstopButtonClicked()));  //add a function for the Start/Stop button click
    QObject::connect(ui->addToCLELog, SIGNAL(clicked()), SLOT(addToCLELogButtonClicked()));
    QObject::connect(ui->resetanc_pb, SIGNAL(clicked()), SLOT(resetButtonClicked()));  //add a function for the ResetAnchors button click

    //QObject::connect(ui->diagnostics, SIGNAL(clicked()), SLOT(diagnosticsClicked()));

    QObject::connect(RTLSControllerApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(saveSettings()));

    RTLSControllerApplication::connectReady(this, "onReady()");

    ui->cleConfig->hide();
}

void ConnectionWidget::onReady()
{
    QObject::connect(RTLSControllerApplication::cleConnection(), SIGNAL(connectionStateChanged(RTLSCLEConnection::ConnectionState)),
                     this, SLOT(connectionStateChanged(RTLSCLEConnection::ConnectionState)));

    QObject::connect(RTLSControllerApplication::control(), SIGNAL(rtlsStateChanged(RTLSCLEConnection::RTLSState)),
                     this, SLOT(rtlsStateChanged(RTLSCLEConnection::RTLSState)));

    QObject::connect(RTLSControllerApplication::mainWindow(), SIGNAL(configureCLEConnection()),
                     this, SLOT(configureCLEConnection()));

    connectionStateChanged(RTLSCLEConnection::Disconnected);

    rtlsStateChanged(RTLSCLEConnection::Unknown);

    //loadSettings();

    ui->runstop_pb->setEnabled(false);

    ui->motionFilter->addItems(RTLSControllerApplication::control()->getMotionFilter());
    QObject::connect(ui->motionFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMotionFiler(int)));

    ui->logging->addItems(RTLSControllerApplication::control()->getLogging());

    //ui->logging->setCurrentIndex(3); //Logging (with diagnostics) is on by default
    ui->logging->setCurrentIndex(0); //No Logging is on by default
    QObject::connect(ui->logging, SIGNAL(currentIndexChanged(int)), this, SLOT(updateLogging(int)));

    QObject::connect(RTLSControllerApplication::control(), SIGNAL(setLogging(int)), this, SLOT(setLogging(int)));
    QObject::connect(RTLSControllerApplication::control(), SIGNAL(setFilter(int)), this, SLOT(setFilter(int)));
    //set default values and
    //connect to CLE on start up
    {
        ui->hostname_le->setText(_hostName);
        ui->control_sb->setValue(_control);
        ui->data_sb->setValue(_data);
        RTLSControllerApplication::cleConnection()->connectToCLE(_hostName, _control, _data);
    }

    QObject::connect(RTLSControllerApplication::control(), SIGNAL(toSwitchonTag()), this, SLOT(toSwitchonTag()));

}

ConnectionWidget::~ConnectionWidget()
{
    delete ui;
}

void ConnectionWidget::updateMotionFiler(int index)
{
     RTLSControllerApplication::control()->sendMotionFilterCmd(index);
}

void ConnectionWidget::setLogging(int index)
{
    ui->logging->setCurrentIndex(index);
}

void ConnectionWidget::setFilter(int index)
{
    ui->motionFilter->setCurrentIndex(index);
}

void ConnectionWidget::updateLogging(int index)
{
     RTLSControllerApplication::control()->sendLoggingCmd(index);
}

void ConnectionWidget::diagnosticsClicked()
{
    //_diagnostics = ui->diagnostics->isChecked();
}

void ConnectionWidget::addToCLELogButtonClicked()
{
    //allow user to input some text and send the string to the CLE
    bool ok;

    //do{
        QString idStr = QInputDialog::getText(nullptr, "CLE Log Text", "", QLineEdit::Normal, "", &ok); //

        if (ok)
        {
            RTLSControllerApplication::control()->sendAddTextToLog(idStr);
        }
        //else //cancel was pressed
        //{
            //break;
        //}
    //}
    //while(1);
}

void ConnectionWidget::resetButtonClicked()
{
    //ui->resetanc_pb->setEnabled(false);   //disable the button after the press and wait for response
    ui->resetanc_pb->setText("Reset in progress");

    RTLSControllerApplication::control()->sendResetCmd();
}


void ConnectionWidget::runstopButtonClicked()
{
    //send the start/stop command to the CLE
    //start == starts the RTLS running
    //stop == stops the RTLS running

    //disable the button after the press and wait for response
    ui->runstop_pb->setEnabled(false);
    ui->runstop_pb->setText("----");

    RTLSControllerApplication::control()->sendStartStopCmd();
}

void ConnectionWidget::connectButtonClicked()
{
    switch (_state)
    {
    case RTLSCLEConnection::Disconnected:
    case RTLSCLEConnection::ConnectionFailed:
    {
        _hostName = ui->hostname_le->text();
        _control = ui->control_sb->value();
        _data = ui->data_sb->value();
        RTLSControllerApplication::cleConnection()->connectToCLE(_hostName, _control, _data);
        break;
    }

    case RTLSCLEConnection::Connecting:
        RTLSControllerApplication::cleConnection()->cancelConnection();
        break;

    case RTLSCLEConnection::Connected:
        RTLSControllerApplication::cleConnection()->closeConnection();
        break;
    }
}

void ConnectionWidget::connectionStateChanged(RTLSCLEConnection::ConnectionState state)
{
    this->_state = state;
    switch(state)
    {
    case RTLSCLEConnection::Disconnected:
    case RTLSCLEConnection::ConnectionFailed:
        ui->connect_pb->setText("Connect");
        ui->runstop_pb->setEnabled(false);
        ui->motionFilter->setEnabled(false);
        ui->logging->setEnabled(false);
        ui->resetanc_pb->setEnabled(false);
        if(RTLSCLEConnection::ConnectionFailed == state)
        {
            //the connection has failed - let the user know
            QMessageBox::warning(nullptr, tr("Error"), "CLE connection has failed, please check the CLE settings.");
            configureCLEConnection();
        }
        break;

    case RTLSCLEConnection::Connecting:
        ui->connect_pb->setText("Cancel");
        break;

    case RTLSCLEConnection::Connected:
        ui->connect_pb->setText("Disconnect");
        ui->runstop_pb->setEnabled(true);
        ui->resetanc_pb->setText("Reset Anchors");
        ui->resetanc_pb->setEnabled(true);
        ui->motionFilter->setEnabled(true);
        ui->logging->setEnabled(true);
        ui->cleConfig->hide();
        //get CLE configuration
        RTLSControllerApplication::control()->sendCLECfgReq();
        //update the anchor list
        RTLSControllerApplication::control()->sendAnchorListReq();
        break;
    default:
        break;
    }

    bool enabled = (state == RTLSCLEConnection::Disconnected || state == RTLSCLEConnection::ConnectionFailed) ? true : false;
    //can change the CLE parameters once the CLE is disconnected
    ui->hostname_le->setEnabled(enabled);
    ui->control_sb->setEnabled(enabled);
    ui->data_sb->setEnabled(enabled);
}

void ConnectionWidget::rtlsStateChanged(RTLSCLEConnection::RTLSState state)
{
    this->_rtlsstate = state;
    switch(state)
    {
        case RTLSCLEConnection::Unknown:
            ui->runstop_pb->setText("Start RTLS");
            ui->runstop_pb->setEnabled(false);
            break;

        case RTLSCLEConnection::Running:
            ui->runstop_pb->setText("Stop RTLS");
            ui->runstop_pb->setEnabled(true);
            break;

        case RTLSCLEConnection::Stopped:
            ui->runstop_pb->setText("Start RTLS");
            ui->runstop_pb->setEnabled(true);
            break;
    }
}

void ConnectionWidget::loadSettings()
{
    QSettings s;

    qDebug() << "Load saved/default configuration" ;

    s.beginGroup("ConnectionWidget");
    //get saved (edited) settings, use defaults if they don't exist
    ui->hostname_le->setText(s.value("hostname", "127.0.0.1").toString());
    ui->control_sb->setValue(s.value("controlPort", 3334).toInt());
    ui->data_sb->setValue(s.value("dataPort", 3335).toInt());
    s.endGroup();
}

void ConnectionWidget::saveSettings()
{
    QSettings s;
    s.beginGroup("ConnectionWidget");
    //save (edited) settings
    //This information is often stored in the system registry on Windows, and in XML preferences files on Mac OS X.
    s.setValue("hostname", ui->hostname_le->text());
    s.setValue("controlPort", ui->control_sb->value());
    s.setValue("dataPort", ui->data_sb->value());
    s.endGroup();
}

//open CLE connection settings widget
void ConnectionWidget::configureCLEConnection(void)
{
    ui->cleConfig->show();
    ui->cleConfig->setEnabled(true);
    ui->hostname_le->setEnabled(true);
    ui->control_sb->setEnabled(true);
    ui->data_sb->setEnabled(true);
}

void ConnectionWidget::toSwitchonTag()
{
    QMessageBox::warning(nullptr, tr("RTLS is Running"), "Switch on the tags!");

}




