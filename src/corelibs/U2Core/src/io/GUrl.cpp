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

#include "GUrl.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

namespace U2 {

    static QString makeFilePathCanonical(const QString& originalUrl) {
        //ensure that name is canonical
        QString result = originalUrl.trimmed();

        QString fileUrlPrefix = "file://";
        if(result.startsWith(fileUrlPrefix)) {
            result = result.mid(fileUrlPrefix.length());
#ifdef Q_OS_WIN
            // on Windows, all slashes after "file:" can be trimmed, on Unix/Mac one must be kept to specify that it's an absolute path
            while(result.startsWith("/")) {
                result = result.mid(1);
            }
#endif
        }

        // Windows drive letter, Qt resource designation or Samba share designation and name
        QString prefix;

        if(originalUrl.startsWith(':')) { // is a Qt resource
            prefix = ":";
            result = result.mid(1);
        } else {
            result = QFileInfo(result).absoluteFilePath();
        }

#ifdef Q_OS_WIN
        bool isSambaPath = false;
        if(result.startsWith("//") && prefix.isEmpty()) {
            // keep Samba share designation
            prefix = "//";
            isSambaPath = true;
        }
#endif

        QStringList parts = result.split('/', QString::SkipEmptyParts);
        if(parts.size() > 0) {
            QStringList canonicalParts;
#ifdef Q_OS_WIN
            // append drive spec letter or Samba server name to the prefix
            if(isSambaPath) {
                prefix += parts.takeFirst();
            } else if(parts.at(0).endsWith(':') && parts.at(0).length() == 2 && prefix.isEmpty()) { // Windows drive letter designation
                prefix = parts.takeFirst();
            }
#endif
            // get rid of redundant '.' and '..' now
            QStringListIterator it(parts);
            while(it.hasNext()) {
                QString part = it.next();
                if(part == ".") {
                    continue;
                } else if(part == "..") {
                    if(!canonicalParts.isEmpty()) {
                        canonicalParts.removeLast();
                    }
                } else if (!part.isEmpty()) {
                    canonicalParts.append(part);
                }
            }
            result = prefix + "/" + canonicalParts.join("/");
        }


        return result;
    }

    GUrlType GUrl::getURLType(const QString& rawUrl) {
        GUrlType result = GUrl_File;
        if (rawUrl.startsWith("http://") || rawUrl.startsWith("https://")) {
            result = GUrl_Http;
        } else if (rawUrl.startsWith("ftp://")) {
            result = GUrl_Ftp;
        } else if( rawUrl.startsWith( VirtualFileSystem::URL_PREFIX ) ) {
            result = GUrl_VFSFile;
        }
        return result;
    }

    // constructs url specified by string. The type is parsed
    GUrl::GUrl(const QString& _urlString) {
        urlString = _urlString;
        type = getURLType(urlString);
        if (type == GUrl_File) {
            urlString = makeFilePathCanonical(urlString);
        }
    }


    // constructs url specified by string. The type provided as param
    GUrl::GUrl(const QString& _urlString, const GUrlType _type) {
        urlString = _urlString;
        type = _type;
        if (type == GUrl_File) {
            urlString = makeFilePathCanonical(urlString);
        }
    }

    GUrl::GUrl(const GUrl& anotherUrl) {
        urlString = anotherUrl.getURLString();
        type = anotherUrl.getType();
    }

    bool GUrl::operator ==(const GUrl& url) const {
        return urlString == url.getURLString();
    }

    bool GUrl::operator !=(const GUrl& url) const {
        return !(*this == url);
    }

    static QString path(const GUrl* url) {
        //TODO: parse HTTP and other formats for path part
        QString result;
        if( url->isVFSFile() ) {
            return result;
        }
        result = url->getURLString();
        return result;
    }

    QString GUrl::dirPath() const {
        QString result;
        if( isVFSFile() ) {
            return result;
        }
        result = QFileInfo(path(this)).absoluteDir().absolutePath();
        return result;
    }

    QString GUrl::fileName() const {
        QString result;
        if( isVFSFile() ) {
            return result;
        }
        result = QFileInfo(path(this)).fileName();
        return result;
    }

    QString GUrl::baseFileName() const {
        QString result;
        if( isVFSFile() ) {
            QStringList args = urlString.split( VirtualFileSystem::URL_NAME_SEPARATOR, QString::SkipEmptyParts, Qt::CaseSensitive );
            if( 2 == args.size() ) {
                result = QFileInfo( args.at( 1 ) ).baseName();
            }
        } else {
            result = QFileInfo(path(this)).baseName();
        }
        return result;
    }

    QString GUrl::lastFileSuffix() const {
        QString result;
        if( isVFSFile() ) {
            return result;
        }
        result = QFileInfo(path(this)).suffix();
        return result;
    }

    QString GUrl::completeFileSuffix() const {
        QString result;
        if( isVFSFile() ) {
            return result;
        }
        result = QFileInfo(path(this)).completeSuffix();
        return result;
    }


    static bool registerMetas()  {
        qRegisterMetaType<GUrl>("GUrl");
        qRegisterMetaTypeStreamOperators<GUrl>("U2::GUrl");

        return true;
    }

    QDataStream &operator<<(QDataStream &out, const GUrl &myObj) {
        out << myObj.getURLString() << myObj.getType();
        return out;
    }

    QDataStream &operator>>(QDataStream &in, GUrl &myObj) {
        QString urlString; in >> urlString;
        int t; in >> t;
        GUrlType type = (GUrlType)t;
        myObj = GUrl(urlString, type);
        return in;
    }


    bool GUrl::registerMeta  = registerMetas();


}//namespace
