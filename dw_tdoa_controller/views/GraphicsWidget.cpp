// -------------------------------------------------------------------------------------------------------------------
//
//  File: GraphicsWidget.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "GraphicsWidget.h"
#include "ui_graphicswidget.h"

#include "RTLSControllerApplication.h"
#include "DataModel.h"
#include "GraphicsDataModel.h"
#include "ViewSettings.h"
#include "RTLSClient.h"

#include <QDomDocument>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QGraphicsRectItem>
#include <QDebug>
#include <QInputDialog>
#include <QFile>
#include <QPen>

#include "rtls_interface.h"

#include <math.h>

GraphicsWidget::GraphicsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GraphicsWidget)
{
    ui->setupUi(this);

    this->scene = new QGraphicsScene(this);
    this->anchorGraphicsModel = new GraphicsDataModel(this->scene, this);
    this->anchorGraphicsModel->setModel(RTLSControllerApplication::model(), RTLSControllerApplication::anchorSelectionModel());

    this->linksGraphicsModel = new GraphicsDataModel(this->scene);

    ui->graphicsView->setScene(this->scene);
    ui->graphicsView->scale(1, -1);

	//tagTable
    //Tag ID, x, y, z, r95, vbat? ...
    QStringList tableHeader;
    tableHeader << "Tag ID/Label" << "X" << "Y" << "Z" << "Blinks\n(%)" << "Loc\n(%)" << "R95";
    ui->tagTable->setColumnCount(8);
    ui->tagTable->setHorizontalHeaderLabels(tableHeader);

    ui->tagTable->setColumnWidth(ColumnID,145);    //ID
    ui->tagTable->setColumnWidth(ColumnX,55); //x
    ui->tagTable->setColumnWidth(ColumnY,55); //y
    ui->tagTable->setColumnWidth(ColumnZ,55); //z
    ui->tagTable->setColumnWidth(ColumnBlinkRx,70); //% Blinks RX
    ui->tagTable->setColumnWidth(ColumnLocations,70); //% Multilaterations Success
    ui->tagTable->setColumnWidth(ColumnR95,70); //R95

    ui->tagTable->setColumnWidth(ColumnIDr,0); //ID raw hex
    ui->tagTable->setColumnHidden(ColumnIDr, true); //ID raw hex


    //set defaults
    tagSize = 0.07;
    historyLength = 20;
    showHistory = true;

    loadConfigFile("./CCtag_config.xml");

    _busy = true ;
    _ignore = true;

    selectedTagIdx = -1;
    RTLSControllerApplication::connectReady(this, "onReady()");
}

void GraphicsWidget::onReady()
{
    this->anchorGraphicsModel->setSelectionMode(RTLSControllerApplication::anchorSelectionMode());
    QObject::connect(RTLSControllerApplication::instance(), SIGNAL(selectionModeChanged(QAbstractItemView::SelectionMode)),
                     this->anchorGraphicsModel, SLOT(setSelectionMode(QAbstractItemView::SelectionMode)));
    QObject::connect(RTLSControllerApplication::anchorSelectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                     this, SLOT(onAnchorSelected(QModelIndex,QModelIndex)));

    QObject::connect(ui->tagTable, SIGNAL(cellChanged(int, int)), this, SLOT(tagTableChanged(int, int)));
    QObject::connect(ui->tagTable, SIGNAL(cellClicked(int, int)), this, SLOT(tagTableClicked(int, int)));
    QObject::connect(ui->tagTable, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));

    _busy = false ;
}

GraphicsWidget::~GraphicsWidget()
{
    delete scene;
    delete ui;
}


GraphicsView *GraphicsWidget::graphicsView()
{
    return ui->graphicsView;
}


