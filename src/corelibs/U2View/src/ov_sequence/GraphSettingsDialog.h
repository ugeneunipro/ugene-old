/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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
#ifndef _U2_GRAPH_SETTINGS_DIALOG_H_
#define _U2_GRAPH_SETTINGS_DIALOG_H_

#include <U2Core/U2Region.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif

namespace U2{

class GSequenceGraphDrawer;
class MinMaxSelectorWidget;
class WindowStepSelectorWidget;

class GraphSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    GraphSettingsDialog(GSequenceGraphDrawer* d, const U2Region& range, QWidget* parent);
    WindowStepSelectorWidget* getWindowSelector() { return wss; }
    MinMaxSelectorWidget* getMinMaxSelector() { return mms; }
    const QMap<QString,QColor>& getColors() { return colorMap; }
private slots:
        void sl_onPickColorButtonClicked();
        void sl_onCancelClicked();
        void sl_onOkClicked();
private:
    WindowStepSelectorWidget* wss;
    MinMaxSelectorWidget* mms;
    QMap<QString,QColor> colorMap;
};

}//ns

#endif //_U2_GRAPH_SETTINGS_DIALOG_H_

