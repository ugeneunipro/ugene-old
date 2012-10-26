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

#ifndef _U2_FINDALGORITHM_H_
#define _U2_FINDALGORITHM_H_

#include <U2Core/U2Region.h>
#include <U2Core/AnnotationData.h>

#include <QtCore/QList>

namespace U2 {


class U2ALGORITHM_EXPORT FindAlgorithmResult {
public:
    FindAlgorithmResult() : err(0) {}
    FindAlgorithmResult(const U2Region& _r, bool t, U2Strand s, int _err) 
        : region(_r), translation(t), strand(s), err(_err){}
    
    void clear() {region.startPos = 0; region.length = 0; translation = false; strand = U2Strand::Direct; err = 0;}
    
    bool isEmpty() const {return region.startPos == 0 && region.length == 0;}

    bool operator ==(const FindAlgorithmResult& o) const {
        return region == o.region && err == o.err && strand == o.strand && translation == o.translation;
    }

    SharedAnnotationData toAnnotation(const QString& name) const {
        SharedAnnotationData data;
        data = new AnnotationData;
        data->name = name;
        data->location->regions << region;
        data->setStrand(strand);
        data->qualifiers.append(U2Qualifier("mismatches", QString::number(err)));
        return data;
    }

    U2Region    region;
    bool        translation;
    U2Strand    strand;
    int         err;

    static QList<SharedAnnotationData> toTable(const QList<FindAlgorithmResult>& res, const QString& name)
    {
        QList<SharedAnnotationData> list;
        foreach (const FindAlgorithmResult& f, res) {
            list.append(f.toAnnotation(name));
        }
        return list;
    }

};

class DNATranslation;

class U2ALGORITHM_EXPORT FindAlgorithmResultsListener {
public:
    virtual ~FindAlgorithmResultsListener(){}
    virtual void onResult(const FindAlgorithmResult& r) = 0;
};

enum FindAlgorithmStrand {
    FindAlgorithmStrand_Both,
    FindAlgorithmStrand_Direct,
    FindAlgorithmStrand_Complement
};

enum FindAlgorithmPatternSettings {
    FindAlgorithmPatternSettings_InsDel,
    FindAlgorithmPatternSettings_Subst,
    FindAlgorithmPatternSettings_RegExp,
    FindAlgorithmPatternSettings_Exact
};

class U2ALGORITHM_EXPORT FindAlgorithmSettings {
public:
    FindAlgorithmSettings(const QByteArray& pattern = QByteArray(),
        FindAlgorithmStrand strand = FindAlgorithmStrand_Direct,
        DNATranslation* complementTT = NULL,
        DNATranslation* proteinTT = NULL,
        const U2Region& searchRegion = U2Region(),
        bool singleShot = false,
        int maxErr = 0,
        FindAlgorithmPatternSettings _patternSettings = FindAlgorithmPatternSettings_Subst, 
        bool ambBases = false,
        int _maxRegExpResult = 100,
        int _maxResult2Find = 5000) : pattern(pattern), strand(strand), complementTT(complementTT), proteinTT(proteinTT),
searchRegion(searchRegion), singleShot(singleShot), maxErr(maxErr), patternSettings (_patternSettings ), useAmbiguousBases (ambBases),
    maxRegExpResult(_maxRegExpResult),maxResult2Find(_maxResult2Find){}

    QByteArray                          pattern;
    FindAlgorithmStrand                 strand;
    DNATranslation*                     complementTT;
    DNATranslation*                     proteinTT;
    U2Region                            searchRegion;
    bool                                singleShot;
    int                                 maxErr;
    FindAlgorithmPatternSettings        patternSettings;
    bool                                useAmbiguousBases;
    int                                 maxRegExpResult;
    int                                 maxResult2Find;
};


class U2ALGORITHM_EXPORT FindAlgorithm {
public:
    // Note: pattern is never affected by either aminoTT or complTT
    
    static void find(
        FindAlgorithmResultsListener* rl, 
        DNATranslation* aminoTT, // if aminoTT!=NULL -> pattern must contain amino data and sequence must contain DNA data
        DNATranslation* complTT, // if complTT!=NULL -> sequence is complemented before comparison with pattern
        FindAlgorithmStrand strand, // if not direct there complTT must not be NULL
        FindAlgorithmPatternSettings patternSettings,
        bool supportAmbigiousBases,
        const char* sequence, 
        int seqLen, 
        const U2Region& range,  
        const char* pattern, 
        int patternLen, 
        bool singleShot,
        int maxErr,
        int maxRegExpResult,
        int& stopFlag, 
        int& percentsCompleted, 
        int& currentPos,
        int currentLen
        );

    static void find(
        FindAlgorithmResultsListener* rl,
        const FindAlgorithmSettings& config,
        const char* sequence, 
        int seqLen, 
        int& stopFlag, 
        int& percentsCompleted, 
        int& currentPos,
        int currentLen) {
            find(rl,
                config.proteinTT,
                config.complementTT,
                config.strand,
                config.patternSettings,
                config.useAmbiguousBases,
                sequence,
                seqLen,
                config.searchRegion,
                config.pattern.constData(),
                config.pattern.length(),
                config.singleShot,
                config.maxErr,
                config.maxRegExpResult,
                stopFlag, 
                percentsCompleted, 
                currentPos,
                currentLen);
    }

    static int estimateRamUsageInMbytes(const FindAlgorithmPatternSettings patternSettings,
        const bool searchInAminoTT, const int patternLength, const int maxError);
};


}//namespace

#endif
