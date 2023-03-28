// -------------------------------------------------------------------------------------------------------------------
//
//  File: ModelInspectorWidget.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "ModelInspectorWidget.h"
#include "ui_ModelInspectorWidget.h"

#include "RTLSControllerApplication.h"
#include "DataModel.h"

#include <QSettings>

ModelInspectorWidget::ModelInspectorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModelInspectorWidget)
{
    ui->setupUi(this);

    QObject::connect(RTLSControllerApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(saveSettings()));

    RTLSControllerApplication::connectReady(this, "onReady()");
}

void ModelInspectorWidget::onReady()
{
    loadSettings();

    ui->anchorTableView->setModel(RTLSControllerApplication::model());

    QObject::connect(ui->anchorTableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(anchorSelected(QModelIndex,QModelIndex)));
}

ModelInspectorWidget::~ModelInspectorWidget()
{
    delete ui;
}

void ModelInspectorWidget::anchorSelected(const QModelIndex & current, const QModelIndex & previous)
{
    Q_UNUSED(previous);

    if (!ui->linksTableView->model())
        ui->linksTableView->setModel(RTLSControllerApplication::model());
    ui->linksTableView->setRootIndex(current);
}



void ModelInspectorWidget::loadSettings()
{
    QSettings s;
    s.beginGroup("ModelInspectorWidget");

    s.beginGroup("anchorHeader");
    ui->anchorTableView->horizontalHeader()->restoreState(s.value("state").toByteArray());
    ui->anchorTableView->horizontalHeader()->restoreGeometry(s.value("geometry").toByteArray());
    s.endGroup();

    s.beginGroup("linkHeader");
    ui->linksTableView->horizontalHeader()->restoreState(s.value("state").toByteArray());
    ui->linksTableView->horizontalHeader()->restoreGeometry(s.value("geometry").toByteArray());
    s.endGroup();

    s.endGroup();
}

void ModelInspectorWidget::saveSettings()
{
    QSettings s;
    s.beginGroup("ModelInspectorWidget");

    s.beginGroup("anchorHeader");
    s.setValue("state", ui->anchorTableView->horizontalHeader()->saveState());
    s.setValue("geometry", ui->anchorTableView->horizontalHeader()->saveGeometry());
    s.endGroup();

    s.beginGroup("linkHeader");
    s.setValue("state", ui->linksTableView->horizontalHeader()->saveState());
    s.setValue("geometry", ui->linksTableView->horizontalHeader()->saveGeometry());
    s.endGroup();

    s.endGroup();
}
