// -------------------------------------------------------------------------------------------------------------------
//
//  File: DataAnchor.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "DataAnchor.h"

#include "DataModel.h"
#include "DataLink.h"
#include "RTLSControl.h"
#include "RTLSControllerApplication.h"

#include <QDebug>
#include <QMenu>

DataAnchor::DataAnchor(DataModel *model, DataAbstractItem *parent, int row, uint64_t id)
    : DataAbstractItem(model, parent, row),
      _dim(2),
      _id(id),
      _x(0),
      _y(0),
      _z(0),
      _master(0),
      _selected(false),
      _connected(NotConnected),
      _number(0),
      _antennaDlyRx(258.114),
      _antennaDlyTx(258.114),
      _xs(0),
      _ys(0),
      _zs(0),
      _position(false),
      _masterId(id),
      _lagDelayUs(2000)
{
}

DataAnchor::~DataAnchor()
{

}

DataAnchor::Type DataAnchor::type()
{
    return Anchor;
}

unsigned int DataAnchor::rowCount() const
{
    return _linkList.size();
}

unsigned int DataAnchor::columnCount() const
{
    return DataLink::ColumnCount;
}


QVariant DataAnchor::data(int column) const
{
    Q_ASSERT(column < ColumnCount);
    switch(column)
    {
    case ColumnID: return QString::number(_id, 16);
    case ColumnNumber: return _number;
    case ColumnX: return _x;
    case ColumnY: return _y;
    case ColumnZ: return _z;
    case ColumnMasterSlave: return _master;
    case ColumnSelected: return _selected;
    case ColumnDlyRx: return _antennaDlyRx;
    case ColumnDlyTx: return _antennaDlyTx;
    case ColumnMasterID: return _masterId;
    case ColumnLagDelay: return _lagDelayUs;
    case ColumnDim : return _dim;
    }

    return QVariant();
}

bool DataAnchor::setData(int column, const QVariant &data)
{
    Q_ASSERT(column < ColumnCount);
    switch(column)
    {
    case ColumnNumber: setNumber(data.toInt()); return true;
    case ColumnX: setX(data.toDouble()); return true;
    case ColumnY: setY(data.toDouble()); return true;
    case ColumnZ: setZ(data.toDouble()); return true;
    case ColumnMasterSlave: setMaster(data.toInt()); return true;
    case ColumnSelected: setSelected(data.toBool()); return true;
    case ColumnDlyRx: setAntennaDlyRx(data.toDouble()); return true;
    case ColumnDlyTx: setAntennaDlyTx(data.toDouble()); return true;
    case ColumnLagDelay: setLagDelayUs(data.toInt()); return true;
    case ColumnDim : setDim(data.toInt());return true;
    }
    return false;
}

DataAbstractItem *DataAnchor::child(int row)
{
    return _linkList.value(row, nullptr);
}

const DataAbstractItem *DataAnchor::child(int row) const
{
    return _linkList.value(row, nullptr);
}

bool DataAnchor::isEditable(int column) const
{
    Q_ASSERT(column < ColumnCount);
    switch(column)
    {
    case ColumnID:
        return false;

    case ColumnNumber:
    case ColumnX:
    case ColumnY:
    case ColumnZ:
    case ColumnMasterSlave:
    case ColumnSelected:
    case ColumnDlyRx:
    case ColumnDlyTx:
    case ColumnDim:
        return true;
    }

    return false;
}

void DataAnchor::setDim(int dim)
{
    _dim = dim;
    emit model() ->dataChanged(index(ColumnDim),index(ColumnDim));

}

void DataAnchor::setNumber(int x)
{
    _number = x;
    emit model()->dataChanged(index(ColumnNumber), index(ColumnNumber));
}

void DataAnchor::setX(double x)
{
    _x = x;
    anchorMoved();
    emit model()->dataChanged(index(ColumnX), index(ColumnX));
}

void DataAnchor::setY(double y)
{
    _y = y;
    anchorMoved();
    emit model()->dataChanged(index(ColumnY), index(ColumnY));
}

void DataAnchor::setZ(double z)
{
    _z = z;
    anchorMoved();
    emit model()->dataChanged(index(ColumnZ), index(ColumnZ));
}

