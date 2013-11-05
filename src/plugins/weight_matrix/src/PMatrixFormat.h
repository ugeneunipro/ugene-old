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

#ifndef _U2_PFM_MATRIX_FORMAT_H_
#define _U2_PFM_MATRIX_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/PFMatrix.h>
#include <U2Core/PWMatrix.h>
#include <U2Gui/ObjectViewTasks.h>

namespace U2 {

class PFMatrixFormat : public DocumentFormat {
    Q_OBJECT
public:
    PFMatrixFormat(QObject* p);
    
    static const DocumentFormatId FORMAT_ID;

    virtual DocumentFormatId getFormatId() const {return FORMAT_ID;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* createNewLoadedDocument(IOAdapterFactory* io, const QString& url, const QVariantMap& fs = QVariantMap());

    virtual FormatCheckResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

protected:
    virtual Document* loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& fs, U2OpStatus& os);

private:
    QString formatName;
};

class PFMatrixObject: public GObject {
    Q_OBJECT
public:
    static const GObjectType TYPE;

    PFMatrixObject(const PFMatrix& _m, const QString& objectName, const QVariantMap& hintsMap = QVariantMap()) 
        : GObject(TYPE, objectName, hintsMap), m(_m){};

    virtual const PFMatrix getMatrix() const {return m;}

    virtual GObject* clone(const U2DbiRef&, U2OpStatus&) const{
        PFMatrixObject* cln = new PFMatrixObject(m, getGObjectName(), getGHintsMap());
        cln->setIndexInfo(getIndexInfo());
        return cln;
    };

protected:
    PFMatrix m;
};

class PFMatrixViewFactory : public GObjectViewFactory {
    Q_OBJECT
public:
    static const PFMatrixViewFactoryId ID;
    PFMatrixViewFactory(QObject* p = NULL) : GObjectViewFactory(ID, tr("PFM Viewer"), p) {}

    virtual bool canCreateView(const MultiGSelection& multiSelection);
    virtual Task* createViewTask(const MultiGSelection& multiSelection, bool single = false);
};

class OpenPFMatrixViewTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenPFMatrixViewTask(Document* doc);
    virtual void open();
private:
    Document* document;
};

class PWMatrixFormat : public DocumentFormat {
    Q_OBJECT
public:
    PWMatrixFormat(QObject* p);

    static const DocumentFormatId FORMAT_ID;

    virtual DocumentFormatId getFormatId() const {return FORMAT_ID;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* createNewLoadedDocument(IOAdapterFactory* io, const QString& url, const QVariantMap& fs = QVariantMap());

    virtual FormatCheckResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

protected:
    virtual Document* loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& fs, U2OpStatus& os);

private:
    QString formatName;
};

class PWMatrixObject: public GObject {
    Q_OBJECT
public:
    static const GObjectType TYPE;

    PWMatrixObject(const PWMatrix& _m, const QString& objectName, const QVariantMap& hintsMap = QVariantMap()) 
        : GObject(TYPE, objectName, hintsMap), m(_m){};

    virtual const PWMatrix getMatrix() const {return m;}

    virtual GObject* clone(const U2DbiRef&, U2OpStatus&) const{
        PWMatrixObject* cln = new PWMatrixObject(m, getGObjectName(), getGHintsMap());
        cln->setIndexInfo(getIndexInfo());
        return cln;
    };

protected:
    PWMatrix m;
};

class PWMatrixViewFactory : public GObjectViewFactory {
    Q_OBJECT
public:
    static const PWMatrixViewFactoryId ID;
    PWMatrixViewFactory(QObject* p = NULL) : GObjectViewFactory(ID, tr("PWM Viewer"), p) {}

    virtual bool canCreateView(const MultiGSelection& multiSelection);
    virtual Task* createViewTask(const MultiGSelection& multiSelection, bool single = false);
};

class OpenPWMatrixViewTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenPWMatrixViewTask(Document* doc);
    virtual void open();
private:
    Document* document;
};

}//namespace

#endif
