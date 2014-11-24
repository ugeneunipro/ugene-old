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
#include "AppSettingsDialogFiller.h"

#include "runnables/qt/ColorDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTTreeWidget.h"
#include "api/GTMouseDriver.h"
#include "api/GTComboBox.h"
#include "api/GTLineEdit.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTListWidget.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QListWidget>
#include <QtGui/QToolButton>
#include <QtGui/QTreeWidget>
#include <QtGui/QComboBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QAbstractButton>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QAbstractButton>
#endif

#include <QTextBrowser>

namespace U2{

#define GT_CLASS_NAME "AppSettingsDialogFiller"
QMap<AppSettingsDialogFiller::Tabs, QString> AppSettingsDialogFiller::initMap(){
        QMap<Tabs, QString> result;
        result.insert(General, "  General");
        result.insert(Resourses, "  Resources");
        result.insert(Network, "  Network");
        result.insert(FileFormat, "  File Format");
        result.insert(Directories, "  Directories");
        result.insert(Logging, "  Logging");
        result.insert(AlignmentColorScheme, "  Alignment Color Scheme");
        result.insert(GenomeAligner, "  Genome Aligner");
        result.insert(WorkflowDesigner, "  Workflow Designer");
        result.insert(ExternalTools, "  External Tools");
        result.insert(OpenCL, "  OpenCL");
        return result;
}

const QMap<AppSettingsDialogFiller::Tabs, QString> AppSettingsDialogFiller::tabMap = initMap();

AppSettingsDialogFiller::AppSettingsDialogFiller(U2OpStatus &os, CustomScenario *customScenario) :
    Filler(os, "AppSettingsDialog", customScenario),
    itemStyle(none),
    r(-1),
    g(-1),
    b(-1)
{
}

#define GT_METHOD_NAME "run"
void AppSettingsDialogFiller::commonScenario(){

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QTreeWidget* tree = qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"tree"));
    GT_CHECK(tree, "tree widger not found");

    QList<QTreeWidgetItem*> items = GTTreeWidget::getItems(tree->invisibleRootItem());
    foreach (QTreeWidgetItem* item,items){
        if(item->text(0)=="  Workflow Designer"){
            GTMouseDriver::moveTo(os,GTTreeWidget::getItemCenter(os,item));
            GTMouseDriver::click(os);
        }
    }
    if(itemStyle!=none){
        QComboBox *styleCombo = qobject_cast<QComboBox*>(GTWidget::findWidget(os,"styleCombo",dialog));
        GTComboBox::setCurrentIndex(os,styleCombo,itemStyle);
    }

    if(r!=-1){
        GTUtilsDialog::waitForDialog(os, new ColorDialogFiller(os,r,g,b));
        QWidget *colorWidget = GTWidget::findWidget(os,"colorWidget",dialog);
        GTWidget::click(os,colorWidget);
    }

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "ok button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setExternalToolPath"
void AppSettingsDialogFiller::setExternalToolPath(U2OpStatus &os, const QString &toolName, const QString &toolPath){
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    openTab(os, ExternalTools);

    QTreeWidget* treeWidget = GTWidget::findExactWidget<QTreeWidget*>(os, "treeWidget", dialog);
    QList<QTreeWidgetItem*> listOfItems = treeWidget->findItems("", Qt::MatchContains | Qt::MatchRecursive);
    bool set = false;
    foreach (QTreeWidgetItem* item, listOfItems){
        if(item->text(0) == toolName){
            QWidget* itemWid = treeWidget->itemWidget(item, 1);
            QLineEdit* lineEdit = itemWid->findChild<QLineEdit*>("PathLineEdit");
            treeWidget->scrollToItem(item);
            GTLineEdit::setText(os, lineEdit, toolPath);
            GTMouseDriver::moveTo(os, GTMouseDriver::getMousePosition() + QPoint(0, -30));
            GTMouseDriver::click(os);
            set = true;
        }
    }
    GT_CHECK(set, "tool " + toolName + " not found in tree view");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getExternalToolPath"
QString AppSettingsDialogFiller::getExternalToolPath(U2OpStatus &os, const QString &toolName){
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK_RESULT(dialog, "activeModalWidget is NULL", "");

    openTab(os, ExternalTools);

    QTreeWidget* treeWidget = GTWidget::findExactWidget<QTreeWidget*>(os, "treeWidget", dialog);
    QList<QTreeWidgetItem*> listOfItems = treeWidget->findItems("", Qt::MatchContains | Qt::MatchRecursive);

    foreach (QTreeWidgetItem* item, listOfItems){
        if(item->text(0) == toolName){
            QWidget* itemWid = treeWidget->itemWidget(item, 1);
            QLineEdit* lineEdit = itemWid->findChild<QLineEdit*>("PathLineEdit");
            return lineEdit->text();
        }
    }
    return "";
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isExternalToolValid"
bool AppSettingsDialogFiller::isExternalToolValid(U2OpStatus &os, const QString &toolName){
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK_RESULT(dialog, "activeModalWidget is NULL", false);

    openTab(os, ExternalTools);

    QTreeWidget* treeWidget = GTWidget::findExactWidget<QTreeWidget*>(os, "treeWidget", dialog);
    QList<QTreeWidgetItem*> listOfItems = treeWidget->findItems("", Qt::MatchContains | Qt::MatchRecursive);
    foreach (QTreeWidgetItem* item, listOfItems){
        if(item->text(0) == toolName){
            GTTreeWidget::click(os, item);
            GTMouseDriver::doubleClick(os);
            QTextBrowser *descriptionTextBrowser = GTWidget::findExactWidget<QTextBrowser*>(os, "descriptionTextBrowser", dialog);
            return descriptionTextBrowser->toPlainText().contains("Version:");
        }
    }
    GT_CHECK_RESULT(false, "external tool " + toolName + " not found in tree view", false);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clearToolPath"
void AppSettingsDialogFiller::clearToolPath(U2OpStatus &os, const QString &toolName) {
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    openTab(os, ExternalTools);

    QTreeWidget* treeWidget = GTWidget::findExactWidget<QTreeWidget*>(os, "treeWidget", dialog);
    QList<QTreeWidgetItem*> listOfItems = treeWidget->findItems("", Qt::MatchContains | Qt::MatchRecursive);
    foreach (QTreeWidgetItem* item, listOfItems){
        if(item->text(0) == toolName){
            QWidget* itemWid = treeWidget->itemWidget(item, 1);
            QToolButton* clearPathButton = itemWid->findChild<QToolButton*>("ClearToolPathButton");
            CHECK_SET_ERR(clearPathButton != NULL, "Clear path button not found");
            treeWidget->scrollToItem(item);
            if (clearPathButton->isEnabled()) {
                GTWidget::click(os, clearPathButton);
            }
        }
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "OpenTab"
void AppSettingsDialogFiller::openTab(U2OpStatus &os, Tabs tab){
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QString itemText = tabMap.value(tab);
    GT_CHECK(!itemText.isEmpty(), "tree element for item not found");

    QTreeWidget* mainTree = GTWidget::findExactWidget<QTreeWidget *>(os, "tree");

    if(mainTree->selectedItems().first()->text(0) != itemText){
        GTTreeWidget::click(os, GTTreeWidget::findItem(os, mainTree, itemText));
    }
    GTGlobals::sleep(300);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

NewColorSchemeCreator::NewColorSchemeCreator(U2OpStatus &_os, QString _schemeName, alphabet _al, Action _act, bool cancel)
    : Filler(_os, "AppSettingsDialog"), schemeName(_schemeName), al(_al), act(_act), cancel(cancel)
{

}

#define GT_CLASS_NAME "NewColorSchemeCreator"
#define GT_METHOD_NAME "run"
void NewColorSchemeCreator::run() {
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QTreeWidget* tree = qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"tree"));
    GT_CHECK(tree, "tree widger not found");

    QList<QTreeWidgetItem*> items = GTTreeWidget::getItems(tree->invisibleRootItem());
    foreach (QTreeWidgetItem* item,items){
        if(item->text(0)=="  Alignment Color Scheme"){
            GTMouseDriver::moveTo(os,GTTreeWidget::getItemCenter(os,item));
            GTMouseDriver::click(os);
        }
    }
    if(act == Delete){
        QListWidget* colorSchemas = qobject_cast<QListWidget*>(GTWidget::findWidget(os, "colorSchemas", dialog));
        GT_CHECK(colorSchemas != NULL, "colorSchemas list widget not found");
        GTListWidget::click(os, colorSchemas, schemeName);
        GTGlobals::sleep(500);

        QWidget* deleteSchemaButton = GTWidget::findWidget(os, "deleteSchemaButton",dialog);
        GT_CHECK (deleteSchemaButton, "deleteSchemaButton not found");
        while(!deleteSchemaButton->isEnabled()){
            uiLog.trace("deleteSchemaButton is disabled");
            GTGlobals::sleep(100);
        }
        GTWidget::click(os, deleteSchemaButton);


    }else if(act == Create){
        QWidget* addSchemaButton = GTWidget::findWidget(os, "addSchemaButton");
        GT_CHECK (addSchemaButton, "addSchemaButton not found");

        GTUtilsDialog::waitForDialog(os, new CreateAlignmentColorSchemeDialogFiller(os, schemeName, al));
        GTWidget::click(os, addSchemaButton);
    }

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = cancel ? box->button(QDialogButtonBox::Cancel) : box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "CreateAlignmentColorSchemeDialogFiller"
#define GT_METHOD_NAME "run"
void CreateAlignmentColorSchemeDialogFiller::run(){
    GTGlobals::sleep( 500 );
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QWidget* w = GTWidget::findWidget(os, "schemeName",dialog);
    QLineEdit* schemeNameLine = qobject_cast<QLineEdit*>(w);
    GT_CHECK(schemeNameLine, "schemeName lineEdit not found ");

    GTLineEdit::setText(os,schemeNameLine, schemeName);

    QComboBox *alphabetComboBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os,"alphabetComboBox",dialog));
    GT_CHECK(alphabetComboBox, "alphabetComboBox lineEdit not found ");

    GTComboBox::setCurrentIndex(os, alphabetComboBox, al);
    GTGlobals::sleep( 500 );

    GTUtilsDialog::waitForDialog(os, new ColorSchemeDialogFiller(os));

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "ok button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "ColorSchemeDialogFiller"
#define GT_METHOD_NAME "run"
void ColorSchemeDialogFiller::run(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QList<QAbstractButton* > list = dialog->findChildren<QAbstractButton*>();
    foreach(QAbstractButton* b, list){
        if (b->text().contains("ok",Qt::CaseInsensitive)){
            GTWidget::click(os, b);
            return;
        }
    }
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);//if ok button not found
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
//QDialogButtonBox *buttonBox;
}
