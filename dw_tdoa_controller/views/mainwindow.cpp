// -------------------------------------------------------------------------------------------------------------------
//
//  File: MainWindow.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "connectionwidget.h"
#include "rtlscleconnection.h"
#include "DataModel.h"
#include "DataAnchor.h"
#include "RTLSControllerApplication.h"
#include "RTLSControl.h"
#include "RTLSClient.h"
#include "ViewSettings.h"

#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#include <QShortcut>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>
#include <QDomDocument>
#include <QFile>
#include <QXmlStreamWriter>
#include <QFileDialog>
#include <QMenu>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QStyleFactory>
#include <QWindow>

//autopositioning feature limited to 4 anchors
#define AUTOPOS_ENABLE (0)

#define GUI_VERSION "version 2.1"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    waitForData = false;
    _RTLSCLEConnection = 0;

    const QStringList styles = QStyleFactory::keys();
    for(QString s : styles)
    {
        if(s.contains("Fusion"))
        {
            QApplication::setStyle("Fusion");
        }
    }

    ui->setupUi(this);

    {
        QWidget *empty = new QWidget(this);
        empty->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
        ui->mainToolBar->addWidget(empty);
    }

    createPopupMenu(ui->viewMenu);

    //add connection widget to the main window
    ConnectionWidget *cWidget = new ConnectionWidget(this);
    ui->mainToolBar->addWidget(cWidget);

    new QShortcut(QKeySequence::SelectAll, this, SLOT(onSelectAll()));

    QObject::connect(RTLSControllerApplication::instance(), SIGNAL(aboutToQuit()), SLOT(saveSettings()));

    // Help Menu=
    ui->menuHelp->addAction(ui->actionAbout);
    connect(ui->actionAbout, SIGNAL(triggered()), SLOT(onAboutAction()));


    //config = new ChannelSettingsWidget(this);
#if (AUTOPOS_ENABLE)
    QAction *autoposition = new QAction("Auto position",this); autoposition->setData(AutoPosition);
    QAction *position = new QAction("Position",this); position->setData(Position);
#endif
    QAction *rotate90 = new QAction("Rotate 90",this); rotate90->setData(Rotate90);
    QAction *flipX = new QAction("Flip X-axis",this); flipX->setData(FlipX);
    QAction *flipY = new QAction("Flip Y-axis",this); flipY->setData(FlipY);

    positionAnchorsSubMenu = new QMenu("Anchor Positioning");
#if (AUTOPOS_ENABLE)
    positionAnchorsSubMenu->addAction(autoposition);
    positionAnchorsSubMenu->addAction(position);
#endif
    positionAnchorsSubMenu->addAction(rotate90);
    positionAnchorsSubMenu->addAction(flipX);
    positionAnchorsSubMenu->addAction(flipY);

    positionAnchors = ui->menuConfig->addMenu(positionAnchorsSubMenu);
    ui->menuConfig->addAction(positionAnchors);
    connect(positionAnchorsSubMenu, SIGNAL(triggered(QAction*)), this, SLOT(onAccumPositionAction(QAction*)));

    chanConfAction = new QAction(tr("&Channel Configuration"), this);
    ui->menuConfig->addAction(chanConfAction);
    connect(chanConfAction, SIGNAL(triggered()), SLOT(onChanConfAction()));

    ui->channel_dw->setWindowTitle("Channel Configuration");
    ui->channel_dw->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->channel_dw->setAcceptDrops(false);
    ui->channel_dw->setFloating(true);

