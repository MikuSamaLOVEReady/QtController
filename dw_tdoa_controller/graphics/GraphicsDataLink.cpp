// -------------------------------------------------------------------------------------------------------------------
//
//  File: GraphicsDataLink.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "GraphicsDataLink.h"

#include "DataLink.h"
#include "DataAnchor.h"

#include <QPen>
#include <QPainter>
#include <QGraphicsSimpleTextItem>
#include <QDebug>

GraphicsDataLink::GraphicsDataLink(DataLink *item, GraphicsDataModel *model)
    : GraphicsDataItem<QGraphicsLineItem>(item, model),
      _link(item)
{
    setZValue(-1);

    _distanceLabel = new QGraphicsSimpleTextItem(this);
    _distanceLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    _distanceLabel->setZValue(1);

    updatePositions();
    updateDistance();
    updateRXRatio();
}

void GraphicsDataLink::updatePositions()
{
    this->setPos((QPointF(_link->parent()->x(), _link->parent()->y()) + QPointF(_link->target()->x(), _link->target()->y())) / 2);
    QLineF line = QLineF(mapFromScene(_link->parent()->x(), _link->parent()->y()),
                         mapFromScene(_link->target()->x(), _link->target()->y()));
    this->setLine(line);
}

void GraphicsDataLink::updateDistance()
{
    QString dist_text = QString::number(_link->undirectedLink()->distance(), 'f', 2);
    QString rfdist_text = QString::number(_link->undirectedLink()->rfDistance(), 'f', 2);

    QString rxratio_text;

    if(_link->rxRatio() < 0)
    {
        rxratio_text = " ";
    }
    else
    {
        rxratio_text = QString::number(_link->rxRatio(), 'f', 2);
    }
    _distanceLabel->setText("d = " + dist_text + " \ntwr_d = " + rfdist_text + "\n" + rxratio_text);

}

void GraphicsDataLink::updateRXRatio()
{
    double rxRatio = _link->rxRatio();

    if (rxRatio > 1) rxRatio = 1;

    if (rxRatio < 0) setPen(QPen(QBrush(Qt::black), 0.005));
    else
    {
        // Simple interpolation from red to green
        QColor c = Qt::black;
        c.setRedF(1 - rxRatio);
        c.setGreenF(rxRatio);

        QPen p = QPen(QBrush(c), 0.005);

        if(rxRatio < 0.75)
            p.setStyle(Qt::DashLine);

        setPen(p);
    }
}

void GraphicsDataLink::modelChanged(int first, int last)
{
    if (first <= DataLink::ColumnDistance && last >= DataLink::ColumnDistance)
        updateDistance();
    else if (first <= DataLink::ColumnRXRatio && last >= DataLink::ColumnRXRatio)
        updateRXRatio();
}

void GraphicsDataLink::parentModelChanged(int first, int last)
{
    if (first <= DataAnchor::ColumnY && last >= DataAnchor::ColumnX)
        updatePositions();
}

void GraphicsDataLink::targetModelChanged(int first, int last)
{
    if (first <= DataAnchor::ColumnY && last >= DataAnchor::ColumnX)
        updatePositions();
}

void GraphicsDataLink::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    GraphicsDataItem<QGraphicsLineItem>::paint(painter, option, widget);
}

QPainterPath GraphicsDataLink::shape() const
{
    QPainterPath path;
    QPainterPathStroker stroker;

    stroker.setWidth(pen().widthF() * 10.);
    path.moveTo(line().p1());
    path.lineTo(line().p2());
    return stroker.createStroke(path);
}

