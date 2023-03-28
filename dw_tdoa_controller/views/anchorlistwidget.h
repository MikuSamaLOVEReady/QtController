// -------------------------------------------------------------------------------------------------------------------
//
//  File: AnchorListWidget.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef ANCHORLISTWIDGET_H
#define ANCHORLISTWIDGET_H

#include <QWidget>
#include <QListView>
#include <QMouseEvent>
#include <QAbstractItemView>
#include <DataAnchor.h>

class Anchor;

namespace Ui {
class AnchorListWidget;
}

class AnchorListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnchorListWidget(QWidget *parent = nullptr);
    ~AnchorListWidget();
    void updateAnchorConnected(int row, bool connected);
    virtual void keyPressEvent(QKeyEvent *event) override;

protected slots:
    void onReady();
    void addAnchorButtonClicked();
    void updateAnchorButtonClicked();
    void selectionModeChanged(QAbstractItemView::SelectionMode mode);
    void showContextMenu(const QPoint& pos);


private:
    Ui::AnchorListWidget *ui;
};

#endif // ANCHORLISTWIDGET_H
