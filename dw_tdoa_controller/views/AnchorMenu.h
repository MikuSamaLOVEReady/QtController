// -------------------------------------------------------------------------------------------------------------------
//
//  File: AnchorMenu.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef ANCHORMENU_H
#define ANCHORMENU_H

#include <QMenu>

class DataAnchor;

class AnchorMenu : public QMenu
{
    Q_OBJECT
public:
    explicit AnchorMenu(QWidget *parent = nullptr);
    virtual ~AnchorMenu() {}

    void setCurrentAnchorSelected(bool);
signals:

public slots:

protected slots:
    void onReady();
    void onSelectionModelChanged();

    void onPowerTestAction();
    void onCommTestAction();
    void onAccumLogAction(QAction *action);
    void onRangingAction();
    void onLocateAction();
    void onDeleteAction();

private:
    DataAnchor *currentAnchor();
    QList<DataAnchor *> selectedAnchors();

    //QAction *masterAction;
    //QAction *slaveAction;
    QAction *powerTestAction;
    QMenu   *accumLogSubMenu;
    QAction *commTestAction;
    QAction *rangingAction;
    QAction *locateAction;
    QAction *deleteAction;
    QAction *accumLogTestAction;
};

#endif // ANCHORMENU_H
