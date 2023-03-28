// -------------------------------------------------------------------------------------------------------------------
//
//  File: DataAbstractItem.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef DATAABSTRACTITEM_H
#define DATAABSTRACTITEM_H

#include <stdint.h>
#include <QMap>
#include <QPersistentModelIndex>

class DataModel;
class QVariant;

/**
 * Abstract base class for items part of the model.
 *
 * Gives access to properties stored by this model, and to its children.
 * It provides a similar api then QAbstractItemModel, but for a single item.
 */
class DataAbstractItem {
public:
    /**
     * Construct a new DataAbstractItem.
     * The row is the initial position of the item in its parent. It is used to build the initial index.
     * If parent is nullptr and row is negative, then the item is the root element of the model.
     * @param model the model which contains this item
     * @param parent the item's parent
     * @param row the item's row index, within the parent
     */
    explicit DataAbstractItem(DataModel *model, DataAbstractItem *parent, int row);
    virtual ~DataAbstractItem() {}

    /**
     * The item's type.
     * This enumeration is used to distinguish this item's type.
     * @see type()
     */
    enum Type {
        Root,
        Anchor,
        Link,
        Tag
    };

    /**
     * @return the item's type.
     */
    virtual Type type() = 0;

    /**
     * @return the number of child items.
     */
    virtual unsigned int rowCount() const = 0;

    /**
     * @return the number of columns child items have.
     */
    virtual unsigned int columnCount() const = 0;

    virtual DataAbstractItem *child(int row);
    virtual const DataAbstractItem *child(int row) const;

    /**
     * Get this item's property associated with the specified column.
     * @param column the column index
     * @return a QVariant containing the data, of a null one if the column is invalid.
     */
    virtual QVariant data(int column) const = 0;
    /**
     * Modify this item's property associated with the specified column.
     * @param column the column index
     * @param data the column's new data
     * @return true if the property was changed succesfully, false otherwise.
     */
    virtual bool setData(int column, const QVariant &data) = 0;

    /**
     * Check whether the property associated with the specified column is editable.
     * @param column index of the column to check for.
     * @return true if the column is editable, false otherwise
     */
    virtual bool isEditable(int column) const = 0;

    /**
     * Get the model index associated with this item
     * @return a QModelIndex representing this item at colum 0.
     */
    QModelIndex index() { return _index; }

    /**
     * Get the model index associated with this item and the specified column.
     * @param column the column of the returned index
     * @return a QModelIndex representing this item at @a column
     */
    QModelIndex index(int column);

    /**
     * @return the model to which this item belongs
     */
    DataModel *model() { return _model; }

    /**
     * @return the item's parent, or nullptr if the item is the root item
     */
    DataAbstractItem *parent() { return _parent; }

private:
    DataModel *_model;
    DataAbstractItem *_parent;
    QPersistentModelIndex _index;
};

#endif // DATAABSTRACTITEM_H
