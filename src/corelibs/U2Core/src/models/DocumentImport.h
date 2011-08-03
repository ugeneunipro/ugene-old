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

class DocumentImporter;
class DocumentProviderTask;
class FormatDetectionResult;


/** Registry for all DocumentImportHandlers */
class U2CORE_EXPORT DocumentImportersRegistry: public QObject {
    Q_OBJECT
public:
    DocumentImportersRegistry(QObject* p = NULL) : QObject(p) {}
    ~DocumentImportersRegistry();

    /** returns handler by its id */
    DocumentImporter* getDocumentImporter(const QString& importerId) const;
 
    /** registers new document import handler */
    void addDocumentImporter(DocumentImporter* i);
    
    const QList<DocumentImporter*>& getImporters() const {return importers;}

private:
    QList<DocumentImporter*> importers;
};


class U2CORE_EXPORT DocumentImporter : public QObject {
    Q_OBJECT
public:
    DocumentImporter(const QString& _id, const QString& _name, QObject* o = NULL) : QObject(o), id(_id), name(_name){}

    virtual FormatCheckResult checkRawData(const QByteArray& rawData, const GUrl& url) = 0;
    
    virtual DocumentProviderTask* createImportTask(const FormatDetectionResult& res, bool showWizard) = 0;

    virtual QString getImporterDescription() const {return importerDescription;}
    
    const QString& getImporterName() const {return name;}

    const QString& getId() const {return id;}

    const QList<QString>& getSupportedFileExtensions() const {return extensions;}
    
protected:
    QString         id;
    QString         name;
    QList<QString>  extensions;
    QString         importerDescription;
};

} //namespace

#endif


