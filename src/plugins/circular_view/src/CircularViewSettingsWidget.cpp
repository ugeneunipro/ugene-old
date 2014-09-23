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

#include "CircularViewSettingsWidget.h"
#include "CircularViewSplitter.h"
#include "CircularViewPlugin.h"

#include <U2Core/U2SafePoints.h>
#include <U2Gui/ShowHideSubgroupWidget.h>

#include <QtGui/QFontDatabase>

namespace U2 {

CircularViewSettingsWidget::CircularViewSettingsWidget(CircularViewSettings* s, CircularViewSplitter* cv)
    : circularViewSplitter(cv),
      settings(s)
{
    SAFE_POINT(s != NULL, tr("Circular view settings is NULL"), );
    setupUi(this);

    initLayout();
    hintLabel->setVisible(cv == NULL);
    connectSlots();
}

void CircularViewSettingsWidget::sl_modifySettings() {
    settings->showTitle = titleCheckBox->isChecked();
    settings->showLength = lengthCheckBox->isChecked();
    settings->titleFontSize = fontSizeSpinBox->value();
    settings->titleFont = fontComboBox->currentText();
    settings->titleBold = boldButton->isChecked();

    settings->showRulerLine = rulerLineCheckBox->isChecked();
    settings->showRulerCoordinates = rulerCoordsCheckBox->isChecked();
    settings->rulerFontSize = rulerFontSizeSpinBox->value();

    settings->labelFontSize = labelFontSizeSpinBox->value();
    if (labelPositionComboBox->currentText() == tr("Inside")) {
        settings->labelMode = CircularViewSettings::Inside;
    }
    if (labelPositionComboBox->currentText() == tr("Outside")) {
        settings->labelMode = CircularViewSettings::Outside;
    }
    if (labelPositionComboBox->currentText() == tr("Inside/Outside")) {
        settings->labelMode = CircularViewSettings::Mixed;
    }
    if (labelPositionComboBox->currentText() == tr("None")) {
        settings->labelMode = CircularViewSettings::None;
    }

    if (circularViewSplitter != NULL) {
        circularViewSplitter->updateViews();
    }
}

void CircularViewSettingsWidget::sl_cvSplitterWasCreatedOrRemoved(CircularViewSplitter *splitter) {
    circularViewSplitter = splitter;
    hintLabel->setVisible(splitter == NULL);
}

void CircularViewSettingsWidget::initLayout() {
    QFontDatabase fontDatabase;
    fontComboBox->addItems(fontDatabase.families(QFontDatabase::Latin)); //! ckeck latin baundary
    fontComboBox->setCurrentIndex( fontComboBox->findText(settings->titleFont));
    fontSizeSpinBox->setValue(settings->titleFontSize);
    rulerFontSizeSpinBox->setValue(settings->rulerFontSize);
    labelFontSizeSpinBox->setValue(settings->labelFontSize);

    ShowHideSubgroupWidget* titleGroup = new ShowHideSubgroupWidget("CV_TITLE", tr("Title"), titleWidget, true);
    cvSettingsMainLayout->addWidget(titleGroup);

    ShowHideSubgroupWidget* rulerGroup = new ShowHideSubgroupWidget("CV_RULER", tr("Ruler"), rulerWidget, true);
    cvSettingsMainLayout->addWidget(rulerGroup);

    ShowHideSubgroupWidget* annotationGroup = new ShowHideSubgroupWidget("CV_ANNOTATION", tr("Annotations"), annotationLabelWidget, true);
    cvSettingsMainLayout->addWidget(annotationGroup);

    hintLabel->setVisible(false);
    cvSettingsMainLayout->addWidget(hintLabel);
}

void CircularViewSettingsWidget::connectSlots() {
    connect(titleCheckBox, SIGNAL(clicked()), SLOT(sl_modifySettings()));
    connect(lengthCheckBox, SIGNAL(clicked()), SLOT(sl_modifySettings()));
    connect(fontSizeSpinBox, SIGNAL(valueChanged(int)), SLOT(sl_modifySettings()));
    connect(fontComboBox, SIGNAL(currentIndexChanged(int)), SLOT(sl_modifySettings()));
    connect(boldButton, SIGNAL(clicked()), SLOT(sl_modifySettings()));

    connect(rulerCoordsCheckBox, SIGNAL(clicked()), SLOT(sl_modifySettings()));
    connect(rulerLineCheckBox, SIGNAL(clicked()), SLOT(sl_modifySettings()));
    connect(rulerFontSizeSpinBox, SIGNAL(valueChanged(int)), SLOT(sl_modifySettings()));

    connect(labelPositionComboBox, SIGNAL(currentIndexChanged(int)), SLOT(sl_modifySettings()));
    connect(labelFontSizeSpinBox, SIGNAL(valueChanged(int)), SLOT(sl_modifySettings()));
}

} // namespace
