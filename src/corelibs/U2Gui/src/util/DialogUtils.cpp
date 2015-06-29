/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QWizard>

#include <U2Core/AppContext.h>
#include <U2Core/DocumentImport.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/FormatUtils.h>
#include <U2Core/Settings.h>
#include <U2Core/Task.h>

#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/U2FileDialog.h>

#include "DialogUtils.h"

namespace U2 {

void DialogUtils::showProjectIsLockedWarning(QWidget* p) {
    QMessageBox::critical(p, tr("Error"), tr("Project is locked"), QMessageBox::Ok, QMessageBox::NoButton);
}

QString DialogUtils::prepareFileFilter(const QString& name, const QStringList& exts, bool any, const QStringList& extra) {
    return FormatUtils::prepareFileFilter(name, exts, any, extra);
}

QString DialogUtils::prepareDocumentsFileFilter(const DocumentFormatId& fid, bool any, const QStringList& extra) {
    return FormatUtils::prepareDocumentsFileFilter(fid, any, extra);
}

QString DialogUtils::prepareDocumentsFileFilter(bool any, const QStringList& extra) {
    return FormatUtils::prepareDocumentsFileFilter(any, extra);
}

QString DialogUtils::prepareDocumentsFileFilter(const DocumentFormatConstraints& c, bool any) {
    return FormatUtils::prepareDocumentsFileFilter(c, any);
}

QString DialogUtils::prepareDocumentsFileFilterByObjType(const GObjectType& t, bool any) {
    return FormatUtils::prepareDocumentsFileFilterByObjType(t, any);
}

QPair<QString, QString> DialogUtils::selectFileForScreenShot(QWidget * parent) {
    QMap<QString, QString> filters;
    filters[ "PNG - Portable Network Graphics (*.png)" ] = "png";
    filters[ "JPG/JPEG format (*.jpg)" ] = "jpg";
    filters[ "TIF - Tagged Image File format (*.tiff)" ] = "tiff";

    LastUsedDirHelper lod("image");
    QString selectedFilter;
    lod.url = U2FileDialog::getSaveFileName(parent, tr("Export alignment image"), lod.dir, QStringList(filters.keys()).join(";;"), &selectedFilter);
    return QPair<QString, QString>(lod.url, filters.value(selectedFilter));
}

void DialogUtils::setWizardMinimumSize(QWizard *wizard) {
    QSize bestSize;
    foreach (int pageId, wizard->pageIds()) {
        QWizardPage *page = wizard->page(pageId);
        page->adjustSize();
        bestSize = bestSize.expandedTo(page->size());
    }
    wizard->setMinimumSize(bestSize);
}


/********************************
* FileLineEdit
********************************/
void FileLineEdit::sl_onBrowse() {
    LastUsedDirHelper lod(type);

    QFileDialog::Options options = 0;
#if defined(Q_OS_MAC) || (QT_VERSION >= 0x050000)
    if (qgetenv("UGENE_GUI_TEST").toInt() == 1 && qgetenv("UGENE_USE_NATIVE_DIALOGS").toInt() == 0) {
        options |= QFileDialog::DontUseNativeDialog;
    }
#endif

    QString name;
    if (multi) {
        QStringList lst = U2FileDialog::getOpenFileNames(NULL, tr("Select file(s)"), lod.dir, FileFilter, NULL, options);
        name = lst.join(";");
        if (!lst.isEmpty()) {
            lod.url = lst.first();
        }
    } else {
        lod.url = name = U2FileDialog::getOpenFileName(NULL, tr("Select file(s)"), lod.dir, FileFilter, NULL, options);
    }
    if (!name.isEmpty()) {
        setText(name);
    }
    setFocus();
}

} // namespace
