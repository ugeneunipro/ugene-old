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

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#endif

#include "ExportCoverageDialogFiller.h"
#include "api/GTCheckBox.h"
#include "api/GTFileDialog.h"
#include "api/GTLineEdit.h"
#include "api/GTSpinBox.h"
#include "api/GTWidget.h"
#include "runnables/qt/MessageBoxFiller.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::BranchSettingsDialogFiller"

const QString ExportCoverageDialogFiller::SPINBOX_MINIMUM = "minimum";
const QString ExportCoverageDialogFiller::SPINBOX_MAXIMUM = "maximum";

ExportCoverageDialogFiller::ExportCoverageDialogFiller(U2OpStatus &os, const QList<Action> &actions) :
    Filler(os, "ExportCoverageDialog"),
    dialog(NULL),
    actions(actions)
{
}

#define GT_METHOD_NAME "run"
void ExportCoverageDialogFiller::run() {
    dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "Active modal widget not found");

    foreach (const Action &action, actions) {
    switch (action.first) {
    case EnterFilePath:
        enterFilePath(action.second);
        break;
    case SelectFile:
        selectFile(action.second);
        break;
    case SetCompress:
        setCompress(action.second);
        break;
    case SetExportCoverage:
        setExportCoverage(action.second);
        break;
    case SetExportBasesCount:
        setExportBasesCount(action.second);
        break;
    case SetThreshold:
        setThreshold(action.second);
        break;
    case CheckFilePath:
        checkFilePath(action.second);
        break;
    case CheckCompress:
        checkCompress(action.second);
        break;
    case CheckExportCoverage:
        checkExportCoverage(action.second);
        break;
    case CheckExportBasesCount:
        checkExportBasesCount(action.second);
        break;
    case CheckThreshold:
        checkThreshold(action.second);
        break;
    case CheckThresholdBounds:
        checkThresholdBounds(action.second);
        break;
    case ExpectMessageBox:
        expectMessageBox();
        break;
    case ClickOk:
        clickOk();
        break;
    case ClickCancel: {
        clickCancel();
        break;
    }
    default:
        os.setError("An unexpected dialog action");
        clickCancel();
        break;
    }
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "enterFilePath"
void ExportCoverageDialogFiller::enterFilePath(const QVariant &actionData) {
    CHECK_OP(os, );
    GT_CHECK(actionData.canConvert<QString>(), "Can't a get file path from the action data");
    GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit *>(os, "leFilePath", dialog), actionData.toString());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "selectFile"
void ExportCoverageDialogFiller::selectFile(const QVariant &actionData) {
    CHECK_OP(os, );
    GT_CHECK(actionData.canConvert<QString>(), "Can't a get file path from the action data");
    const QString dirPath = QFileInfo(actionData.toString()).dir().path();
    QDir().mkpath(dirPath);
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os,
                                                           dirPath,
                                                           QFileInfo(actionData.toString()).fileName(),
                                                           GTFileDialogUtils::Save));
    GTWidget::click(os, GTWidget::findWidget(os, "tbFilePath", dialog));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setCompress"