void GraphicsWidget::loadConfigFile(QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug(qPrintable(QString("Error: Cannot read file %1 %2").arg(filename).arg(file.errorString())));
        return;
    }

    QDomDocument doc;
    QString error;
    int errorLine;
    int errorColumn;

    if(doc.setContent(&file, false, &error, &errorLine, &errorColumn))
    {
        qDebug() << "file error !!!" << error << errorLine << errorColumn;
    }

    QDomElement config = doc.documentElement();

    if( config.tagName() == "config" )
    {
        QDomNode n = config.firstChild();
        while( !n.isNull() )
        {
            QDomElement e = n.toElement();
            if( !e.isNull() )
            {
                if( e.tagName() == "tag_cfg" )
                {
                    tagSize = (e.attribute( "size", "" )).toDouble();
                    historyLength = (e.attribute( "history", "" )).toInt();
                }
                else
                if( e.tagName() == "tag" )
                {
                    bool ok;
                    quint64 id = (e.attribute( "ID", "" )).toULongLong(&ok, 16);
                    QString label = (e.attribute( "label", "" ));

                    taglabels.insert(id, label);
                }
            }

            n = n.nextSibling();
        }

    }

    file.close();
}

QDomElement TagToNode( QDomDocument &d, quint64 id, QString label )
{
    QDomElement cn = d.createElement( "tag" );
    cn.setAttribute("ID", QString::number(id, 16));
    cn.setAttribute("label", label);
    return cn;
}

void GraphicsWidget::saveConfigFile(QString filename)
{
    QFile file( filename );

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug(qPrintable(QString("Error: Cannot read file %1 %2").arg("./CCtag_config.xml").arg(file.errorString())));
        return;
    }

    QDomDocument doc;

    // Adding tag config root
    QDomElement config = doc.createElement("config");
    doc.appendChild(config);

    QDomElement cn = doc.createElement( "tag_cfg" );
    cn.setAttribute("size", QString::number(tagSize));
    cn.setAttribute("history", QString::number(historyLength));
    config.appendChild(cn);

    //update the map
    QMap<quint64, QString>::iterator i = taglabels.begin();

    while (i != taglabels.end())
    {
        config.appendChild( TagToNode(doc, i.key(), i.value()) );

        i++;
    }

    QTextStream ts( &file );
    ts << doc.toString();

    file.close();

    qDebug() << doc.toString();
}

void GraphicsWidget::clearTags(void)
{
    qDebug() << "table rows " << ui->tagTable->rowCount();

    while (ui->tagTable->rowCount())
    {
        QTableWidgetItem* item = ui->tagTable->item(0, ColumnIDr);

        if(item)
        {
            bool ok;
            quint64 tagID = item->text().toULongLong(&ok, 16);
            //clear scene from any tags
            Tag *tag = tags.value(tagID, nullptr);
            if(tag->r95p) //remove R95
            {
                //re-size the elipse... with a new rad value...
                tag->r95p->setOpacity(0); //hide it

                this->scene->removeItem(tag->r95p);
                delete(tag->r95p);
                tag->r95p = nullptr;
            }
            if(tag->avgp) //remove average
            {
                //re-size the elipse... with a new rad value...
                tag->avgp->setOpacity(0); //hide it

                this->scene->removeItem(tag->avgp);
                delete(tag->avgp);
                tag->avgp = nullptr;
            }
            if(tag->_tagLabel) //remove label
            {
                //re-size the elipse... with a new rad value...
                tag->_tagLabel->setOpacity(0); //hide it

                this->scene->removeItem(tag->_tagLabel);
                delete(tag->_tagLabel);
                tag->_tagLabel = nullptr;
            }

            //remove history...
            for(int idx=0; idx<historyLength; idx++ )
            {
                QAbstractGraphicsShapeItem *tag_p = tag->p[idx];
                if(tag_p)
                {
                    tag_p->setOpacity(0); //hide it

                    this->scene->removeItem(tag_p);
                    delete(tag_p);
                    tag_p = nullptr;
                    tag->p[idx] = 0;

                    qDebug() << "hist remove tag " << idx;
                }
            }
            {
                QMap<quint64, Tag*>::iterator i = tags.find(tagID);

                if(i != tags.end()) tags.erase(i);
            }
        }
        ui->tagTable->removeRow(0);
    }

    //clear tag table
    ui->tagTable->clearContents();

    qDebug() << "clear tags/tag table";

}

