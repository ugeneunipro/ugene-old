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
    connect(this, SIGNAL(editingFinished()), this, SLOT(sl_editingFinished()));
}

void URLLineEdit::sl_onBrowse() {
    this->browse(false);
}

void URLLineEdit::sl_onBrowseWithAdding() {
    this->browse(true);
}

void URLLineEdit::sl_editingFinished(){
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

} // U2
