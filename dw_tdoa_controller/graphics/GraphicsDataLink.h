// -------------------------------------------------------------------------------------------------------------------
//
//  File: GraphicsDataLink.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef GRAPHICSDATALINK_H
#define GRAPHICSDATALINK_H

#include "GraphicsDataItem.h"
#include <QGraphicsPathItem>

class DataLink;
class QGraphicsSimpleTextItem;

/**
 * Represent a link as a graphics item.
 *
 * It draws a line between the two items, and displays the distance between them.
 */
class GraphicsDataLink : public GraphicsDataItem<QGraphicsLineItem>
{
public:
    GraphicsDataLink(DataLink *item, GraphicsDataModel *model);
    DataLink *link() { return _link; }

    virtual void modelChanged(int first, int last);

    /**
     * React to changes in the link's parent model
     * @param first the left-most changed column
     * @param last the right-most changed column
     */
    virtual void parentModelChanged(int first, int last);

    /**
     * React to changes in the link's target model
     * @param first the left-most changed column
     * @param last the right-most changed column
     */
    virtual void targetModelChanged(int first, int last);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QPainterPath shape() const;

protected:
    /**
     * React to changes in the parent or target positions.
     */
    void updatePositions();

    /**
     * React to changes in the distance between the parent and the target.
     */
    void updateDistance();

    /**
     * React to changes in the ratio of received packets.
     */
    void updateRXRatio();

private:
    DataLink *_link;
    QGraphicsSimpleTextItem *_distanceLabel;
};

#endif // GRAPHICSDATALINK_H
