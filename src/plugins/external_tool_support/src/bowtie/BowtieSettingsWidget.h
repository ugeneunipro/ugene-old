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

#ifndef _U2_BOWTIE_SETTINGS_WIDGET_H_
#define _U2_BOWTIE_SETTINGS_WIDGET_H_

#include "ui/ui_BowtieSettings.h"
#include "U2View/DnaAssemblyGUIExtension.h"

namespace U2 {

class BowtieSettingsWidget : public DnaAssemblyAlgorithmMainWidget, Ui_BowtieSettings {
    Q_OBJECT
public:
    BowtieSettingsWidget(QWidget *parent);
    QMap<QString, QVariant> getDnaAssemblyCustomSettings();
    void buildIndexUrl(const GUrl &url);
    bool isParametersOk(QString &);
};

class BowtieGUIExtensionsFactory : public DnaAssemblyGUIExtensionsFactory {
    DnaAssemblyAlgorithmMainWidget *createMainWidget(QWidget *parent);
    DnaAssemblyAlgorithmBuildIndexWidget *createBuildIndexWidget(QWidget *parent);
    bool hasMainWidget();
    bool hasBuildIndexWidget();
};

} // namespace U2

#endif // _U2_BOWTIE_SETTINGS_WIDGET_H_
