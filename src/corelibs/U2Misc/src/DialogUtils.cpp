#include "DialogUtils.h"

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/Settings.h>
#include <U2Core/Task.h>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
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
        result.append(prepareFileFilter(df->getFormatName(), df->getSupportedDocumentFileExtensions(), false, effectiveExtra));
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


#define SETTINGS_ROOT QString("gui/")

QString DialogUtils::getLastOpenFileDir(const QString& toolType, const QString& defaultVal) {
    QString key = SETTINGS_ROOT + (toolType.isEmpty() ? "" : toolType + "/") + "lastDir";
    QString defDir = defaultVal;
    if (defDir.isEmpty() && toolType.isEmpty()) {
        defDir = QDir::searchPaths( PATH_PREFIX_DATA ).first() + "/samples";
    }
    QString res = AppContext::getSettings()->getValue(key, defDir).toString();
    return res;
}

void DialogUtils::setLastOpenFileDir(const QString& ld, const QString& toolType) {
    QString key = SETTINGS_ROOT + (toolType.isEmpty() ? "" : toolType + "/") + "lastDir";
    AppContext::getSettings()->setValue(key, ld);
}

LastOpenDirHelper::LastOpenDirHelper(const QString& d, const QString& defaultVal) {
    domain = d;
    dir = DialogUtils::getLastOpenFileDir(domain, defaultVal);
}

LastOpenDirHelper::~LastOpenDirHelper() {
    saveURLDir2LastOpenDir();
}

void LastOpenDirHelper::saveURLDir2LastOpenDir() {
    if (!url.isEmpty()) {
        QString newDir = QFileInfo(url).absoluteDir().absolutePath();
        if (dir != newDir) {
            DialogUtils::setLastOpenFileDir(newDir, domain);
        }
    }
}

/********************************
* FileLineEdit
********************************/
void FileLineEdit::sl_onBrowse() {
    LastOpenDirHelper lod(type);

    QString name;
    if (multi) {
        QStringList lst = QFileDialog::getOpenFileNames(NULL, tr("Select file(s)"), lod.dir, FileFilter);
        name = lst.join(";");
        if (!lst.isEmpty()) {
            lod.url = lst.first();
        }
    } else {
        lod.url = name = QFileDialog::getSaveFileName(NULL, tr("Select a file"), lod.dir, FileFilter, 0, QFileDialog::DontConfirmOverwrite);
    }
    if (!name.isEmpty()) {
        setText(name);
    }
    setFocus();
}

} // namespace
