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

#ifndef _U2_BAM_ALIGNMENT_H_
#define _U2_BAM_ALIGNMENT_H_

#include <QtCore/QFlags>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QVariant>

namespace U2 {
namespace BAM {

class Alignment
{
public:
    class CigarOperation
    {
    public:
        enum Operation
        {
            AlignmentMatch,
            Insertion,
            Deletion,
            Skipped,
            SoftClip,
            HardClip,
            Padding,
            SequenceMatch,
            SequenceMismatch
        };
        CigarOperation(int length, Operation operation);
        int getLength()const;
        Operation getOperation()const;
        void setLength(int length);
        void setOperation(Operation operation);
    private:
        int length;
        Operation operation;
    };
    enum Flag {
        None = 0, 
        Fragmented = 1 << 0,
        FragmentsAligned = 1 << 1,
        Unmapped = 1 << 2,
        NextUnmapped = 1 << 3,
        Reverse = 1 << 4,
        NextReverse = 1 << 5,
        FirstInTemplate = 1 << 6,
        LastInTemplate = 1 << 7,
        SecondaryAlignment = 1 << 8,
        FailsChecks = 1 << 9,
        Duplicate = 1 << 10
    };
    Q_DECLARE_FLAGS(Flags, Flag)
    Alignment();
    int getReferenceId()const;
    int getPosition()const;
    int getBin()const;
    int getMapQuality()const;
    Flags getFlags()const;
    int getNextReferenceId()const;
    int getNextPosition()const;
    int getTemplateLength()const;
    const QByteArray &getName()const;
    const QList<CigarOperation> &getCigar()const;
    const QByteArray &getSequence()const;
    const QByteArray &getQuality()const;
    const QMap<QByteArray, QVariant> &getOptionalFields()const;
    void setReferenceId(int referenceId);
    void setPosition(int position);
    void setBin(int bin);
    void setMapQuality(int mapQuality);
    void setFlags(Flags flags);
    void setNextReferenceId(int nextReferenceId);
    void setNextPosition(int nextPosition);
    void setTemplateLength(int templateLength);
    void setName(const QByteArray &name);
    void setCigar(const QList<CigarOperation> &cigar);
    void setSequence(const QByteArray &sequence);
    void setQuality(const QByteArray &quality);
    void setOptionalFields(const QMap<QByteArray, QVariant> &optionalFields);

    static int computeLength(const Alignment &alignment);
private:
    int referenceId;
    int position;
    int bin;
    int mapQuality;
    Flags flags;
    int nextReferenceId;
    int nextPosition;
    int templateLength;
    QByteArray name;
    QList<CigarOperation> cigar;
    QByteArray sequence;
    QByteArray quality;
    QMap<QByteArray, QVariant> optionalFields;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Alignment::Flags)

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_ALIGNMENT_H_
