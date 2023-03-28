// -------------------------------------------------------------------------------------------------------------------
//
//  File: DataModel.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "DataModel.h"

#include "DataAbstractItem.h"
#include "DataRoot.h"
#include "DataAnchor.h"
#include "DataLink.h"

#include <QCoreApplication>
#include <QDebug>

DataModel::DataModel(QObject *parent, int id) :
    QAbstractItemModel(parent),
    _root(new DataRoot(this, id)),
    _id(id)
{
    //qDebug() << "DecaRoot created" << id;
}

int DataModel::rowCount(const QModelIndex &parent) const
{
    return this->item(parent)->rowCount();
}

int DataModel::columnCount(const QModelIndex &parent) const
{
    return this->item(parent)->columnCount();
}

QVariant DataModel::data(const QModelIndex &index, int role) const
{
    QVariant d;

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        d = this->item(index)->data(index.column());
    }

    if (d.type() == QVariant::Double && role == Qt::DisplayRole)
        d = QString::number(d.toDouble(), 'f', 3);

    return d;
}

bool DataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return this->item(index)->setData(index.column(), value);
    }
    else
        return false;
}

QModelIndex DataModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    DataAbstractItem *parentItem = this->item(parent);
    DataAbstractItem *item = parentItem->child(row);

    //qDebug() << "DataModel::index ID" << _id << row << column << parent ;

    if (item)
        return createIndex(row, column, item);
    else
        return QModelIndex();
}

QModelIndex DataModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    DataAbstractItem *item = this->item(index);
    if (item->parent())
        return item->parent()->index();
    else
        return QModelIndex();
}

Qt::ItemFlags DataModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (this->item(index)->isEditable(index.column()))
        flags |= Qt::ItemIsEditable;

    return flags;
}

DataAbstractItem *DataModel::item(const QModelIndex &index) const
{
    DataAbstractItem *item;
    if (index.isValid())
        item = static_cast<DataAbstractItem *>(index.internalPointer());
    else
        item = _root;

    Q_ASSERT(item);

    return item;
}

int DataModel::anchorIdx(uint64_t id)
{
    return _root->anchorIdx(id);
}

DataAnchor *DataModel::anchor(uint64_t id, bool add)
{
    return _root->anchor(id, add);
}

DataAnchor *DataModel::anchor(int numid)
{
    return _root->anchor(numid);
}

const QList<DataAnchor *> &DataModel::anchors() const
{
    return _root->anchors();
}

void DataModel::removeAnchor(uint64_t id, QModelIndex currentIndex)
{
    return _root->removeAnchor(id, currentIndex);
}

DataUndirectedLink *DataModel::link(DataAnchor *anchorA, DataAnchor *anchorB, bool add)
{
    DataLink *link = anchorA->link(anchorB);

    if (link)
    {
        //qDebug() << "link exists " ;
        return link->undirectedLink();
    }
    else if (add)
    {
        //qDebug() << "add new link " ;
        //if A ID > B ID then swap A & B contents... so anchor A ID is always < anchor B ID in a link?
        if (anchorA->id() > anchorB->id())
            std::swap(anchorA, anchorB);

        return new DataUndirectedLink(this, anchorA, anchorB);
    }
    else
        return nullptr;
}

Q_GLOBAL_STATIC(DataModel, staticEmptyDataModel)
DataModel *DataModel::staticEmptyModel()
{
    return staticEmptyDataModel();
}
