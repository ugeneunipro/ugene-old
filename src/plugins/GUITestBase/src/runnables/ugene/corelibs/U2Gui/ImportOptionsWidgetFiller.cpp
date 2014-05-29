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

#include <QtGui/QCheckBox>
#include <QtGui/QLineEdit>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>

#include <U2Gui/ImportOptionsWidget.h>

#include "GTUtilsDialog.h"
#include "ImportOptionsWidgetFiller.h"
#include "api/GTCheckBox.h"
#include "api/GTLineEdit.h"
#include "api/GTRadioButton.h"
#include "api/GTSpinBox.h"
#include "api/GTWidget.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ImportOptionsWidgetFiller"

const QString ImportOptionsWidgetFiller::DESTINATION_FOLDER = "destination_folder";
const QString ImportOptionsWidgetFiller::KEEP_FOLDERS_STRUCTURE = "keep_folders_structure";
const QString ImportOptionsWidgetFiller::PROCESS_FOLDERS_RECUSIVELY = "process_folders_recusively";
const QString ImportOptionsWidgetFiller::CREATE_SUBFOLDER_FOR_TOP_LEVEL_FOLDER = "create_subfolder_for_top_level_folder";
const QString ImportOptionsWidgetFiller::CREATE_SUBFOLDER_FOR_EACH_FILE = "create_subfolder_for_each_file";
const QString ImportOptionsWidgetFiller::IMPORT_UNKNOWN_AS_UDR = "import_unknown_as_udr";
const QString ImportOptionsWidgetFiller::MULTI_SEQUENCE_POLICY = "multi_sequence_policy";
const QString ImportOptionsWidgetFiller::CREATE_SUBFOLDER_FOR_DOCUMENT = "create_subfolder_for_document";
const QString ImportOptionsWidgetFiller::MERGE_MULTI_SEQUENCE_POLICY_SEPARATOR_SIZE = "merge_multi_sequence_policy_separator_size";

#define GT_METHOD_NAME "fill"
void ImportOptionsWidgetFiller::fill(U2OpStatus &os, ImportOptionsWidget *optionsWidget, const QVariantMap &data) {
    Q_UNUSED(os);
    GT_CHECK(NULL != optionsWidget, "optionsWidget is NULL");

    setDestinationFolder(os, optionsWidget, data);
    setKeepFoldersStructure(os, optionsWidget, data);
    setProcessFoldersRecusively(os, optionsWidget, data);
    setCreateSubfolderForTopLevelFolder(os, optionsWidget, data);
    setCreateSubfolderForEachFile(os, optionsWidget, data);
    setImportUnknownAsUdr(os, optionsWidget, data);
    setMultiSequencePolicy(os, optionsWidget, data);
    setCreateSubfolderForDocument(os, optionsWidget, data);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setDestinationFolder"
void ImportOptionsWidgetFiller::setDestinationFolder(U2OpStatus &os, ImportOptionsWidget *optionsWidget, const QVariantMap &data) {
    CHECK(data.contains(DESTINATION_FOLDER), );
    QLineEdit* destFolder = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "leBaseFolder", optionsWidget));
    GT_CHECK(NULL != destFolder, "destFolder is NULL");
    GTLineEdit::setText(os, destFolder, data.value(DESTINATION_FOLDER).toString());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setKeepFoldersStructure"
void ImportOptionsWidgetFiller::setKeepFoldersStructure(U2OpStatus& os, ImportOptionsWidget *optionsWidget, const QVariantMap &data) {
    CHECK(data.contains(KEEP_FOLDERS_STRUCTURE), );
    QCheckBox* keepStructure = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "cbKeepStructure", optionsWidget));
    GT_CHECK(NULL != keepStructure, "keepStructure is NULL");
    GTCheckBox::setChecked(os, keepStructure, data.value(KEEP_FOLDERS_STRUCTURE).toBool());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setProcessFoldersRecusively"
void ImportOptionsWidgetFiller::setProcessFoldersRecusively(U2OpStatus &os, ImportOptionsWidget *optionsWidget, const QVariantMap &data) {
    CHECK(data.contains(PROCESS_FOLDERS_RECUSIVELY), );
    QCheckBox* processRecursively = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "cbRecursively", optionsWidget));
    GT_CHECK(NULL != processRecursively, "processRecursively is NULL");
    GTCheckBox::setChecked(os, processRecursively, data.value(PROCESS_FOLDERS_RECUSIVELY).toBool());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setCreateSubfolderForTopLevelFolder"
