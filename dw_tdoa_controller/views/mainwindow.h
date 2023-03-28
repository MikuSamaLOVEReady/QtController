// -------------------------------------------------------------------------------------------------------------------
//
//  File: MainWindow.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "rtlscleconnection.h"
#include "ChannelSettingsWidget.h"
#include <QMainWindow>
#include <QAbstractSocket>
#include <QLabel>
#include <QtSerialPort/QSerialPort>

namespace Ui {
class MainWindow;
}

class RTLSCLEConnection;
class GraphicsWidget;
class ChannelSettingsWidget;
class AnchorListWidget;

/**
 * The MainWindow class is the RTLS Controller Main Window widget.
 *
 * It is responsible for setting up all the dock widgets inside it, as weel as the central widget.
 * It also handles global shortcuts (Select All) and the Menu bar.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    enum AnchorPosition {
        AutoPosition = 0,   ///< auto position
        Position,           ///< just position (wil use current RF distances
        Rotate90,           ///< rotate 90deg anti-clockwise
        FlipY,              ///< flip around X axis
        FlipX               ///< flip around Y axis
    };


    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    GraphicsWidget *graphicsWidget();
    ChannelSettingsWidget* channelSettings();
    AnchorListWidget *anchorListWidget();
    /**
     * Create a new menu with the window's action
     * @see QMainWindow::createPopupMenu()
     * @return the new menu instance
     */
    virtual QMenu *createPopupMenu();

    /**
     * createPopupMenu adds the windows actions to \a menu.
     * The are the actions to hide or show dock widgets.
     * @param menu the QMenu instance to which the actions should be added
     * @return the menu object
     */
    QMenu *createPopupMenu(QMenu *menu);

    void saveConfigFile(QString filename, QString cfg);
    void loadConfigFile(QString filename);

    QStringList portsList();
    int getTagBlinkRate(int index);

signals:
    void configureCLEConnection(void);
    void reSyncAnchors(void);
//    void calibrateAntennas(void);
    void locateAnchors(void);
    void repositionAnchors(void);
    void rotate90Anchors(void);
    void flipXorYAnchors(int);
    void getCfg(void);
    void nextCmd(void);

protected slots:
    void onReady();
    void connectionStateChanged(RTLSCLEConnection::ConnectionState);

    void onSelectAll();

    void loadSettings();
    void saveSettings();

    void onAboutAction();
    void onConfigAction();

//    void onCalibTestAction();
    void onReSyncAction();
    void onChanConfAction();
    void onCLEConfigAction();
    //void onPositionAnchors();
    void onAccumPositionAction(QAction *action);
    void onSaveConfig();
    void onSaveAsConfig();
    void onLoadConfig();

    void onMiniMapView();

    void onCloseConfigWidget(void);

    void rangingProgress(QString status);
    void commsTestProgress(QString status);
    void statusBarMessage(QString status);
    void disableReSync(bool status);
    void positionError(void);

    void onREKManual();
    void onREKQuickStart();

private:
    Ui::MainWindow *const ui;

    QMenu  *positionAnchorsSubMenu;
    QMenu  *helpMenu;
    QLabel *infoLabel;

    QAction *reSyncAction;
    QAction *cleConfigAction;
    QAction *positionAnchors;
//    QAction *calibTestAction;
    QAction *chanConfAction;
    QAction *saveConfig;
    QAction *saveAsConfig;
    QAction *loadConfig;
    QAction *REKManual;
    QAction *REKQuickStart;

    QMenu *accumLogSubMenu;

    QSerialPort *serial;

    QList<QSerialPortInfo>	portInfo ;
    QStringList ports;

    QList<QByteArray> cmdList ;

    bool waitForData ;
    int _RTLSCLEConnection ;
};


#endif // MAINWINDOW_H
