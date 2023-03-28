// -------------------------------------------------------------------------------------------------------------------
//
//  File: GraphicsDataAnchor.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef GRAPHICSDATAANCHOR_H
#define GRAPHICSDATAANCHOR_H

#include "GraphicsDataItem.h"
#include <QGraphicsRectItem>

class DataAnchor;
class DataAnchor;
class GraphicsDataModel;
class QVariant;
class QGraphicsSceneContextMenuEvent;

/**
 * Represent an anchor as a graphics item.
 */
class GraphicsDataAnchor : public GraphicsDataItem<QGraphicsRectItem>
{
public:
    /**
     * Contructs a new GraphicsDataAnchor.
     * @param anchor the anchor model this object should represent
     * @param graphicsModel the GraphicsDataModel in which this object belongs.
     */
    GraphicsDataAnchor(DataAnchor *anchor, GraphicsDataModel *graphicsModel);


    virtual void modelChanged(int first, int last);

    DataAnchor *anchor() { return _anchor; }

protected:
    virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant & value);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    /**
     * React to changes in the anchor's position.
     */
    void updatePosition();

    /**
     * React to changes in the anchor being a master or not
     */
    void updateMaster();

    /**
     * React to changes in the anchor being selected or not
     */
    void updateSelected();

    /**
     * React to changes in the anchor being connected to the CLE or not
     */
    void updateConnected();

private:
    DataAnchor *_anchor;
    int _updatingPosition;
    QGraphicsSimpleTextItem *_numberLabel;
};

#endif // GRAPHICSDATAANCHOR_H
