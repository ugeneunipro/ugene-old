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

#include "GTUtilsDialog.h"
#include "api/GTCheckBox.h"
#include "api/GTComboBox.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTRadioButton.h"
#include "api/GTSpinBox.h"
#include "api/GTPlainTextEdit.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include <U2Gui/MainWindow.h>
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QMenu>
#include <QtGui/QRadioButton>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QSpinBox>
#include <QtGui/QToolButton>
#include <QtGui/QDialogButtonBox>
#include <QDebug>


namespace U2 {

void GUIDialogWaiter::wait() {
    QWidget *widget = NULL;

    switch (type) {
        case Modal:
            widget = QApplication::activeModalWidget();
            break;

        case Popup:
            widget = QApplication::activePopupWidget();
            break;

        default:
            break;
    }

    if (!widget) {
        return;
    }

    if (r && !hadRun) {
        hadRun = true;
        r->run();
    }
}

#define GT_CLASS_NAME "GTUtilsDialog"

#define GT_METHOD_NAME "waitForDialog"
void GTUtilsDialog::waitForDialog(U2OpStatus &os, Runnable *r, GUIDialogWaiter::DialogType type, bool failOnNoDialog) {

    GUIDialogWaiter waiter(r, type);
    QTimer t;

    t.connect(&t, SIGNAL(timeout()), &waiter, SLOT(wait()));
    t.start(100);

    GTGlobals::sleep(1000);
    if (failOnNoDialog) {
        GT_CHECK(waiter.hadRun == true, "no dialog");
    }
}
#undef GT_METHOD_NAME

void GTUtilsDialog::preWaitForDialog(U2OpStatus &os, Runnable *r, GUIDialogWaiter::DialogType _type, int msec)
{
    GUIDialogWaiter *waiter = new GUIDialogWaiter(r, _type);
    QTimer *t = new QTimer;

    t->connect(t, SIGNAL(timeout()), waiter, SLOT(wait()));
    t->start(msec);
}

#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::SmithWatermanDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::SmithWatermanDialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QAbstractButton *cancelButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "bttnCancel", dialog));
    GT_CHECK(cancelButton != NULL, "Radiobutton is NULL");

    if (button == Cancel) {
        GTWidget::click(os, cancelButton);
        return;
    }

    GT_CHECK(true, "Not implemented");
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::CreateAnnotationDialogChecker"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::CreateAnnotationDialogChecker::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QAbstractButton *radioButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "existingObjectRB", dialog));
    GT_CHECK(radioButton != NULL, "Radiobutton is NULL");
    GT_CHECK(!radioButton->isEnabled() && !radioButton->isCheckable(), "radioButton is enabled or checkable");

