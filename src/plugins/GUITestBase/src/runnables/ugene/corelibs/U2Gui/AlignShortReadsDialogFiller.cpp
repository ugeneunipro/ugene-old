/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "AlignShortReadsDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTCheckBox.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "runnables/qt/MessageBoxFiller.h"
#include <QtGui/QApplication>
#include <QtGui/QComboBox>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::AlignShortReadsFiller"
#define GT_METHOD_NAME "run"

void AlignShortReadsFiller::run() {
    SAFE_POINT_EXT(parameters, GT_CHECK(0, "Invalid input parameters: NULL pointer"), );

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    setCommonParameters(dialog);
    CHECK_OP(os, );

    setAdditionalParameters(dialog);
    CHECK_OP(os, );

    GTGlobals::sleep(500);
    QWidget* assembleyButton = GTWidget::findWidget(os, "assembleyButton", dialog);
    CHECK_OP(os, );
    GT_CHECK(assembleyButton, "assembleyButton is NULL");
    GTWidget::click(os, assembleyButton);
    CHECK_OP(os, );
}

void AlignShortReadsFiller::setCommonParameters(QWidget* dialog) {
    QComboBox* methodNamesBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "methodNamesBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(methodNamesBox, "methodNamesBox is NULL");
    GTComboBox::setIndexWithText(os, methodNamesBox, parameters->getAlignmentMethod());
    CHECK_OP(os, );

    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, parameters->refDir, parameters->refFileName));
    CHECK_OP(os, );

    QWidget* addRefButton = GTWidget::findWidget(os, "addRefButton", dialog);
    CHECK_OP(os, );
    GTWidget::click(os, addRefButton);
    CHECK_OP(os, );

    if (!parameters->useDefaultResultPath) {
        GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils (os, parameters->resultDir, parameters->resultFileName));
        CHECK_OP(os, );

        QWidget* setResultFileNameButton = GTWidget::findWidget(os, "setResultFileNameButton", dialog);
        CHECK_OP(os, );
        GTWidget::click(os, setResultFileNameButton);
        CHECK_OP(os, );
    }

    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, parameters->readsDir, parameters->readsFileName));
    CHECK_OP(os, );

    QWidget* addShortreadsButton = GTWidget::findWidget(os, "addShortreadsButton", dialog);
    CHECK_OP(os, );
    GTWidget::click(os, addShortreadsButton);
    CHECK_OP(os, );

    QComboBox* libraryComboBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "libraryComboBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(libraryComboBox, "libraryComboBox is NULL");
    GTComboBox::setIndexWithText(os, libraryComboBox, parameters->getLibrary());
    CHECK_OP(os, );

    QCheckBox* prebuiltIndexCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "prebuiltIndexCheckBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(prebuiltIndexCheckBox, "prebuiltIndexCheckBox is NULL");
    GTCheckBox::setChecked(os, prebuiltIndexCheckBox, parameters->prebuiltIndex);
    CHECK_OP(os, );

    QCheckBox* samBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "samBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(samBox, "samBox is NULL");
    if (samBox->isEnabled()) {
        GTCheckBox::setChecked(os, samBox, parameters->samOutput);
        CHECK_OP(os, );
    }
}

void AlignShortReadsFiller::setAdditionalParameters(QWidget* dialog) {
    Bowtie2Parameters* bowtie2Parameters = dynamic_cast<Bowtie2Parameters*>(parameters);
    if (bowtie2Parameters) {
        setBowtie2AdditionalParameters(bowtie2Parameters, dialog);
        CHECK_OP(os, );
    }
}

