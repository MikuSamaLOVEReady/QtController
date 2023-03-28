// -------------------------------------------------------------------------------------------------------------------
//
//  File: GraphicsDataModel.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef GRAPHICSDATAMODEL_H
#define GRAPHICSDATAMODEL_H

#include <QObject>

#include <QAbstractItemView>
#include <QPersistentModelIndex>

class QModelIndex;
class QGraphicsScene;
class DataModel;
class QItemSelectionModel;
class GraphicsDataAnchor;
class GraphicsDataLink;

/**
 * The GraphicsDataModel class maps items from a DataModel into a QGraphicsScene.
 *
 * Whenever a item is inserted into the model, the GraphicsDataModel creates a GraphicsDataAnchor or GraphicsDataLink, which ever is appropriate.\n
 * It updates existing GraphicsDataItem whenever the model changes, by calling GraphicsDataItem::modelChanged().
 * Updates in the other direction are handled directly by individual GraphicsDataItem%s (@see GraphicsDataAnchor::itemChange()).
 *
 * The GraphicsDataModel supports only one level inside the model.
 * If the root index corresponds to the DataRoot, then only the anchors will be mapped. If the root index corresponds to a DataAnchor, only links of this anchor will be mapped.\n
 * Multiple instances of GraphicsDataAnchor can be used with a single scene and model to show multiple levels of items.
 */
class GraphicsDataModel : public QObject
{
    Q_OBJECT
public:
    explicit GraphicsDataModel(QGraphicsScene *scene, QObject *parent = 0);

    DataModel *model();
    QModelIndex rootIndex() { return _rootIndex; }
    QItemSelectionModel *selectionModel() { return _selectionModel; }
    QAbstractItemView::SelectionMode selectionMode() { return _selectionMode; }

signals:

public slots:
    /**
     * Change the model and selection model this instance maps into the scene.
     * If \a selectionModel is NULL, a new one is allocated.
     * @param model the new model
     * @param selectionModel the new selection model, or NULL
     */
    void setModel(DataModel *model, QItemSelectionModel *selectionModel = NULL);

    /**
     * Change the root index.
     * Only child of that index will be mapped by the GraphicsDataModel.
     *
     * When this function is called, previously mapped items are removed,
     * @param root
     */
    void setRootIndex(const QModelIndex &root);

    /**
     * Change the selection mode.
     * Only QAbstractItemView::SingleSelection and QAbstractItemView::ExtendedSelection modes are supported.
     * @param mode the new seection mode.
     */
    void setSelectionMode(QAbstractItemView::SelectionMode mode) { _selectionMode = mode; }

protected slots:
    void rowsInserted(const QModelIndex & parent, int start, int end);
    void dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void modelReset();

    void rowsRemoved(const QModelIndex & parent, int start, int end);

private:
    QHash<QPersistentModelIndex, GraphicsDataAnchor *> _anchors;
    QHash<QPersistentModelIndex, GraphicsDataLink *> _links;
    QMultiMap<QPersistentModelIndex, GraphicsDataLink *> _linkParents;
    QMultiMap<QPersistentModelIndex, GraphicsDataLink *> _linkTargets;

    QGraphicsScene *_scene;
    DataModel *_model;
    QPersistentModelIndex _rootIndex;
    QItemSelectionModel *_selectionModel;
    QAbstractItemView::SelectionMode _selectionMode;
};

#endif // GRAPHICSDATAMODEL_H