void GraphicsWidget::itemSelectionChanged(void)
{
    QList <QTableWidgetItem *>  l = ui->tagTable->selectedItems();
}

void GraphicsWidget::tagTableChanged(int r, int c)
{
    if(!_ignore)
    {
        Tag *tag = nullptr;
        bool ok;
        quint64 tagId = (ui->tagTable->item(r,ColumnIDr)->text()).toULongLong(&ok, 16);
        tag = tags.value(tagId, nullptr);

        if(!tag) return;

        if(c == ColumnID) //label has changed
        {
            QString newLabel = ui->tagTable->item(r,ColumnID)->text();

            tag->tagLabelStr = newLabel;
            tag->_tagLabel->setText(newLabel);

            //update the map
            QMap<quint64, QString>::iterator i = taglabels.find(tagId);

            if(i == taglabels.end()) //did not find the label
            {
                //insert the new value
                taglabels.insert(tagId, newLabel);
            }
            else //if (i != taglabels.end()) // && i.key() == tagId)
            {
                taglabels.erase(i); //erase the key
                taglabels.insert(tagId, newLabel);
            }
        }
    }
}

void GraphicsWidget::tagTableClicked(int r, int c)
{
    Tag *tag = nullptr;
    bool ok;
    quint64 tagId = (ui->tagTable->item(r,ColumnIDr)->text()).toULongLong(&ok, 16);
    tag = tags.value(tagId, nullptr);

    selectedTagIdx = r;

    if(!tag) return;

    if(c == ColumnR95) //toggle R95 display
    {
        QTableWidgetItem *pItem = ui->tagTable->item(r, c);
        tag->r95show = (pItem->checkState() == Qt::Checked) ? true : false;
    }

    if(c == ColumnID) //toggle label
    {
        QTableWidgetItem *pItem = ui->tagTable->item(r, c);
        tag->showLabel = (pItem->checkState() == Qt::Checked) ? true : false;

        tag->_tagLabel->setOpacity(tag->showLabel ? 1.0 : 0.0);
    }

}

/**
 * @fn    tagIDtoString
 * @brief  convert hex Tag ID to string (preappend 0x)
 *
 * */
void GraphicsWidget::tagIDtoString(quint64 tagId, QString *t)
{
    *t = "0x"+QString::number(tagId, 16);
}

int GraphicsWidget::findTagRowIndex(QString &t)
{
    for (int ridx = 0 ; ridx < ui->tagTable->rowCount() ; ridx++ )
    {
        QTableWidgetItem* item = ui->tagTable->item(ridx, ColumnIDr);
        if(item->text() == t)
        {
            return ridx;
        }
    }

    return -1;
}

/**
 * @fn    insertTag
 * @brief  insert Tag into the tagTable at row ridx
 *
 * */
void GraphicsWidget::insertTag(int ridx, QString &t, bool showR95, bool showLabel, QString &l)
{
    _ignore = true;

    ui->tagTable->insertRow(ridx);
    for( int col = ColumnID ; col < ui->tagTable->columnCount(); col++)
    {
        QTableWidgetItem* item = new QTableWidgetItem();
        QTableWidgetItem *pItem = new QTableWidgetItem();
        if(col == ColumnID )
        {
            if(showLabel)
            {
                pItem->setCheckState(Qt::Checked);
                pItem->setText(l);
            }
            else
            {
                pItem->setCheckState(Qt::Unchecked);
                pItem->setText(t);
            }
            item->setFlags((item->flags() ^ Qt::ItemIsEditable) | Qt::ItemIsSelectable);
            //ui->tagTable->setItem(ridx, col, item);
            ui->tagTable->setItem(ridx, col, pItem);
        }
        else
        {
            if(col == ColumnIDr)
            {
                item->setText(t);
            }

            item->setFlags((item->flags() ^ Qt::ItemIsEditable) | Qt::ItemIsSelectable);
            ui->tagTable->setItem(ridx, col, item);
        }

        if(col == ColumnR95)
        {
            if(showR95)
            {
                pItem->setCheckState(Qt::Checked);
            }
            else
            {
                pItem->setCheckState(Qt::Unchecked);
            }
            pItem->setText(" ");
            ui->tagTable->setItem(ridx,col,pItem);
        }
   }

   _ignore = false; //we've added a row
}

