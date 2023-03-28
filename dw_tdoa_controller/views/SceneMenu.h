// -------------------------------------------------------------------------------------------------------------------
//
//  File: SceneMenu.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef SCENEMENU_H
#define SCENEMENU_H

#include <QMenu>

class SceneMenu : public QMenu
{
    Q_OBJECT
public:
    explicit SceneMenu(const QPointF &scenePos, QWidget *parent = 0);

protected slots:
    void onAddAnchor();

private:
    QPointF _scenePos;
};

#endif // SCENEMENU_H
