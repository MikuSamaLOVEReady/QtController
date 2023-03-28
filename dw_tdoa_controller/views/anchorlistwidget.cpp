// -------------------------------------------------------------------------------------------------------------------
//
//  File: AnchorListWidget.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "anchorlistwidget.h"
#include "ui_anchorlistwidget.h"

#include "RTLSControllerApplication.h"
#include "DataModel.h"
#include "DataAnchor.h"
#include "AnchorMenu.h"
#include "RTLSControl.h"

#include <QInputDialog>
#include <QDebug>

AnchorListWidget::AnchorListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnchorListWidget)
{
    ui->setupUi(this);

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);

    QObject::connect(ui->updateAnchor_pb, SIGNAL(clicked()), this, SLOT(updateAnchorButtonClicked())); //add a function for the Remove button click
    QObject::connect(ui->addAnchor_pb, SIGNAL(clicked()), this, SLOT(addAnchorButtonClicked())); //add a function for the Add button click
    QObject::connect(ui->listView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    RTLSControllerApplication::connectReady(this, "onReady()");
}

void AnchorListWidget::keyPressEvent(QKeyEvent *event) {
       if (event->matches(QKeySequence::SelectAll)) {
           event->ignore();
       } else {
          QWidget::keyPressEvent(event);
       }
}

void AnchorListWidget::onReady()
{
    //qDebug() << "AnchorListWidget set model" ;
    ui->listView->setModel(RTLSControllerApplication::model());
    ui->listView->selectionModel()->deleteLater();
    ui->listView->setSelectionModel(RTLSControllerApplication::anchorSelectionModel());
    ui->listView->setSelectionMode(RTLSControllerApplication::anchorSelectionMode());

    QObject::connect(RTLSControllerApplication::instance(), SIGNAL(selectionModeChanged(QAbstractItemView::SelectionMode)), this, SLOT(selectionModeChanged(QAbstractItemView::SelectionMode)));

}

AnchorListWidget::~AnchorListWidget()
{
    delete ui;
}


void AnchorListWidget::addAnchorButtonClicked()
{
    bool ok;
    QString idStr = QInputDialog::getText(this, "Anchor ID", "Anchor ID", QLineEdit::Normal, "", &ok); // TODO: add custom dialog with format checking ...
    if (ok)
    {
        uint64_t id = idStr.toULongLong(&ok, 16);
        if (ok)
        {
            DataAnchor *anchor = RTLSControllerApplication::model()->anchor(id, true);
            RTLSControllerApplication::anchorSelectionModel()->setCurrentIndex(anchor->index(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows); // Switch to the new anchor
            RTLSControllerApplication::control()->sendConfiguration(anchor);
        }
    }
}

void AnchorListWidget::updateAnchorButtonClicked()
{
   //Show all ?
   /*int count = ui->listView->children().count();

   //show all
   for(int i = 0; i<count; i++)
   {
      ui->listView->setRowHidden(i, false);
   }

   qDebug() << "update button SHOW all";
   */

   RTLSControllerApplication::control()->sendAnchorListReq();

}

void AnchorListWidget::updateAnchorConnected(int row, bool connected)
{
    //can hide the rows
    ui->listView->setRowHidden(row, !connected);
}

void AnchorListWidget::selectionModeChanged(QAbstractItemView::SelectionMode mode)
{
    ui->listView->setSelectionMode(mode);
}

void AnchorListWidget::showContextMenu(const QPoint &pos)
{
    AnchorMenu menu;
    menu.exec(ui->listView->mapToGlobal(pos));
}