    QComboBox *comboBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "existingObjectCombo", dialog));
    GT_CHECK(comboBox != NULL, "ComboBox is NULL");
    GT_CHECK(comboBox->count() == 0, "comboBox count is not 0");

    QAbstractButton *b = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "existingObjectButton", dialog));
    GT_CHECK(b != NULL, "Button is NULL");
    GT_CHECK(b->isEnabled() == false, "Button is enabled");

    QWidget *cancelButton = GTWidget::findWidget(os, "cancel_button", dialog);
    GT_CHECK(cancelButton != NULL, "Cancel button is NULL");
    GTWidget::click(os, cancelButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::CreateAnnotationDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::CreateAnnotationDialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QLineEdit *groupNameLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "groupNameEdit", dialog));
    GT_CHECK(groupNameLineEdit != NULL, "LineEdit is NULL");
    GTLineEdit::setText(os, groupNameLineEdit, groupName);

    QLineEdit *annotationNameLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "annotationNameEdit", dialog));
    GT_CHECK(annotationNameLineEdit != NULL, "LineEdit is NULL");
    GTLineEdit::setText(os, annotationNameLineEdit, annotationName);

    QLineEdit *locationLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "locationEdit", dialog));
    GT_CHECK(locationLineEdit != NULL, "LineEdit is NULL");
    GTLineEdit::setText(os, locationLineEdit, location);

    QAbstractButton *createButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "create_button", dialog));
    GTWidget::click(os, createButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::ExportProjectDialogChecker"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::ExportProjectDialogChecker::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QLineEdit *projectFileLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectFileEdit", dialog));
    GT_CHECK(projectFileLineEdit != NULL, "LineEdit is NULL");
    GT_CHECK(projectFileLineEdit->text() == projectName, "Project name is not " + projectName);

    QAbstractButton *cancelButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "CancelButton", dialog));
    GT_CHECK(cancelButton != NULL, "CancelButton is NULL");
    GTWidget::click(os, cancelButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::ExportProjectDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::ExportProjectDialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QLineEdit *projectFolderLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "exportFolderEdit", dialog));
    GT_CHECK(projectFolderLineEdit != NULL, "LineEdit is NULL");
    GTLineEdit::setText(os, projectFolderLineEdit, projectFolder);

    QLineEdit *projectFileLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectFileEdit", dialog));
    GT_CHECK(projectFileLineEdit != NULL, "LineEdit is NULL");
    if (!projectName.isEmpty()) {
        GTLineEdit::setText(os, projectFileLineEdit, projectName);
    }

    QAbstractButton *okButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "OKButton", dialog));
    GT_CHECK(okButton != NULL, "CancelButton is NULL");
    GTWidget::click(os, okButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::MessageBoxDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::MessageBoxDialogFiller::run() {

    QWidget* activeModal = QApplication::activeModalWidget();
    QMessageBox *messageBox = qobject_cast<QMessageBox*>(activeModal);
    GT_CHECK(messageBox != NULL, "messageBox is NULL");

    QAbstractButton* button = messageBox->button(b);
    GT_CHECK(button != NULL, "There is no such button in messagebox");

    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
 
#define GT_CLASS_NAME "GTUtilsDialog::SaveProjectAsDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::SaveProjectAsDialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QLineEdit *projectNameEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectNameEdit", dialog));
    GTLineEdit::setText(os, projectNameEdit, projectName);

    QLineEdit *projectFolderEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectFolderEdit", dialog));
    GTLineEdit::setText(os, projectFolderEdit, projectFolder);

    QLineEdit *projectFileEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectFileEdit", dialog));
    GTLineEdit::setText(os, projectFileEdit, projectFile);

    QAbstractButton *createButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "createButton", dialog));
    GTWidget::click(os, createButton);
 
    GTUtilsDialog::MessageBoxDialogFiller filler(os, QMessageBox::Yes);
    GTUtilsDialog::waitForDialog(os, &filler, GUIDialogWaiter::Modal, false); // MessageBox question appears only if there is already a file on a disk
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

void GTUtilsDialog::PopupChooser::run()
{
    GTGlobals::sleep(1000);
    QMenu* activePopupMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    GTMenu::clickMenuItem(os, activePopupMenu, namePath, useMethod);
}