void GraphicsWidget::onAnchorSelected(const QModelIndex & current, const QModelIndex & previous)
{
    Q_UNUSED(previous);

    if (!this->linksGraphicsModel->model())
        this->linksGraphicsModel->setModel(RTLSControllerApplication::model());
    this->linksGraphicsModel->setRootIndex(current);
}

/**
 * @fn    addNewTag
 * @brief  add new Tag with tagId into the tags QMap
 *
 * */
void GraphicsWidget::addNewTag(quint64 tagId)
{
    Tag *tag;
    QString taglabel = taglabels.value(tagId, nullptr);

    qDebug() << "Add new Tag: 0x" + QString::number(tagId, 16);

    //insert into table, and create an array to hold history of its positions
    tags.insert(tagId,new(Tag));
    tag = this->tags.value(tagId, nullptr);
    tag->p.resize(historyLength);


    tag->tsprev = 0;
    tag->r95show = false;
    tag->showLabel = (taglabel != nullptr) ? true : false;
    tag->tagLabelStr = taglabel;
    //add ellipse for the R95 - set to transparent until we get proper r95 data
    tag->r95p = this->scene->addEllipse(-0.1, -0.1, 0, 0);
    tag->r95p->setOpacity(0);
    tag->r95p->setPen(Qt::NoPen);
    tag->r95p->setBrush(Qt::NoBrush);
    //add ellipse for average point - set to transparent until we get proper average data
    tag->avgp = this->scene->addEllipse(-0.025, -0.025, 0.05, 0.05);
    tag->avgp->setBrush(Qt::NoBrush);
    tag->avgp->setPen(Qt::NoPen);
    //add text label and hide until we see if user has enabled showLabel
    {
        tag->_tagLabel = new QGraphicsSimpleTextItem(nullptr);
        tag->_tagLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        tag->_tagLabel->setZValue(1);
        tag->_tagLabel->setText(taglabel);
        this->scene->addItem(tag->_tagLabel);
    }

}

/**
 * @fn    tagPos
 * @brief  update tag position on the screen (add to scene if it does not exist)
 *
 * */
