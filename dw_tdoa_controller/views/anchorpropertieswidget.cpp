// -------------------------------------------------------------------------------------------------------------------
//
//  File: AnchorPropertiesWidget.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "anchorpropertieswidget.h"
#include "ui_anchorpropertieswidget.h"

#include "RTLSControllerApplication.h"
#include "RTLSControl.h"
#include "DataModel.h"
#include "DataAnchor.h"
#include "DataLink.h"

#include <QDataWidgetMapper>
#include <QStyledItemDelegate>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QtGui>

#define VALID_RF_DIST (0.01f) //in meters

AnchorPropertiesWidget::AnchorPropertiesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnchorPropertiesWidget)
{
    ui->setupUi(this);

    _mapper = new QDataWidgetMapper(this);

    _ignore = false;

    RTLSControllerApplication::connectReady(this, "onReady()");
}

void AnchorPropertiesWidget::onReady()
{
    QObject::connect(RTLSControllerApplication::anchorSelectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(currentRowChanged(QModelIndex,QModelIndex)));

    _mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    _mapper->setModel(RTLSControllerApplication::model());

    _mapper->addMapping(ui->anchorId_lb, DataAnchor::ColumnID, "text");
    _mapper->addMapping(ui->anchorNum_sb, DataAnchor::ColumnNumber);
    _mapper->addMapping(ui->xcoord_sb,   DataAnchor::ColumnX);
    _mapper->addMapping(ui->ycoord_sb,   DataAnchor::ColumnY);
    _mapper->addMapping(ui->zcoord_sb,   DataAnchor::ColumnZ);

    _mapper->addMapping(ui->selected_cb, DataAnchor::ColumnSelected, "checked");
    _mapper->addMapping(ui->antennaDlyRx,  DataAnchor::ColumnDlyRx);
    _mapper->addMapping(ui->antennaDlyTx,  DataAnchor::ColumnDlyTx);
    _mapper->addMapping(ui->lagDelayus,  DataAnchor::ColumnLagDelay);

    _mapper->addMapping(ui->dim_bx, DataAnchor::ColumnDim);

    QObject::connect(ui->selected_cb, SIGNAL(clicked()), _mapper, SLOT(submit())); // Bug with QDataWidgetMapper (QTBUG-1818)

    QObject::connect(ui->configure, SIGNAL(clicked()), this, SLOT(configureClicked()));

    QObject::connect(ui->mastersList, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSelectedMaster(int)));

    QObject::connect(ui->addPB, SIGNAL(clicked()), this, SLOT(addPBClicked()));
    QObject::connect(ui->clearPB, SIGNAL(clicked()), this, SLOT(clearPBClicked()));

    QObject::connect(ui->slave_cb, SIGNAL(clicked()), this, SLOT(configureMasterClicked()));
    QObject::connect(ui->master_cb, SIGNAL(clicked()), this, SLOT(configureMasterClicked()));
    QObject::connect(ui->primarymaster_cb, SIGNAL(clicked()), this, SLOT(configureMasterClicked()));

    QObject::connect(ui->masterList, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(masterListClicked(QListWidgetItem * )));

    // Set to not-selected
    currentRowChanged(QModelIndex(), QModelIndex());
    ui->configure->setEnabled(false);   
}

AnchorPropertiesWidget::~AnchorPropertiesWidget()
{
    delete ui;
    delete _mapper;
}

void AnchorPropertiesWidget::addPBClicked()
{
    bool ok ;
    QString anchId = ui->anchorId_lb->text();
    uint64_t anchIdNum = anchId.toULongLong( &ok, 16);
    _anch = RTLSControllerApplication::model()->anchor(anchIdNum, false);
    //need to open a combo box in which the user can select a master anchor to add to the list

    QString item = QInputDialog::getItem(nullptr, tr("Select master anchor"),
                                         tr("Master:"), _masterlist, 0, false, &ok);
    if (ok && !item.isEmpty())
    {
        uint64_t id = item.toLongLong(&ok, 16);
        if(id != anchIdNum) //if master can't add itself
        {
            _anch->addMasterAnchor(id);

            if (ok)
                configureSlaveMasterList();
        }
    }
}

