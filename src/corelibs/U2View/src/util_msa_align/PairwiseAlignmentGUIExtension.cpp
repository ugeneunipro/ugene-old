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

#include "PairwiseAlignmentGUIExtension.h"

namespace U2 {

PairwiseAlignmentMainWidget::PairwiseAlignmentMainWidget(QWidget* parent, QVariantMap* s) :
    QWidget(parent), externSettings(s), externSettingsExists(s != NULL) {
}

PairwiseAlignmentMainWidget::~PairwiseAlignmentMainWidget() {
    getPairwiseAlignmentCustomSettings(true);
}

void PairwiseAlignmentMainWidget::sl_externSettingsInvalide() {
    externSettingsExists = false;
}

QMap<QString, QVariant> PairwiseAlignmentMainWidget::getPairwiseAlignmentCustomSettings(bool append = false) {
    if (append == true && externSettingsExists == true) {
        foreach (QString key, innerSettings.keys()) {
            externSettings->insert(key, innerSettings.value(key));
        }
    }
    return innerSettings;
}

void PairwiseAlignmentMainWidget::fillInnerSettings() {
}

bool PairwiseAlignmentGUIExtensionFactory::hasMainWidget(const QWidget* parent) {
    return mainWidgets.contains(parent);
}

void PairwiseAlignmentGUIExtensionFactory::sl_widgetDestroyed(QObject * obj) {
    foreach (PairwiseAlignmentMainWidget* mainWidget, mainWidgets.values()) {
        if (obj == mainWidget) {
            mainWidgets.remove(mainWidgets.key(mainWidget));
        }
    }
}

}   //namespace
