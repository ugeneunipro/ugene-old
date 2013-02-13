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

#ifndef _U2_TOPHAT_WORKER_H
#define _U2_TOPHAT_WORKER_H

#include "TopHatSettings.h"

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>


namespace U2 {
namespace LocalWorkflow {

class TopHatPrompter : public PrompterBase<TopHatPrompter>
{
    Q_OBJECT

public:
    TopHatPrompter(Actor* parent = 0);

protected:
    QString composeRichDoc();
};

class DatasetData {
public:
    DatasetData(bool groupByDatasets);
    bool isGroup() const;

    /** Initialize the data by dataset on first check */
    bool isCurrent(const QString &dataset);
    void replaceCurrent(const QString &dataset);

private:
    bool groupByDatasets;
    QString currentDataset;
    bool inited;

private:
    void init(const QString &dataset);
};


class TopHatWorker : public BaseWorker
{
    Q_OBJECT

public:
    TopHatWorker(Actor* actor);

    void init();
    Task *tick();
    void cleanup();
    QStringList getOutputFiles();

private slots:
    void sl_topHatTaskFinished();

protected:
    IntegralBus* input;
    IntegralBus* output;
    TopHatSettings settings;
    QStringList outputFiles;

    bool settingsAreCorrect;

    /**
     * If the second slot is binded the reads are considered to be paired
     * and are input from both slots (1:1).
     */
    bool bindedToSecondSlot;
    DatasetData datasetsData;

private:
    void initDatasetData();
    QList<Actor*> getProducers(const QString &portId, const QString &slotId) const;
    Task * runTophat();
    /** Returns Tophat task is the dataset is changed */
    Task * checkDatasets(const QVariantMap &data);
};


class TopHatWorkerFactory : public DomainFactory
{
public:
    static const QString ACTOR_ID;
    static void init();
    TopHatWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* actor) { return new TopHatWorker(actor); }

    static const QString OUT_DIR;
    static const QString BOWTIE_INDEX_DIR;
    static const QString BOWTIE_INDEX_BASENAME;
    static const QString REF_SEQ;
    static const QString MATE_INNER_DISTANCE;
    static const QString MATE_STANDARD_DEVIATION;
    static const QString LIBRARY_TYPE;
    static const QString NO_NOVEL_JUNCTIONS;
    static const QString RAW_JUNCTIONS;
    static const QString KNOWN_TRANSCRIPT;
    static const QString MAX_MULTIHITS;
    static const QString SEGMENT_LENGTH;
    static const QString DISCORDANT_PAIR_ALIGNMENTS;
    static const QString FUSION_SEARCH;
    static const QString TRANSCRIPTOME_ONLY;
    static const QString TRANSCRIPTOME_MAX_HITS;
    static const QString PREFILTER_MULTIHITS;
    static const QString MIN_ANCHOR_LENGTH;
    static const QString SPLICE_MISMATCHES;
    static const QString READ_MISMATCHES;
    static const QString SEGMENT_MISMATCHES;
    static const QString SOLEXA_1_3_QUALS;
    static const QString BOWTIE_VERSION;
    static const QString BOWTIE_N_MODE;
    static const QString BOWTIE_TOOL_PATH;
    static const QString SAMTOOLS_TOOL_PATH;
    static const QString EXT_TOOL_PATH;
    static const QString TMP_DIR_PATH;

    static const QString FIRST_IN_SLOT_ID;
    static const QString SECOND_IN_SLOT_ID;
    static const QString DATASET_IN_SLOT_ID;

    static const QString OUT_MAP_DESCR_ID;
    static const QString ACCEPTED_HITS_SLOT_ID;
};

} // namespace LocalWorkflow
} // namespace U2

#endif
