// -------------------------------------------------------------------------------------------------------------------
//
//  File: GraphicsWidget.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef GRAPHICSWIDGET_H
#define GRAPHICSWIDGET_H

#include <QGraphicsRectItem>
#include <QWidget>
#include <QAbstractItemView>


namespace Ui {
class GraphicsWidget;
}

class GraphicsDataModel;
class QGraphicsScene;
class QModelIndex;
class GraphicsView;
class QAbstractGraphicsShapeItem;
class QGraphicsItem;

struct Tag
{
    Tag(void)
    {
        idx = 0;
        ridx = 0;
    }

    quint64 id;
    int idx;
    int ridx;
    QVector<QAbstractGraphicsShapeItem *> p;
    QAbstractGraphicsShapeItem *avgp;
    QAbstractGraphicsShapeItem *r95p;
    bool r95show;

    double tsprev; //previous timestamp in sec

    bool showLabel;
    QGraphicsSimpleTextItem *_tagLabel;
    QString tagLabelStr;
};

class GraphicsWidget : public QWidget
{
    Q_OBJECT

public:

    enum Column {
        ColumnID = 0,   ///< 64 bit address of the anchor (uint64)
        ColumnX,        ///< X coordinate (double)
        ColumnY,        ///< Y coordinate (double)
        ColumnZ,        ///< Z coordinate (double)
        ColumnBlinkRx,   ///< % of received blinks
        ColumnLocations, ///< % of successful multilaterations
        ColumnR95,       ///< R95

        ColumnIDr,       ///< ID raw (hex) hidden
        ColumnCount
    };

    explicit GraphicsWidget(QWidget *parent = 0);
    ~GraphicsWidget();

    GraphicsView *graphicsView();

    int findTagRowIndex(QString &t);
    void insertTag(int ridx, QString &t, bool showR95, bool showLabel, QString &l);
    void tagIDtoString(quint64 tagId, QString *t);
    void addNewTag(quint64 tagId);

    void loadConfigFile(QString filename);
    void saveConfigFile(QString filename);

public slots:
    void tagPos(quint64 tagId, double x, double y, double z);
    void tagStats(quint64 tagId, double x, double y, double z, double r95);
    //void anchor(quint64 anchId, double x, double y, double z);
    void tagCLEStats(quint64 tagId, double blinkrx, double multirate);


    void tagTableChanged(int r, int c);
    void tagTableClicked(int r, int c);
    void itemSelectionChanged(void);
    void clearTags(void);

    void setShowTagHistory(bool);
protected slots:
    void onReady();
    void onAnchorSelected(const QModelIndex & current, const QModelIndex & previous);

protected:
    void tagHistory(quint64 tagId);

private:
    Ui::GraphicsWidget *ui;
    QGraphicsScene *scene;
    GraphicsDataModel *anchorGraphicsModel;
    GraphicsDataModel *linksGraphicsModel;
    QMap<quint64, Tag*> tags;
    QMap<quint64, QString> taglabels;

    float tagSize;
    int   historyLength;
    bool showHistory;
    bool _busy;
    bool _ignore;

    int selectedTagIdx;
};

#endif // GRAPHICSWIDGET_H
