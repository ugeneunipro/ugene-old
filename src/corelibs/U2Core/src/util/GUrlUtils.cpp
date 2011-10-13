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

#include "GUrlUtils.h"

#include <U2Core/Log.h>
#include <U2Core/Task.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatus.h>

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
    SAFE_POINT(!typeExt.isEmpty(), "Type extension is empty!", GUrl());
    
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
    QString rolled = GUrlUtils::rollFileName(original, "_oldcopy", excludeList);
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
QString GUrlUtils::prepareFileLocation(const QString& filePath, U2OpStatus& os) {
    QFileInfo fi(filePath);
    QString dirPath = fi.absoluteDir().absolutePath();
    QString canonicalDirPath = prepareDirLocation(dirPath, os);
    CHECK_OP(os, QString());
    // examples with "/" at the end: 
    QString result = canonicalDirPath + (canonicalDirPath.endsWith("/") ? "" : "/") + fi.fileName();
    return result;
}

// checks that dir path is valid. Creates the directory if needed. 
// Returns canonical directory path. Does not affect directory if already exists.
// Sample usage: processing URLs in "save dir" inputs
QString GUrlUtils::prepareDirLocation(const QString& dirPath, U2OpStatus& os) {
    CHECK_EXT(!dirPath.isEmpty(), os.setError(tr("Directory is not specified")), QString());
    QDir targetDir(dirPath);
    if (!targetDir.exists()) {
        QString absPath = targetDir.absolutePath();
        if (!targetDir.mkpath(absPath)) {
            os.setError(tr("Directory can't be created: %1").arg(absPath));
            return QString();
        } 
        targetDir = QDir(absPath); //It looks like QT caches results for QDir? Create new QDir instance in this case!
        if (!targetDir.isReadable()) {
            os.setError(tr("Directory can't be read: %1").arg(absPath));
            return QString();
        }
    }
    QString result = targetDir.canonicalPath();
    return result;
}


QString GUrlUtils::prepareTmpFileLocation(const QString& dir, const QString& prefix, const QString& ext, U2OpStatus& os) {
    int i = 0;
    QString result;
    while (true) {
        QString name = QString("%1_%2.%3").arg(prefix).arg(i).arg(ext);
        QString filePath = prepareFileLocation(dir + "/" + name, os);
        CHECK_OP(os, result);
        if (!QFile::exists(filePath)) {
            result = filePath;
            break;
        }
        i++;
    }
    return result;
}


void GUrlUtils::removeDir(const QString& dirPath, U2OpStatus& os) {
    QDir dir(dirPath);
    CHECK(dir.exists(), );
    QList<QFileInfo> entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
    foreach(const QFileInfo& info, entries) {
        if (info.isDir()) {
            removeDir(info.absoluteFilePath(), os);
        } else {
            QFile::remove(info.absoluteFilePath());
        }
        CHECK_OP(os, );
    }
    QDir().rmdir(dirPath);
}

}//namespace
