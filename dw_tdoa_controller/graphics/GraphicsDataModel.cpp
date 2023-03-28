// -------------------------------------------------------------------------------------------------------------------
//
//  File: GraphicsDataModel.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "GraphicsDataModel.h"
#include "DataModel.h"
#include "DataAnchor.h"
#include "GraphicsDataAnchor.h"
#include "DataLink.h"
#include "GraphicsDataLink.h"
#include <QGraphicsScene>
#include <QDebug>

GraphicsDataModel::GraphicsDataModel(QGraphicsScene *scene, QObject *parent)
    : QObject(parent),
      _scene(scene),
      _model(NULL),
      _selectionModel(NULL),
      _selectionMode(QAbstractItemView::SingleSelection)
{
    setModel(NULL);
}

DataModel *GraphicsDataModel::model()
{
    return (_model == DataModel::staticEmptyModel() ? 0 : _model);
}

void GraphicsDataModel::setModel(DataModel *model, QItemSelectionModel *selectionModel)
{
    //qDebug() << "GraphicsDataModel::setModel" ;

    if (_model)
    {
        //qDebug() << "model !NULL" << model->id();

        QObject::disconnect(_model, SIGNAL(modelReset()), this, SLOT(modelReset()));
        QObject::disconnect(_model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));
        //QObject::disconnect(_model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rowsRemoved(QModelIndex,int,int))); // TODO
        QObject::disconnect(_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex,QModelIndex)));
    }

    if (_selectionModel)
    {
        QObject::disconnect(_selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
    }

    _model = model;
    _selectionModel = selectionModel;
    _rootIndex = QModelIndex();

    if (_model == NULL)
        _model = DataModel::staticEmptyModel();
    if (_selectionModel == NULL)
        _selectionModel = new QItemSelectionModel(_model, this);

    //if(model != NULL) qDebug() << "connect model id" << model->id() << "to this GraphicsDataModel";

    QObject::connect(_model, SIGNAL(modelReset()), this, SLOT(modelReset()));
    QObject::connect(_model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));
    //QObject::connect(_model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rowsRemoved(QModelIndex,int,int))); // TODO
    QObject::connect(_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex,QModelIndex)));

    QObject::connect(_selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged(QItemSelection,QItemSelection)));

    modelReset();
}

void GraphicsDataModel::setRootIndex(const QModelIndex &root)
{
    if (!root.isValid())
        return;

    //qDebug() << "set root index/model reset" << _model->id();

    Q_ASSERT(root.model() == _model);
    _rootIndex = root;
    modelReset();
}

void GraphicsDataModel::rowsRemoved(const QModelIndex & parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);

    modelReset();
}

void GraphicsDataModel::rowsInserted(const QModelIndex & parent, int start, int end)
{
    if (parent != _rootIndex)
        return;

    //qDebug() << "GraphicsDataModel::rowsInserted - model id" << _model->id();

    for (int row = start; row <= end; row ++)
    {
        QPersistentModelIndex index = _model->index(row, 0, parent);

        //qDebug() << "index "  << index;

        if (DataAnchor *anchor = dynamic_cast<DataAnchor *>(_model->item(index)))
        {
            GraphicsDataAnchor *item = new GraphicsDataAnchor(anchor, this);
            _scene->addItem(item);
            _anchors.insert(index, item);
        }
        else if (DataLink *link = dynamic_cast<DataLink *>(_model->item(index)))
        {
            GraphicsDataLink *item = new GraphicsDataLink(link, this);
            _scene->addItem(item);
            _links.insert(index, item);
            _linkParents.insert(link->parent()->index(), item);
            _linkTargets.insert(link->target()->index(), item);
        }
    }
}

void GraphicsDataModel::dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
    for (int row = topLeft.row(); row <= bottomRight.row(); row ++)
    {
        QPersistentModelIndex index = _model->index(row, 0, topLeft.parent());
        if (GraphicsDataAnchor *anchor = _anchors.value(index))
        {
            anchor->modelChanged(topLeft.column(), bottomRight.column());
        }
        else if (GraphicsDataLink *link = _links.value(index))
        {
            link->modelChanged(topLeft.column(), bottomRight.column());
        }

        if (_model->item(index)->type() == DataAbstractItem::Anchor)
        {
            QMultiMap<QPersistentModelIndex, GraphicsDataLink *>::iterator it = _linkParents.find(index);
            for(;it != _linkParents.end();++it)
            {
                it.value()->parentModelChanged(topLeft.column(), bottomRight.column());
            }
            it = _linkTargets.find(index);
            for(;it != _linkTargets.end();++it)
            {
                it.value()->targetModelChanged(topLeft.column(), bottomRight.column());
            }
        }
    }
}

void GraphicsDataModel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    foreach (const QModelIndex &i, selected.indexes())
    {
        GraphicsDataAnchor *item = _anchors.value(i);
        if (item)
            item->setSelected(true);
    }

    foreach (const QModelIndex &i, deselected.indexes())
    {
        QGraphicsItem *item = _anchors.value(i);
        if (item)
            item->setSelected(false);
    }
}

void GraphicsDataModel::modelReset()
{
    foreach(GraphicsDataAnchor *anchor, _anchors)
    {
        _scene->removeItem(anchor);
        delete anchor;
    }
    foreach(GraphicsDataLink *link, _links)
    {
        _scene->removeItem(link);
        delete link;
    }

    _anchors.clear();
    _links.clear();
    _linkParents.clear();
    _linkTargets.clear();

    if (_model->hasChildren(_rootIndex))
        rowsInserted(_rootIndex, 0, _model->rowCount(_rootIndex));
}

