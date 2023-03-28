// -------------------------------------------------------------------------------------------------------------------
//
//  File: DataRoot.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "DataRoot.h"
#include "DataAnchor.h"
#include "DataModel.h"

#include <QDebug>

DataRoot::DataRoot(DataModel *model, int id)
    : DataAbstractItem(model, nullptr, -1)
{
    _id = id;
}

DataAbstractItem::Type DataRoot::type()
{
    return Root;
}

unsigned int DataRoot::rowCount() const
{
    return _anchorList.size();
}

unsigned int DataRoot::columnCount() const
{
    return DataAnchor::ColumnCount;
}

QVariant DataRoot::data(int column) const
{
    Q_UNUSED(column);
    return QVariant();
}

bool DataRoot::setData(int column, const QVariant &data)
{
    Q_UNUSED(column);
    Q_UNUSED(data);
    return false;
}

DataAbstractItem *DataRoot::child(int row)
{
    return _anchorList.value(row, nullptr);
}

const DataAbstractItem *DataRoot::child(int row) const
{
    return _anchorList.value(row, nullptr);
}

bool DataRoot::isEditable(int column) const
{
    Q_UNUSED(column);
    return false;
}

int DataRoot::anchorIdx(uint64_t id)
{
    for(int i=0; i<_anchorList.size(); i++)
    {
        if(_anchorList.at(i)->id() == id)
        {
            return i;
        }
    }

    return -1;
}

DataAnchor *DataRoot::anchor(uint64_t id, bool add)
{
    DataAnchor *anchor = _anchors.value(id);
    if (!anchor && add)
    {
        int row = _anchorList.size();
        //qDebug() << "id " << id << " index " << index() << " row " << row ;
        model()->beginInsertColumns(index(), row, row);
        anchor = new DataAnchor(model(), this, row, id);
        _anchors.insert(id, anchor);
        _anchorList.append(anchor);
        model()->endInsertRows();
    }

    return anchor;
}

void DataRoot::removeAnchor(uint64_t id, QModelIndex currentIndex)
{
    Q_UNUSED(currentIndex);

    DataAnchor *anchor = _anchors.value(id);

    //model()->beginRemoveRows(currentIndex, 0, 0);
    //model()->beginRemoveColumns(currentIndex, 0, DataAnchor::ColumnCount);
    //model()->endRemoveColumns();
    //model()->beginInsertColumns(index(), row, row);

    _anchors.remove(id);
    _anchorList.removeOne(anchor);

    //model()->endRemoveRows();
    return;
}

DataAnchor *DataRoot::anchor(int numid)
{
    DataAnchor *anchor = nullptr;

    for(int i=0; i<_anchorList.size(); i++)
    {
        //get from selected anchors the anchor with a matching number
        if(_anchorList.at(i)->selected())
        {
            if(numid == _anchorList.at(i)->number())
            {
                anchor = _anchorList.at(i);
                break;
            }
        }
    }

    return anchor;
}

const QList<DataAnchor *> &DataRoot::anchors() const
{
    return _anchorList;
}

