/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or * modify it under the terms of the GNU General Public License
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

#include <U2Core/DocumentUtils.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/AppResources.h>

#include "Bowtie2Support.h"
#include "Bowtie2Task.h"

namespace U2 {

// Bowtie2BuildIndexTask

Bowtie2BuildIndexTask::Bowtie2BuildIndexTask(const QString &referencePath, const QString &indexPath):
    Task("Build Bowtie2 index", TaskFlags_NR_FOSE_COSC),
    referencePath(referencePath),
    indexPath(indexPath)
{
}

void Bowtie2BuildIndexTask::prepare() {
    {
        QFileInfo file(referencePath);
        if(!file.exists()) {
            stateInfo.setError(tr("Reference file \"%1\" does not exist").arg(referencePath));
            return;
        }
    }

    QStringList arguments;
    arguments.append(referencePath);
    arguments.append(indexPath);

    ExternalToolRunTask *task = new ExternalToolRunTask(ET_BOWTIE2_BUILD, arguments, &logParser);
    addSubTask(task);
}

// Bowtie2AlignTask

Bowtie2AlignTask::Bowtie2AlignTask(const DnaAssemblyToRefTaskSettings &settings):
    Task("Bowtie2 reads assembly", TaskFlags_NR_FOSE_COSC),
    settings(settings)
{
}

void Bowtie2AlignTask::prepare() {
    {
        QStringList indexSuffixes;
        indexSuffixes << ".1.bt2" << ".2.bt2" << ".3.bt2" << ".4.bt2" << ".rev.1.bt2" << ".rev.2.bt2";

        if(settings.indexFileName.isEmpty()) {
            if(settings.prebuiltIndex) {
                QString indexName = QFileInfo(settings.refSeqUrl.getURLString()).fileName();
                for (int i = 0; i < indexSuffixes.size(); ++i) {
                    indexName.remove(indexSuffixes[i]);
                }
                settings.indexFileName = settings.refSeqUrl.dirPath() + "/" + indexName;
            } else {
                settings.indexFileName = settings.resultFileName.dirPath() + "/" + settings.resultFileName.baseFileName();
            }
        }

        for(int i=0; i < 6; i++) {
            QFileInfo file(settings.indexFileName + indexSuffixes[i]);
            if(!file.exists()) {
                stateInfo.setError(tr("Reference index file \"%1\" does not exist").arg(settings.indexFileName + indexSuffixes[i]));
                return;
            }
        }
    }

    QStringList arguments;
    arguments.append(settings.getCustomValue(Bowtie2Task::OPTION_MODE, "--end-to-end").toString());

    arguments.append("-N");
    arguments.append(settings.getCustomValue(Bowtie2Task::OPTION_MISMATCHES, 0).toString());

    arguments.append(QString("-L"));
    arguments.append(settings.getCustomValue(Bowtie2Task::OPTION_SEED_LEN, 20).toString());

    arguments.append(QString("--dpad"));
    arguments.append(settings.getCustomValue(Bowtie2Task::OPTION_DPAD, 15).toString());

    arguments.append(QString("--gbar"));
    arguments.append(settings.getCustomValue(Bowtie2Task::OPTION_GBAR, 4).toString());

    {
        int seed = settings.getCustomValue(Bowtie2Task::OPTION_SEED, -1).toInt();
        if(-1 != seed) {
            arguments.append(QString("--seed"));
            arguments.append(QString::number(seed));
        }
    }

    {
        int threads = settings.getCustomValue(Bowtie2Task::OPTION_THREADS, 1).toInt();
        arguments.append(QString("--threads"));
        arguments.append(QString::number(threads));
    }

    if (settings.getCustomValue(Bowtie2Task::OPTION_NOMIXED, false).toBool()) {
        arguments.append("--no-mixed");
    }
    if (settings.getCustomValue(Bowtie2Task::OPTION_NODISCORDANT, false).toBool()) {
        arguments.append("--no-discordant");
    }
    if(settings.getCustomValue(Bowtie2Task::OPTION_NOFW, false).toBool()) {
        arguments.append("--nofw");
    }
    if(settings.getCustomValue(Bowtie2Task::OPTION_NORC, false).toBool()) {
        arguments.append("--norc");
    }
    if(settings.getCustomValue(Bowtie2Task::OPTION_NOOVERLAP, false).toBool()) {
        arguments.append("--no-overlap");
    }
    if(settings.getCustomValue(Bowtie2Task::OPTION_NOCONTAIN, false).toBool()) {
        arguments.append("--no-contain");
    }

    // We assume all datasets have the same format
    // QSEQ format is not supported
    if(!settings.shortReadSets.isEmpty())
    {
        QList<GUrl> shortReadUrls = settings.getShortReadUrls();
        QList<FormatDetectionResult> detectionResults = DocumentUtils::detectFormat(shortReadUrls.first());
        if(!detectionResults.isEmpty()) {
            if(detectionResults.first().format->getFormatId() == BaseDocumentFormats::FASTA) {
                arguments.append("-f");
            } else if(detectionResults.first().format->getFormatId() == BaseDocumentFormats::RAW_DNA_SEQUENCE) {
                arguments.append("-r");
            } else if (detectionResults.first().format->getFormatId() == BaseDocumentFormats::FASTQ) {
                arguments.append("-q");
            }else{
                setError(tr("Unknown short reads format %1").arg(detectionResults.first().format->getFormatId()));
            }
        }
    } else {
        setError("Short read list is empty!");
        return;
    }

    arguments.append(settings.indexFileName);
    {
        // we assume that all datasets have same library type
        ShortReadSet::LibraryType libType = settings.shortReadSets.at(0).type;
        int setCount = settings.shortReadSets.size();

        // unpaired reeds
        if (libType == ShortReadSet::SingleEndReads ) {
            QStringList readUrlsArgument;
            for(int index = 0;index < setCount;index++) {
                readUrlsArgument.append(settings.shortReadSets[index].url.getURLString());
            }
            arguments.append("-U");
            arguments.append(readUrlsArgument.join(","));
        } else {
            // paired reeds: mate 1s and 2s
            QStringList upstreamReads,downstreamReads;

            for ( int i = 0; i<setCount; ++i) {
                const ShortReadSet& set = settings.shortReadSets.at(i);
                if (set.order == ShortReadSet::UpstreamMate) {
                    upstreamReads.append(set.url.getURLString());
                } else {
                    downstreamReads.append(set.url.getURLString());
                }
            }

            if ( upstreamReads.count() != downstreamReads.count() ) {
                setError("Unequal number of upstream and downstream reads!");
                return;
            }

            arguments.append("-1");
            arguments.append(upstreamReads.join(","));
            arguments.append("-2");
            arguments.append(downstreamReads.join(","));
        }
    }
    arguments.append("-S");
    arguments.append(settings.resultFileName.getURLString());

    ExternalToolRunTask *task = new ExternalToolRunTask(ET_BOWTIE2_ALIGN, arguments, &logParser);
    addSubTask(task);
}

// Bowtie2Task

const QString Bowtie2Task::taskName = "Bowtie2";

const QString Bowtie2Task::OPTION_MODE = "mode";
const QString Bowtie2Task::OPTION_MISMATCHES = "mismatches";
const QString Bowtie2Task::OPTION_SEED_LEN = "seedLen";
const QString Bowtie2Task::OPTION_DPAD = "dpad";
const QString Bowtie2Task::OPTION_GBAR = "gbar";
const QString Bowtie2Task::OPTION_SEED = "seed";
const QString Bowtie2Task::OPTION_OFFRATE = "offrate";
const QString Bowtie2Task::OPTION_THREADS = "threads";

const QString Bowtie2Task::OPTION_NOMIXED = "no-mixed";
const QString Bowtie2Task::OPTION_NODISCORDANT = "no-discordant";
const QString Bowtie2Task::OPTION_NOFW = "nofw";
const QString Bowtie2Task::OPTION_NORC = "norc";
const QString Bowtie2Task::OPTION_NOOVERLAP = "no-overlap";
const QString Bowtie2Task::OPTION_NOCONTAIN = "no-contain";

Bowtie2Task::Bowtie2Task(const DnaAssemblyToRefTaskSettings &settings, bool justBuildIndex):
    DnaAssemblyToReferenceTask(settings, TaskFlags_NR_FOSE_COSC, justBuildIndex)
{
}

void Bowtie2Task::prepare() {
    if(!settings.prebuiltIndex) {
        QString indexFileName = settings.indexFileName;
        if(indexFileName.isEmpty()) {
            if(justBuildIndex) {
                indexFileName = settings.refSeqUrl.dirPath() + "/" + settings.refSeqUrl.baseFileName();
            } else {
                indexFileName = settings.resultFileName.dirPath() + "/" + settings.resultFileName.baseFileName();
            }
        }
        buildIndexTask = new Bowtie2BuildIndexTask(settings.refSeqUrl.getURLString(), indexFileName);
    }
    if(!justBuildIndex) {
        alignTask = new Bowtie2AlignTask(settings);
    }

    if(!settings.prebuiltIndex) {
        addSubTask(buildIndexTask);
    } else if(!justBuildIndex) {
        addSubTask(alignTask);
    } else {
        assert(false);
    }
}

Task::ReportResult Bowtie2Task::report() {
    if(!justBuildIndex) {
        haveResults = true;
    }
    return ReportResult_Finished;
}

QList<Task *> Bowtie2Task::onSubTaskFinished(Task *subTask) {
    QList<Task *> result;
    if((subTask == buildIndexTask) && !justBuildIndex) {
        result.append(alignTask);
    }
    return result;
}

// Bowtie2TaskFactory

DnaAssemblyToReferenceTask *Bowtie2TaskFactory::createTaskInstance(const DnaAssemblyToRefTaskSettings &settings, bool justBuildIndex) {
    return new Bowtie2Task(settings, justBuildIndex);
}

} // namespace U2
