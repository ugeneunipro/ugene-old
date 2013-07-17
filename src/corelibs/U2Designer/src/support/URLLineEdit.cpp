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

#include <QFileDialog>
#include <QFocusEvent>
#include <QLayout>

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GUrlUtils.h>

#include <U2Lang/SchemaConfig.h>

#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/SuggestCompleter.h>

#include "PropertyWidget.h"

#include "URLLineEdit.h"

namespace U2 {

class FilenameCompletionFiller : public CompletionFiller {
public:
    FilenameCompletionFiller(URLWidget *_widget)
        :CompletionFiller(), widget(_widget)
    {

    }

    virtual QStringList getSuggestions(const QString &str) {
        QString fileFormat = DelegateTags::getString(widget->tags(), "format");
        QString fName = str;
        if(fName.endsWith(".")){
            fName = fName.left(fName.size()-1);
        }

        QStringList choices, hits;
        QFileInfo f(fName);
        QString curExt = f.suffix(), baseName = f.completeBaseName(), completeFileName = f.fileName();
        DocumentFormat *format = AppContext::getDocumentFormatRegistry()->getFormatById(fileFormat);
        CHECK(NULL != format, QStringList());
        QStringList formats = format->getSupportedDocumentFileExtensions();
        CHECK(formats.size() > 0, QStringList());
        formats.append("gz");
        choices.append(completeFileName);
        foreach(QString ext, formats){
            if(!curExt.isEmpty()){
                if (ext.startsWith(curExt, Qt::CaseInsensitive)){
                    choices.append(baseName + "." + ext);
                    if (ext != "gz"){
                        choices.append(baseName + "." + ext + ".gz");
                    }
                }
            }
        }

        if(choices.size() == 1){
            foreach(QString ext, formats){
                choices.append(completeFileName + "." + ext);
                if (ext != "gz"){
                    choices.append(completeFileName + "." + ext + ".gz");
                }
            }
        }

        return choices;
    }

    virtual QString finalyze(const QString &editorText, const QString &suggestion) {
        QString path = editorText;
        path.replace("\\", "/");

        int slashPos = path.lastIndexOf("/");
        QString dirPath = path.left(slashPos + 1);

        return dirPath + suggestion;
    }

private:
    URLWidget *widget;
};

URLLineEdit::URLLineEdit(const QString &type,
                         bool multi,
                         bool isPath,
                         bool saveFile,
                         URLWidget *_parent)
: QLineEdit(_parent), schemaConfig(NULL), type(type), multi(multi),
isPath(isPath), saveFile(saveFile), parent(_parent) {
    if (saveFile && NULL != parent) {
        new BaseCompleter(new FilenameCompletionFiller(parent), this);
    }
}

CompletionFiller * URLLineEdit::getCompletionFillerInstance() {
    if (saveFile && NULL != parent) {
        return new FilenameCompletionFiller(parent);
    }
}

void URLLineEdit::sl_onBrowse() {
    this->browse(false);
}

void URLLineEdit::sl_onBrowseWithAdding() {
    this->browse(true);
}

void URLLineEdit::browse(bool addFiles) {
    QString FileFilter;
    if (NULL != parent) {
        FileFilter = DelegateTags::getString(parent->tags(), "filter");
    }
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

void URLLineEdit::focusOutEvent(QFocusEvent *event) {
    QLineEdit::focusOutEvent(event);
    sl_completionFinished();
}

void URLLineEdit::checkExtension(QString &name) {
    QString fileFormat;
    if (NULL != parent) {
        fileFormat = DelegateTags::getString(parent->tags(), "format");
    }
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

void URLLineEdit::sl_completionFinished() {
    emit si_finished();
}

} // U2
