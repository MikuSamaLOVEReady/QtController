// -------------------------------------------------------------------------------------------------------------------
//
//  File: ViewSettings.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "ViewSettings.h"

ViewSettings::ViewSettings(QObject *parent)
    : QObject(parent),
      m_gridWidth(0.2),
      m_gridHeight(0.2),
      m_floorplanFlipX(false),
      m_floorplanFlipY(false),
      m_floorplanXScale(100),
      m_floorplanYScale(100),
      m_floorplanXOffset(0),
      m_floorplanYOffset(0),
      m_showOrigin(true),
      m_showGrid(true),
      m_floorplanPath("")
{
    QObject::connect(this, SIGNAL(gridWidthChanged(double)), this, SLOT(viewSettingsChanged()));
    QObject::connect(this, SIGNAL(gridHeightChanged(double)), this, SLOT(viewSettingsChanged()));

    QObject::connect(this, SIGNAL(floorplanFlipXChanged(bool)), this, SLOT(viewSettingsChanged()));
    QObject::connect(this, SIGNAL(floorplanFlipYChanged(bool)), this, SLOT(viewSettingsChanged()));

    QObject::connect(this, SIGNAL(floorplanXScaleChanged(double)), this, SLOT(viewSettingsChanged()));
    QObject::connect(this, SIGNAL(floorplanYScaleChanged(double)), this, SLOT(viewSettingsChanged()));

    QObject::connect(this, SIGNAL(showGridChanged(bool)), this, SLOT(viewSettingsChanged()));
    QObject::connect(this, SIGNAL(showOriginChanged(bool)), this, SLOT(viewSettingsChanged()));

    QObject::connect(this, SIGNAL(floorplanXOffsetChanged(double)), this, SLOT(viewSettingsChanged()));
    QObject::connect(this, SIGNAL(floorplanYOffsetChanged(double)), this, SLOT(viewSettingsChanged()));

    QObject::connect(this, SIGNAL(floorplanPixmapChanged()), this, SLOT(viewSettingsChanged()));
}

bool ViewSettings::originShow()
{
    return m_showOrigin;
}

bool ViewSettings::gridShow()
{
    return m_showGrid;
}

void ViewSettings::setShowOrigin(bool set)
{
    m_showOrigin = set;

    emit showGO(m_showOrigin, m_showGrid);
}

void ViewSettings::setShowGrid(bool set)
{
    m_showGrid = set;

    emit showGO(m_showOrigin, m_showGrid);
}

double ViewSettings::gridHeight() const
{
    return m_gridHeight;
}

double ViewSettings::gridWidth() const
{
    return m_gridWidth;
}

bool ViewSettings::floorplanFlipX() const
{
    return m_floorplanFlipX;
}

bool ViewSettings::floorplanFlipY() const
{
    return m_floorplanFlipY;
}

double ViewSettings::floorplanXScale() const
{
    return m_floorplanXScale;
}

double ViewSettings::floorplanYScale() const
{
    return m_floorplanYScale;
}

double ViewSettings::floorplanXOffset() const
{
    return m_floorplanXOffset;
}

double ViewSettings::floorplanYOffset() const
{
    return m_floorplanYOffset;
}

const QPixmap &ViewSettings::floorplanPixmap() const
{
    return m_floorplanPixmap;
}

const QString &ViewSettings::getFloorplanPath()
{
    return m_floorplanPath;
}

QTransform ViewSettings::floorplanTransform() const
{
    return m_floorplanTransform;
}

void ViewSettings::setGridWidth(double arg)
{
    if (m_gridWidth != arg) {
        m_gridWidth = arg;
        emit gridWidthChanged(arg);
    }
}

void ViewSettings::setGridHeight(double arg)
{
    if (m_gridHeight != arg) {
        m_gridHeight = arg;
        emit gridHeightChanged(arg);
    }
}

void ViewSettings::floorplanFlipX(bool arg)
{
    if (m_floorplanFlipX != arg) {
        m_floorplanFlipX = arg;
        emit floorplanFlipXChanged(arg);
    }
}

void ViewSettings::floorplanFlipY(bool arg)
{
    if (m_floorplanFlipY != arg) {
        m_floorplanFlipY = arg;
        emit floorplanFlipYChanged(arg);
    }
}

void ViewSettings::setFloorplanXScale(double arg)
{
    if (m_floorplanXScale != arg) {
        m_floorplanXScale = arg;
        emit floorplanXScaleChanged(arg);
    }
}

void ViewSettings::setFloorplanYScale(double arg)
{
    if (m_floorplanYScale != arg) {
        m_floorplanYScale = arg;
        emit floorplanYScaleChanged(arg);
    }
}

void ViewSettings::setFloorplanXOffset(double arg)
{
    if (m_floorplanXOffset != arg) {
        m_floorplanXOffset = arg;
        emit floorplanXOffsetChanged(arg);
    }
}

void ViewSettings::setFloorplanYOffset(double arg)
{
    if (m_floorplanYOffset != arg) {
        m_floorplanYOffset = arg;
        emit floorplanYOffsetChanged(arg);
    }
}

void ViewSettings::setFloorplanPixmap(const QPixmap &arg)
{
    m_floorplanPixmap = arg;
    emit floorplanPixmapChanged();
}

void ViewSettings::setFloorplanPath(const QString &arg)
{
    m_floorplanPath = arg;
}

void ViewSettings::setFloorplanPathN(void)
{
    if (!m_floorplanPath.isNull())
        emit setFloorPlanPic();
}

void ViewSettings::viewSettingsChanged()
{
    QTransform t;
    double xscale = floorplanXScale();
    double yscale = floorplanYScale();
    double xoffset = floorplanXOffset();
    double yoffset = floorplanYOffset();

    if (!floorplanPixmap().isNull() && xscale != 0 && yscale != 0)
    {
        if (floorplanFlipX())
        {
            t.scale(1, -1);
        }
        if (floorplanFlipY())
        {
            t.scale(-1, 1);
        }

        t.scale(1. / xscale, 1. / yscale);
        t.translate(-xoffset, -yoffset);
    }

    m_floorplanTransform = t;
    emit floorplanChanged();
}
