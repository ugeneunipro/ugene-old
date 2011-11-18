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

#ifndef _U2_DNA_SEQUENCE_OBJECT_H_
#define _U2_DNA_SEQUENCE_OBJECT_H_

#include <U2Core/GObject.h>
#include <U2Core/U2Type.h>
#include <U2Core/DNASequence.h>

namespace U2 {

class DNAAlphabet;
class U2Region;
class U2OpStatus;


class U2CORE_EXPORT U2SequenceObject : public GObject {
    Q_OBJECT
public:
    U2SequenceObject(const QString& name, const U2EntityRef& seqRef, const QVariantMap& hintsMap = QVariantMap());
    
    U2EntityRef getSequenceRef() const {return getEntityRef();}

    qint64 getSequenceLength() const;

    QString getSequenceName() const ;

    DNASequence getWholeSequence() const;
    
    QByteArray getWholeSequenceData() const;
    
    QByteArray getSequenceData(const U2Region& r) const;
    
    void setWholeSequence(const DNASequence& seq);

    bool isCircular() const;
    
    void setCircular(bool v);

    DNAAlphabet* getAlphabet() const;

    void replaceRegion(const U2Region& region, const DNASequence& seq, U2OpStatus& os);

    GObject* clone(const U2DbiRef& ref, U2OpStatus& os) const;

    bool checkConstraints(const GObjectConstraints* c) const;

    void setQuality(const DNAQuality& q);

    DNAQuality getQuality() const;

	void setSequenceInfo(const QVariantMap& info);

	QVariantMap getSequenceInfo() const;

	QString getStringAttribute(const QString& seqAttr) const;

	void setStringAttribute(const QString& newStringAttributeValue, const QString& type);

	qint64 getIntegerAttribute(const QString& seqAttr) const;

	void setIntegerAttribute(int newIntegerAttributeValue, const QString& type);

	double getRealAttribute(const QString& seqAttr) const;

	void setRealAttribute(double newRealAttributeValue, const QString& type);

	QByteArray getByteArrayAttribute(const QString& seqAttr) const;

	void setByteArrayAttribute(const QByteArray& newByteArrayAttributeValue, const QString& type);

	

signals:
    void si_sequenceChanged();

protected:
    mutable DNAAlphabet*    cachedAlphabet;
    mutable qint64          cachedLength;
    mutable QString         cachedName;
    mutable TriState        cachedCircular;

};


class U2CORE_EXPORT U2SequenceObjectConstraints : public GObjectConstraints   {
    Q_OBJECT
public:
    U2SequenceObjectConstraints(QObject* p = NULL);
    qint64          sequenceSize;
    DNAAlphabetType alphabetType;
};


}//namespace


#endif
