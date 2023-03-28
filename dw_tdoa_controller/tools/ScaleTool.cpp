// -------------------------------------------------------------------------------------------------------------------
//
//  File: ScaleTool.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "ScaleTool.h"

#include "RTLSControllerApplication.h"
#include "mainwindow.h"
#include "ViewSettings.h"

#include <QCursor>
#include <QInputDialog>
#include <QPainter>
#include <QDebug>

ScaleTool::ScaleTool(Axis a, QObject *parent) :
    AbstractTool(parent),
    axis(a),
    state(FirstPoint)
{
}

QCursor ScaleTool::cursor()
{
    return Qt::CrossCursor;
}

void ScaleTool::draw(QPainter *painter, const QRectF &rect, const QPointF &cursor)
{
    Q_UNUSED(rect)

    if (state == SecondPoint)
    {
        painter->save();
        painter->setPen(QPen(QBrush(Qt::black), 0));
        painter->drawLine(first, cursor);
        painter->restore();
    }
}

void ScaleTool::clicked(const QPointF &scenePos)
{
    ViewSettings *vs = RTLSControllerApplication::viewSettings();

    if (state == FirstPoint)
    {
        first = scenePos;
        state = SecondPoint;
    }
    else if (state == SecondPoint)
    {
        double p1 = (axis == XAxis) ? first.x() : first.y();
        double p2 = (axis == XAxis) ? scenePos.x() : scenePos.y();

        bool ok;
        double distance = QInputDialog::getDouble(RTLSControllerApplication::mainWindow(), "Distance", "Distance between the two points", qAbs(p2 - p1), 0, 1000, 3, &ok);

        if (ok)
        {
            double r = qAbs(p2 - p1) / distance;
            if (axis == XAxis)
                vs->setFloorplanXScale(r * vs->floorplanXScale());
            else
                vs->setFloorplanYScale(r * vs->floorplanYScale());
        }
        emit done();
    }
}

void ScaleTool::cancel()
{
    emit done(false);

    first = QPointF();
    state = FirstPoint;
}
