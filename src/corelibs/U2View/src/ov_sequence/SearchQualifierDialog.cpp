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

#include "SearchQualifierDialog.h"
#include <ui/ui_SearchQualifierDialog.h>

#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/TextUtils.h>

#include <QtGui/QMessageBox>
#include <QtGui/QKeyEvent>

namespace U2 {

SearchQualifierDialog::SearchQualifierDialog(QWidget* p, AnnotationsTreeView * _treeView)
:QDialog(p)
,treeView(_treeView)
,groupToSearchIn(NULL)
{


     ui = new Ui_SearchQualifierDialog;
     ui->setupUi(this);

     ui->valueEdit->installEventFilter(this);

     SAFE_POINT(treeView != NULL, "Tree Veiw is NULL", );

     clearPrevResults();

     AVItem* currentItem = static_cast<AVItem*>(treeView->tree->currentItem());
     switch (currentItem->type){
         case AVItemType_Group:
             {
                 groupToSearchIn = currentItem;
                 break;
             }
         case AVItemType_Annotation:
             {
                 parentAnnotationofPrevResult = currentItem;
                 groupToSearchIn = treeView->findGroupItem(currentItem->getAnnotationGroup());
                 break;
             }
         case AVItemType_Qualifier:
             {
                 AVItem* annotation = dynamic_cast<AVItem*>(currentItem->parent());
                 if (annotation && annotation->type == AVItemType_Annotation){
                     parentAnnotationofPrevResult = annotation;
                 }
                 groupToSearchIn = treeView->findGroupItem(currentItem->getAnnotationGroup());
                 break;
             }
     }
     QString groupName = groupToSearchIn->getAnnotationGroup()->getGroupName();
     if(groupName == AnnotationGroup::ROOT_GROUP_NAME){
         AnnotationTableObject* aObj = groupToSearchIn->getAnnotationTableObject();
         groupName = aObj->getGObjectName();
     }
     ui->groupLabel->setText(groupName);


}

bool SearchQualifierDialog::eventFilter(QObject *obj, QEvent *e) {
    Q_UNUSED(obj);
    QEvent::Type t = e->type();
    if (t == QEvent::KeyPress) {
        QKeyEvent* ke = (QKeyEvent*)e;
        int key = ke->key();
        if (key == Qt::Key_Tab) {
            ui->nameEdit->setFocus();
            return true;
        } 
        if (key == Qt::Key_Enter || key == Qt::Key_Return) {
            accept();
            return true;
        }
    }
    return false;
}
static QString simplify(const QString& s) {
    QString res = s;
    res = res.replace("\t", "    ");
    res = res.replace("\r", "");
    res = res.replace("\n", " ");
    res = res.trimmed();
    return res;
}

void SearchQualifierDialog::accept() {

    QString name = simplify(ui->nameEdit->text());
    QString val = simplify(ui->valueEdit->text());
    if (!(name.length() < 20 && TextUtils::fits(TextUtils::QUALIFIER_NAME_CHARS, name.toAscii().data(), name.length()))) {
        QMessageBox::critical(this, tr("Error!"), tr("Illegal qualifier name"));
        return;
    }
    if (!Annotation::isValidQualifierValue(val)) {
        QMessageBox::critical(this, tr("Error!"), tr("Illegal qualifier value"));
        return;
    }

    FindQualifierTask* findTask = new FindQualifierTask(treeView, groupToSearchIn, name, val, ui->exactButton->isChecked(), parentAnnotationofPrevResult, indexOfPrevResult);
    connect(findTask, SIGNAL( si_stateChanged() ), SLOT( sl_searchTaskStateChanged() ));
    TaskScheduler* s = AppContext::getTaskScheduler();
    s->registerTopLevelTask(findTask);
}

SearchQualifierDialog::~SearchQualifierDialog( ) {
    clearPrevResults();
    delete ui;
}

void SearchQualifierDialog::sl_searchTaskStateChanged( ) {
    FindQualifierTask* task = qobject_cast<FindQualifierTask *>(sender());
    if(!task || task->isCanceled() || !task->isFinished()){
        return;
    }
    
    if (task->isFound()){
        //save results
        parentAnnotationofPrevResult = task->getResultAnnotation();
        indexOfPrevResult = task->getIndexOfResult();
    } 
    else{
        //dialog
        int result = QMessageBox::question(this,
                                        tr("Search Complete"), 
                                        tr("The end of the annotation tree has been reached. Would you like to start the search from the beginning?"), 
                                        QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes){
            clearPrevResults();
        } 
    }
}

void SearchQualifierDialog::clearPrevResults(){
    parentAnnotationofPrevResult = NULL;
    indexOfPrevResult = -1;
}

}//namespace
