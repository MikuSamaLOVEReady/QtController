// -------------------------------------------------------------------------------------------------------------------
//
//  File: DataModel.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef DATAMODEL_H
#define DATAMODEL_H

#include "DataRoot.h"

#include <QAbstractItemModel>

class DataAbstractItem;
class DataAnchor;
class DataUndirectedLink;

/**
 * The DataModel class provides a QAbstractItemModel subclass representing the system's topology.
 * When subclassing QAbstractItemModel, at the very least you must implement:
 * index(), parent(), rowCount(), columnCount(), and data().
 * These functions are used in all read-only models, and form the basis of editable models.
 * To enable editing in your model, you must also implement setData(),
 * and reimplement flags() to ensure that ItemIsEditable is returned.
 *
 * Qt's model/view architecture provides a standard way for views to manipulate information
 * in a data source, using an abstract model of the data to simplify and standardize the way
 * it is accessed. Simple models represent data as a table of items, and allow views to access
 * this data via an index-based system. More generally, models can be used to represent data in
 * the form of a tree structure by allowing each item to act as a parent to a table of child items.
 */
class DataModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit DataModel(QObject *parent = 0, int id = 0);

    /// @{
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    /// @}

    /**
     * Get the item associated with the specified index.
     * @param index the index to look for
     * @return the item at the specified index
     */
    DataAbstractItem *item(const QModelIndex &index) const;

    /**
     * Get the _anchorList item index with the specified id.
     * @param id the anchor's number id
     * @return the index in the _anchorList of the found anchor, or -1 if none is found
     */
    int anchorIdx(uint64_t id);

    /**
     * Get the anchor item with the specified ID.
     * @param id the anchor's 64 bit ID
     * @param add if true and no anchor is found, a new one is added to the model and returned.
     * @return the found anchor, or a new one if none is found and @a add is true
     * @see DataRoot::anchor()
     */
    DataAnchor *anchor(uint64_t id, bool add = false);

    /**
     * Get the anchor item with the specified ID.
     * @param id the anchor's number ID
     * @return the found anchor, or a new one if none is found
     * @see DataRoot::anchor()
     */
    DataAnchor *anchor(int numid);

    /**
     * Get the list of anchors
     * @return the list of anchor items existing in the model.
     * @see DataRoot::anchors()
     */
    const QList<DataAnchor *> &anchors() const;

    void removeAnchor(uint64_t id, QModelIndex currentIndex);

    /**
     * Get the DataUndirectedLink object between two anchors.
     * @param anchorA the first anchor
     * @param anchorB the second anchor
     * @param add if true and no link between the specified anchors exists, a new one is added to the model and returned.
     * @return the found undirected link, or a new one if none is found and @a add is true
     * @see DataAnchor::link()
     */
    DataUndirectedLink *link(DataAnchor *anchorA, DataAnchor *anchorB, bool add = false);

    /**
     * Get a static empty model object.
     * The return value is always the same.
     * @return a static instance of this class.
     */
    static DataModel *staticEmptyModel();


    int id(void) { return _id; }

protected:

signals:

public slots:

private:
    DataRoot *_root;

    int _id;

    friend class DataAbstractItem;
    friend class DataAnchor;
    friend class DataRoot;
};

#endif // DATAMODEL_H
