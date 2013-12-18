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

#ifndef _U2_ANNOTATION_DATA_H_
#define _U2_ANNOTATION_DATA_H_

#include <U2Core/U2Location.h>
#include <U2Core/U2Qualifier.h>

#include <QtCore/QSharedData>
#include <QtCore/QVector>
#include <QtCore/QStringList>

namespace U2 {

class U2CORE_EXPORT AnnotationData : public QSharedData {
public:
    AnnotationData() : caseAnnotation( false ) {location = new U2LocationData();}

    inline AnnotationData& operator= ( const AnnotationData & a );

    inline bool operator== (const AnnotationData &other) const;

    inline void removeAllQualifiers(const QString& name, QStringList& values);

    inline void findQualifiers(const QString& name, QVector<U2Qualifier>& res) const;

    inline QString findFirstQualifierValue(const QString& name) const;

    bool isJoin() const {return location->isJoin();}
    
    bool isOrder() const {return location->isOrder();}
    
    U2Strand getStrand() const  {return location->strand;}

    void setStrand(U2Strand s)  {location->strand = s;}
    
    U2LocationOperator getLocationOperator() const {return location->op;}

    void setLocationOperator(U2LocationOperator o)  {location->op = o;}

    const QVector<U2Region>& getRegions() const {return location->regions;}

    QString                 name;
    U2Location              location;
    QVector<U2Qualifier>    qualifiers;
    bool                    caseAnnotation;
};

typedef QSharedDataPointer<AnnotationData> SharedAnnotationData;

U2CORE_EXPORT QDataStream& operator>>(QDataStream& dataStream, TriState& state);
U2CORE_EXPORT QDataStream& operator<<(QDataStream& dataStream, const TriState& state);
U2CORE_EXPORT QDataStream& operator>>(QDataStream& dataStream, U2Qualifier& q);
U2CORE_EXPORT QDataStream& operator<<(QDataStream& dataStream, const U2Qualifier& q);
U2CORE_EXPORT QDataStream& operator>>(QDataStream& dataStream, AnnotationData& data);
U2CORE_EXPORT QDataStream& operator<<(QDataStream& dataStream, const AnnotationData& data);

AnnotationData& AnnotationData::operator= ( const AnnotationData & a ) { 
    name = a.name; 
    location = a.location; 
    qualifiers = a.qualifiers;
    return *this;
}

bool AnnotationData::operator== (const AnnotationData &other) const {
    if (this->name != other.name) {
        return false;
    }

    if (this->qualifiers.size() != other.qualifiers.size()) {
        return false;
    }
    foreach (const U2Qualifier &q, this->qualifiers) {
        if (!other.qualifiers.contains(q)) {
            return false;
        }
    }

    if (this->location != other.location) {
        return false;
    }

    return true;
}

void AnnotationData::removeAllQualifiers(const QString& name, QStringList& values) {
    for (int i=qualifiers.size(); --i>=0;) {
        const U2Qualifier& q = qualifiers.at(i);
        if (q.name == name) {
            values.append(q.value);
            qualifiers.remove(i);
        }
    }
}

void AnnotationData::findQualifiers(const QString& name, QVector<U2Qualifier>& res) const {
    foreach (const U2Qualifier& q, qualifiers) {
        if (q.name == name) {
            res.append(q);
        }
    }
}

QString AnnotationData::findFirstQualifierValue(const QString& name) const {
    foreach (const U2Qualifier& q, qualifiers) {
        if (q.name == name) {
            return q.value;
        }
    }
    return QString::null;
}


}//namespace

Q_DECLARE_METATYPE(U2::AnnotationData);
Q_DECLARE_METATYPE(U2::SharedAnnotationData);
Q_DECLARE_METATYPE(QList<U2::SharedAnnotationData>);

#endif