void AnchorPropertiesWidget::masterListClicked(QListWidgetItem * pItem)
{
    bool ok ;
    bool enter = 0;
    QString anchId = ui->anchorId_lb->text();
    uint64_t anchIdNum = anchId.toULongLong( &ok, 16);
    _anch = RTLSControllerApplication::model()->anchor(anchIdNum, false);

    if(!_anch) return;

    if(enter == 0)
    {
        if(pItem)
        {
            double rfDistance;
            DataAnchor *master = nullptr;
            DataAnchor *a = nullptr;
            DataUndirectedLink *undirectedLink = nullptr;
            uint64_t id = pItem->text().toULongLong( &ok, 16);

            enter = 1;
            a = RTLSControllerApplication::model()->anchor(anchIdNum, true);
            master = RTLSControllerApplication::model()->anchor(id, true);

            if(id < anchIdNum)
                undirectedLink = RTLSControllerApplication::model()->link(master, a, true);
            else
                undirectedLink = RTLSControllerApplication::model()->link(a, master, true);

            if (undirectedLink)
            {
                rfDistance = undirectedLink->rfDistance();
            }

            qDebug() << "RF distance = " << rfDistance;

            if(rfDistance > VALID_RF_DIST) //RF distance exists - ask if this should be used
            {
                //we need to set RF distance flag so it will be sent to the CLE
                if(pItem->checkState() == Qt::Checked)
                {
                    undirectedLink->setUseRFDistance(1);
                    //QMessageBox::warning(nullptr, tr("RF distance"), "Using RF distance to master for sync.");
                }
                else
                {
                    undirectedLink->setUseRFDistance(0);
                    //QMessageBox::warning(nullptr, tr("RF distance"), "Using geometric distance to master for sync.");
                }
            }
            else
            {
                pItem->setCheckState(Qt::Unchecked);
                //cannot set checkbox
                QMessageBox::warning(nullptr, tr("RF distance"), "RF distance to master is not valid.");
            }
        }
        enter = 0;
    }
}

void AnchorPropertiesWidget::clearPBClicked()
{
    bool ok ;
    QString anchId = ui->anchorId_lb->text();
    uint64_t anchIdNum = anchId.toULongLong( &ok, 16);
    _anch = RTLSControllerApplication::model()->anchor(anchIdNum, false);

    //remove all master anchor IDs from the list
    ui->masterList->clear();

   if(!_anch) return;

   _anch->clearMasterAnchors();
}

void AnchorPropertiesWidget::configureClicked()
{
    //send the latest anchor configuration to the CLE
    RTLSControllerApplication::control()->sendConfiguration();
}

void AnchorPropertiesWidget::updateSelectedMaster(int index)
{
    if(!_ignore)
    {
        if((_masterlist.size() > index) && (index >= 0))
        {
            bool ok;
            _anch->setMasterId(_masterlist.at(index).toLongLong(&ok, 16));
        }
    }
}

