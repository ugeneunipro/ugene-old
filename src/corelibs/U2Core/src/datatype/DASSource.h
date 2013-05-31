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

#ifndef _U2_DAS_SOURCE_H_
#define _U2_DAS_SOURCE_H_

#include <U2Core/global.h>

namespace U2 {

//type of a reference data (DNA sequence, protein, ...)
enum DASReferenceType{
    DASProteinSequence = 0,
    DASDNASequence
};

/** Class to store DAS source. It can be a reference sequence source, annotation source etc.
reference type is created for consistency. For instance protein sequence annotation cannot have annotations of DNA sequences*/

class U2CORE_EXPORT DASSource {
public:
    DASSource(){};

    DASSource(const QString& _id, const QString& _name, const QString& _url, DASReferenceType _refType, const QString& _hint = QString(), const QString& _description = QString())
        :id(_id), name(_name), url(_url), referenceType(_refType), hint(_hint), description(_description){};

    const QString& getId () const { return id;} 
    const QString& getName () const { return name;}
    const QString& getUrl () const { return url;}
    const QString& getDescription () const { return description;}
    const QString& getHint () const { return hint;}
    DASReferenceType getReferenceType () const { return referenceType;}

    bool isValid() const{return !id.isEmpty() && !url.isEmpty();}

private:
    QString id;
    QString name;
    QString url;
    QString description;
    QString hint;
    DASReferenceType referenceType;
};

/**Object type that is need to form a request string*/
enum DASObjectType{
    DASSequence = 0,
    DASFeatures
};


class U2CORE_EXPORT DASSourceRegistry : public QObject {
public:
    
    static QString getRequestURLString(const DASSource& source, const QString& accId, DASObjectType& requestType);

    DASSourceRegistry();

    ~DASSourceRegistry();

    virtual DASSource findById(const QString& id) const;

    virtual DASSource findByName(const QString& name) const;
    
    virtual QList<DASSource> getReferenceSources() const;

    virtual QList<DASSource> getFeatureSources() const;

    virtual QList<DASSource> getFeatureSourcesByType(DASReferenceType rType) const;

private:
    void initDefaultSources();

private:
    QList<DASSource> referenceSources;
    QList<DASSource> featureSources;
    
};

}//namespace

#endif //_U2_DAS_SOURCE_H_

