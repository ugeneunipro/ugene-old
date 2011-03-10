#include "GUrlUtils.h"

#include <U2Core/Log.h>
#include <U2Core/Task.h>

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

namespace U2 {

QString GUrlUtils::getUncompressedExtension(const GUrl& url) {
    QString ext = url.lastFileSuffix();
    if (ext == "gz") {
        QString completeSuffix = url.completeFileSuffix();
        QStringList suffixList = completeSuffix.split(".");
        ext = suffixList.empty() ? QString() :  suffixList.first();
    }
    return ext;
}

GUrl GUrlUtils::ensureFileExt(const GUrl& url, const QStringList& typeExt) {
    assert(!typeExt.isEmpty());
    if( url.isVFSFile() ) {
        return url;
    }
    
    if (typeExt.isEmpty()) {
        return url;
    }

    if (typeExt.contains(GUrlUtils::getUncompressedExtension(url))) {
        return url;
    }
    return GUrl(url.getURLString() + "."  + typeExt.first(), url.getType());
}


bool GUrlUtils::renameFileWithNameRoll(const QString& original, TaskStateInfo& ti, const QSet<QString>& excludeList, Logger* log) {
    QString rolled = GUrlUtils::rollFileName(original, "_copy", excludeList);
    if (rolled == original) {
        return true;
    }
    if (QFile(original).rename(rolled)) {
        if (log) {
            log->details(tr("Renamed %1 to %2").arg(original).arg(rolled));
        }
        return true;
    } else {
        ti.setError(tr("Failed to rename %1 to %2").arg(original).arg(rolled));
        return false;
    }
}


QString GUrlUtils::rollFileName(const QString& originalUrl, const QString& rolledSuffix, const QSet<QString>& excludeList) {
    QString pre = originalUrl, post; //pre and post url parts. A number will be placed between
    int idx = pre.lastIndexOf(".");

    if (idx != -1) {
        QString extSuffix = pre.mid(idx);
        if(extSuffix == ".gz") {
            pre.chop(extSuffix.length());
            idx = pre.lastIndexOf(".");
            if(idx != -1) {
                extSuffix = pre.mid(idx) + extSuffix;
                pre.append(".gz");
            }
        }
        if (!extSuffix.contains("\\")) {
            post = extSuffix;
            pre.chop(extSuffix.length());
        }
    }
    QString resultUrl = originalUrl;
    int i = 0;
    while (QFile::exists(resultUrl) || excludeList.contains(resultUrl)) {
        resultUrl = pre + rolledSuffix + QString("%1").arg(++i) + post;
    }
    return resultUrl;
}


QUrl GUrlUtils::gUrl2qUrl( const GUrl& gurl) {
    if( gurl.isVFSFile() ) {
        return QUrl();
    }
    
    QString str = gurl.getURLString();
    if (str.startsWith("http://") || str.startsWith("https://") || str.startsWith("file://")) {
        return str;
    } else {
        return QUrl("file:///" + QFileInfo(str).absoluteFilePath());
    }
}

QList<QUrl> GUrlUtils::gUrls2qUrls( const QList<GUrl>& gurls) {
    QList<QUrl> urls;
    foreach(const GUrl& gurl, gurls) {
        urls << gUrl2qUrl(gurl);
    }
    return urls;
}

GUrl GUrlUtils::qUrl2gUrl( const QUrl& qurl) {
    QString str = qurl.toString();
    return GUrl(str);
}

QList<GUrl> GUrlUtils::qUrls2gUrls( const QList<QUrl>& qurls) {
    QList<GUrl> urls;
    foreach(const QUrl& qurl, qurls) {
        urls << qUrl2gUrl(qurl);
    }
    return urls;
}


QString GUrlUtils::prepareFileName(const QString& url, int count, const QStringList& typeExt) {
    return prepareFileName(url, QString("%1").arg(count, 3, 10, QChar('0')), typeExt);
}

QString GUrlUtils::prepareFileName(const QString& url, const QString& baseSuffix, const QStringList& typeExt) {    
    QFileInfo fi(url);
    QStringList suffixList = fi.completeSuffix().split(".");
    QString ext;
    foreach(const QString& suffix, suffixList) {
        if (typeExt.contains(suffix)) {
            ext = suffix;
            break;
        }
    }
    if (ext.isEmpty()) {
        return url + QString(".%1.%2").arg(baseSuffix).arg(typeExt.first());
    }
    QString base = fi.baseName();
    while(!suffixList.isEmpty()) {
        QString nextSuffix = suffixList.takeFirst();
        if (nextSuffix == ext) {
            break;
        }
        base += "." + nextSuffix;
    }
    QString result = QString("%1/%2.%3.%4").arg(fi.dir().path()).arg(base).arg(baseSuffix).arg(ext);
    if (!suffixList.isEmpty()) {
        result.append(".").append(suffixList.join("."));
    }

    return result;
}

// checks that file path is valid: creates required directory if needed. 
// Returns canonical path to file. Does not create nor remove file, affects just directory
// Sample usage: processing URLs in "save file" inputs
QString GUrlUtils::prepareFileLocation(const QString& filePath, QString& errorMessage) {
    QFileInfo fi(filePath);
    QString dirPath = fi.absoluteDir().absolutePath();
    QString canonicalDirPath = prepareDirLocation(dirPath, errorMessage);
    if (!errorMessage.isEmpty()) {
        return QString();
    }
    // examples with "/" at the end: 
    QString result = canonicalDirPath + (canonicalDirPath.endsWith("/") ? "" : "/") + fi.fileName();
    return result;
}

// checks that dir path is valid. Creates the directory if needed. 
// Returns canonical directory path. Does not affect directory if already exists.
// Sample usage: processing URLs in "save dir" inputs
QString GUrlUtils::prepareDirLocation(const QString& dirPath, QString& errorMessage) {
    if (dirPath.isEmpty()) {
        errorMessage = tr("Directory is not specified");
        return QString();
    }
    QDir targetDir(dirPath);
    if (!targetDir.exists()) {
        QString absPath = targetDir.absolutePath();
        if (!targetDir.mkpath(absPath)) {
            errorMessage = tr("Directory can't be created: %1").arg(absPath);
            return QString();
        } 
        targetDir = QDir(absPath); //It looks like QT caches results for QDir? Create new QDir instance in this case!
        if (!targetDir.isReadable()) {
            errorMessage = tr("Directory can't be read: %1").arg(absPath);
            return QString();
        }
    }
    QString result = targetDir.canonicalPath();
    return result;
}

}//namespace
