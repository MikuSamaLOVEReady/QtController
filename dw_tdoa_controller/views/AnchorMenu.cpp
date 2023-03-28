// -------------------------------------------------------------------------------------------------------------------
//
//  File: AnchorMenu.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "AnchorMenu.h"

#include "DataAnchor.h"
#include "DataModel.h"
#include "GraphicsView.h"
#include "RTLSControl.h"

#include <RTLSControllerApplication.h>

AnchorMenu::AnchorMenu(QWidget *parent) : QMenu(parent)
{
    accumLogSubMenu = new QMenu("Log Accumulator");
    QAction *ccp = new QAction("CCP",this);ccp->setData(1);
    QAction *blink = new QAction("Blink",this);blink->setData(0);

    accumLogSubMenu->addAction(ccp);
    accumLogSubMenu->addAction(blink);
    connect(accumLogSubMenu, SIGNAL(triggered(QAction*)), this, SLOT(onAccumLogAction(QAction*)));

    accumLogTestAction = addMenu(accumLogSubMenu);

    addSeparator();
    locateAction = addAction("Pan to", this, SLOT(onLocateAction()));

    rangingAction = addAction("Two-Way Ranging", this, SLOT(onRangingAction()));

    addSeparator();
    powerTestAction = addAction("Power Test", this, SLOT(onPowerTestAction()));
    commTestAction = addAction("Communication Test", this, SLOT(onCommTestAction()));

    //deleteAction = addAction("Delete", this, SLOT(onDeleteAction()));

    RTLSControllerApplication::connectReady(this, "onReady()");
}

void AnchorMenu::onReady()
{
    QObject::connect(RTLSControllerApplication::anchorSelectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                     this, SLOT(onSelectionModelChanged()));
    QObject::connect(RTLSControllerApplication::anchorSelectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                     this, SLOT(onSelectionModelChanged()));
    QObject::connect(RTLSControllerApplication::model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                     this, SLOT(onSelectionModelChanged()));

    onSelectionModelChanged();
}

void AnchorMenu::onSelectionModelChanged()
{
    bool hasSelection = RTLSControllerApplication::anchorSelectionModel()->hasSelection();
    commTestAction->setEnabled(hasSelection);
    rangingAction->setEnabled(hasSelection);
    locateAction->setEnabled(hasSelection);
    accumLogTestAction->setEnabled(hasSelection);
    powerTestAction->setEnabled(hasSelection);
}

void AnchorMenu::onPowerTestAction()
{
    RTLSControllerApplication::control()->powerTest(selectedAnchors());
}

void AnchorMenu::onCommTestAction()
{
    RTLSControllerApplication::control()->communicationTest(selectedAnchors());
}

void AnchorMenu::onAccumLogAction(QAction *action)
{
    RTLSControllerApplication::control()->accumLog(action->data().toInt(),selectedAnchors());
}

void AnchorMenu::onRangingAction()
{
    RTLSControllerApplication::control()->range(selectedAnchors());
}

void AnchorMenu::onLocateAction()
{
    QRectF visibleRect = RTLSControllerApplication::graphicsView()->visibleRect();

    QPoint p = QPoint(currentAnchor()->x(), currentAnchor()->y());

    visibleRect.moveCenter(p);
    RTLSControllerApplication::graphicsView()->setVisibleRect(visibleRect);
}

void AnchorMenu::onDeleteAction()
{
    QModelIndex currentIndex = RTLSControllerApplication::anchorSelectionModel()->currentIndex();
    DataAbstractItem *item = RTLSControllerApplication::model()->item(currentIndex);
    DataAnchor *anchor = dynamic_cast<DataAnchor *>(item);

    qDebug() << "remove anchor " << anchor->id();
    RTLSControllerApplication::model()->removeAnchor(anchor->id(), currentIndex);

    delete(item);
}

void AnchorMenu::setCurrentAnchorSelected(bool set)
{
    currentAnchor()->setSelected(set);
}


DataAnchor *AnchorMenu::currentAnchor()
{
    QModelIndex currentIndex = RTLSControllerApplication::anchorSelectionModel()->currentIndex();
    DataAbstractItem *item = RTLSControllerApplication::model()->item(currentIndex);

    if (!item || (item->type() != DataAbstractItem::Anchor))
        return nullptr;

    return dynamic_cast<DataAnchor *>(item);
}

QList<DataAnchor *> AnchorMenu::selectedAnchors()
{
    QList<DataAnchor *> selection;

    foreach (const QModelIndex &i, RTLSControllerApplication::anchorSelectionModel()->selectedRows())
    {
        DataAbstractItem *item = RTLSControllerApplication::model()->item(i);
        if (item->type() == DataAbstractItem::Anchor)
        {
            DataAnchor *anchor = dynamic_cast<DataAnchor *>(item);
            if (anchor)
                selection << anchor;
        }
    }

    return selection;
}
