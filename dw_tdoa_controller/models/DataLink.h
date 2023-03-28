// -------------------------------------------------------------------------------------------------------------------
//
//  File: DataLink.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef DATALINK_H
#define DATALINK_H

#include "DataAbstractItem.h"

class DataAnchor;

class DataUndirectedLink;

/**
 * The DataLink class represents a directed link between two anchors.
 *
 * DataLink%s come in pairs, from one anchor (parent) to another (target).
 * The shared properties are saved in a common DataUndirectedLink class.
 *
 * It stores the target anchor's ID, the ratio of received packets and RF distance (range measure by TWR between parent and target anchors).
 * Additionnaly it exposes through its columns properties stored by the DataUndirectedLink.
 *
 * @see DataUndirectedLink
 */
class DataLink : public DataAbstractItem
{
public:
    enum Column {
        ColumnTarget = 0, ///< 64 bit id of the target anchor
        ColumnRXRatio,    ///< Ratio of received packets during communication tests, or -1 if unknown (double)

        ColumnDistance,   ///< Physical distance between the two anchors (double)
        ColumnRFDistance, ///< Distance measured by two-way ranging, or -1 if unknown (double)

        ColumnCount
    };

    DataLink(DataModel *model, DataAnchor *parent, DataAnchor *target, DataUndirectedLink *undirectedLink, int row);

    /// @{
    virtual Type type();

    virtual unsigned int rowCount() const;
    virtual unsigned int columnCount() const;

    virtual DataAbstractItem *child(int row);
    virtual const DataAbstractItem *child(int row) const;

    virtual QVariant data(int column) const;
    virtual bool setData(int column, const QVariant &data);

    virtual bool isEditable(int column) const;
    /// @}

    /**
     * @return the ratio of received packets during the communication test
     */
    double rxRatio() { return _rxRatio; }

    /**
     * @param rxRatio the new ratio of received packets during the communication test
     */
    void setRXRatio(double rxRatio);

    /**
     * @return this link's parent anchor
     */
    DataAnchor *parent() { return _parent; }
    const DataAnchor *parent() const { return _parent; }

    /**
     * @return the shared DataUndirectedLink instance associated with this link
     */
    DataUndirectedLink *undirectedLink(){ return _undirectedLink; }
    const DataUndirectedLink *undirectedLink() const { return _undirectedLink; }

    /**
     * @return this link's target anchor
     */
    DataAnchor *target() { return _target; }
    const DataAnchor *target() const { return _target; }

private:
    DataAnchor *_parent;
    DataAnchor *_target;

    DataUndirectedLink *_undirectedLink;

    double _rxRatio;
};

/**
 * The DataUndirectedLink class stores data shared among a pair of DataLink%s.
 */
class DataUndirectedLink
{
public:
    /**
     * Construct a new DataUndirectedLink object.
     * \a anchorA and \b anchorB must belong to the same model.
     * You shouldn't create DataUndirectedLink objects directly. Use DataModel::link() instead.
     *
     * @param model The model which contains the new links.
     * @param anchorA the first anchor (parent)
     * @param anchorB the second anchor (target)
     */
    DataUndirectedLink(DataModel *model, DataAnchor *anchorA, DataAnchor *anchorB);

    /**
     * @return the model which conatins the links
     */
    DataModel *model() { return _model; }

    /**
     * @return the first directed link
     */
    DataLink *linkA() { return _linkA; }

    /**
     * @return the second directed link
     */
    DataLink *linkB() { return _linkB; }

    /**
     * @return the first anchor instance
     */
    DataAnchor *anchorA() { return _anchorA; }

    /**
     * @return the second anchor instance
     */
    DataAnchor *anchorB() { return _anchorB; }

    /**
     * @return the physical distance between the two anchors
     */
    double distance() const { return _distance; }

    /**
     * @brief Recalculate the distance between the two anchors.
     * @param updateModel if true the DataModel::dataChanged signal will be emitted
     */
    void updateDistance(bool updateModel = true);

    /**
     * @return the distance between the two anchors, as measured by two way ranging
     */
    double rfDistance() const { return _rfDistance; }

    /**
     * @return 1 or 0 depending if the RF distance should be used for master-slave sync
     */
    int rfUseDistance() const { return _rfUseDistance; }

    /**
     * @param rfDistance the new distance between the two anchors, as measured by two way ranging
     */
    void setRFDistance(double rfDistance);

    /**
     * @param rfUseDistance configure whether to use the RF distance for master-slave sync
     */
    void setUseRFDistance(int rfUseDistance);

private:
    DataModel *_model;
    DataAnchor *_anchorA;
    DataAnchor *_anchorB;
    DataLink *_linkA;
    DataLink *_linkB;

    double _distance;
    double _rfDistance;
    int _rfUseDistance;
};

#endif // DATAABSTRACTLINK_H
