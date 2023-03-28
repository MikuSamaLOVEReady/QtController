// -------------------------------------------------------------------------------------------------------------------
//
//  File: GraphicsDataItem.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "GraphicsDataItem.h"

#include "GraphicsDataModel.h"
#include "DataAbstractItem.h"
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>


template<class T>
GraphicsDataItem<T>::GraphicsDataItem(DataAbstractItem *item, GraphicsDataModel *model)
    : _item(item),
      _graphicsModel(model)
{
}

template<class T>
QVariant GraphicsDataItem<T>::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        // Don't let the scene override whether we're selected.
        return QVariant(_graphicsModel->selectionModel()->isSelected(_item->index()));
    }

    return T::itemChange(change, value);
}

template<class T>
void GraphicsDataItem<T>::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (T::flags() & QGraphicsItem::ItemIsSelectable) {
        _graphicsModel->selectionModel()->setCurrentIndex(_item->index(), selectionCommand(event));
    } else if (!(T::flags() & QGraphicsItem::ItemIsMovable)) {
        event->ignore();
    }
}

template<class T>
void GraphicsDataItem<T>::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    {
        // We don't want QGraphicsItem to handle selections.
        bool selectable = (T::flags() &  QGraphicsItem::ItemIsSelectable);
        T::setFlag(QGraphicsItem::ItemIsSelectable, false);
        T::mouseReleaseEvent(event);
        T::setFlag(QGraphicsItem::ItemIsSelectable, selectable);
    }


    if (T::flags() & QGraphicsItem::ItemIsSelectable) {
        _graphicsModel->selectionModel()->setCurrentIndex(_item->index(), selectionCommand(event));
    }
}

template<class T>
QItemSelectionModel::SelectionFlags GraphicsDataItem<T>::selectionCommand(const QGraphicsSceneMouseEvent *event)
{
    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Rows;
    bool multiSelect = (_graphicsModel->selectionMode() == QAbstractItemView::ExtendedSelection) && ((event->modifiers() & Qt::ControlModifier) != 0);
    bool moved = (event->scenePos() != event->buttonDownScenePos(event->button()));

    switch(event->type())
    {
    case QEvent::GraphicsSceneMousePress:
        if (!T::isSelected() && !multiSelect)
            flags |= QItemSelectionModel::ClearAndSelect;
        break;
    case QEvent::GraphicsSceneMouseRelease:
        if (!moved) {
            if (multiSelect) {
                flags |= QItemSelectionModel::Toggle;
            } else if (event->button() == Qt::LeftButton){
                flags |= QItemSelectionModel::ClearAndSelect;
            }
        }

    default:
        break;
    }

    return flags;
}

template class GraphicsDataItem<QGraphicsRectItem>;
template class GraphicsDataItem<QGraphicsLineItem>;
template class GraphicsDataItem<QGraphicsPathItem>;