#define GT_CLASS_NAME "GTUtilsDialog::RemoteDBDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::RemoteDBDialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QLineEdit *idLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "idLineEdit", dialog));
    GTLineEdit::setText(os, idLineEdit, resID);

    QLineEdit *saveFilenameLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "saveFilenameLineEdit", dialog));
    if(!saveDirPath.isEmpty()){
        GTLineEdit::setText(os, saveFilenameLineEdit, saveDirPath);
    }

    QComboBox *databasesBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "databasesBox", dialog));
    GTComboBox::setCurrentIndex(os, databasesBox, DBItemNum);

    if (pressCancel) {
        QAbstractButton *rejectButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "rejectButton", dialog));
        GTWidget::click(os, rejectButton);
    }
    else {
        QAbstractButton *acceptButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "acceptButton", dialog));
        GTWidget::click(os, acceptButton);
    }
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::ExportToSequenceFormatFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::ExportToSequenceFormatFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *lineEdit = dialog->findChild<QLineEdit*>();
    GT_CHECK(lineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, lineEdit, path + name);

    QRadioButton *button = dialog->findChild<QRadioButton*>(QString::fromUtf8("keepGapsRB"));
    GT_CHECK(button != NULL, "radio button not found");
    GTRadioButton::click(os, button);

    GTGlobals::sleep(100);

    QPushButton *okButton = dialog->findChild<QPushButton*>(QString::fromUtf8("okButton"));
    GT_CHECK(okButton != NULL, "OK button not found");
    GTWidget::click(os, okButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::ExportSelectedRegionFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::ExportSelectedRegionFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "fileNameEdit", dialog));
    GT_CHECK(lineEdit != NULL, "File name line edit not found");
    GTLineEdit::setText(os, lineEdit, path + name);

    QPushButton *okButton = dialog->findChild<QPushButton*>(QString::fromUtf8("exportButton"));
    GT_CHECK(okButton != NULL, "Export button not found");
    GTWidget::click(os, okButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::ExportSequenceAsAlignmentFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::ExportSequenceAsAlignmentFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *lineEdit = dialog->findChild<QLineEdit*>();
    GT_CHECK(lineEdit != NULL, "line edit not found");

    GTLineEdit::setText(os, lineEdit, path + name);

    QComboBox *comboBox = dialog->findChild<QComboBox*>();
    GT_CHECK(comboBox != NULL, "ComboBox not found");

    int index = comboBox->findText(comboBoxItems[format]);
    GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));
    GTComboBox::setCurrentIndex(os, comboBox, index);

    QCheckBox *checkBox = dialog->findChild<QCheckBox*>(QString::fromUtf8("addToProjectBox"));
    GTCheckBox::setChecked(os, checkBox, addToProject);

    QPushButton *exportButton = dialog->findChild<QPushButton*>(QString::fromUtf8("okButton"));
    GT_CHECK(exportButton != NULL, "Export button not found");

    GTWidget::click(os, exportButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::CopyToFileAsDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::CopyToFileAsDialogFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *lineEdit = dialog->findChild<QLineEdit*>("fileNameEdit");
    GT_CHECK(lineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, lineEdit, path + name);

    QComboBox *comboBox = dialog->findChild<QComboBox*>("formatCombo");
    GT_CHECK(comboBox != NULL, "ComboBox not found");
    int index = comboBox->findText(comboBoxItems[format]);

    GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));
    if (comboBox->currentIndex() != index){
        GTComboBox::setCurrentIndex(os, comboBox, index);
    }

    QCheckBox *compressCheckBox = dialog->findChild<QCheckBox*>(QString::fromUtf8("compressCheck"));
    GT_CHECK(compressCheckBox != NULL, "Check box not found");
    GTCheckBox::setChecked(os, compressCheckBox, compressFile);

    QCheckBox *addCheckBox = dialog->findChild<QCheckBox*>(QString::fromUtf8("addToProjCheck"));
    GT_CHECK(addCheckBox != NULL, "Check box not found");
    GTCheckBox::setChecked(os, addCheckBox, addToProject);

    QPushButton *btSave = dialog->findChild<QPushButton*>(QString::fromUtf8("createButton"));
    GT_CHECK(btSave != NULL, "Save button not found");
    GTWidget::click(os, btSave);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::exportSequenceOfSelectedAnnotationsFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::ExportSequenceOfSelectedAnnotationsFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *fileNameEdit = dialog->findChild<QLineEdit*>("fileNameEdit");
    GT_CHECK(fileNameEdit != NULL, "fileNameEdit not found");
    GTLineEdit::setText(os, fileNameEdit, path);

    GTGlobals::sleep(200);

    QComboBox *comboBox = dialog->findChild<QComboBox*>();
    GT_CHECK(comboBox != NULL, "ComboBox not found");
    int index = comboBox->findText(comboBoxItems[format]);

    GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));
    if (comboBox->currentIndex() != index){
        GTComboBox::setCurrentIndex(os, comboBox, index);
    }

    GTGlobals::sleep(200);

    QCheckBox *projectCheckBox = dialog->findChild<QCheckBox*>(QString::fromUtf8("addToProjectBox"));
    GT_CHECK(projectCheckBox != NULL, "addToProjectBox not found");
    GTCheckBox::setChecked(os, projectCheckBox, addToProject);

    GTGlobals::sleep(200);

    QCheckBox *annotationsCheckBox = dialog->findChild<QCheckBox*>(QString::fromUtf8("withAnnotationsBox"));
    GT_CHECK(annotationsCheckBox != NULL, "Check box not found");
    GTCheckBox::setChecked(os, annotationsCheckBox, exportWithAnnotations);

    GTGlobals::sleep(200);

    QRadioButton *mergeButton = dialog->findChild<QRadioButton*>(mergeRadioButtons[options]);

    GT_CHECK(mergeButton != NULL, "Radio button " + mergeRadioButtons[options] + " not found");
    if (mergeButton->isEnabled()){
        GTRadioButton::click(os, mergeButton);
    }

    GTGlobals::sleep(200);

    QSpinBox *mergeSpinBox = dialog->findChild<QSpinBox*>("mergeSpinBox");
    GT_CHECK(mergeSpinBox != NULL, "SpinBox not found");
    GTSpinBox::setValue(os, mergeSpinBox, gapLength);

    GTGlobals::sleep(200);

    QPushButton *exportButton = dialog->findChild<QPushButton*>(QString::fromUtf8("exportButton"));
    GT_CHECK(exportButton != NULL, "Export button not found");
    GTWidget::click(os, exportButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::ExportAnnotationsFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::ExportAnnotationsFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *lineEdit = dialog->findChild<QLineEdit*>("fileNameEdit");
    GT_CHECK(lineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, lineEdit, exportToFile);

    QComboBox *comboBox = dialog->findChild<QComboBox*>();
    GT_CHECK(comboBox != NULL, "ComboBox not found");

    int index = comboBox->findText(comboBoxItems[format]);
    GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));
    if (comboBox->currentIndex() != index){
        GTComboBox::setCurrentIndex(os, comboBox, index);
    }

    QCheckBox *checkButton = dialog->findChild<QCheckBox*>(QString::fromUtf8("exportSequenceCheck"));
    GT_CHECK(checkButton != NULL, "Check box not found");
    GTCheckBox::setChecked(os, checkButton, saveSequencesUnderAnnotations);

    checkButton = dialog->findChild<QCheckBox*>(QString::fromUtf8("exportSequenceNameCheck"));
    GT_CHECK(checkButton != NULL, "Check box not found");
    GTCheckBox::setChecked(os, checkButton, saveSequenceNames);

    QDialogButtonBox* buttonBox = dialog->findChild<QDialogButtonBox*>("buttonBox");

    GT_CHECK(buttonBox != NULL, "buttonBox is NULL");
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    GT_CHECK(okButton != NULL, "okButton is NULL");
    GTWidget::click(os, okButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::exportSequenceOfSelectedAnnotationsFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::selectSequenceRegionDialogFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    if (selectAll) {
        QToolButton *min = dialog->findChild<QToolButton*>("minButton");
        QToolButton *max = dialog->findChild<QToolButton*>("maxButton");
        GT_CHECK(min != NULL, "Min button not found");
        GT_CHECK(max != NULL, "Max button not found");

        GTWidget::click(os, min);
        GTGlobals::sleep(500);
        GTWidget::click(os, max);
        GTGlobals::sleep(500);

        if (len != NULL) {
            QLineEdit *endEdit = dialog->findChild<QLineEdit*>("endEdit");
            GT_CHECK(endEdit != NULL, "QLineEdit \"endEdit\" not found");
            *len = endEdit->text().toInt();
        }
    } else if (rangeType == Single) {
        GT_CHECK(minVal <= maxVal, "Value \"min\" greater then \"max\"");

        QLineEdit *startEdit = dialog->findChild<QLineEdit*>("startEdit");
        QLineEdit *endEdit = dialog->findChild<QLineEdit*>("endEdit");
        GT_CHECK(startEdit != NULL, "QLineEdit \"startEdit\" not found");
        GT_CHECK(endEdit != NULL, "QLineEdit \"endEdit\" not found");

        if (length == 0) {
            GTLineEdit::setText(os, startEdit, QString::number(minVal));
            GTLineEdit::setText(os, endEdit, QString::number(maxVal));
        } else {
            int min = startEdit->text().toInt();
            int max = endEdit->text().toInt();
            GT_CHECK(max - min >= length, "Invalid argument \"length\"");

            if (fromBegin) {
                GTLineEdit::setText(os, startEdit, QString::number(1));
                GTLineEdit::setText(os, endEdit, QString::number(length));
            } else {
                GTLineEdit::setText(os, startEdit, QString::number(max - length + 1));
                GTLineEdit::setText(os, endEdit, QString::number(max));
            }
        }
    } else {
        GT_CHECK(! multipleRange.isEmpty(), "Range is empty");

        QRadioButton *multipleButton = dialog->findChild<QRadioButton*>("miltipleButton");
        GT_CHECK(multipleButton != NULL, "RadioButton \"miltipleButton\" not found");
        GTRadioButton::click(os, multipleButton);

        QLineEdit *regionEdit = dialog->findChild<QLineEdit*>("multipleRegionEdit");
        GT_CHECK(regionEdit != NULL, "QLineEdit \"multipleRegionEdit\" not foud");
        GTLineEdit::setText(os, regionEdit, multipleRange);
    }

    QPushButton *okButton = dialog->findChild<QPushButton*>("okButton");
    GT_CHECK(okButton != NULL, "OK button not found");
    GTWidget::click(os, okButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::createDocumentFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::CreateDocumentFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QPlainTextEdit *plainText = dialog->findChild<QPlainTextEdit*>("sequenceEdit");
    GT_CHECK(plainText != NULL, "plain text not found");
    GTPlainTextEdit::setPlainText(os, plainText, pasteDataHere);

    QLineEdit *lineEdit = dialog->findChild<QLineEdit*>("filepathEdit");
    GT_CHECK(lineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, lineEdit, documentLocation);

    QComboBox *comboBox = dialog->findChild<QComboBox*>();
    GT_CHECK(comboBox != NULL, "ComboBox not found");

    int index = comboBox->findText(comboBoxItems[format]);
    GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));
    if (comboBox->currentIndex() != index){
        GTComboBox::setCurrentIndex(os, comboBox, index);
    }

    QLineEdit *lineEditName = dialog->findChild<QLineEdit*>("nameEdit");
    GT_CHECK(lineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, lineEditName, sequenceName);

    QPushButton *createButton = dialog->findChild<QPushButton*>(QString::fromUtf8("OKButton"));
    GT_CHECK(createButton != NULL, "Create button not found");

    GTWidget::click(os, createButton);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::insertSequenceFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::InsertSequenceFiller::run()
{
    qDebug() <<  "IN RUN";
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QPlainTextEdit *plainText = dialog->findChild<QPlainTextEdit*>("sequenceEdit");
    GT_CHECK(plainText != NULL, "plain text not found");
    GTPlainTextEdit::setPlainText(os, plainText, pasteDataHere);

    QString radioButtonName;
    switch (regionResolvingMode) {
        case Resize:
            radioButtonName = "resizeRB";
            break;

        case Remove:
            radioButtonName = "removeRB";
            break;

        case SplitJoin:
            radioButtonName = "splitRB";
            break;

        default:
        case SplitSeparate:
            radioButtonName = "split_separateRB";
            break;
    }
    QRadioButton *regionResolvingMode = dialog->findChild<QRadioButton*>(radioButtonName);//"regionResolvingMode");
    GT_CHECK(regionResolvingMode != NULL, "regionResolvingMode not found");
    GTRadioButton::click(os, regionResolvingMode);

    QSpinBox *insertPositionSpin = dialog->findChild<QSpinBox*>("insertPositionSpin");
    GT_CHECK(insertPositionSpin != NULL, "insertPositionSpin not found");
    GTSpinBox::setValue(os, insertPositionSpin, insertPosition);

    QGroupBox *checkButton = dialog->findChild<QGroupBox*>(QString::fromUtf8("saveToAnotherBox"));
    GT_CHECK(checkButton != NULL, "Check box not found");

    if ((saveToNewFile && !checkButton->isChecked()) ||
       (!saveToNewFile && checkButton->isChecked())) {
        QPoint checkPos;
        switch(useMethod) {
        case GTGlobals::UseMouse:
            checkPos = QPoint(checkButton->rect().left() + 12, checkButton->rect().top() + 12);
            GTMouseDriver::moveTo(os, checkButton->mapToGlobal(checkPos));
            GTMouseDriver::click(os);
            break;
        case GTGlobals::UseKey:
            GTWidget::setFocus(os, checkButton);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
            break;
        }
    }

    GTGlobals::sleep(1000);

    QCheckBox *checkButton1 = dialog->findChild<QCheckBox*>(QString::fromUtf8("mergeAnnotationsBox"));
    GT_CHECK(checkButton1 != NULL, "Check box not found");
    GTCheckBox::setChecked(os, checkButton1, mergeAnnotations);

    if (saveToNewFile) {
        QLineEdit *lineEdit = dialog->findChild<QLineEdit*>("filepathEdit");
        GT_CHECK(lineEdit != NULL, "line edit not found");
        GTLineEdit::setText(os, lineEdit, documentLocation);
    }

    QComboBox *comboBox = dialog->findChild<QComboBox*>();
    GT_CHECK(comboBox != NULL, "ComboBox not found");

    int index = comboBox->findText(comboBoxItems[format]);
    GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));
    if (comboBox->currentIndex() != index){
        GTComboBox::setCurrentIndex(os, comboBox, index);
    }

    QPushButton *createButton = dialog->findChild<QPushButton*>(QString::fromUtf8("OKButton"));
    GT_CHECK(createButton != NULL, "Create button not found");

    GTWidget::click(os, createButton);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::RemovePartFromSequenceDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::RemovePartFromSequenceDialogFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    if (!range.isEmpty()){
        QLineEdit *removeLocationEdit = dialog->findChild<QLineEdit*>(QString::fromUtf8("removeLocationEdit"));
        GT_CHECK(removeLocationEdit != NULL, "QLineEdit \"removeLocationEdit\" not found");
        GTLineEdit::setText(os, removeLocationEdit, range);
    }
    if (removeType == Resize) {
        QRadioButton *resizeRB = dialog->findChild<QRadioButton*>(QString::fromUtf8("resizeRB"));
        GT_CHECK(resizeRB != NULL, "radio button not found");
        GTRadioButton::click(os, resizeRB);
    }
    else {
        QRadioButton *removeRB = dialog->findChild<QRadioButton*>(QString::fromUtf8("removeRB"));
        GT_CHECK(removeRB != NULL, "radio button not found");
        GTRadioButton::click(os, removeRB);
    }

    GTGlobals::sleep(1000);
    if (saveNew) {
        QGroupBox *saveToAnotherBox =  dialog->findChild<QGroupBox*>(QString::fromUtf8("saveToAnotherBox"));
        saveToAnotherBox->setChecked(true);
        QLineEdit *filepathEdit = dialog->findChild<QLineEdit*>(QString::fromUtf8("filepathEdit"));
        GT_CHECK(filepathEdit != NULL, "QLineEdit \"filepathEdit\" not found");

        GTLineEdit::setText(os, filepathEdit, saveToFile);
    }
    if (format != FASTA){
        QComboBox *formatBox = dialog->findChild<QComboBox*>(QString::fromUtf8("formatBox"));
        GT_CHECK(formatBox != NULL, "format box not found");
        int index = formatBox->findText(comboBoxItems[format]);
        GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));

        GTComboBox::setCurrentIndex(os, formatBox, index);
    }
    QPushButton *okButton = dialog->findChild<QPushButton*>("OKButton");
    GT_CHECK(okButton != NULL, "OK button not found");
    GTWidget::click(os, okButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::SequenceReadingModeSelectorDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::SequenceReadingModeSelectorDialogFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");
    if (readingMode == Separate) {
        QRadioButton *separateRB = dialog->findChild<QRadioButton*>(QString::fromUtf8("separateRB"));
        GT_CHECK(separateRB != NULL, "radio button not found");
        GTRadioButton::click(os, separateRB);
    }
    if (readingMode == Merge) {
        QRadioButton *mergeRB = dialog->findChild<QRadioButton*>(QString::fromUtf8("mergeRB"));
        GT_CHECK(mergeRB != NULL, "radio button not found");
        GTRadioButton::click(os, mergeRB);

        QSpinBox *mergeSpinBox = dialog->findChild<QSpinBox*>(QString::fromUtf8("mergeSpinBox"));
        GT_CHECK(mergeSpinBox != NULL, "merge spin box not found");
        GTSpinBox::setValue(os, mergeSpinBox, bases);
    }
    if (readingMode == Join) {
        QRadioButton *malignmentRB = dialog->findChild<QRadioButton*>(QString::fromUtf8("malignmentRB"));
        GT_CHECK(malignmentRB != NULL, "radio button not found");
        GTRadioButton::click(os, malignmentRB);
    }
    if (readingMode == Align) {
        QRadioButton *refalignmentRB = dialog->findChild<QRadioButton*>(QString::fromUtf8("refalignmentRB"));
        GT_CHECK(refalignmentRB != NULL, "radio button not found");
        GTRadioButton::click(os, refalignmentRB);
    }
    QDialogButtonBox *buttonBox = dialog->findChild<QDialogButtonBox*>(QString::fromUtf8("buttonBox"));
    GT_CHECK(buttonBox != NULL, "buttonBox not found");

    QPushButton *button = buttonBox->button(QDialogButtonBox::Ok);
    GT_CHECK(button != NULL, "standart button not found");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::ZoomToRangeDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::ZoomToRangeDialogFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *from = dialog->findChild<QLineEdit*>("start_edit_line");
    QLineEdit *to = dialog->findChild<QLineEdit*>("end_edit_line");
    GT_CHECK(from != NULL, "\"start_edit_line\" to  not found");
    GT_CHECK(to != NULL, "\"end_edit_line\" to  not found");

    GTLineEdit::setText(os, from, QString::number(minVal));
    GTLineEdit::setText(os, to, QString::number(maxVal));

    QPushButton *okButton = dialog->findChild<QPushButton*>("ok_button");
    GT_CHECK(okButton != NULL, "Button \"Ok\" to  not found");

    GTWidget::click(os, okButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}//namespace
