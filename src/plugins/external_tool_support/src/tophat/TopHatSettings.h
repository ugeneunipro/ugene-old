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

#ifndef _U2_TOPHAT_SETTINGS_H_
#define _U2_TOPHAT_SETTINGS_H_

#include "../RnaSeqCommon.h"

#include <U2Core/DNASequenceObject.h>

#include <U2Lang/DbiDataHandler.h>
#include <U2Lang/DbiDataStorage.h>
#include <U2Lang/WorkflowContext.h>


namespace U2 {

class TopHatSettings
{
public:
    TopHatSettings();

    // Workflow element parameters
    QString             bowtieIndexPathAndBasename;
    int                 mateInnerDistance;
    int                 mateStandardDeviation;
    RnaSeqLibraryType   libraryType;
    bool                noNovelJunctions;
    QString             rawJunctions;
    QString             knownTranscript;
    int                 maxMultihits;
    int                 segmentLength;
    bool                fusionSearch;
    bool                transcriptomeOnly;
    int                 transcriptomeMaxHits;
    bool                prefilterMultihits;
    int                 minAnchorLength;
    int                 spliceMismatches;
    int                 transcriptomeMismatches;
    int                 genomeReadMismatches;
    int                 readMismatches;
    int                 segmentMismatches;
    bool                solexa13quals;
    BowtieMode          bowtieMode;
    bool                useBowtie1;
    QString             bowtiePath;
    QString             samtoolsPath;

    /**
     * Input reads: if reads are paired both lists are filled.
     * Otherwise only seqIds list is filled.
     * Note that the pairedSeqIds list is used to detect whether paired reads were input (empty/not empty).
     */
    Workflow::WorkflowContext*              workflowContext;
    Workflow::DbiDataStorage*               storage;
    QList<Workflow::SharedDbiDataHandler>   seqIds;
    QList<Workflow::SharedDbiDataHandler>   pairedSeqIds;
};

}

#endif
