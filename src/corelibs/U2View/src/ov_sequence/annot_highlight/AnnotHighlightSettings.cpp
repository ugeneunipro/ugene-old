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

#include "AnnotHighlightSettings.h"

#include <U2Core/AnnotationSettings.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/U2SafePoints.h>


namespace U2 {

const QString AnnotHighlightSettingsWidget::LIGHT_RED_COLOR = "rgb(231, 94, 80)";

AnnotHighlightSettingsWidget::AnnotHighlightSettingsWidget(QWidget* parent)
{
    setupUi(this);

    connect(checkShowHideAnnots, SIGNAL(stateChanged(int)), SLOT(sl_onShowHideChanged(int)));
    connect(checkShowOnTranslation, SIGNAL(stateChanged(int)), SLOT(sl_onShowOnTranslationChanged(int)));
    connect(checkVisualQualifier, SIGNAL(stateChanged(int)), SLOT(sl_onShowQualifierChanged(int)));
    connect(editQualifiers, SIGNAL(textChanged(const QString&)), SLOT(sl_onEditQualifiersChanged(const QString&)));
}

void AnnotHighlightSettingsWidget::setSettings(AnnotationSettings* annotSettings, bool disableShowTranslations)
{
    SAFE_POINT(0 != annotSettings, "Annotation settings equals to NULL!",);

    currentSettings = annotSettings;

    checkShowHideAnnots->setCheckState(annotSettings->visible ? Qt::Checked : Qt::Unchecked);
    checkShowOnTranslation->setCheckState(annotSettings->amino ? Qt::Checked : Qt::Unchecked);
    checkVisualQualifier->setCheckState(annotSettings->showNameQuals ? Qt::Checked : Qt::Unchecked);

    QString qualifiersNames = annotSettings->nameQuals.join(",");
    editQualifiers->setText(qualifiersNames);
    editQualifiers->home(false);

    checkShowOnTranslation->setDisabled(disableShowTranslations);
}

void AnnotHighlightSettingsWidget::sl_onShowHideChanged(int checkedState)
{
    SAFE_POINT(0 != currentSettings, "An annotation should always be selected!",);
    currentSettings->visible = (checkedState == Qt::Checked) ? true : false;
    emit si_annotSettingsChanged(currentSettings);
}

void AnnotHighlightSettingsWidget::sl_onShowOnTranslationChanged(int checkedState)
{
    SAFE_POINT(0 != currentSettings, "An annotation should always be selected!",);
    currentSettings->amino = (checkedState == Qt::Checked) ? true : false;
    emit si_annotSettingsChanged(currentSettings);
}

void AnnotHighlightSettingsWidget::sl_onShowQualifierChanged(int checkedState)
{
    SAFE_POINT(0 != currentSettings, "An annotation should always be selected!",);
    currentSettings->showNameQuals = (checkedState == Qt::Checked) ? true : false;
    emit si_annotSettingsChanged(currentSettings);
}

void AnnotHighlightSettingsWidget::sl_onEditQualifiersChanged(const QString& inputNameQuals)
{
    SAFE_POINT(0 != currentSettings, "An annotation should always be selected!",);
    QStringList qualifierNames = inputNameQuals.split(',', QString::SkipEmptyParts);

    // Validate the input
    foreach (QString qualifierName, qualifierNames) {
        if (!Annotation::isValidQualifierName(qualifierName)) {
            setIncorrectState();
            return;
        }
    }

    // If the qualifier names are correct
    currentSettings->nameQuals = qualifierNames;
    setCorrectState();
    emit si_annotSettingsChanged(currentSettings);
}


void AnnotHighlightSettingsWidget::setIncorrectState()
{
    editQualifiers->setStyleSheet("background-color: " + LIGHT_RED_COLOR + ";");
}


void AnnotHighlightSettingsWidget::setCorrectState()
{
    editQualifiers->setStyleSheet("background-color: white;");
}


} // namespace
