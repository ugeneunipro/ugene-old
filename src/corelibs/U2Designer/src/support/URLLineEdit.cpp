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

#include <QFileDialog>
#include <QFocusEvent>
#include <QLayout>

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GUrlUtils.h>

#include <U2Gui/LastUsedDirHelper.h>

#include "URLLineEdit.h"

namespace U2 {

URLLineEdit::URLLineEdit(const QString &filter,
                         const QString &type,
                         bool multi,
                         bool isPath,
                         bool saveFile,
                         QWidget *parent,
                         const QString &format)
: QLineEdit(parent), FileFilter(filter), type(type), multi(multi),
isPath(isPath), saveFile(saveFile), fileFormat(format)
{
    completer = new GSuggestCompletion(fileFormat, this);
    connect(this, SIGNAL(editingFinished()), this, SLOT(sl_editingFinished()));
}

void URLLineEdit::sl_onBrowse() {
    this->browse(false);
}

void URLLineEdit::sl_onBrowseWithAdding() {
    this->browse(true);
}

void URLLineEdit::sl_editingFinished(){
    /*
    QString name = text();
    DocumentFormat *format = AppContext::getDocumentFormatRegistry()->getFormatById(fileFormat);
    if (NULL != format && !name.isEmpty()) {
        QString newName(name);
        GUrl url(newName);
        QString lastSuffix = url.lastFileSuffix();
        if ("gz" == lastSuffix) {
            int dotPos = url.getURLString().length() - lastSuffix.length() - 1;
            if ((dotPos >= 0) && (QChar('.') == url.getURLString()[dotPos])) {
                newName = url.getURLString().left(dotPos);
                GUrl tmp(newName);
                lastSuffix = tmp.lastFileSuffix(); 
            }
        }
        bool foundExt = false;
        foreach (QString supExt, format->getSupportedDocumentFileExtensions()) {
            if (lastSuffix == supExt) {
                foundExt = true;
                break;
            }
        }
        if (!foundExt) {
            name = name + "." + format->getSupportedDocumentFileExtensions().first();
        } else {
            int dotPos = newName.length() - lastSuffix.length() - 1;
            if ((dotPos < 0) || (QChar('.') != newName[dotPos])) {
                name = name + "." + format->getSupportedDocumentFileExtensions().first();
            }
        }
    }
    setText(name);
    */
}

void URLLineEdit::browse(bool addFiles) {
    LastUsedDirHelper lod(type);
    QString lastDir = lod.dir;
    if(!text().isEmpty()) {
        QString curPath(text());
        int slashPos = curPath.lastIndexOf("/");
        slashPos = qMax(slashPos, curPath.lastIndexOf("\\"));
        if (slashPos >= 0) {
            QDir dir(curPath.left(slashPos + 1));
            if (dir.exists()) {
                lastDir = dir.absolutePath();
            }
        }
    }

    QString name;
    if(isPath || multi){
        QStringList lst;
        if (isPath) {
            QString dir = QFileDialog::getExistingDirectory(NULL, tr("Select a directory"), lastDir);
            lst << dir;
        } else {
            lst = QFileDialog::getOpenFileNames(NULL, tr("Select file(s)"), lastDir, FileFilter);
        }

        if (addFiles) {
            name = this->text();
            if (!lst.isEmpty()) {
                name += ";";
            }
        }
        name += lst.join(";");
        if (!lst.isEmpty()) {
            lod.url = lst.first();
        }
    } else {
        if(saveFile) {
            lod.url = name = QFileDialog::getSaveFileName(NULL, tr("Select a file"), lastDir, FileFilter, 0, QFileDialog::DontConfirmOverwrite);
            this->checkExtension(name);
        } else {
            lod.url = name = QFileDialog::getOpenFileName(NULL, tr("Select a file"), lastDir, FileFilter );
        }
    }
    if (!name.isEmpty()) {
        if (name.length() > this->maxLength()) {
            this->setMaxLength(name.length() + this->maxLength());
        }
        setText(name);
    }
    setFocus();
}

void URLLineEdit::focusOutEvent (QFocusEvent *event) {
    QLineEdit::focusOutEvent(event);
    // TODO: fix this low level code. It is made for fixing UGENE-577
    if (Qt::MouseFocusReason == event->reason()) {
        QLayout *layout = this->parentWidget()->layout();
        for (int i=1; i<layout->count(); i++) { //for each QToolButton in the layout
            QWidget *w = layout->itemAt(i)->widget();
            if (w->underMouse()) {
                return;
            }
        }
    }
    emit si_finished();
}

void URLLineEdit::checkExtension(QString &name) {
    DocumentFormat *format = AppContext::getDocumentFormatRegistry()->getFormatById(fileFormat);
    if (NULL != format && !name.isEmpty()) {
        QString newName(name);
        GUrl url(newName);
        QString lastSuffix = url.lastFileSuffix();
        if ("gz" == lastSuffix) {
            int dotPos = newName.length() - lastSuffix.length() - 1;
            if ((dotPos >= 0) && (QChar('.') == newName[dotPos])) {
                newName = url.getURLString().left(dotPos);
                GUrl tmp(newName);
                lastSuffix = tmp.lastFileSuffix(); 
            }
        }
        bool foundExt = false;
        foreach (QString supExt, format->getSupportedDocumentFileExtensions()) {
            if (lastSuffix == supExt) {
                foundExt = true;
                break;
            }
        }
        if (!foundExt) {
            name = name + "." + format->getSupportedDocumentFileExtensions().first();
        } else {
            int dotPos = newName.length() - lastSuffix.length() - 1;
            if ((dotPos < 0) || (QChar('.') != newName[dotPos])) {
                name = name + "." + format->getSupportedDocumentFileExtensions().first();
            }
        }
    }
}

bool URLLineEdit::isMulti() {
    return multi;
}

GSuggestCompletion::GSuggestCompletion(QString fileFormat, QLineEdit *parent): QObject(parent), editor(parent), fileFormat(fileFormat){
    popup = new QTreeWidget;
    popup->setWindowFlags(Qt::Popup);
    popup->setFocusPolicy(Qt::NoFocus);
    popup->setFocusProxy(parent);
    popup->setMouseTracking(true);

    popup->setColumnCount(1);
    popup->setUniformRowHeights(true);
    popup->setRootIsDecorated(false);
    popup->setEditTriggers(QTreeWidget::NoEditTriggers);
    popup->setSelectionBehavior(QTreeWidget::SelectRows);
    popup->setFrameStyle(QFrame::Box | QFrame::Plain);
    popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popup->header()->hide();

    popup->installEventFilter(this);

    connect(popup, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(doneCompletion()));
    connect(editor, SIGNAL(textEdited(QString)), SLOT(sl_textEdited(QString)));
}

GSuggestCompletion::~GSuggestCompletion(){
    delete popup;
}

bool GSuggestCompletion::eventFilter(QObject *obj, QEvent *ev){
    if (obj != popup)
        return false;

    if (ev->type() == QEvent::MouseButtonPress) {
        popup->hide();
        editor->setFocus();
        return true;
    }

    if (ev->type() == QEvent::KeyPress) {

        bool consumed = false;
        int key = static_cast<QKeyEvent*>(ev)->key();
        switch (key) {
         case Qt::Key_Enter:
         case Qt::Key_Return:
             doneCompletion();
             consumed = true;

         case Qt::Key_Escape:
             editor->setFocus();
             popup->hide();
             consumed = true;

         case Qt::Key_Up:
         case Qt::Key_Down:
             /*
         case Qt::Key_Left:
         case Qt::Key_Right:
         */
         case Qt::Key_Home:
         case Qt::Key_End:
         case Qt::Key_PageUp:
         case Qt::Key_PageDown:
             break;

         default:
             editor->setFocus();
             editor->event(ev);
             break;
        }

        return consumed;
    }

    return false;
}

void GSuggestCompletion::showCompletion(const QStringList &choices){
    if (choices.isEmpty()){
        popup->hide();
        return;
    }
    const QPalette &pal = editor->palette();
    QColor color = pal.color(QPalette::Disabled, QPalette::WindowText);

    popup->setUpdatesEnabled(false);
    popup->clear();
    for (int i = 0; i < choices.count(); ++i) {
        QTreeWidgetItem * item;
        item = new QTreeWidgetItem(popup);
        item->setText(0, choices[i]);
    }
    popup->setCurrentItem(popup->topLevelItem(0));
    popup->resizeColumnToContents(0);
    popup->adjustSize();
    popup->setUpdatesEnabled(true);

    int h = popup->sizeHintForRow(0) * qMin(10, choices.count()) + 3;
    popup->resize(editor->width(), h);
    popup->move(editor->mapToGlobal(QPoint(0, editor->height())));
    popup->show();
}

void GSuggestCompletion::doneCompletion(){
    popup->hide();
    editor->setFocus();
    QTreeWidgetItem *item = popup->currentItem();
    if (item) {
        editor->setText(item->text(0));
        QMetaObject::invokeMethod(editor, "returnPressed");
    }
}

void GSuggestCompletion::sl_textEdited(const QString &fileName){
    if (fileName.isEmpty()){
        popup->hide();
        return;
    }

    QString fName = fileName;
    if(fName.endsWith(".")){
        fName = fName.left(fName.size()-1);
    }

    QStringList choices, hits;
    QFileInfo f(fileName);
    QString curExt = f.suffix(), baseName = f.completeBaseName();
    DocumentFormat *format = AppContext::getDocumentFormatRegistry()->getFormatById(fileFormat);
    foreach(QString ext, format->getSupportedDocumentFileExtensions()){
        if(!curExt.isEmpty()){
            if (ext.startsWith(curExt, Qt::CaseInsensitive)){
                choices.append(baseName + "." + ext);
            }
        }  
    }
    if(choices.isEmpty()){
        foreach(QString ext, format->getSupportedDocumentFileExtensions()){
            choices.append(fileName + "." + ext);
            choices.append(fileName + "." + ext + ".gz");
        }
    }
    //TODO: add ".gz" to choises

    showCompletion(choices);
}

} // U2
