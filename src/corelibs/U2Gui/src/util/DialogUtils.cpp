/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "DialogUtils.h"

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentImport.h>
#include <U2Core/Settings.h>
#include <U2Core/Task.h>

#include <U2Gui/LastUsedDirHelper.h>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include <QtCore/QCoreApplication>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

namespace U2 {

static bool isKDE() {
    static bool result = false;

#if defined Q_WS_X11
    static bool checked = false;
    
    if (!checked) {
        QString ds = qgetenv("DESKTOP_SESSION");
        QString uid = qgetenv("KDE_SESSION_UID");
        QString version = qgetenv("KDE_SESSION_VERSION");
        result = (ds == "kde") || uid.toInt() > 0 || version.toInt() > 0;
        checked = true;
    }
#endif

    return result;
}

static QString getAllFilesFilter() {
    if (isKDE()) {
        return "*";
    }
    return "*.*";
}

void DialogUtils::showProjectIsLockedWarning(QWidget* p) {
    QMessageBox::critical(p, tr("Error"), tr("Project is locked"), QMessageBox::Ok, QMessageBox::NoButton);
}



QString DialogUtils::prepareFileFilter(const QString& name, const QStringList& exts, bool any, const QStringList& extra) {
    QString line = name + " (";
    foreach(QString ext, exts) {
        line+=" *."+ext;
    }
    foreach(QString ext, exts) {
        foreach(QString s, extra) {
            line+=" *."+ext+s;
        }
    }
    line+=" )";
    if (any) {
        line += ";;" + tr("All files") + " ( "+getAllFilesFilter()+" )";
    }
    return line;
}

static QStringList getExtra(DocumentFormat* df, const QStringList& originalExtra) {
    bool useExtra = !df->getFlags().testFlag(DocumentFormatFlag_NoPack);
    if (useExtra) {
        return originalExtra;
    }
    return QStringList();
}


QString DialogUtils::prepareDocumentsFileFilter(const DocumentFormatId& fid, bool any, const QStringList& extra) {
    DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(fid);
    QStringList effectiveExtra = getExtra(df, extra);
    QString result = prepareFileFilter(df->getFormatName(), df->getSupportedDocumentFileExtensions(), any, effectiveExtra);
    return result;
}


QString DialogUtils::prepareDocumentsFileFilter(bool any, const QStringList& extra) {
    DocumentFormatRegistry* fr = AppContext::getDocumentFormatRegistry();
    QList<DocumentFormatId> ids = fr->getRegisteredFormats();
    QStringList result;
    foreach(DocumentFormatId id , ids) {
        DocumentFormat* df = fr->getFormatById(id);
        QStringList effectiveExtra = getExtra(df, extra);
        result << prepareFileFilter(df->getFormatName(), df->getSupportedDocumentFileExtensions(), false, effectiveExtra);
    }
    foreach(DocumentImporter* importer, fr->getImportSupport()->getImporters()) {
        QStringList importerExts = importer->getSupportedFileExtensions();
        result << prepareFileFilter(importer->getImporterName(), importerExts, false, QStringList());
    }

    result.sort();
    if (any) {
        result.prepend(tr("All files") + " ( " + getAllFilesFilter() + " )");
    }
    return result.join(";;");
}

QString DialogUtils::prepareDocumentsFileFilter(const DocumentFormatConstraints& c, bool any) {
    QStringList result;

    QList<DocumentFormatId> ids = AppContext::getDocumentFormatRegistry()->getRegisteredFormats();
    foreach(const DocumentFormatId& id, ids) {
        DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(id);
        if (df->checkConstraints(c)) {
            result.append(prepareDocumentsFileFilter(id, false));
        }
    }
    result.sort();
    if (any) {
        result.prepend(tr("All files") + " (" + getAllFilesFilter() + " )");
    }
    return result.join(";;");
}

QString DialogUtils::prepareDocumentsFileFilterByObjType(const GObjectType& t, bool any) {
    DocumentFormatConstraints c;
    c.supportedObjectTypes += t;
    return prepareDocumentsFileFilter(c, any);
}

QPair<QString, QString> DialogUtils::selectFileForScreenShot(QWidget * parent) {
    QMap<QString, QString> filters;
    filters[ "PNG - Portable Network Graphics (*.png)" ] = "png";
    filters[ "JPG/JPEG format (*.jpg)" ] = "jpg";
    filters[ "TIF - Tagged Image File format (*.tiff)" ] = "tiff";

    LastUsedDirHelper lod("image");
    QString selectedFilter;
    lod.url = QFileDialog::getSaveFileName(parent, tr("Export alignment image"), lod.dir, QStringList(filters.keys()).join(";;"), &selectedFilter);
    return QPair<QString, QString>(lod.url, filters.value(selectedFilter));
}


/********************************
* FileLineEdit
********************************/
void FileLineEdit::sl_onBrowse() {
    LastUsedDirHelper lod(type);

    QString name;
    if (multi) {
        QStringList lst = QFileDialog::getOpenFileNames(NULL, tr("Select file(s)"), lod.dir, FileFilter);
        name = lst.join(";");
        if (!lst.isEmpty()) {
            lod.url = lst.first();
        }
    } else {
        lod.url = name = QFileDialog::getOpenFileName(NULL, tr("Select file(s)"), lod.dir, FileFilter);
    }
    if (!name.isEmpty()) {
        setText(name);
    }
    setFocus();
}

} // namespace
