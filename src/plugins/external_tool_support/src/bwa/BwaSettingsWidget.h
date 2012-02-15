/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_BWA_SETTINGS_WIDGET_H_
#define _U2_BWA_SETTINGS_WIDGET_H_

#include "ui/ui_BwaSettings.h"
#include "ui/ui_BwaBuildSettings.h"
#include "U2View/DnaAssemblyGUIExtension.h"

namespace U2 {

class BwaSettingsWidget : public DnaAssemblyAlgorithmMainWidget, Ui_BwaSettings {
    Q_OBJECT
public:
    BwaSettingsWidget(QWidget *parent);
    QMap<QString, QVariant> getDnaAssemblyCustomSettings();
    void buildIndexUrl(const GUrl &url);
    bool isParametersOk(QString &);
};

class BwaBuildSettingsWidget : public DnaAssemblyAlgorithmBuildIndexWidget, Ui_BwaBuildSettings {
    Q_OBJECT
public:
    BwaBuildSettingsWidget(QWidget *parent);
    virtual QMap<QString,QVariant> getBuildIndexCustomSettings();
    virtual QString getIndexFileExtension();
    virtual void buildIndexUrl(const GUrl& url);
};

class BwaGUIExtensionsFactory : public DnaAssemblyGUIExtensionsFactory {
    DnaAssemblyAlgorithmMainWidget *createMainWidget(QWidget *parent);
    DnaAssemblyAlgorithmBuildIndexWidget *createBuildIndexWidget(QWidget *parent);
    bool hasMainWidget();
    bool hasBuildIndexWidget();
};

} // namespace U2

#endif // _U2_BWA_SETTINGS_WIDGET_H_
