/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_CIRCULAR_VIEW_SETTINGS_WIDGET_H_
#define _U2_CIRCULAR_VIEW_SETTINGS_WIDGET_H_

#include <qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QWidget>
#else
#include <QtWidgets/QWidget>
#endif

#include "ui/ui_CircularViewSettingsWidget.h"
#include "CircularView.h"

namespace U2 {

class CircularViewSplitter;

class CircularViewSettingsWidget : public QWidget, private Ui_CircularViewSettingsWidget {
    Q_OBJECT
public:
    CircularViewSettingsWidget(CircularViewSettings* settings, CircularViewSplitter* splitter);

signals:
    void si_settingsChanged();
    void si_openCvButtonClicked(CircularViewSettings*);

private slots:
    void sl_modifySettings();
    void sl_cvSplitterWasCreatedOrRemoved(CircularViewSplitter*, CircularViewSettings*);
    void sl_openCvButton();

private:
    void initLayout();
    void connectSlots();

private:
    CircularViewSplitter* circularViewSplitter;
    CircularViewSettings* settings;
    QWidget*             settingsWidget;
};

} // namepace

#endif // _U2_CIRCULAR_VIEW_SETTINGS_WIDGET_H_
