/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_FEATURE_H_
#define _U2_FEATURE_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Region.h>

#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtCore/QSharedData>

namespace U2 {

/** Feature location */
class U2CORE_EXPORT U2FeatureLocation {
public:
    U2FeatureLocation() {}
    U2FeatureLocation(U2Strand strand_, const U2Region & region_) : strand(strand_), region(region_) {}

    /**  Strand of the location. */
    U2Strand    strand;
    
    /** Sequence region */
    U2Region   region;

    bool operator==(const U2FeatureLocation& l) const;
    
    bool operator!=(const U2FeatureLocation& l) const {return !(*this == l);}
};

inline bool U2FeatureLocation::operator==(const U2FeatureLocation& l) const {
    bool res = region == l.region && strand == l.strand;
    return res;
}

/** 
    Sequence annotation. Used to annotate set of sequence regions
*/
class U2CORE_EXPORT U2Feature: public U2Entity {
public:
    U2Feature() : version(0), treeVersion(0) {}

    /** Sequence this annotation is related to */
    U2DataId                    sequenceId;

    /** Parent feature id */
    U2DataId                    parentFeatureId;

    /** Name of the feature. Not a FeatureKey, because in that case can be features with no name */
    QString                     name;

    /** Annotation location */
    U2FeatureLocation           location;

    /** Version of the feature. Same as modification count */
    qint64                      version;

    /** Version of the feature sub-tree. Same as modification count for subtree*/
    qint64                      treeVersion;
};


/**
    Feature key
*/
class U2CORE_EXPORT U2FeatureKey {
public:
    /** Feature key name*/
    QString name;

    /** Feature key value */
    QString value;


    /** Constructs new empty (and invalid) key */
    U2FeatureKey(){}
    
    /** Constructs new feature key instance with key and value set */
    U2FeatureKey(const QString& _name, const QString& _value) : name(_name), value(_value){}

    /** feature key is valid if  its name and value is not empty */
    bool isValid() const {return !name.isEmpty();}

    /** Any two keys are equal if their names & values are equal */
    bool operator== ( const U2FeatureKey & k ) const { return k.name == name && k.value == value; }
    
    /** Any two keys are not equal if either their names or values are  not equal */
    bool operator!= ( const U2FeatureKey & k ) const { return !(*this == k); }

};


} //namespace

#endif
