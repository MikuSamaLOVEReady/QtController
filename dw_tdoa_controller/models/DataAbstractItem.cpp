// -------------------------------------------------------------------------------------------------------------------
//
//  File: DataAbstractItem.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "DataAbstractItem.h"

#include "DataModel.h"

DataAbstractItem::DataAbstractItem(DataModel *model, DataAbstractItem *parent, int row)
    : _model(model),
      _parent(parent)
{
    if (row < 0)
        _index = QModelIndex();
    else
        _index = model->createIndex(row, 0, this);
}

QModelIndex DataAbstractItem::index(int column)
{
    return model()->createIndex(_index.row(), column, this);
}

DataAbstractItem *DataAbstractItem::child(int row)
{
    Q_UNUSED(row);
    return nullptr;
}

const DataAbstractItem *DataAbstractItem::child(int row) const
{
    Q_UNUSED(row);
    return nullptr;
}
