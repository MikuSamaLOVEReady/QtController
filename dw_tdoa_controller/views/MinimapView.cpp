// -------------------------------------------------------------------------------------------------------------------
//
//  File: MinimapView.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "MinimapView.h"

#include "RTLSControllerApplication.h"
#include "ViewSettings.h"
#include "GraphicsView.h"

#include <QMouseEvent>
#include <qmath.h>

MinimapView::MinimapView(QWidget *parent) :
    QGraphicsView(parent),
    scene(new QGraphicsScene(this))
{
    setScene(scene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setDragMode(QGraphicsView::NoDrag);

    scale(1, -1);

    RTLSControllerApplication::connectReady(this, "onReady()");
}

void MinimapView::onReady()
{
    QObject::connect(RTLSControllerApplication::viewSettings(), SIGNAL(floorplanChanged()), this, SLOT(floorplanChanged()));
    QObject::connect(RTLSControllerApplication::graphicsView(), SIGNAL(visibleRectChanged(QRectF)), this, SLOT(visibleRectChanged()));
}

void MinimapView::floorplanChanged()
{
    ViewSettings *vs = RTLSControllerApplication::viewSettings();
    QRectF sceneRect = vs->floorplanTransform().map(QRectF(0, 0, vs->floorplanPixmap().width(), vs->floorplanPixmap().height())).boundingRect();

    scene->setSceneRect(sceneRect);
    this->fitInView(sceneRect, Qt::KeepAspectRatio);
}

void MinimapView::visibleRectChanged()
{
    scene->update();
}

void MinimapView::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect)
    const QPixmap &pm = RTLSControllerApplication::viewSettings()->floorplanPixmap();
    if (!pm.isNull())
    {
        painter->save();
        painter->setTransform(RTLSControllerApplication::viewSettings()->floorplanTransform(), true);
        painter->setPen(QPen(QBrush(Qt::black), 1));
        painter->drawPixmap(0, 0, pm);
        painter->restore();

        painter->setPen(QPen(QBrush(Qt::red), 0.1));
        painter->drawRect(RTLSControllerApplication::graphicsView()->visibleRect());
    }
}

void MinimapView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    ViewSettings *vs = RTLSControllerApplication::viewSettings();
    QRectF sceneRect = vs->floorplanTransform().map(QRectF(0, 0, vs->floorplanPixmap().width(), vs->floorplanPixmap().height())).boundingRect();
    this->fitInView(sceneRect, Qt::KeepAspectRatio);
}

void MinimapView::mousePressEvent(QMouseEvent *event)
{
    QRectF visibleRect = RTLSControllerApplication::graphicsView()->visibleRect();
    visibleRect.moveCenter(mapToScene(event->pos()));
    RTLSControllerApplication::graphicsView()->setVisibleRect(visibleRect);
}

void MinimapView::mouseMoveEvent(QMouseEvent *event)
{
    QRectF visibleRect = RTLSControllerApplication::graphicsView()->visibleRect();
    visibleRect.moveCenter(mapToScene(event->pos()));
    RTLSControllerApplication::graphicsView()->setVisibleRect(visibleRect);
}

void MinimapView::wheelEvent(QWheelEvent *event)
{
    qreal s = pow((double)2, event->delta() / 360.0);
    RTLSControllerApplication::graphicsView()->scaleView(s, s);
}