//    calibTestAction = new QAction(tr("&Antenna Calibration"), this);
//    ui->menuConfig->addAction(calibTestAction);
//    connect(calibTestAction, SIGNAL(triggered()), SLOT(onCalibTestAction()));

    reSyncAction = new QAction(tr("&Re Sync"), this);
    ui->menuConfig->addAction(reSyncAction);
    connect(reSyncAction, SIGNAL(triggered()), SLOT(onReSyncAction()));

    ui->menuConfig->addSeparator();

    cleConfigAction = new QAction(tr("&CLE Configuration"), this);
    ui->menuConfig->addAction(cleConfigAction);
    connect(cleConfigAction, SIGNAL(triggered()), SLOT(onCLEConfigAction()));

    ui->menuConfig->addSeparator();

    loadConfig = new QAction(tr("&Load Configuration"), this);
    ui->menuConfig->addAction(loadConfig);
    connect(loadConfig, SIGNAL(triggered()), SLOT(onLoadConfig()));

    saveAsConfig = new QAction(tr("&Save Configuration"), this);
    ui->menuConfig->addAction(saveAsConfig);
    connect(saveAsConfig, SIGNAL(triggered()), SLOT(onSaveAsConfig()));

    saveConfig = new QAction(tr("&Save As New Configuration"), this);
    ui->menuConfig->addAction(saveConfig);
    connect(saveConfig, SIGNAL(triggered()), SLOT(onSaveConfig()));

    infoLabel = new QLabel(parent);

    serial = new QSerialPort(this);

    //center on the screen.
    QDesktopWidget desk;
    QRect screenres = desk.screenGeometry(0); //obsolete
    ui->channel_dw->setGeometry(QRect(screenres.width()/2,screenres.height()/2,screenres.width()/2,screenres.height()/2));

    ui->channel_dw->close();
    ui->inspector_dw->close();
    ui->viewSettings_dw->close();
    ui->minimap_dw->close();

    connect(ui->minimap_dw->toggleViewAction(), SIGNAL(triggered()), SLOT(onMiniMapView()));


    RTLSControllerApplication::connectReady(this, "onReady()");
}