void AnchorPropertiesWidget::configureSlaveMasterList()
{
    bool ok ;
    bool rfDistValid = false;
    int rfDistValidUse = 0;
    //QStandardItem* item;
    QListWidgetItem* item;
    QString anchId = ui->anchorId_lb->text();
    uint64_t anchIdNum = anchId.toULongLong( &ok, 16);
    _anch = RTLSControllerApplication::model()->anchor(anchIdNum, false);

    //remove all
    ui->masterList->clear();

    if(!_anch) return;

    qDebug() << "anchor master list: " << _anch->masterAnchorListSize();
    //add new items to the combobox
    for(int m=0; m<_anch->masterAnchorListSize(); m++)
    {
        //for each slave - master combo, check if RF distance exists
        rfDistValid = false;
        {
            float rfDistance = VALID_RF_DIST;
            DataAnchor *master = nullptr;
            DataUndirectedLink *undirectedLink = nullptr;

            master = RTLSControllerApplication::model()->anchor(_anch->getMasterAnchorAddrAt(m), true);

            if(_anch->getMasterAnchorAddrAt(m) < anchIdNum)
                undirectedLink = RTLSControllerApplication::model()->link(master, _anch, true);
            else
                undirectedLink = RTLSControllerApplication::model()->link(_anch, master, true);

            if (undirectedLink)
            {
                rfDistance = static_cast<float>(undirectedLink->rfDistance());
            }

            if(rfDistance > VALID_RF_DIST) //RF distance exists - ask if this should be used
            {
                rfDistValid = true;
                rfDistValidUse = undirectedLink->rfUseDistance();
            }
        }

        item = new QListWidgetItem();
        if(rfDistValidUse == 1)
        {
            item->setCheckState(Qt::Checked);
        }
        else
        {
            item->setCheckState(Qt::Unchecked);
        }
        if(rfDistValid)
        {
            item->setToolTip(QString("Can use RF distance."));
        }
        else
        {
            item->setToolTip(QString("NO RF distance."));
        }

        item->setFlags((item->flags() | Qt::ItemIsUserCheckable) | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setText(QString::number(_anch->getMasterAnchorAddrAt(m), 16));
        item->setTextAlignment(Qt::AlignHCenter);
        ui->masterList->addItem(item);
        //qDebug() << "anchor master list: Item " << m << item->text();
    }
}

void AnchorPropertiesWidget::configureRole()
{
    bool ok ;
    QString anchId = ui->anchorId_lb->text();
    uint64_t anchIdNum = anchId.toULongLong( &ok, 16);
    _anch = RTLSControllerApplication::model()->anchor(anchIdNum, false);

    ui->master_cb->setChecked(false);
    ui->primarymaster_cb->setChecked(false);
    ui->slave_cb->setChecked(false);

    if (_anch->master() == PRIMARY_MASTER)
    {
        ui->primarymaster_cb->setChecked(true);
    }
    else if(_anch->master() == SECONDARY_MASTER)
    {
        ui->master_cb->setChecked(true);
    }
    else
    {
        ui->slave_cb->setChecked(true);
    }
}


void AnchorPropertiesWidget::configureMasterList()
{
    bool ok ;
    QString anchId = ui->anchorId_lb->text();
    uint64_t anchIdNum = anchId.toULongLong( &ok, 16);
    _anch = RTLSControllerApplication::model()->anchor(anchIdNum, false);

    _ignore = true;

    //populate the list of other masters (including self)
    const QList<DataAnchor *> &anchors = RTLSControllerApplication::model()->anchors();
    //first check if at least one master anchor (for REK there should only be one)
    if (!anchors.size())
        return;

    _masterlist.clear();

    if(_anch->master() > 0) //only add to list if master anchor
        _masterlist << anchId;

    foreach(DataAnchor *a, anchors)
    {
        if(a->id() == anchIdNum)
            continue; //skip self

        if(a->master() > 0) //if this anchor has been selected to be used in this RTLS/this CLE
        {
            _masterlist << (QString::number(a->id(), 16));
        }
    }

    //remove all items from the combobox
    while(ui->mastersList->count())
    {
        ui->mastersList->removeItem(0);
    }

    //add new items to the combobox
    ui->mastersList->addItems(_masterlist);

    for (int i = 0; i < ui->mastersList->count(); ++i)
    {
        uint64_t id = _masterlist.at(i).toULongLong( &ok, 16);
        if ( id == _anch->masterId())
        {
            ui->mastersList->setCurrentIndex(i);
        }
    }

    ui->primaryMaster->setEnabled(_anch->master()>0);

    _ignore = false;
}

void AnchorPropertiesWidget::configureMasterClicked()
{
    bool ok ;
    QString anchId = ui->anchorId_lb->text();
    uint64_t anchIdNum = anchId.toULongLong( &ok, 16);
    _anch = RTLSControllerApplication::model()->anchor(anchIdNum, false);

    configureMasterList();

    if (ui->master_cb->isChecked())
    {//enable the primary master selection box
        ui->primaryMaster->setEnabled(true);
        _anch->setMaster(SECONDARY_MASTER);
    }
    else if (ui->primarymaster_cb->isChecked())
    {//disable the primary master selection box
        ui->primaryMaster->setEnabled(false);
        _anch->setMaster(PRIMARY_MASTER);
        _anch->setMasterId(0);
    }
    else
    {//disable master selection box for slave
        ui->primaryMaster->setEnabled(false);
        _anch->setMaster(SLAVE);
    }

}

void AnchorPropertiesWidget::currentRowChanged(const QModelIndex & current, const QModelIndex & previous)
{
    Q_UNUSED(previous)

    if (current.isValid())
    {
        ui->anchorNum_sb->setEnabled(true);
        ui->xcoord_sb->setEnabled(true);
        ui->ycoord_sb->setEnabled(true);
        ui->zcoord_sb->setEnabled(true);
        ui->dim_bx->setEnabled(true);

        ui->selected_cb->setEnabled(true);
        ui->antennaDlyRx->setEnabled(true);
        ui->antennaDlyTx->setEnabled(true);
        ui->label_d->setEnabled(true);
        ui->label_d_2->setEnabled(true);
        ui->anchorRole->setEnabled(true);
        ui->label_x->setEnabled(true);
        ui->label_y->setEnabled(true);
        ui->label_z->setEnabled(true);
        ui->configure->setEnabled(true);
        ui->antennaDly->setEnabled(true);
        ui->masterList->setEnabled(true);
        ui->addPB->setEnabled(true);
        ui->clearPB->setEnabled(true);
    } else {
        ui->anchorId_lb->setText("No Anchor selected.");
        ui->anchorNum_sb->setEnabled(false);
        ui->xcoord_sb->setEnabled(false);
        ui->ycoord_sb->setEnabled(false);
        ui->zcoord_sb->setEnabled(false);
        ui->dim_bx->setEnabled(false);

        ui->selected_cb->setEnabled(false);

        ui->antennaDlyRx->setEnabled(false);
        ui->antennaDlyTx->setEnabled(false);
        ui->label_d->setEnabled(false);
        ui->label_d_2->setEnabled(false);
        ui->anchorRole->setEnabled(false);
        ui->label_x->setEnabled(false);
        ui->label_y->setEnabled(false);
        ui->label_z->setEnabled(false);
        ui->configure->setEnabled(false);
        ui->antennaDly->setEnabled(false);
        ui->primaryMaster->setEnabled(false);
        ui->masterList->setEnabled(false);
        ui->addPB->setEnabled(false);
        ui->clearPB->setEnabled(false);
    }

    _mapper->setCurrentModelIndex(current);

    if (current.isValid())
    {
        configureRole();
        configureMasterClicked();
        //configureMasterList();
        configureSlaveMasterList();
    }
}
