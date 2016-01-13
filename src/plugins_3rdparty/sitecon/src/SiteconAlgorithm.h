/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_SITECON_ALG_H_
#define _U2_SITECON_ALG_H_

#include <U2Core/U2Region.h>
#include <U2Core/Task.h>

#include <QtCore/QVector>

namespace U2 {

class MAlignment;
class DiPropertySitecon;

//average and deviation for one property
class DiStat {
public:
    DiStat(DiPropertySitecon* p, float d, float a): prop(p), sdeviation(d), average(a), weighted(false){}
    DiStat(): prop(NULL), sdeviation(-1), average(-1), weighted(false){}

    DiPropertySitecon* prop;
    float       sdeviation;
    float       average;
    bool        weighted;
};
typedef QVector<DiStat> PositionStats;

enum SiteconWeightAlg {
    SiteconWeightAlg_None,
    SiteconWeightAlg_Alg2
};

class SiteconBuildSettings {
public:
    SiteconBuildSettings() : windowSize(0), randomSeed(0), secondTypeErrorCalibrationLen(100*1000), 
        chisquare(0.95f),numSequencesInAlignment(0), weightAlg(SiteconWeightAlg_None)
    {
        acgtContent[0] = acgtContent[1] = acgtContent[2] = acgtContent[3] = 25;
    }

    int                 windowSize;
    int                 randomSeed;
    int                 secondTypeErrorCalibrationLen;
    float               chisquare;
    int                 numSequencesInAlignment;
    SiteconWeightAlg    weightAlg;
    int                 acgtContent[4];
    QList<DiPropertySitecon*>  props;
};


class SiteconModel {
public:
    SiteconModel(){ deviationThresh = -1;}
    QString                 aliURL;
    QString                 modelName;
    QString                 description;
    SiteconBuildSettings    settings;
    QVector<PositionStats>  matrix;
    QVector<float>          err1;
    QVector<float>          err2;
    float                   deviationThresh;
    bool checkState(bool doAssert = true) const;
    bool operator !=(const SiteconModel& model) const;
};

class DNATranslation;
class SiteconAlgorithm : public QObject {
    Q_OBJECT
public:
    static QVector<PositionStats> calculateDispersionAndAverage(const MAlignment& ma, const SiteconBuildSettings& s, TaskStateInfo& ts);

    static float calculatePSum(const char* seq, int len, const QVector<PositionStats>& normalizedMatrix, 
                               const SiteconBuildSettings& settings, float devThreshold, DNATranslation* complMap = NULL);
        
    static QVector<float> calculateFirstTypeError(const MAlignment& ma, const SiteconBuildSettings& s, TaskStateInfo& ts);

    static QVector<float> calculateSecondTypeError(const QVector<PositionStats>& matrix, const SiteconBuildSettings& s, TaskStateInfo& ts);

    static QVector<PositionStats> normalize(const QVector<PositionStats>& matrix, const SiteconBuildSettings& s);

    static int calculateWeights(const MAlignment& ma, QVector<PositionStats>& matrix, 
                                const SiteconBuildSettings& settings, bool matrixIsNormalized, TaskStateInfo& s);

    static void calculateACGTContent(const MAlignment& ma, SiteconBuildSettings& bs);

    static QByteArray generateRandomSequence(const int* actgContent, int seqLen, TaskStateInfo& ts);
};

}//namespace

#endif
