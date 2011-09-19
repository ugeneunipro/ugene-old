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
#include <U2Core/U2Region.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequence.h>
#include <U2Core/U2AbstractDbi.h>

namespace U2 {

class U2SequenceDbi;

//TODO: rename to U2SequenceGObject
class  U2CORE_EXPORT DNASequenceObject: public GObject {
    Q_OBJECT
public:
    DNASequenceObject(const QString& name, const DNASequence& seq, const QVariantMap& hintsMap = QVariantMap());
    ~DNASequenceObject();
    
    U2Region getSequenceRange() const {return U2Region(0, dnaSeq.seq.length());}

    const QByteArray& getSequence() const {return dnaSeq.seq;}

    DNAAlphabet* getAlphabet() const {return dnaSeq.alphabet;}
    
    const DNAQuality& getQuality() const { return dnaSeq.quality; }

    const DNASequence& getDNASequence() const {return dnaSeq;}

    int getSequenceLen() const {return dnaSeq.length();}

    const QString getSequenceName() const {return dnaSeq.getName(); }

    virtual GObject* clone() const;

    void setBase(int pos, char base);

    virtual bool checkConstraints(const GObjectConstraints* c) const;

    void setSequence(const DNASequence& seq);

    void setQuality(const DNAQuality& quality);

    bool isCircular() const {return dnaSeq.circular;}

    void setCircular(bool val);

    U2SequenceDbi* asDbi() const {return dbi;}

    U2DataId getDbiObjectId() const;

signals:
    void si_sequenceChanged();

protected:
    DNASequence     dnaSeq;
    U2SequenceDbi*  dbi;
};

class U2CORE_EXPORT DNASequenceObjectConstraints : public GObjectConstraints   {
    Q_OBJECT
public:
    DNASequenceObjectConstraints(QObject* p = NULL);
    int exactSequenceSize;
    DNAAlphabetType alphabetType;
};

class U2CORE_EXPORT DNASequenceObjectSequenceDbiWrapper: public QObject, public U2SimpleSequenceDbi {
public:
    DNASequenceObjectSequenceDbiWrapper(const QList<DNASequenceObject*> & _seqObjs, U2Dbi * root);
    
    U2Sequence getSequenceObject(const U2DataId& sequenceId, U2OpStatus& os);

    QByteArray getSequenceData(const U2DataId& sequenceId, const U2Region& region, U2OpStatus& os);

private:
    QList<DNASequenceObject*> seqObjs;
};

}//namespace


#endif
