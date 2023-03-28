// -------------------------------------------------------------------------------------------------------------------
//
//  File: DataLink.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "DataLink.h"

#include "DataAnchor.h"
#include "DataModel.h"

#include <qmath.h>
#include <QDebug>

DataLink::DataLink(DataModel *model, DataAnchor *parent, DataAnchor *target, DataUndirectedLink *undirectedLink, int row)
    : DataAbstractItem(model, parent, row),
      _parent(parent),
      _target(target),
      _undirectedLink(undirectedLink),
      _rxRatio(-1)
{
}

DataAbstractItem::Type DataLink::type()
{
    return Link;
}

unsigned int DataLink::rowCount() const
{
    return 0;
}

unsigned int DataLink::columnCount() const
{
    return 0;
}

DataAbstractItem *DataLink::child(int row)
{
    Q_UNUSED(row);
    return nullptr;
}

const DataAbstractItem *DataLink::child(int row) const
{
    Q_UNUSED(row);
    return nullptr;
}

QVariant DataLink::data(int column) const
{
    Q_ASSERT(column < ColumnCount);

    switch (column)
    {
    case ColumnTarget: return QString::number(_target->id(), 16);
    case ColumnRXRatio: return _rxRatio;
    case ColumnDistance: return undirectedLink()->distance();
    case ColumnRFDistance: return undirectedLink()->rfDistance();
    }

    return QVariant();
}

bool DataLink::setData(int column, const QVariant &data)
{
    Q_ASSERT(column < ColumnCount);

    switch (column)
    {
    case ColumnRFDistance: undirectedLink()->setRFDistance(data.toDouble()); return true;
    case ColumnRXRatio: setRXRatio(data.toDouble()); return true;
    }

    return false;
}

bool DataLink::isEditable(int column) const
{
    Q_ASSERT(column < ColumnCount);

    switch (column)
    {
    case ColumnTarget: return false;
    case ColumnDistance: return false;
    case ColumnRFDistance: return true;
    case ColumnRXRatio: return true;
    }
    return false;
}

void DataLink::setRXRatio(double rxRatio)
{
    _rxRatio = rxRatio;
    emit model()->dataChanged(index(ColumnRXRatio), index(ColumnRXRatio));
}

DataUndirectedLink::DataUndirectedLink(DataModel *model, DataAnchor *anchorA, DataAnchor *anchorB)
    : _model(model),
      _anchorA(anchorA),
      _anchorB(anchorB)
{
    Q_ASSERT(anchorA->model() == model);
    Q_ASSERT(anchorB->model() == model);
    Q_ASSERT(anchorA->id() < anchorB->id());

    updateDistance(false); // Don't signal the model, the link isn't inserted yet.

    // Do this last, as everything must be initialized before inserting the links in the model
    _linkA = anchorA->addLink(anchorB, this);
    _linkB = anchorB->addLink(anchorA, this);
}

void DataUndirectedLink::setRFDistance(double rfDistance)
{
    _rfDistance = rfDistance;
    emit _model->dataChanged(_linkA->index(DataLink::ColumnRFDistance), _linkA->index(DataLink::ColumnDistance));
    emit _model->dataChanged(_linkB->index(DataLink::ColumnRFDistance), _linkB->index(DataLink::ColumnDistance));
}

void DataUndirectedLink::setUseRFDistance(int rfDistance)
{
    _rfUseDistance = rfDistance;
}

void DataUndirectedLink::updateDistance(bool updateModel)
{
    double xdiff = _anchorA->x() - _anchorB->x();
    double ydiff = _anchorA->y() - _anchorB->y();
    double zdiff = _anchorA->z() - _anchorB->z();

    _distance = qSqrt(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);

    if (updateModel)
    {
        emit _model->dataChanged(_linkA->index(DataLink::ColumnDistance), _linkA->index(DataLink::ColumnDistance));
        emit _model->dataChanged(_linkB->index(DataLink::ColumnDistance), _linkB->index(DataLink::ColumnDistance));
    }
}
