// -------------------------------------------------------------------------------------------------------------------
//
//  File: DataRoot.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef DATAROOT_H
#define DATAROOT_H

#include "DataAbstractItem.h"

#include <QMap>
#include <QList>

class QVariant;
class DataModel;
class DataAnchor;

/**
 * The DataRoot class represents the root item of the model.
 * It has no properties, and DataAnchor children.
 */
class DataRoot : public DataAbstractItem
{
public:
    /**
     * Construct a new DataRoot item.
     * @param model the model this item is the root of.
     */
    explicit DataRoot(DataModel *model, int id = 0);

    /// @{
    virtual Type type();

    virtual unsigned int rowCount() const;
    virtual unsigned int columnCount() const;

    virtual QVariant data(int column) const;
    virtual bool setData(int column, const QVariant &data);

    virtual DataAbstractItem *child(int row);
    virtual const DataAbstractItem *child(int row) const;

    virtual bool isEditable(int column) const;
    /// @}

    /**
     * Get the anchor item with the specified id.
     * @param id the anchor's 64 bit id
     * @param add if true and no anchor is found, a new one is added to the model and returned.
     * @return the found anchor, or a new one if none is found and @a add is true
     */
    DataAnchor *anchor(uint64_t id, bool add = false);

    void removeAnchor(uint64_t id, QModelIndex currentIndex);

    /**
     * Get the anchor item with the specified id.
     * @param id the anchor's number id
     * @return the found anchor, or a new one if none is found and @a add is true
     */
    DataAnchor *anchor(int id);

    /**
     * Get the _anchorList item index with the specified id.
     * @param id the anchor's number id
     * @return the index in the _anchorList of the found anchor, or -1 if none is found
     */
    int anchorIdx(uint64_t id);

    /**
     * Get the list of anchors
     * @return the list of anchor items existing in the model.
     */
    const QList<DataAnchor *> &anchors() const;


private:
    QMap<uint64_t, DataAnchor *> _anchors;
    QList<DataAnchor *> _anchorList;

    int _id;
};

#endif // DATAROOT_H
