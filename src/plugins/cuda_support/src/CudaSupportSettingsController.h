/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef __CUDA_SUPPORT_SETTINGS_CONTROLLER__
#define __CUDA_SUPPORT_SETTINGS_CONTROLLER__

#include <QtCore/QVector>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>

#include <U2Gui/AppSettingsGUI.h>

namespace U2 {

#define CudaSupportSettingsPageId QString("css")

class CudaSupportSettingsPageController : public AppSettingsGUIPageController {
    Q_OBJECT
public:
    CudaSupportSettingsPageController( const QString & _displayMsg, QObject * p = 0 );

    virtual AppSettingsGUIPageState * getSavedState();

    virtual void saveState( AppSettingsGUIPageState * s );

    virtual AppSettingsGUIPageWidget * createWidget( AppSettingsGUIPageState* state );

private:
    QString displayMsg;
};

class CudaSupportSettingsPageState : public AppSettingsGUIPageState {
    Q_OBJECT
public:
    CudaSupportSettingsPageState( int num_gpus );
    QVector<bool> enabledGpus;
};

class CudaSupportSettingsPageWidget : public AppSettingsGUIPageWidget {
    Q_OBJECT
public:
    CudaSupportSettingsPageWidget( const QString & _msg, CudaSupportSettingsPageController * ctrl );

    virtual void setState( AppSettingsGUIPageState * state );

    virtual AppSettingsGUIPageState* getState( QString & err ) const;

    QString onlyMsg;

    QList<QCheckBox *> gpuEnableChecks;
};

} //namespace

#endif //__CUDA_SUPPORT_SETTINGS_CONTROLLER__
