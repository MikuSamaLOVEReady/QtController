// -------------------------------------------------------------------------------------------------------------------
//
//  File: AnchorPropertiesWidget.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef ANCHORPROPERTIESWIDGET_H
#define ANCHORPROPERTIESWIDGET_H

#include <QWidget>
#include <QListWidget>

namespace Ui {
class AnchorPropertiesWidget;
}

class QDataWidgetMapper;
class QModelIndex;
class DataAnchor;

class AnchorPropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnchorPropertiesWidget(QWidget *parent = 0);
    ~AnchorPropertiesWidget();

public slots:
    void masterListClicked(QListWidgetItem *item);

protected slots:
    void onReady();
    void currentRowChanged(const QModelIndex & current, const QModelIndex & previous);

    void addPBClicked();
    void clearPBClicked();
    void configureClicked();
    void configureMasterClicked();
    void configureMasterList();
    void configureRole();
    void configureSlaveMasterList();
    void updateSelectedMaster(int index);

protected:
    //void mousePressEvent(QMouseEvent * event);

private:
    Ui::AnchorPropertiesWidget *ui;

    QDataWidgetMapper *_mapper;

    QList<QString> _masterlist ; //list of master anchors

    DataAnchor *_anch;

    bool _ignore;
};

#endif // ANCHORPROPERTIESWIDGET_H