void MainWindow::onReady()
{
    QObject::connect(RTLSControllerApplication::cleConnection(), SIGNAL(connectionStateChanged(RTLSCLEConnection::ConnectionState)),
                     this, SLOT(connectionStateChanged(RTLSCLEConnection::ConnectionState)));
#if (AUTOPOS_ENABLE)
    QObject::connect(this, SIGNAL(locateAnchors()),
                     RTLSControllerApplication::instance(), SLOT(autoPositionAnchors()));

    QObject::connect(this, SIGNAL(repositionAnchors()),
                     RTLSControllerApplication::instance(), SLOT(positionAnchorsFromRange()));
#endif
    QObject::connect(this, SIGNAL(rotate90Anchors()),
                     RTLSControllerApplication::instance(), SLOT(rotate90Anchors()));

    QObject::connect(this, SIGNAL(flipXorYAnchors(int)),
                     RTLSControllerApplication::instance(), SLOT(flipXYAnchors(int)));

//    QObject::connect(this, SIGNAL(calibrateAntennas()),
//                     RTLSControllerApplication::control(), SLOT(calibrateAntennas()));

    //loadSettings(); - move this to be done once the CLE is connected as the configuration will be set to it

    connect(RTLSControllerApplication::channelSettings(), SIGNAL(closeConfigWidget()), this, SLOT(onCloseConfigWidget()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

QStringList MainWindow::portsList()
{
    return ports;
}

ChannelSettingsWidget *MainWindow::channelSettings()
{
    return ui->channel_settings;
}

GraphicsWidget *MainWindow::graphicsWidget()
{
    return ui->graphicsWidget;
}

AnchorListWidget *MainWindow::anchorListWidget()
{
    return ui->anchor_list;
}

QMenu *MainWindow::createPopupMenu()
{
    return createPopupMenu(new QMenu());
}

QMenu *MainWindow::createPopupMenu(QMenu *menu)
{
    menu->addAction(ui->anchorList_dw->toggleViewAction());
    menu->addAction(ui->anchorProps_dw->toggleViewAction());
    menu->addAction(ui->viewSettings_dw->toggleViewAction());
    menu->addAction(ui->minimap_dw->toggleViewAction());

    return menu;
}

void MainWindow::onMiniMapView()
{
    //check if we have loaded floorplan before we open mini map
    //if no floor plan close minimap
    QString path = RTLSControllerApplication::viewSettings()->getFloorplanPath();

    if(path == "") //no floorplan loaded
    {
        ui->minimap_dw->close();
        //qDebug() << "close minimap" ;
        QMessageBox::warning(nullptr, tr("Error"), "No floorplan loaded. Please upload floorplan to use mini-map.");
    }
}

void MainWindow::connectionStateChanged(RTLSCLEConnection::ConnectionState state)
{
    switch(state)
    {
        case RTLSCLEConnection::Connecting:
        {
            statusBar()->showMessage(QString("Connecting to CLE ..."));
            break;
        }
        case RTLSCLEConnection::Connected:
        {
            statusBar()->showMessage("Connection Successful.", 15);
            _RTLSCLEConnection = 1;
            loadSettings();
            break;
        }
        case RTLSCLEConnection::ConnectionFailed:
        {
            statusBar()->showMessage("Connection failed.");
            _RTLSCLEConnection = 0;
            loadSettings();
            break;
        }
        case RTLSCLEConnection::Disconnected:
        {
            statusBar()->clearMessage();
            break;
        }
    }

    if(state != RTLSCLEConnection::Connecting)
    {
        RTLSControllerApplication::client()->setGWReady(true);
    }

}

void MainWindow::onSelectAll()
{
    if (RTLSControllerApplication::model()->anchors().isEmpty())
        return;

    QModelIndex first = RTLSControllerApplication::model()->anchors().first()->index();
    QModelIndex last = RTLSControllerApplication::model()->anchors().last()->index();
    QItemSelection selection(first, last);

    RTLSControllerApplication::anchorSelectionModel()->select(selection, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
}

void MainWindow::loadSettings()
{
    QSettings s;
    s.beginGroup("MainWindow");
    QVariant state = s.value("window-state");
    if (state.convert(QVariant::ByteArray))
    {
        this->restoreState(state.toByteArray());
    }

    QVariant geometry = s.value("window-geometry");
    if (geometry.convert(QVariant::ByteArray))
    {
        this->restoreGeometry(geometry.toByteArray());
    }
    else
    {
        this->showMaximized();
    }
    s.endGroup();

    loadConfigFile("./CCview_config.xml");
    loadConfigFile("./CCchan_config.xml");
    RTLSControllerApplication::instance()->loadConfigFile("./CCanch_config.xml", _RTLSCLEConnection);
    graphicsWidget()->loadConfigFile("./CCtag_config.xml");
}

void MainWindow::saveSettings()
{
    QSettings s;
    s.beginGroup("MainWindow");
    s.setValue("window-geometry", saveGeometry());
    s.setValue("window-state",    saveState());
    s.endGroup();

    //save the anchor settings
    //done only when user selects this from the Configuration menu
}


void MainWindow::onAccumPositionAction(QAction *action)
{
    int x = action->data().toInt();
    //which menu item has been selected
    qDebug() << "Position Anchors" << action->data().toInt();

    switch(action->data().toInt())
    {
        case AutoPosition:
            emit locateAnchors();
        break;
        case Position:
            emit repositionAnchors();
        break;
        case Rotate90:
            emit rotate90Anchors();
        break;
        case FlipY:
        case FlipX:
            emit flipXorYAnchors(x-FlipY);
        break;
    }

}

/*void MainWindow::onPositionAnchors()
{
    emit locateAnchors();
}*/

void MainWindow::onAboutAction()
{
    infoLabel->setText(tr("Invoked <b>Help|About</b>"));

    QMessageBox::about(this, tr("About"),
            tr("<b>TDoA RTLS Controller</b>"
               "<br>" GUI_VERSION " (" __DATE__
               ") <br>Copyright (C) 2017-2019, Decawave Ltd.\n"
               "<br>www.decawave.com"));
}

void MainWindow::onConfigAction()
{
    //config->setAutoFillBackground(true);

    //config->show();
}

void MainWindow::onCloseConfigWidget()
{
    //chanConfAction->setVisible();
    ui->channel_dw->close();
    //ui->menuConfig->close();
}

void MainWindow::onLoadConfig()
{
    //load view settings
    loadConfigFile("./CCview_config.xml");
    QString filename = QFileDialog::getOpenFileName(this, tr("Load RF Config"), ".", tr("Config Files (*.xml)"));
    loadConfigFile(filename);

    //load the anchor settings
    filename = QFileDialog::getOpenFileName(graphicsWidget(),tr("Load Anchor Config"), ".", tr("Config Files (*.xml)"));
    RTLSControllerApplication::instance()->loadConfigFile(filename, _RTLSCLEConnection);
}

void MainWindow::onSaveConfig()
{
    //save view settings
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Channel Config"), "CCchan_config.xml", tr("Config Files (*.xml)"));
    saveConfigFile(filename, "chan_cfg");

    saveConfigFile("./CCview_config.xml", "view_cfg");

    //save the anchor settings
    filename = QFileDialog::getSaveFileName(graphicsWidget(),tr("Save Anchor Config"), "CCanch_config.xml", tr("Config Files (*.xml)"));
    RTLSControllerApplication::instance()->saveConfigFile(filename);

    graphicsWidget()->saveConfigFile("./CCtag_config.xml");
}

void MainWindow::onSaveAsConfig()
{
    //save channel,view & anchor settings
    saveConfigFile("CCchan_config.xml", "chan_cfg");
    saveConfigFile("./CCview_config.xml", "view_cfg");
    RTLSControllerApplication::instance()->saveConfigFile("CCanch_config.xml");
    graphicsWidget()->saveConfigFile("./CCtag_config.xml");
}

void MainWindow::onCLEConfigAction()
{
    emit configureCLEConnection();
}

void MainWindow::onReSyncAction()
{
    emit reSyncAnchors();
}



//void MainWindow::onCalibTestAction()
//{
//    emit calibrateAntennas();
//}

void MainWindow::loadConfigFile(QString filename)
{
    QFile file(filename);

    if (!file.open(QFile::ReadWrite | QFile::Text))
    {
        qDebug(qPrintable(QString("Error: Cannot read file %1 %2").arg(filename).arg(file.errorString())));
        return;
    }

    QDomDocument doc;

    doc.setContent(&file, false);

    QDomElement root = doc.documentElement();

    qDebug() << root.tagName() ;


    if( root.tagName() == "config" )
    {
        //there are acnhors saved in the config file
        //populate the _model anchor list

        QDomNode n = root.firstChild();
        while( !n.isNull() )
        {
            QDomElement e = n.toElement();
            if( !e.isNull() )
            {
                if( e.tagName() == "view_cfg" )
                {

                    RTLSControllerApplication::viewSettings()->setGridWidth((e.attribute( "gridW", "" )).toDouble());
                    RTLSControllerApplication::viewSettings()->setGridHeight((e.attribute( "gridH", "" )).toDouble());
                    RTLSControllerApplication::viewSettings()->setShowGrid(((e.attribute( "gridS", "" )).toInt() == 1) ? true : false);
                    RTLSControllerApplication::viewSettings()->setShowOrigin(((e.attribute( "originS", "" )).toInt() == 1) ? true : false);
                    RTLSControllerApplication::viewSettings()->setFloorplanPath(e.attribute( "fplan", "" ));
                    RTLSControllerApplication::viewSettings()->setFloorplanXOffset((e.attribute( "offsetX", "" )).toDouble());
                    RTLSControllerApplication::viewSettings()->setFloorplanYOffset((e.attribute( "offsetY", "" )).toDouble());
                    RTLSControllerApplication::viewSettings()->setFloorplanXScale((e.attribute( "scaleX", "" )).toDouble());
                    RTLSControllerApplication::viewSettings()->setFloorplanYScale((e.attribute( "scaleY", "" )).toDouble());
                    RTLSControllerApplication::viewSettings()->floorplanFlipX((e.attribute( "flipX", "" )).toInt());
                    RTLSControllerApplication::viewSettings()->floorplanFlipY((e.attribute( "flipY", "" )).toInt());

                    RTLSControllerApplication::viewSettings()->setFloorplanPathN();

                }

                if( e.tagName() == "chan_cfg" )
                {
                    RTLSControllerApplication::channelSettings()->setCurrentChannel((e.attribute( "chan", "" )).toInt());
                    RTLSControllerApplication::channelSettings()->setCurrentPRF((e.attribute( "prf", "" )).toInt());
                    RTLSControllerApplication::channelSettings()->setCurrentDataRate((e.attribute( "datarate", "" )).toInt());
                    RTLSControllerApplication::channelSettings()->setCurrentPreambleLength((e.attribute( "plen", "" )).toInt());
                    RTLSControllerApplication::channelSettings()->setCurrentPreambleCode((e.attribute( "pcode", "" )).toInt());
                    RTLSControllerApplication::channelSettings()->setCurrentPAC((e.attribute( "pac", "" )).toInt());
                    RTLSControllerApplication::channelSettings()->setCurrentNSFD((e.attribute( "nssfd", "" )).toInt());

                    //once loaded send the RF configuration to CLE
                    RTLSControllerApplication::control()->sendRFConfiguration(false);
                }
            }

            n = n.nextSibling();
        }

    }

    file.close(); //close the file
}

void MainWindow::saveConfigFile(QString filename, QString cfg)
{
    QFile file(filename);

    /*if (!file.open(QFile::ReadWrite | QFile::Text))
    {
        qDebug(qPrintable(QString("Error: Cannot read file %1 %2").arg(filename).arg(file.errorString())));
        return;
    }*/

    QDomDocument doc;
    //doc.setContent(&file, false);

    //save the graphical information
    QDomElement info = doc.createElement("config");
    doc.appendChild(info);

    qDebug() << info.tagName() ;

    if(cfg == "view_cfg")
    {
        QDomElement cn = doc.createElement( "view_cfg" );

        cn.setAttribute("gridW",  QString::number(RTLSControllerApplication::viewSettings()->gridWidth(), 'g', 3));
        cn.setAttribute("gridH",  QString::number(RTLSControllerApplication::viewSettings()->gridHeight(), 'g', 3));
        cn.setAttribute("gridS",  QString::number((RTLSControllerApplication::viewSettings()->gridShow() == true) ? 1 : 0));
        cn.setAttribute("originS",  QString::number((RTLSControllerApplication::viewSettings()->originShow() == true) ? 1 : 0));
        cn.setAttribute("flipX",  QString::number(RTLSControllerApplication::viewSettings()->floorplanFlipX(), 10));
        cn.setAttribute("flipY",  QString::number(RTLSControllerApplication::viewSettings()->floorplanFlipY(), 10));
        cn.setAttribute("scaleX",  QString::number(RTLSControllerApplication::viewSettings()->floorplanXScale(),'g', 3));
        cn.setAttribute("scaleY",  QString::number(RTLSControllerApplication::viewSettings()->floorplanYScale(), 'g', 3));
        cn.setAttribute("offsetX",  QString::number(RTLSControllerApplication::viewSettings()->floorplanXOffset(), 'g', 3));
        cn.setAttribute("offsetY",  QString::number(RTLSControllerApplication::viewSettings()->floorplanYOffset(), 'g', 3));
        cn.setAttribute("fplan", RTLSControllerApplication::viewSettings()->getFloorplanPath());
        info.appendChild( cn );

    }

    if(cfg == "chan_cfg")
    {
        QDomElement cn = doc.createElement( "chan_cfg" );

        cn.setAttribute("chan",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentChannel(), 10));
        cn.setAttribute("prf",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentPRF(), 10));
        cn.setAttribute("datarate",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentDataRate(), 10));
        cn.setAttribute("plen",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentPreambleLength(), 10));
        cn.setAttribute("pcode",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentPreambleCode(), 10));
        cn.setAttribute("nssfd",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentNSFD(), 10));
        cn.setAttribute("pac",  QString::number(RTLSControllerApplication::channelSettings()->getCurrentPAC(), 10));
        info.appendChild( cn );
    }

    //file.close(); //close the file and overwrite with new info

    file.open(QIODevice::WriteOnly | QIODevice::Text);


    QTextStream ts( &file );
    ts << doc.toString();

    qDebug() << doc.toString();

    file.close();
}