void AlignShortReadsFiller::setBowtie2AdditionalParameters(Bowtie2Parameters* bowtie2Parameters, QWidget* dialog) {
    // Parameters
    QComboBox* modeComboBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "modeComboBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(modeComboBox, "modeComboBox is NULL");
    GTComboBox::setIndexWithText(os, modeComboBox, bowtie2Parameters->getMode());
    CHECK_OP(os, );

    QSpinBox* mismatchesSpinBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "mismatchesSpinBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(mismatchesSpinBox, "mismatchesSpinBox is NULL");
    GTSpinBox::setValue(os, mismatchesSpinBox, bowtie2Parameters->numberOfMismatches);
    CHECK_OP(os, );

    QCheckBox* seedlenCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "seedlenCheckBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(seedlenCheckBox, "seedlenCheckBox is NULL");
    GTCheckBox::setChecked(os, seedlenCheckBox, bowtie2Parameters->seedLengthCheckBox);
    CHECK_OP(os, );

    QSpinBox* seedlenSpinBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "seedlenSpinBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(seedlenSpinBox, "seedlenSpinBox is NULL");
    GTSpinBox::setValue(os, seedlenSpinBox, bowtie2Parameters->seedLength);
    CHECK_OP(os, );

    QCheckBox* dpadCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "dpadCheckBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(dpadCheckBox, "dpadCheckBox is NULL");
    GTCheckBox::setChecked(os, dpadCheckBox, bowtie2Parameters->addColumnsToAllowGapsCheckBox);
    CHECK_OP(os, );

    QSpinBox* dpadSpinBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "dpadSpinBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(dpadSpinBox, "dpadSpinBox is NULL");
    GTSpinBox::setValue(os, dpadSpinBox, bowtie2Parameters->addColumnsToAllowGaps);
    CHECK_OP(os, );

    QCheckBox* gbarCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "gbarCheckBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(gbarCheckBox, "gbarCheckBox is NULL");
    GTCheckBox::setChecked(os, gbarCheckBox, bowtie2Parameters->disallowGapsCheckBox);
    CHECK_OP(os, );

    QSpinBox* gbarSpinBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "gbarSpinBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(gbarSpinBox, "gbarSpinBox is NULL");
    GTSpinBox::setValue(os, gbarSpinBox, bowtie2Parameters->disallowGaps);
    CHECK_OP(os, );

    QCheckBox* seedCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "seedCheckBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(seedCheckBox, "seedCheckBox is NULL");
    GTCheckBox::setChecked(os, seedCheckBox, bowtie2Parameters->seedCheckBox);
    CHECK_OP(os, );

    QSpinBox* seedSpinBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "seedSpinBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(seedSpinBox, "seedSpinBox is NULL");
    GTSpinBox::setValue(os, seedSpinBox, bowtie2Parameters->seed);
    CHECK_OP(os, );

    QSpinBox* threadsSpinBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "threadsSpinBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(seedSpinBox, "threadsSpinBox is NULL");
    if ( threadsSpinBox->minimum( ) <= bowtie2Parameters->threads
         && threadsSpinBox->maximum( ) >= bowtie2Parameters->threads )
    {
        GTSpinBox::setValue(os, threadsSpinBox, bowtie2Parameters->threads);
        CHECK_OP(os, );
    }

    // Flags
    QCheckBox* nomixedCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "nomixedCheckBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(nomixedCheckBox, "nomixedCheckBox is NULL");
    GTCheckBox::setChecked(os, nomixedCheckBox, bowtie2Parameters->noUnpairedAlignments);
    CHECK_OP(os, );

    QCheckBox* nodiscordantCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "nodiscordantCheckBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(nodiscordantCheckBox, "nodiscordantCheckBox is NULL");
    GTCheckBox::setChecked(os, nodiscordantCheckBox, bowtie2Parameters->noDiscordantAlignments);
    CHECK_OP(os, );

    QCheckBox* nofwCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "nofwCheckBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(nofwCheckBox, "nofwCheckBox is NULL");
    GTCheckBox::setChecked(os, nofwCheckBox, bowtie2Parameters->noForwardOrientation);
    CHECK_OP(os, );

    QCheckBox* norcCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "norcCheckBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(norcCheckBox, "norcCheckBox is NULL");
    GTCheckBox::setChecked(os, norcCheckBox, bowtie2Parameters->noReverseComplementOrientation);
    CHECK_OP(os, );

    QCheckBox* nooverlapCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "nooverlapCheckBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(nooverlapCheckBox, "nooverlapCheckBox is NULL");
    GTCheckBox::setChecked(os, nooverlapCheckBox, bowtie2Parameters->noOverlappingMates);
    CHECK_OP(os, );

    QCheckBox* nocontainCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "nocontainCheckBox", dialog));
    CHECK_OP(os, );
    GT_CHECK(nocontainCheckBox, "nocontainCheckBox is NULL");
    GTCheckBox::setChecked(os, nocontainCheckBox, bowtie2Parameters->noMatesContainingOneAnother);
    CHECK_OP(os, );
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
