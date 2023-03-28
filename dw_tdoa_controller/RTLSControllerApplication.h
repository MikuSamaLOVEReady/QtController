// -------------------------------------------------------------------------------------------------------------------
//
//  File: RTLSControllerApplication.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef RTLSCONTROLLERAPPLICATION_H
#define RTLSCONTROLLERAPPLICATION_H


#define REK_PRODUCT 0 //set this to 1 when building for REK product

#include "DataAnchor.h"
#include <QApplication>
#include <QAbstractItemView>
#include <qthread.h>
#include <QFile>

class DataModel;
class QItemSelectionModel;
class ViewSettings;
class MainWindow;
class RTLSCLEConnection;
class GraphicsWidget;
class GraphicsView;
class RTLSControl;
class RTLSClient;
class ChannelSettingsWidget;
class AnchorListWidget;

/**
 * The RTLSControllerApplication class is a singleton class which handles the application.
 *
 * It allows getting pointers to global objects within the application.
 * These pointers should not be used during initial initialization (in constructors), as it is niot guaranteed that they have bben allocated and initialized yet.\n
 * Instead, any setup which requires these pointers should be split in another function.\n
 * connectReady() may be used to run that specific function once all the resources are allocated.
 * The slot passed as the second argument will either run immediately if setup is already done, or connected to a signal that will be emitted once initialization is complete.
 * @code
 * RTLSControllerApplication::connectReady(this, "onReady()");
 * @endcode
 *
 */
class RTLSControllerApplication : public QApplication  //, virtual public QThread
{
    Q_OBJECT
public:
    explicit RTLSControllerApplication(int &argc, char ** argv);
    virtual ~RTLSControllerApplication();

    static RTLSControllerApplication *instance();

    static DataModel *model();
    static QItemSelectionModel *anchorSelectionModel();
    static QAbstractItemView::SelectionMode anchorSelectionMode();

    static ViewSettings *viewSettings();

    static RTLSCLEConnection *cleConnection();
    static RTLSControl *control();
    static RTLSClient *client();
    static MainWindow *mainWindow();
    static QFile *debugFile();


    static AnchorListWidget *anchorListWidget();
    static GraphicsWidget *graphicsWidget();
    static GraphicsView *graphicsView();

    static ChannelSettingsWidget* channelSettings();

    static void setAnchorSelectionMode(QAbstractItemView::SelectionMode mode);

    /**
     * Call \a member of \a receiver once initialization is complete.
     * The function is either called reight away if initialization is already done, or connected to the ready() signal.
     * The method must be registered within Qt's meta object system, using Q_INVOKABLE, or declaring it as a slot.
     */
    static void connectReady(QObject *receiver, const char *member, Qt::ConnectionType type = Qt::AutoConnection);

    void saveConfigFile(QString filename);
    void loadConfigFile(QString filename, int connection);

    bool calculateDistance(double d12, double d13, double d14, double d23, double d24, double d34);


signals:
    /**
     * Emitted when the inizialization is complete.
     * Because this signal is only emitted once at application startup, the connectReady() should be used instead.
     */
    void ready();
    void selectionModeChanged(QAbstractItemView::SelectionMode mode);
    void positionError(void);

public slots:

    void autoPositionAnchors();
    void positionAnchorsFromRange();
    void rotate90Anchors();
    void flipXYAnchors(int X);

private:
    DataModel *_model;
    QItemSelectionModel *_anchorSelectionModel;
    QAbstractItemView::SelectionMode _anchorSelectionMode;

    ViewSettings *_viewSettings;

    RTLSCLEConnection *_cleConnection;
    RTLSControl *_control;
    RTLSClient *_client;
    MainWindow *_mainWindow;

    //Debug
    QFile *_debugfile;

    bool _ready;

    QList<DataAnchor *> anchorsRngList ;
};

#endif // RTLSCONTROLLERAPPLICATION_H