void MainWindow::rangingProgress(QString status)
{
    statusBar()->showMessage(status);
    QMessageBox::about(nullptr, tr("Ranging ..."), status);
}

void MainWindow::commsTestProgress(QString status)
{
    statusBar()->showMessage(status);
    QMessageBox::about(nullptr, tr("ComTestRanging ..."), status);
}


void MainWindow::positionError(void)
{
    QMessageBox::critical(this, tr("Position Error"), tr("The Two-way ranging test calculated ranges are invalid."));
}

void MainWindow::statusBarMessage(QString status)
{
    statusBar()->showMessage(status); //ui->statusBar->showMessage(status);
}

void MainWindow::disableReSync(bool status)
{
    reSyncAction->setDisabled(status);
}

void MainWindow::onREKManual()
{
    //QString path = QDir::currentPath();
    //path.append("/../Docs/REK1101_User_Manual.pdf");
    QString path ="../Docs/REK1101_User_Manual.pdf";
    QUrl pathUrl = QUrl::fromLocalFile(path);
    QDesktopServices::openUrl(pathUrl);

}

void MainWindow::onREKQuickStart()
{
    //QString path = QDir::currentPath();
    //path.append("/../Docs/REK1101_Quick_Start_Guide.pdf");
    QString path ="../Docs/REK1101_Quick_Start_Guide.pdf";
    QUrl pathUrl = QUrl::fromLocalFile(path);
    QDesktopServices::openUrl(pathUrl);
}

void MainWindow::onChanConfAction(void)
{
    ui->channel_dw->toggleViewAction();
    ui->channel_dw->show();
//    ui->channel_dw->setEnabled(true);
//    ui->channel_dw->setFloating(true);
    ui->channel_dw->topLevelChanged(true);
}