void ExportCoverageDialogFiller::setCompress(const QVariant &actionData) {
    CHECK_OP(os, );
    GT_CHECK(actionData.canConvert<bool>(), "Can't get a checkbox state from the action data");
    GTCheckBox::setChecked(os, GTWidget::findExactWidget<QCheckBox *>(os, "chbCompress", dialog), actionData.toBool());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setExportCoverage"
void ExportCoverageDialogFiller::setExportCoverage(const QVariant &actionData) {
    CHECK_OP(os, );
    GT_CHECK(actionData.canConvert<bool>(), "Can't get a checkbox state from the action data");
    GTCheckBox::setChecked(os, GTWidget::findExactWidget<QCheckBox *>(os, "chbCoverage", dialog), actionData.toBool());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setExportBasesCount"
void ExportCoverageDialogFiller::setExportBasesCount(const QVariant &actionData) {
    CHECK_OP(os, );
    GT_CHECK(actionData.canConvert<bool>(), "Can't get a checkbox state from the action data");
    GTCheckBox::setChecked(os, GTWidget::findExactWidget<QCheckBox *>(os, "chbBasesCount", dialog), actionData.toBool());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setThreshold"
void ExportCoverageDialogFiller::setThreshold(const QVariant &actionData) {
    CHECK_OP(os, );
    GT_CHECK(actionData.canConvert<int>(), "Can't get a threshold value from the action data");
    GTSpinBox::setValue(os, GTWidget::findExactWidget<QSpinBox *>(os, "sbThreshold", dialog), actionData.toInt());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkFilePath"
void ExportCoverageDialogFiller::checkFilePath(const QVariant &actionData) {
    CHECK_OP(os, );
    GT_CHECK(actionData.canConvert<QString>(), "Can't get an expected file path from the action data");
    QLineEdit *leFilePath = GTWidget::findExactWidget<QLineEdit *>(os, "leFilePath", dialog);
    GT_CHECK(NULL != leFilePath, "A file path line edit wasn't found");
    GT_CHECK(leFilePath->text() == actionData.toString(), QString("An unexpected file path: expected '%1', got '%2'").arg(actionData.toString()).arg(leFilePath->text()));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkCompress"
void ExportCoverageDialogFiller::checkCompress(const QVariant &actionData) {
    CHECK_OP(os, );
    GT_CHECK(actionData.canConvert<bool>(), "Can't get an expected checkbox state from the action data");
    QCheckBox *chbCompress = GTWidget::findExactWidget<QCheckBox *>(os, "chbCompress", dialog);
    GT_CHECK(NULL != chbCompress, "A compress checkbox wasn't found");
    GT_CHECK(chbCompress->isChecked() == actionData.toBool(), "An unexpected checkbox state");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkExportCoverage"
void ExportCoverageDialogFiller::checkExportCoverage(const QVariant &actionData) {
    CHECK_OP(os, );
    GT_CHECK(actionData.canConvert<bool>(), "Can't get an expected checkbox state from the action data");
    QCheckBox *chbCoverage = GTWidget::findExactWidget<QCheckBox *>(os, "chbCoverage", dialog);
    GT_CHECK(NULL != chbCoverage, "An export coverage checkbox wasn't found");
    GT_CHECK(chbCoverage->isChecked() == actionData.toBool(), "An unexpected checkbox state");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkExportBasesCount"
void ExportCoverageDialogFiller::checkExportBasesCount(const QVariant &actionData) {
    CHECK_OP(os, );
    GT_CHECK(actionData.canConvert<bool>(), "Can't get an expected checkbox state from the action data");
    QCheckBox *chbBasesCount = GTWidget::findExactWidget<QCheckBox *>(os, "chbBasesCount", dialog);
    GT_CHECK(NULL != chbBasesCount, "An export bases count checkbox wasn't found");
    GT_CHECK(chbBasesCount->isChecked() == actionData.toBool(), "An unexpected checkbox state");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkThreshold"
void ExportCoverageDialogFiller::checkThreshold(const QVariant &actionData) {
    CHECK_OP(os, );
    GT_CHECK(actionData.canConvert<int>(), "Can't get an expected spinbox value from the action data");
    QSpinBox *sbThreshold = GTWidget::findExactWidget<QSpinBox *>(os, "sbThreshold", dialog);
    GT_CHECK(NULL != sbThreshold, "A threshold spinbox wasn't found");
    GT_CHECK(sbThreshold->value() == actionData.toInt(), "An unexpected spinbox value");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkThresholdBounds"
void ExportCoverageDialogFiller::checkThresholdBounds(const QVariant &actionData) {
    GT_CHECK(actionData.canConvert<QVariantMap>(), "Can't get a map with expected spinbox boundsvalues from the action data");
    QSpinBox *sbThreshold = GTWidget::findExactWidget<QSpinBox *>(os, "sbThreshold", dialog);
    GT_CHECK(NULL != sbThreshold, "A threshold spinbox wasn't found");
    const QVariantMap spinboxBounds = actionData.toMap();
    GT_CHECK(spinboxBounds.contains(SPINBOX_MINIMUM), "A minimum spinbox value is not defined");
    GT_CHECK(spinboxBounds.contains(SPINBOX_MAXIMUM), "A maximum spinbox value is not defined");
    GTSpinBox::checkLimits(os, sbThreshold, spinboxBounds[SPINBOX_MINIMUM].toInt(), spinboxBounds[SPINBOX_MAXIMUM].toInt());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "expectMessageBox"
void ExportCoverageDialogFiller::expectMessageBox() {
    CHECK_OP(os, );
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickOk"
void ExportCoverageDialogFiller::clickOk() {
    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickCancel"
void ExportCoverageDialogFiller::clickCancel() {
    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
