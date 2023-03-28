// -------------------------------------------------------------------------------------------------------------------
//
//  File: GraphicsDataAnchor.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "GraphicsDataAnchor.h"

#include "DataAnchor.h"
#include "AnchorMenu.h"

#include <QPainter>
#include <QPen>
#include <QMenu>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QDebug>

GraphicsDataAnchor::GraphicsDataAnchor(DataAnchor *anchor, GraphicsDataModel *graphicsModel)
    : GraphicsDataItem(anchor, graphicsModel),
      _anchor(anchor),
      _updatingPosition(0)
{
    _numberLabel = new QGraphicsSimpleTextItem(this);
    _numberLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    _numberLabel->setZValue(1);

    setRect(-0.04, -0.04, 0.08, 0.08);
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setPen(Qt::NoPen);

    updatePosition();
    updateMaster();
    updateSelected();
}

QVariant GraphicsDataAnchor::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant & value)
{
    if (change == QGraphicsItem::ItemPositionChange)
    {
        QPointF f = value.toPointF();

        if (!f.isNull())
        {
            _updatingPosition ++;
            _anchor->setCoordinates(f.x(), f.y());
            _updatingPosition --;
        }
    }

    return GraphicsDataItem::itemChange(change, value);
}

void GraphicsDataAnchor::updatePosition()
{
    if (!_updatingPosition) // Avoid infinite recursion
        setPos(_anchor->x(), _anchor->y());
}

void GraphicsDataAnchor::updateMaster()
{
    switch(_anchor->master())
    {
        case PRIMARY_MASTER:
            setBrush(QBrush(Qt::darkBlue));
        break;
        case SECONDARY_MASTER:
            setBrush(QBrush(Qt::blue));
        break;
        default:
            setBrush(QBrush(Qt::red));
        break;
    }
}

void GraphicsDataAnchor::updateSelected()
{
    //TODO what to display for selected anchors?
    //qDebug() << "update Selected anchors";
}

void GraphicsDataAnchor::updateConnected()
{

    if(_anchor->connected() == 1) //Eth
    {

    }
    else
    {
        //setBrush(QBrush(_anchor->master() ? Qt::blue : Qt::red));
    }
}

void GraphicsDataAnchor::modelChanged(int first, int last)
{
    if (first <= DataAnchor::ColumnY && last >= DataAnchor::ColumnX)
    {
        updatePosition();
    }
    else if (first <= DataAnchor::ColumnMasterSlave && last >= DataAnchor::ColumnMasterSlave)
    {
        updateMaster();
    }
    else if (first <= DataAnchor::ColumnSelected && last >= DataAnchor::ColumnSelected)
    {
        updateSelected();
    }
}

void GraphicsDataAnchor::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    QRadialGradient radialGrad;

    //painter->setPen(pen());
    if(_anchor->connected() == 1) //Eth
    {
        QPen pen = painter->pen() ;
        pen.setColor(Qt::green);
        pen.setStyle(Qt::SolidLine);
        pen.setWidthF(0.03);

        painter->setPen(pen);
    }
    else
    {
        QPen pen = painter->pen() ;
        pen.setColor(Qt::white);
        pen.setStyle(Qt::SolidLine);
        pen.setWidthF(0.03);
        //painter->setPen(Qt::NoPen);
        painter->setPen(pen);
    }

    if (this->isSelected())
    {
        painter->setBrush(brush());
        setRect(-0.1, -0.1, 0.2, 0.2);
    }
    else
    {
        radialGrad.setColorAt(0,QColor::fromRgb(255, 0, 0));
        radialGrad.setColorAt(0.2, QColor::fromRgb(255,182,193));
        painter->setBrush(QBrush(radialGrad));
        radialGrad.setCenter (_anchor->x(),_anchor->y());
        setRect(-0.25, -0.25, 0.5, 0.5);
//        painter->setBrush(QBrush(brush().color().darker()));
//        setRect(-0.05, -0.05, 0.1, 0.1);
    }

    if(_anchor->selected()) //this anchor is selected
    {
        if(_anchor->number() > 0) //show the anchor number
        {
            //double xp = _anchor->x()+0.05;
            //double yp = _anchor->y()+0.05;

            QString numbertext = QString::number(_anchor->number(), 10);
            QFont f = _numberLabel->font();
            f.setPointSizeF(18);
            _numberLabel->setFont(f);
            _numberLabel->setPen(painter->pen());
            if(_anchor->master() > 0)
            {
                _numberLabel->setText("  M " + numbertext + " " );
            }
            else
            {
                _numberLabel->setText("  S " + numbertext + " " );
            }
            _numberLabel->setBrush(QBrush(Qt::black));
            //_numberLabel->setPos(xp, yp);
            //qDebug() << _numberLabel->x() << _numberLabel->y();
            //qDebug() << _anchor->number() << "x" << _anchor->x() << xp << "y" << _anchor->y() << yp;
        }
        painter->setOpacity(1.0);
    }
    else
    {
        painter->setOpacity(0.2);
        _numberLabel->setText("      ");
        _numberLabel->setBrush(QBrush(brush().color().lighter()));
    }

    painter->drawRect(rect());
}

void GraphicsDataAnchor::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (scene() && scene()->mouseGrabberItem() == this)
        ungrabMouse();

    AnchorMenu menu;
    menu.exec(event->screenPos());
}
