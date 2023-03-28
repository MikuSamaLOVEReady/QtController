// -------------------------------------------------------------------------------------------------------------------
//
//  File: OriginTool.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "OriginTool.h"

#include "ViewSettings.h"
#include "RTLSControllerApplication.h"
#include "GraphicsView.h"

#include <QCursor>

OriginTool::OriginTool(QObject *parent) :
    AbstractTool(parent)
{
}

QCursor OriginTool::cursor()
{
    return Qt::CrossCursor;
}

void OriginTool::clicked(const QPointF &scenePos)
{
    ViewSettings *vs = RTLSControllerApplication::viewSettings();
    QPointF b = vs->floorplanTransform().inverted().map(scenePos);
    vs->setFloorplanXOffset(b.x());
    vs->setFloorplanYOffset(b.y());

    RTLSControllerApplication::graphicsView()->translateView(-scenePos.x(), -scenePos.y());

    emit done();
}
