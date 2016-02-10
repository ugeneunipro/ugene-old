/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QColorDialog>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QPlastiqueStyle>
#else
#include <QProxyStyle>
#include <QStyleFactory>
#endif

#include <U2Gui/HelpButton.h>

#include "BranchSettingsDialog.h"

namespace U2 {

BranchSettingsDialog::BranchSettingsDialog(QWidget *parent, const OptionsMap& settings)
    : BaseSettingsDialog(parent)
{
    changedSettings[BRANCH_COLOR] = settings[BRANCH_COLOR];
    changedSettings[BRANCH_THICKNESS] = settings[BRANCH_THICKNESS];
    setupUi(this);
    new HelpButton(this, buttonBox, "17467702");

    thicknessSpinBox->setValue(changedSettings[BRANCH_THICKNESS].toInt());

#if (QT_VERSION < 0x050000) //Qt 5
    QStyle *buttonStyle = new QPlastiqueStyle;
#else
    QStyle *buttonStyle = new QProxyStyle(QStyleFactory::create("fusion"));
#endif
    buttonStyle->setParent(colorButton);
    colorButton->setStyle(buttonStyle);

    updateColorButton();

    connect(colorButton, SIGNAL(clicked()), SLOT(sl_colorButton()));
}

void BranchSettingsDialog::updateColorButton() {
    QColor branchColor = qvariant_cast<QColor>(changedSettings[BRANCH_COLOR]);
    QPalette palette = colorButton->palette();
    palette.setColor(colorButton->backgroundRole(), branchColor);
    colorButton->setPalette(palette);
}

void BranchSettingsDialog::sl_colorButton() {
    QColorDialog::ColorDialogOptions options;
#ifdef Q_OS_MAC
    if (qgetenv("UGENE_GUI_TEST").toInt() == 1 && qgetenv("UGENE_USE_NATIVE_DIALOGS").toInt() == 0) {
        options |= QColorDialog::DontUseNativeDialog;
    }
#endif

    QColor branchColor = qvariant_cast<QColor>(changedSettings[BRANCH_COLOR]);
    QColor newColor = QColorDialog::getColor(branchColor, this, tr("Select Color"), options);
    if (newColor.isValid()) {
        changedSettings[BRANCH_COLOR] = newColor;
        updateColorButton();
    }
}

void BranchSettingsDialog::accept() {
    changedSettings[BRANCH_THICKNESS] = thicknessSpinBox->value();
    QDialog::accept();
}

} //namespace
