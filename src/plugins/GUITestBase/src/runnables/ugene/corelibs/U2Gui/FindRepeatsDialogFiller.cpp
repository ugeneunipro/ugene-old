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

#include "FindRepeatsDialogFiller.h"
#include <primitives/GTWidget.h>
#include <primitives/GTSpinBox.h>
#include <primitives/GTTabWidget.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QAbstractButton>
#include <QtGui/QCheckBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QSpinBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QSpinBox>
#endif

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::FindRepeatsDialogFiller"
#define GT_METHOD_NAME "run"

FindRepeatsDialogFiller::FindRepeatsDialogFiller(HI::GUITestOpStatus &_os, const QString & _resultFilesPath,
    bool _searchInverted, int minRepeatLength, int repeatsIdentity, int minDistance)
    : Filler(_os, "FindRepeatsDialog"), button(Start), resultAnnotationFilesPath(_resultFilesPath),
    searchInverted(_searchInverted), minRepeatLength(minRepeatLength), repeatsIdentity(repeatsIdentity), minDistance(minDistance)
{
    GTGlobals::sleep(10);
}

FindRepeatsDialogFiller::FindRepeatsDialogFiller(HI::GUITestOpStatus &os, CustomScenario *scenario) :
    Filler(os, "FindRepeatsDialog", scenario),
    button(Start),
    searchInverted(false),
    minRepeatLength(0),
    repeatsIdentity(0),
    minDistance(0)
{

}

void FindRepeatsDialogFiller::commonScenario() {
    GTGlobals::sleep(1000);
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if (button == Cancel) {
        QAbstractButton *cancelButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "cancelButton", dialog));
        GTWidget::click(os, cancelButton);
        return;
    }

    QTabWidget *tabWidget = qobject_cast<QTabWidget*>(GTWidget::findWidget(os, "tabWidget", dialog));
    GTTabWidget::setCurrentIndex(os, tabWidget, 0);

    if (-1 != minRepeatLength) {
        QSpinBox *minLenBox = qobject_cast<QSpinBox *>(GTWidget::findWidget(os, "minLenBox", dialog));
        GTSpinBox::setValue(os, minLenBox, minRepeatLength, GTGlobals::UseKeyBoard);
    }

    if (-1 != repeatsIdentity) {
        QSpinBox *identityBox = qobject_cast<QSpinBox *>(GTWidget::findWidget(os, "identityBox", dialog));
        GTSpinBox::setValue(os, identityBox, repeatsIdentity);
    }

    if (-1 != minDistance) {
        QSpinBox *minDistBox = qobject_cast<QSpinBox *>(GTWidget::findWidget(os, "minDistBox", dialog));
        GTSpinBox::setValue(os, minDistBox, minDistance);
    }

    QLineEdit *resultLocationEdit = qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "leNewTablePath", dialog));
    resultLocationEdit->setText(resultAnnotationFilesPath);

    GTTabWidget::setCurrentIndex(os, tabWidget, 1);

    QCheckBox *invertedRepeatsIndicator = qobject_cast<QCheckBox *>(GTWidget::findWidget(os, "invertCheck", dialog));
    invertedRepeatsIndicator->setChecked(searchInverted);

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

} // namespace U2