void GraphicsWidget::tagPos(quint64 tagId, double x, double y, double z)
{
    static float h = 0.1;
    float s = 0.5, v = 0.98;

    if(_busy)
    {
        qDebug() << "(Widget - busy IGNORE) Tag: 0x" + QString::number(tagId, 16) << " " << x << " " << y << " " << z;
    }
    else
    {
        Tag *tag = nullptr;
        QString t ;

        _busy = true ;
        tagIDtoString(tagId, &t); //convert uint64 to string

        tag = tags.value(tagId, nullptr);

        if(!tag) //add new tag to the tags array
        {
            //tag does not exist, so create a new one
            addNewTag(tagId);
            tag = this->tags.value(tagId, nullptr);
        }

        if(!tag->p[tag->idx]) //we have not added this object yet to the history array
        {
            QAbstractGraphicsShapeItem *tag_pt = this->scene->addEllipse(-1*tagSize/2, -1*tagSize/2, tagSize, tagSize);
            tag->p[tag->idx] = tag_pt;
            tag_pt->setPen(Qt::NoPen);

            if(tag->idx > 0) //use same brush settings for existing tag ID
            {
                tag_pt->setBrush(tag->p[0]->brush());
            }
            else //new brush... new tag ID as idx = 0
            {
                QBrush b = QBrush(QColor::fromHsvF(h, s, v));

                //set the brush colour (average point is darker of the same colour
                tag_pt->setBrush(b);
                tag->avgp->setBrush(b.color().darker());
                tag->_tagLabel->setBrush(b.color().dark());
            }

            tag_pt->setToolTip(t);

            //update colour for next tag
            if(tag->idx == 0) //keep same colour for same tag ID
            {
                h+=0.618034;
                if (h>=1)
                    h-= 1;

                tag->ridx = findTagRowIndex(t);

                if(tag->ridx == -1)
                {
                    tag->ridx = ui->tagTable->rowCount();

                    insertTag(tag->ridx, t, tag->r95show, tag->showLabel, tag->tagLabelStr);
                }
            }           
        }

        ui->tagTable->item(tag->ridx,ColumnX)->setText(QString::number(x, 'f', 3) + " m");
        ui->tagTable->item(tag->ridx,ColumnY)->setText(QString::number(y, 'f', 3) + " m");
        ui->tagTable->item(tag->ridx,ColumnZ)->setText(QString::number(z, 'f', 3) + " m");

        tag->p[tag->idx]->setPos(x, y);

        if(showHistory)
        {
            tagHistory(tagId);
            tag->idx = (tag->idx+1)%historyLength;
        }
        else
        {
            //index will stay at 0
            tag->p[tag->idx]->setOpacity(1);
        }

        tag->_tagLabel->setPos(x + 0.15, y + 0.15);
        _busy = false ;

        //qDebug() << "Tag: 0x" + QString::number(tagId, 16) << " " << x << " " << y << " " << z;
	}
}


void GraphicsWidget::tagStats(quint64 tagId, double x, double y, double z, double r95)
{
    if(_busy)
    {
        qDebug() << "(busy IGNORE) R95: 0x" + QString::number(tagId, 16) << " " << x << " " << y << " " << z << " " << r95;
    }
    else
    {
        Tag *tag = nullptr;

        _busy = true ;

        tag = tags.value(tagId, nullptr);

        if(!tag) //add new tag to the tags array
        {
            addNewTag(tagId);
            tag = this->tags.value(tagId, nullptr);
        }

        if (tag)
        {
            //static float h = 0.1;
            //float s = 0.5, v = 0.98;
            double rad = r95*2;

            if(tag->r95p)
            {
                //re-size the elipse... with a new rad value...
                tag->r95p->setOpacity(0); //hide it

                this->scene->removeItem(tag->r95p);
                delete(tag->r95p);
                tag->r95p = nullptr;
            }

            //else //add r95 circle

            {
                //add R95 circle
                tag->r95p = this->scene->addEllipse(-1*r95, -1*r95, rad, rad);
                tag->r95p->setPen(Qt::NoPen);
                tag->r95p->setBrush(Qt::NoBrush);

                if( tag->r95show && (rad <= 1))
                {
                    QPen pen = QPen(Qt::darkGreen);
                    pen.setStyle(Qt::DashDotDotLine);
                    pen.setWidthF(0.04);

                    tag->r95p->setOpacity(0.5);
                    tag->r95p->setPen(pen);
                    //tag->r95p->setBrush(QBrush(Qt::green, Qt::Dense7Pattern));
                    tag->r95p->setBrush(Qt::NoBrush);
                }
                else if (tag->r95show && ((rad > 1)/*&&(rad <= 2)*/))
                {
                    QPen pen = QPen(Qt::darkRed);
                    pen.setStyle(Qt::DashDotDotLine);
                    pen.setWidthF(0.05);

                    tag->r95p->setOpacity(0.5);
                    tag->r95p->setPen(pen);
                    //tag->r95p->setBrush(QBrush(Qt::darkRed, Qt::Dense7Pattern));
                    tag->r95p->setBrush(Qt::NoBrush);
                }

            }

            //update Tag R95 value in the table
            {
                QString t ;
                int ridx = 0;

                tagIDtoString(tagId, &t);

                ridx = findTagRowIndex(t);

                if(ridx != -1)
                {
                    ui->tagTable->item(ridx,ColumnR95)->setText(QString::number(r95, 'f', 3) + " m");
                }
                else
                {

                }
            }


            if(!tag->avgp) //add the average point
            {
                QBrush b = tag->p[0]->brush().color().darker();

                tag->avgp = this->scene->addEllipse(-0.025, -0.025, 0.05, 0.05);
                tag->avgp->setBrush(b);
                tag->avgp->setPen(Qt::NoPen);
            }

            //if  (rad > 2)
            if(!tag->r95show)
            {
                 tag->avgp->setOpacity(0);
            }
            else
            {
                tag->avgp->setOpacity(1);
                tag->avgp->setPos(x, y); //move it to the avg x and y values
            }

            tag->r95p->setPos(x, y); //move r95 center to the avg x and y values
        }
        else
        {
            //ERROR - there has to be a tag already...
            //ignore this statistics report
        }

        _busy = false ;

        qDebug() << "R95: 0x" + QString::number(tagId, 16) << " " << x << " " << y << " " << z << " " << r95;

    }
}

