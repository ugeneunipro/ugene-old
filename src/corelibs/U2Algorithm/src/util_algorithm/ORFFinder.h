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

#ifndef _U2_ORFFINDALGORITHM_H_
#define _U2_ORFFINDALGORITHM_H_

#include <U2Core/U2Region.h>
#include <U2Core/AnnotationData.h>

#include <QtCore/QList>

namespace U2 {

class DNATranslation;

class U2ALGORITHM_EXPORT ORFFindResult {
public:
    ORFFindResult () : region(0, 0), frame(0){}
    ORFFindResult (const U2Region& _r, int frame) 
        : region(_r), frame(frame){}
    
    void clear() {region.startPos = 0; region.length = 0; frame = 0;}
    
    bool isEmpty() const {return region.startPos == 0 && region.length == 0;}

    bool operator ==(const ORFFindResult& o) const {
        return region == o.region && frame == o.frame;
    }

    SharedAnnotationData toAnnotation(const QString& name) const {
        SharedAnnotationData data;
        data = new AnnotationData;
        data->name = name;
        data->location->regions << region;
        data->setStrand(frame < 0 ? U2Strand::Complementary : U2Strand::Direct);
        //data->qualifiers.append(U2Qualifier("frame", QString::number(frame)));
        data->qualifiers.append(U2Qualifier("dna_len", QString::number(region.length)));
        if (region.length >= 6) { // 3 bp - end codon
            data->qualifiers.append(U2Qualifier("protein_len", QString::number(region.length/3)));
        }
        return data;
    }

    U2Region region;
    int frame;

    static QList<SharedAnnotationData> toTable(const QList<ORFFindResult>& res, const QString& name)
    {
        QList<SharedAnnotationData> list;
        foreach (const ORFFindResult& f, res) {
            list.append(f.toAnnotation(name));
        }
        return list;
    }
};

class U2ALGORITHM_EXPORT ORFFindResultsListener {
public:
    virtual ~ORFFindResultsListener(){}
    virtual void onResult(const ORFFindResult& r) = 0;
};

enum ORFAlgorithmStrand {
    ORFAlgorithmStrand_Both,
    ORFAlgorithmStrand_Direct,
    ORFAlgorithmStrand_Complement
};

struct U2ALGORITHM_EXPORT ORFSettingsKeys {
public:
    static const QString STRAND;
    static const QString GENETIC_CODE;
    static const QString SEARCH_REGION;
    static const QString MIN_LEN;
    static const QString MUST_FIT;
    static const QString MUST_INIT;
    static const QString ALLOW_ALT_START;
};

class U2ALGORITHM_EXPORT ORFAlgorithmSettings {
public:
    ORFAlgorithmSettings(ORFAlgorithmStrand strand = ORFAlgorithmStrand_Both,
        DNATranslation* complementTT = NULL,
        DNATranslation* proteinTT = NULL,
        const U2Region& searchRegion = U2Region(),
        int minLen = 0,
        bool mustFit = false,
        bool mustInit = true,
        bool allowAltStart = false
        ) : strand(strand), complementTT(complementTT), proteinTT(proteinTT),
        searchRegion(searchRegion), minLen(minLen), mustFit(mustFit), 
        mustInit(mustInit), allowAltStart(allowAltStart) {}
    
    ORFAlgorithmStrand      strand;
    DNATranslation*         complementTT;
    DNATranslation*         proteinTT;
    U2Region                searchRegion;
    int                     minLen;
    bool                    mustFit;
    bool                    mustInit;
    bool                    allowAltStart;
    static const QString    ANNOTATION_GROUP_NAME;
};


class U2ALGORITHM_EXPORT ORFFindAlgorithm {
public:

    static void find(
        ORFFindResultsListener* rl,
        const ORFAlgorithmSettings& config,
        const char* sequence, 
        int seqLen, 
        int& stopFlag, 
        int& percentsCompleted);
};

}//namespace

#endif
