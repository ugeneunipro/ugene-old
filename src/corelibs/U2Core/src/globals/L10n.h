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

#ifndef _U2_L10N_H_
#define _U2_L10N_H_

#include <U2Core/global.h>
#include <U2Core/GUrl.h>

namespace U2 {

class U2CORE_EXPORT L10N: public QObject {
    Q_OBJECT
public:

    // general purpose messages
    static QString errorTitle() { return tr("Error!");}
    static QString warningTitle() { return tr("Warning!");}
    static QString badArgument(const QString& name) { return tr("Internal error, bad argument: %1").arg(name);}
    static QString outOfMemory() {return tr("Out of memory!");}
    static QString internalError() {return tr("Internal error!");}

    // messages to work with files
    static QString errorOpeningFileRead(const GUrl& url) { return tr("Error opening URL for read: '%1'").arg(url.getURLString());}
    static QString errorOpeningFileWrite(const GUrl& url) { return tr("Error opening URL for write: '%1'").arg(url.getURLString());}
    static QString errorWritingFile(const GUrl& url) { return tr("Write error: '%1'").arg(url.getURLString());}
    static QString errorReadingFile(const GUrl& url) { return tr("Read error: %1").arg(url.getURLString());}
    static QString errorFileTooLarge(const GUrl& url) { return tr("File is too large: '%1'").arg(url.getURLString());}
    static QString errorFileNotFound(const GUrl& url) {return tr("File not found %1").arg(url.getURLString());}
    static QString errorIsNotAFile(const GUrl& url) {return tr("%1 is not a file").arg(url.getURLString());}

    static QString notValidFileFormat(const QString& type, const GUrl& url) {return tr("Invalid file format. Expected file format: %1,  File: %2").arg(type).arg(url.getURLString());}
    static QString notSupportedFileFormat(const GUrl& url) {return tr("Invalid file format! File: %1").arg(url.getURLString());}

    // working with qt plugins: images, scripts, styles
    static QString errorImageSave(const QString& file, const QString& format) {
        return tr("Can't save image to file %1, format %2").arg(file).arg(format);
    }
    static QString errorDocumentNotFound(const GUrl& url) { return tr("Document not found: %1").arg(url.getURLString());}
    static QString suffixBp() { return tr(" bp");}

};

}//ns

#endif
