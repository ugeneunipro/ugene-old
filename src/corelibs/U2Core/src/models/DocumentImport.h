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

#ifndef _U2_DOCUMENT_IMPORT_H_
#define _U2_DOCUMENT_IMPORT_H_

#include <U2Core/DocumentModel.h>

namespace U2 {

class DocumentImportHandler;
class DocumentProviderTask;


/** Result of document format detection for import */
class U2CORE_EXPORT DocumentImportHandle {
public:
    /** id of the import handler */
    QString                 handlerId;
    /** name of the source file format */
    QString                 sourceFormat;
    /** detection score */
    FormatDetectionScore   sourceFormatDetectionResult;
};

/** Registry for all DocumentImportHandlers */
class U2CORE_EXPORT DocumentImportSupport : public QObject {
    Q_OBJECT
public:
    DocumentImportSupport(QObject* p = NULL) : QObject(p) {}
    ~DocumentImportSupport();

    /** Finds importers valid for the data provided */
    QList<DocumentImportHandle> findImportHandlers(const QByteArray& rawData, const GUrl& url) const;
    
    /** returns handler by its id */
    DocumentImportHandler* getDocumentImportHandler(const QString& handlerId) const;
 
    /** registers new document import handler */
    void addDocumentImportHandler(DocumentImportHandler* h);
private:
    QList<DocumentImportHandler*> importHandlers;
};


class U2CORE_EXPORT DocumentImportHandler : public QObject {
    Q_OBJECT
public:
    DocumentImportHandler(const QString& _id, const QString& _name, QObject* o = NULL) : QObject(o), id(_id), name(_name){}

    virtual DocumentImportHandle checkData(const QByteArray& rawData, const GUrl& url) = 0;
    
    virtual DocumentProviderTask* createImportTask(const DocumentImportHandle& handle, bool showWizard) = 0;
    
    const QString& getName() const {return name;}

    const QString& getId() const {return id;}

protected:
    QString id;
    QString name;
};

} //namespace

#endif