void ImportOptionsWidgetFiller::setCreateSubfolderForTopLevelFolder(U2OpStatus &os, ImportOptionsWidget *optionsWidget, const QVariantMap &data) {
    CHECK(data.contains(CREATE_SUBFOLDER_FOR_TOP_LEVEL_FOLDER), );
    QCheckBox* subfolderForEachFolder = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "cbCreateSubfoldersForTopLevelFolder", optionsWidget));
    GT_CHECK(NULL != subfolderForEachFolder, "subfolderForeachFolder is NULL");
    GTCheckBox::setChecked(os, subfolderForEachFolder, data.value(CREATE_SUBFOLDER_FOR_TOP_LEVEL_FOLDER).toBool());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setCreateSubfolderForEachFile"
void ImportOptionsWidgetFiller::setCreateSubfolderForEachFile(U2OpStatus &os, ImportOptionsWidget *optionsWidget, const QVariantMap &data) {
    CHECK(data.contains(CREATE_SUBFOLDER_FOR_EACH_FILE), );
    QCheckBox* subfolderForEachFile = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "cbCreateSubfoldersForFiles", optionsWidget));
    GT_CHECK(NULL != subfolderForEachFile, "subfolderForEachFile is NULL");
    GTCheckBox::setChecked(os, subfolderForEachFile, data.value(CREATE_SUBFOLDER_FOR_EACH_FILE).toBool());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setImportUnknownAsUdr"
void ImportOptionsWidgetFiller::setImportUnknownAsUdr(U2OpStatus &os, ImportOptionsWidget *optionsWidget, const QVariantMap &data) {
    CHECK(data.contains(IMPORT_UNKNOWN_AS_UDR), );
    QCheckBox* importUnknown = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "cbImportUnrecognized", optionsWidget));
    GT_CHECK(NULL != importUnknown, "importUnknown is NULL");
    GTCheckBox::setChecked(os, importUnknown, data.value(IMPORT_UNKNOWN_AS_UDR).toBool());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setMultiSequencePolicy"
void ImportOptionsWidgetFiller::setMultiSequencePolicy(U2OpStatus &os, ImportOptionsWidget *optionsWidget, const QVariantMap &data) {
    CHECK(data.contains(MULTI_SEQUENCE_POLICY), );

    ImportToDatabaseOptions::MultiSequencePolicy policy = static_cast<ImportToDatabaseOptions::MultiSequencePolicy>(data.value(MULTI_SEQUENCE_POLICY).toInt());
    switch (policy) {
    case ImportToDatabaseOptions::SEPARATE: {
        QRadioButton* separate = qobject_cast<QRadioButton*>(GTWidget::findWidget(os, "rbSeparate", optionsWidget));
        GT_CHECK(NULL != separate, "separate is NULL");
        GTRadioButton::click(os, separate);
        break;
    }
    case ImportToDatabaseOptions::MERGE: {
        QRadioButton* merge = qobject_cast<QRadioButton*>(GTWidget::findWidget(os, "rbMerge", optionsWidget));
        GT_CHECK(NULL != merge, "merge is NULL");
        GTRadioButton::click(os, merge);
        setMergeMultiSequencePolicySeparatorSize(os, optionsWidget, data);
        break;
    }
    case ImportToDatabaseOptions::MALIGNMENT: {
        QRadioButton* malignment = qobject_cast<QRadioButton*>(GTWidget::findWidget(os, "rbMalignment", optionsWidget));
        GT_CHECK(NULL != malignment, "malignment is NULL");
        GTRadioButton::click(os, malignment);
        break;
    }
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setCreateSubfolderForDocument"
void ImportOptionsWidgetFiller::setCreateSubfolderForDocument(U2OpStatus &os, ImportOptionsWidget *optionsWidget, const QVariantMap &data) {
    CHECK(data.contains(CREATE_SUBFOLDER_FOR_DOCUMENT), );
    QCheckBox* subfolderForDocument = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "cbCreateSubfoldersforDocs", optionsWidget));
    GT_CHECK(NULL != subfolderForDocument, "subfolderForDocument is NULL");
    GTCheckBox::setChecked(os, subfolderForDocument, data.value(CREATE_SUBFOLDER_FOR_DOCUMENT).toBool());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setMergeMultiSequencePolicySeparatorSize"
void ImportOptionsWidgetFiller::setMergeMultiSequencePolicySeparatorSize(U2OpStatus &os, ImportOptionsWidget *optionsWidget, const QVariantMap &data) {
    CHECK(data.contains(MERGE_MULTI_SEQUENCE_POLICY_SEPARATOR_SIZE), );
    QSpinBox* separatorSize = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "sbMerge", optionsWidget));
    GT_CHECK(NULL != separatorSize, "separatorSize is NULL");
    GTSpinBox::setValue(os, separatorSize, data.value(MERGE_MULTI_SEQUENCE_POLICY_SEPARATOR_SIZE).toInt());
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
