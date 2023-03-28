// -------------------------------------------------------------------------------------------------------------------
//
//  File: SceneMenu.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "SceneMenu.h"

#include "RTLSControllerApplication.h"
#include "DataModel.h"
#include "DataAnchor.h"
#include "RTLSControl.h"
#include "RTLSClient.h"

#include <QInputDialog>

SceneMenu::SceneMenu(const QPointF &scenePos, QWidget *parent) :
    QMenu(parent),
    _scenePos(scenePos)
{
    addAction("Add Anchor", this, SLOT(onAddAnchor()));
}

void SceneMenu::onAddAnchor()
{
    bool ok;
    QString idStr = QInputDialog::getText(nullptr, "Anchor ID", "Anchor ID", QLineEdit::Normal, "", &ok); // TODO: add custom dialog with format checking ...
    if (ok)
    {
        uint64_t id = idStr.toULongLong(&ok, 16);
        if (ok)
        {
            DataAnchor *anchor = RTLSControllerApplication::model()->anchor(id, true);
            anchor->setCoordinates(_scenePos.x(), _scenePos.y());
            RTLSControllerApplication::anchorSelectionModel()->setCurrentIndex(anchor->index(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows); // Switch to the new anchor
            RTLSControllerApplication::control()->sendConfiguration(anchor);
        }
    }
}