void GraphicsWidget::tagCLEStats(quint64 tagId, double blinkrx, double multirate)
{
    int ridx = -1;
    if(_busy)
    {
        qDebug() << "(Widget - busy IGNORE) report: 0x" + QString::number(tagId, 16) ;
    }
    else
    {   //find this tag in the table
        _busy = true ;

        Tag *tag = nullptr;
        QString t ;

        tagIDtoString(tagId, &t);

        tag = tags.value(tagId, nullptr);

        if(!tag) //add new tag to the tags array
        {
            addNewTag(tagId);
            tag = this->tags.value(tagId, nullptr);

            tag->ridx = findTagRowIndex(t);

            if(tag->ridx == -1)
            {
                tag->ridx = ui->tagTable->rowCount();

                insertTag(tag->ridx, t, tag->r95show, tag->showLabel, tag->tagLabelStr);
            }
        }
        else //already added / in the table
        {
            ridx = findTagRowIndex(t);
        }

        if(ridx != -1)
        {
            ui->tagTable->item(ridx,ColumnBlinkRx)->setText(QString::number(blinkrx));
            ui->tagTable->item(ridx,ColumnLocations)->setText(QString::number(multirate));
        }
    }
    _busy = false ;
}

void GraphicsWidget::tagHistory(quint64 tagId)
{
    int i = 0;
    int j = 0;

    Tag *tag = this->tags.value(tagId, nullptr);
    for(i = 0; i < historyLength; i++)
    {
        QAbstractGraphicsShapeItem *tag_p = tag->p[i];

        if(!tag_p)
        {
            break;
        }
        else
        {
            j = (tag->idx - i); //element at index is opaque
            if(j<0) j+= historyLength;
            tag_p->setOpacity(1-(float)j/historyLength);
        }
    }
}


void GraphicsWidget::setShowTagHistory(bool set)
{
    _busy = true ;

    if(set != showHistory) //the value has changed
    {
        //for each tag
        if(set == false) //we want to hide history - clear the array
        {
            QMap<quint64, Tag*>::iterator i = tags.begin();

            while(i != tags.end())
            {
                Tag *tag = i.value();
                for(int idx=0; idx<historyLength; idx++ )
                {
                    QAbstractGraphicsShapeItem *tag_p = tag->p[idx];
                    if(tag_p)
                    {
                        tag_p->setOpacity(0); //hide it

                        this->scene->removeItem(tag_p);
                        delete(tag_p);
                        tag_p = nullptr;
                        tag->p[idx] = 0;
                    }
                }
                tag->idx = 0; //reset history
                i++;
            }
        }
        else
        {

        }

        showHistory = set; //update the value
    }

    _busy = false;
}
