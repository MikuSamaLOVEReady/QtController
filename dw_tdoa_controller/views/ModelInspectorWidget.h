// -------------------------------------------------------------------------------------------------------------------
//
//  File: ModelInspectorWidget.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef MODELINSPECTORWIDGET_H
#define MODELINSPECTORWIDGET_H

#include <QWidget>

namespace Ui {
class ModelInspectorWidget;
}

class ModelInspectorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModelInspectorWidget(QWidget *parent = 0);
    ~ModelInspectorWidget();

protected slots:
    void onReady();
    void anchorSelected(const QModelIndex & current, const QModelIndex & previous);
    void saveSettings(); //save configuration
    void loadSettings(); //load saved configuration
                         //ZS: should add restore to defaults ?

private:
    Ui::ModelInspectorWidget *ui;
};

#endif // MODELINSPECTORWIDGET_H
