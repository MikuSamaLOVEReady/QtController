// -------------------------------------------------------------------------------------------------------------------
//
//  File: GraphicsDataItem.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef GRAPHICSDATAITEM_H
#define GRAPHICSDATAITEM_H

#include <QItemSelectionModel>
#include <QGraphicsItem>

class DataAbstractItem;
class GraphicsDataModel;

class QGraphicsSceneMouseEvent;

/**
 * Represent an item of the model as a graphics item.
 *
 * This is an abstract class. Subclasses should be defined for each type of representable item.
 *
 * Because subclasses may want to use various QGraphicsItem subclasses, this class is a template and inherits its template parameter.
 * This allows to inherit any QGraphicsItem implementation.
 */
template <class T>
class GraphicsDataItem : public T
{
public:
    GraphicsDataItem(DataAbstractItem *item, GraphicsDataModel *model);

    /**
     * Update the item due to changes in the model.
     * @param first the left-most changed column
     * @param last the right-most changed column
     */
    virtual void modelChanged(int first, int last) = 0;

protected:
    virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant & value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    QItemSelectionModel::SelectionFlags selectionCommand(const QGraphicsSceneMouseEvent *event);

private:
    DataAbstractItem *_item;
    GraphicsDataModel *_graphicsModel;
};

#endif // GRAPHICSDATAITEM_H