void DataAnchor::setAntennaDlyRx(double dly)
{
    _antennaDlyRx = dly;
    emit model()->dataChanged(index(ColumnDlyRx), index(ColumnDlyRx));
}

void DataAnchor::setAntennaDlyTx(double dly)
{
    _antennaDlyTx = dly;
    emit model()->dataChanged(index(ColumnDlyTx), index(ColumnDlyTx));
}

void DataAnchor::setMasterId(uint64_t id)
{
    _masterId = id;
    emit model()->dataChanged(index(ColumnMasterID), index(ColumnMasterID));
}

void DataAnchor::setLagDelayUs(int delay)
{
    _lagDelayUs = delay;
    emit model()->dataChanged(index(ColumnLagDelay), index(ColumnLagDelay));
}

void DataAnchor::setCoordinates(double x, double y)
{
    _x = x;
    _y = y;
    anchorMoved();
    emit model()->dataChanged(index(ColumnX), index(ColumnY));
}

void DataAnchor::setCoordinates(double x, double y, double z)
{
    _x = x;
    _y = y;
    _z = z;
    anchorMoved();
    emit model()->dataChanged(index(ColumnX), index(ColumnZ));
}

void DataAnchor::setMaster(int master)
{
    _master = master;
    emit model()->dataChanged(this->index(ColumnMasterSlave), this->index(ColumnMasterSlave));
}

void DataAnchor::setSelected(bool selected)
{
    _selected = selected;
    emit model()->dataChanged(this->index(ColumnSelected), this->index(ColumnSelected));
}

void DataAnchor::setConnected(_connected_e connected)
{
    _connected = connected;
    emit model()->dataChanged(this->index(ColumnConnected), this->index(ColumnConnected));
}

void DataAnchor::addMasterAnchor(uint64_t id)
{
    //check if this ID is already in the list
    for(int m=0; m<_masteranchors.size(); m++)
    {
        //get the link if it exists...
        if(id == _masteranchors.at(m))
            return;
    }

    _masteranchors << id;
}

uint64_t DataAnchor::getMasterAnchorAddrAt(int i)
{
    Q_ASSERT(i < _masteranchors.size());

    return _masteranchors.at(i);
}

void DataAnchor::addReferencePair(uint64_t id, uint64_t tagid, double x, double y, double z, int offset, int blinks)
{
    ReferencePair rp;

    //check if this pair is already in the list
    for(int m=0; m<_refpairs.size(); m++)
    {
        rp.anchorid = _refpairs.at(m).anchorid;
        rp.tagid = _refpairs.at(m).tagid;

        //get the link if it exists...
        if((id == rp.anchorid) && (tagid == rp.tagid))
        {
            return;
        }

        //qDebug() << "link exists" << QString::number(rp.anchorid, 16) << QString::number(rp.tagid, 16);
    }

    //got here so lets add it
    rp.anchorid = id;
    rp.tagid = tagid;
    rp.offset = offset;
    rp.tagx = x;
    rp.tagy = y;
    rp.tagz = z;
    rp.blinks = blinks;

    //qDebug() << "add link" << QString::number(rp.anchorid, 16) << QString::number(rp.tagid, 16);
    //qDebug() << "add link" << QString::number(rp.tagx) << QString::number(rp.tagy) << QString::number(rp.tagz);

    _refpairs << rp;
}

ReferencePair DataAnchor::getReferencePairAt(int i)
{
    Q_ASSERT(i < _refpairs.size());

    return _refpairs.at(i);
}


DataLink * DataAnchor::link(DataAnchor *target)
{
    return _links.value(target->id());
}

DataLink *DataAnchor::addLink(DataAnchor *target, DataUndirectedLink *undirectedLink)
{
    DataLink *link = _links.value(target->id());
    Q_ASSERT(!link); // TODO: Can that happen ? How to handle it ?

    int row = _linkList.size();
    model()->beginInsertColumns(index(), row, row);
    link = new DataLink(model(), this, target, undirectedLink, row);
    _links.insert(target->id(), link);
    _linkList.append(link);
    model()->endInsertRows();

    return link;
}

void DataAnchor::anchorMoved()
{
    foreach (DataLink *l, _linkList)
    {
        l->undirectedLink()->updateDistance();
    }
}
