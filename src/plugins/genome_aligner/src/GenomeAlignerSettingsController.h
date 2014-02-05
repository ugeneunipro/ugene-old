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

#ifndef _U2_GA_SETTINGS_GUI_CONTROLLER_H_
#define _U2_GA_SETTINGS_GUI_CONTROLLER_H_

#include <ui/ui_GenomeAlignerSettingsWidget.h>

#include <U2Gui/AppSettingsGUI.h>

namespace U2 {

class GenomeAlignerSettingsUtils {
public:
    static QString getIndexDir();
    static void setIndexDir(const QString &indexDir);
};

/************************************************************************/
/* Genome Aligner Settings Controller                                   */
/************************************************************************/
#define GenomeAlignerSettingsPageId     QString("gas")

class GenomeAlignerSettingsPageController : public AppSettingsGUIPageController {
    Q_OBJECT
public:
    GenomeAlignerSettingsPageController(QObject* p = NULL);

    virtual AppSettingsGUIPageState* getSavedState();

    virtual void saveState(AppSettingsGUIPageState* s);

    virtual AppSettingsGUIPageWidget* createWidget(AppSettingsGUIPageState* data);
};

class GenomeAlignerSettingsPageState : public AppSettingsGUIPageState {
    Q_OBJECT
public:
    QString indexDir;
};

class GenomeAlignerSettingsPageWidget: public AppSettingsGUIPageWidget, public Ui_GenomeAlignerSettingsWidget {
    Q_OBJECT
public:
    GenomeAlignerSettingsPageWidget(GenomeAlignerSettingsPageController* ctrl);

    virtual void setState(AppSettingsGUIPageState* state);

    virtual AppSettingsGUIPageState* getState(QString& err) const;

private slots:
    void sl_onIndexDirButton();
};

} // U2

#endif // _U2_GA_SETTINGS_GUI_CONTROLLER_H_
