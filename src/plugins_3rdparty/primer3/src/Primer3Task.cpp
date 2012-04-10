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

#include <cstdlib>
#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/SequenceWalkerTask.h>
#include <U2Core/CreateAnnotationTask.h>
#include "primer3_main.h"
#include "boulder_input.h"
#include "Primer3Task.h"

namespace U2 {

// Primer

Primer::Primer():
    start(0),
    length(0),
    meltingTemperature(0),
    gcContent(0),
    selfAny(0),
    selfEnd(0),
    endStability(0)
{
}

Primer::Primer(const primer_rec &primerRec):
    start(primerRec.start),
    length(primerRec.length),
    meltingTemperature(primerRec.temp),
    gcContent(primerRec.gc_content),
    selfAny(primerRec.self_any),
    selfEnd(primerRec.self_end),
    endStability(primerRec.end_stability)
{
}

int Primer::getStart()const
{
    return start;
}

int Primer::getLength()const
{
    return length;
}

double Primer::getMeltingTemperature()const
{
    return meltingTemperature;
}

double Primer::getGcContent()const
{
    return gcContent;
}

short Primer::getSelfAny()const
{
    return selfAny;
}

short Primer::getSelfEnd()const
{
    return selfEnd;
}

double Primer::getEndStabilyty()const
{
    return endStability;
}

void Primer::setStart(int start)
{
    this->start = start;
}

void Primer::setLength(int length)
{
    this->length = length;
}

void Primer::setMeltingTemperature(double meltingTemperature)
{
    this->meltingTemperature = meltingTemperature;
}

void Primer::setGcContent(double gcContent)
{
    this->gcContent = gcContent;
}

void Primer::setSelfAny(short selfAny)
{
    this->selfAny = selfAny;
}

void Primer::setSelfEnd(short selfEnd)
{
    this->selfEnd = selfEnd;
}

void Primer::setEndStability(double endStability)
{
    this->endStability = endStability;
}

// PrimerPair

PrimerPair::PrimerPair():
    leftPrimer(NULL),
    rightPrimer(NULL),
    internalOligo(NULL),
    complAny(0),
    complEnd(0),
    productSize(0),
    quality(0),
    complMeasure(0)
{
}

PrimerPair::PrimerPair(const primer_pair &primerPair, int offset):
    leftPrimer((NULL == primerPair.left)? NULL:new Primer(*primerPair.left)),
    rightPrimer((NULL == primerPair.right)? NULL:new Primer(*primerPair.right)),
    internalOligo((NULL == primerPair.intl)? NULL:new Primer(*primerPair.intl)),
    complAny(primerPair.compl_any),
    complEnd(primerPair.compl_end),
    productSize(primerPair.product_size),
    quality(primerPair.pair_quality),
    complMeasure(primerPair.compl_measure)
{
    if(NULL != leftPrimer.get())
    {
        leftPrimer->setStart(leftPrimer->getStart() + offset);
    }
    if(NULL != rightPrimer.get())
    {
        rightPrimer->setStart(rightPrimer->getStart() + offset);
    }
    if(NULL != internalOligo.get())
    {
        internalOligo->setStart(internalOligo->getStart() + offset);
    }
}

PrimerPair::PrimerPair(const PrimerPair &primerPair):
        leftPrimer((NULL == primerPair.leftPrimer.get())? NULL:new Primer(*primerPair.leftPrimer)),
        rightPrimer((NULL == primerPair.rightPrimer.get())? NULL:new Primer(*primerPair.rightPrimer)),
        internalOligo((NULL == primerPair.internalOligo.get())? NULL:new Primer(*primerPair.internalOligo)),
        complAny(primerPair.complAny),
        complEnd(primerPair.complEnd),
        productSize(primerPair.productSize),
        quality(primerPair.quality),
        complMeasure(primerPair.complMeasure)
{
}

const PrimerPair &PrimerPair::operator=(const PrimerPair &primerPair)
{
    leftPrimer.reset((NULL == primerPair.leftPrimer.get())? NULL:new Primer(*primerPair.leftPrimer));
    rightPrimer.reset((NULL == primerPair.rightPrimer.get())? NULL:new Primer(*primerPair.rightPrimer));
    internalOligo.reset((NULL == primerPair.internalOligo.get())? NULL:new Primer(*primerPair.internalOligo));
    complAny = primerPair.complAny;
    complEnd = primerPair.complEnd;
    productSize = primerPair.productSize;
    quality = primerPair.quality;
    complMeasure = primerPair.complMeasure;
    return *this;
}

Primer *PrimerPair::getLeftPrimer()const
{
    return leftPrimer.get();
}

Primer *PrimerPair::getRightPrimer()const
{
    return rightPrimer.get();
}

Primer *PrimerPair::getInternalOligo()const
{
    return internalOligo.get();
}

short PrimerPair::getComplAny()const
{
    return complAny;
}

short PrimerPair::getComplEnd()const
{
    return complEnd;
}

int PrimerPair::getProductSize()const
{
    return productSize;
}

void PrimerPair::setLeftPrimer(Primer *leftPrimer)
{
    this->leftPrimer.reset((NULL == leftPrimer)? NULL:new Primer(*leftPrimer));
}

void PrimerPair::setRightPrimer(Primer *rightPrimer)
{
    this->rightPrimer.reset((NULL == rightPrimer)? NULL:new Primer(*rightPrimer));
}

void PrimerPair::setInternalOligo(Primer *internalOligo)
{
    this->internalOligo.reset((NULL == internalOligo)? NULL:new Primer(*internalOligo));
}

void PrimerPair::setComplAny(short complAny)
{
    this->complAny = complAny;
}

void PrimerPair::setComplEnd(short complEnd)
{
    this->complEnd = complEnd;
}

void PrimerPair::setProductSize(int productSize)
{
    this->productSize = productSize;
}

bool PrimerPair::operator<(const PrimerPair &pair)const
{
    if(quality < pair.quality)
    {
        return true;
    }
    if(quality > pair.quality)
    {
        return false;
    }

    if (complMeasure < pair.complMeasure)
    {
        return true;
    }
    if (complMeasure > pair.complMeasure)
    {
        return false;
    }

    if(leftPrimer->getStart() > pair.leftPrimer->getStart())
    {
        return true;
    }
    if(leftPrimer->getStart() < pair.leftPrimer->getStart())
    {
        return false;
    }

    if(rightPrimer->getStart() < pair.rightPrimer->getStart())
    {
        return true;
    }
    if(rightPrimer->getStart() > pair.rightPrimer->getStart())
    {
        return false;
    }

    if(leftPrimer->getLength() < pair.leftPrimer->getLength())
    {
        return true;
    }
    if(leftPrimer->getLength() > pair.leftPrimer->getLength())
    {
        return false;
    }

    if(rightPrimer->getLength() < pair.rightPrimer->getLength())
    {
        return true;
    }
    if(rightPrimer->getLength() > pair.rightPrimer->getLength())
    {
        return false;
    }

    return false;
}

// Primer3Task

namespace
{
    bool clipRegion(QPair<int, int> &region, const QPair<int, int> &clippingRegion)
    {
        int start = region.first;
        int end = region.first + region.second;
        start = qMax(start, clippingRegion.first);
        end = qMin(end, clippingRegion.first + clippingRegion.second);
        if(start > end)
        {
            return false;
        }
        region.first = start;
        region.second = end - start;
        return true;
    }
}

Primer3Task::Primer3Task(const Primer3TaskSettings &settingsArg):
    Task(tr("Pick primers task"), TaskFlag_ReportingIsEnabled),
    settings(settingsArg)
{
    GCOUNTER( cvar, tvar, "Primer3Task" );
    {
        QPair<int, int> region = settings.getIncludedRegion();
        region.first -= settings.getFirstBaseIndex();
        settings.setIncludedRegion(region);
    }
    offset = settings.getIncludedRegion().first;
    settings.setSequence(settings.getSequence().mid(
            settings.getIncludedRegion().first, settings.getIncludedRegion().second));
    settings.setSequenceQuality(settings.getSequenceQuality().mid(
            settings.getIncludedRegion().first, settings.getIncludedRegion().second));
    settings.setIncludedRegion(qMakePair(0, settings.getIncludedRegion().second));
    if(!PR_START_CODON_POS_IS_NULL(settings.getSeqArgs()))
    {
        int startCodonPosition = PR_DEFAULT_START_CODON_POS;
        if(settings.getIntProperty("PRIMER_START_CODON_POSITION", &startCodonPosition))
        {
            settings.setIntProperty("PRIMER_START_CODON_POSITION",
                                    startCodonPosition - settings.getFirstBaseIndex());
        }
    }
    {
        QList<QPair<int, int> > regionList;
        QPair<int, int> region;
        foreach(region, settings.getTarget())
        {
            region.first -= settings.getFirstBaseIndex();
            region.first -= offset;
            if(clipRegion(region, settings.getIncludedRegion()))
            {
                regionList.append(region);
            }
        }
        settings.setTarget(regionList);
    }
    {
        QList<QPair<int, int> > regionList;
        QPair<int, int> region;
        foreach(region, settings.getExcludedRegion())
        {
            region.first -= settings.getFirstBaseIndex();
            region.first -= offset;
            if(clipRegion(region, settings.getIncludedRegion()))
            {
                regionList.append(region);
            }
        }
        settings.setExcludedRegion(regionList);
    }
    {
        QList<QPair<int, int> > regionList;
        QPair<int, int> region;
        foreach(region, settings.getInternalOligoExcludedRegion())
        {
            region.first -= settings.getFirstBaseIndex();
            region.first -= offset;
            if(clipRegion(region, settings.getIncludedRegion()))
            {
                regionList.append(region);
            }
        }
        settings.setInternalOligoExcludedRegion(regionList);
    }
}

void Primer3Task::run()
{
    if(!settings.getRepeatLibrary().isEmpty())
    {
        read_seq_lib(&settings.getPrimerArgs()->repeat_lib, settings.getRepeatLibrary().constData(), "mispriming library");
        if(NULL != settings.getPrimerArgs()->repeat_lib.error.data)
        {
            pr_append_new_chunk(&settings.getPrimerArgs()->glob_err, settings.getPrimerArgs()->repeat_lib.error.data);
            pr_append_new_chunk(&settings.getSeqArgs()->error, settings.getPrimerArgs()->repeat_lib.error.data);
            return;
        }
    }
    if(!settings.getMishybLibrary().isEmpty())
    {
        read_seq_lib(&settings.getPrimerArgs()->io_mishyb_library, settings.getMishybLibrary().constData(), "internal oligo mishyb library");
        if(NULL != settings.getPrimerArgs()->io_mishyb_library.error.data)
        {
            pr_append_new_chunk(&settings.getPrimerArgs()->glob_err, settings.getPrimerArgs()->io_mishyb_library.error.data);
            pr_append_new_chunk(&settings.getSeqArgs()->error, settings.getPrimerArgs()->io_mishyb_library.error.data);
            return;
        }
    }
    primers_t primers = runPrimer3(settings.getPrimerArgs(), settings.getSeqArgs(), &stateInfo.cancelFlag, &stateInfo.progress);

    bestPairs.clear();
    for(int index = 0;index < primers.best_pairs.num_pairs;index++)
    {
        bestPairs.append(PrimerPair(primers.best_pairs.pairs[index], offset));
    }

    if(primers.best_pairs.num_pairs > 0)
    {
        std::free(primers.best_pairs.pairs);
        primers.best_pairs.pairs = NULL;
    }
    if(NULL != primers.left)
    {
        std::free(primers.left);
        primers.left = NULL;
    }
    if(NULL != primers.right)
    {
        std::free(primers.right);
        primers.right = NULL;
    }
    if(NULL != primers.intl)
    {
        std::free(primers.intl);
        primers.intl = NULL;
    }
}

Task::ReportResult Primer3Task::report()
{
    if(!settings.getError().isEmpty())
    {
        stateInfo.setError(settings.getError());
    }
    return Task::ReportResult_Finished;
}

void Primer3Task::sumStat(Primer3TaskSettings *st) {
    st->getSeqArgs()->left_expl.considered += settings.getSeqArgs()->left_expl.considered;
    st->getSeqArgs()->left_expl.ns += settings.getSeqArgs()->left_expl.ns;
    st->getSeqArgs()->left_expl.target += settings.getSeqArgs()->left_expl.target;
    st->getSeqArgs()->left_expl.excluded += settings.getSeqArgs()->left_expl.excluded;
    st->getSeqArgs()->left_expl.gc += settings.getSeqArgs()->left_expl.gc;
    st->getSeqArgs()->left_expl.gc_clamp += settings.getSeqArgs()->left_expl.gc_clamp;
    st->getSeqArgs()->left_expl.temp_min += settings.getSeqArgs()->left_expl.temp_min;
    st->getSeqArgs()->left_expl.temp_max += settings.getSeqArgs()->left_expl.temp_max;
    st->getSeqArgs()->left_expl.compl_any += settings.getSeqArgs()->left_expl.compl_any;
    st->getSeqArgs()->left_expl.compl_end += settings.getSeqArgs()->left_expl.compl_end;
    st->getSeqArgs()->left_expl.poly_x += settings.getSeqArgs()->left_expl.poly_x;
    st->getSeqArgs()->left_expl.stability += settings.getSeqArgs()->left_expl.stability;
    st->getSeqArgs()->left_expl.ok += settings.getSeqArgs()->left_expl.ok;

    st->getSeqArgs()->right_expl.considered += settings.getSeqArgs()->right_expl.considered;
    st->getSeqArgs()->right_expl.ns += settings.getSeqArgs()->right_expl.ns;
    st->getSeqArgs()->right_expl.target += settings.getSeqArgs()->right_expl.target;
    st->getSeqArgs()->right_expl.excluded += settings.getSeqArgs()->right_expl.excluded;
    st->getSeqArgs()->right_expl.gc += settings.getSeqArgs()->right_expl.gc;
    st->getSeqArgs()->right_expl.gc_clamp += settings.getSeqArgs()->right_expl.gc_clamp;
    st->getSeqArgs()->right_expl.temp_min += settings.getSeqArgs()->right_expl.temp_min;
    st->getSeqArgs()->right_expl.temp_max += settings.getSeqArgs()->right_expl.temp_max;
    st->getSeqArgs()->right_expl.compl_any += settings.getSeqArgs()->right_expl.compl_any;
    st->getSeqArgs()->right_expl.compl_end += settings.getSeqArgs()->right_expl.compl_end;
    st->getSeqArgs()->right_expl.poly_x += settings.getSeqArgs()->right_expl.poly_x;
    st->getSeqArgs()->right_expl.stability += settings.getSeqArgs()->right_expl.stability;
    st->getSeqArgs()->right_expl.ok += settings.getSeqArgs()->right_expl.ok;

    st->getSeqArgs()->pair_expl.considered += settings.getSeqArgs()->pair_expl.considered;
    st->getSeqArgs()->pair_expl.product += settings.getSeqArgs()->pair_expl.product;
    st->getSeqArgs()->pair_expl.compl_end += settings.getSeqArgs()->pair_expl.compl_end;
    st->getSeqArgs()->pair_expl.ok += settings.getSeqArgs()->pair_expl.ok;
}

QList<PrimerPair> Primer3Task::getBestPairs()const
{
    return bestPairs;
}

// Primer3SWTask

Primer3SWTask::Primer3SWTask(const Primer3TaskSettings &settingsArg):
    Task("Pick primers SW task", TaskFlags_NR_FOSCOE),
    settings(settingsArg)
{
    setMaxParallelSubtasks(MAX_PARALLEL_SUBTASKS_AUTO);
}

void Primer3SWTask::prepare()
{
    if((settings.getIncludedRegion().first < settings.getFirstBaseIndex()) ||
       (settings.getIncludedRegion().second <= 0) ||
       (settings.getIncludedRegion().first + settings.getIncludedRegion().second >
        settings.getSequence().size() + settings.getFirstBaseIndex()))
    {
        setError("invalid included region");
        return;
    }
    QVector<U2Region> regions = SequenceWalkerTask::splitRange(
        U2Region(settings.getIncludedRegion().first, settings.getIncludedRegion().second),
        CHUNK_SIZE, 0, CHUNK_SIZE/2, false);
    foreach(U2Region region, regions)
    {
        Primer3TaskSettings regionSettings = settings;
        regionSettings.setIncludedRegion(qMakePair((int)region.startPos, (int)region.length));
        Primer3Task *task = new Primer3Task(regionSettings);
        regionTasks.append(task);
        addSubTask(task);
    }
}

Task::ReportResult Primer3SWTask::report()
{
    foreach(Primer3Task *task, regionTasks)
    {
        bestPairs.append(task->getBestPairs());
    }
    if(regionTasks.size() > 1)
    {
        qStableSort(bestPairs);
        int pairsCount = 0;
        if(!settings.getIntProperty("PRIMER_NUM_RETURN", &pairsCount))
        {
            setError("can't get PRIMER_NUM_RETURN property");
            return Task::ReportResult_Finished;
        }
        bestPairs = bestPairs.mid(0, pairsCount);
    }
    return Task::ReportResult_Finished;
}

QList<PrimerPair> Primer3SWTask::getBestPairs()const
{
    return bestPairs;
}

//////////////////////////////////////////////////////////////////////////
////Primer3ToAnnotationsTask

Primer3ToAnnotationsTask::Primer3ToAnnotationsTask( const Primer3TaskSettings &settings,
AnnotationTableObject * aobj_, const QString & groupName_, const QString & annName_ ) :
Task(tr("Search primers to annotations"), /*TaskFlags_NR_FOSCOE*/TaskFlags(TaskFlag_NoRun) | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled), settings(settings), aobj(aobj_), 
    groupName(groupName_), annName(annName_), searchTask(NULL), findExonsTask(NULL)
{
}

void Primer3ToAnnotationsTask::prepare()
{
    searchTask = new Primer3SWTask(settings);
    addSubTask(searchTask);
}

QString Primer3ToAnnotationsTask::generateReport() const {
    QString res;

    foreach(Primer3Task *t, searchTask->regionTasks) {
        t->sumStat(&searchTask->settings);
    }

    oligo_stats leftStats = searchTask->settings.getSeqArgs()->left_expl;
    oligo_stats rightStats = searchTask->settings.getSeqArgs()->right_expl;
    pair_stats pairStats = searchTask->settings.getSeqArgs()->pair_expl;

    if(!leftStats.considered) {
        leftStats.considered = leftStats.ns + leftStats.target + leftStats.excluded + leftStats.gc + leftStats.gc_clamp + leftStats.temp_min 
            + leftStats.temp_max + leftStats.compl_any + leftStats.compl_end + leftStats.poly_x + leftStats.stability + leftStats.ok;
    }

    if(!rightStats.considered) {
        rightStats.considered = rightStats.ns + rightStats.target + rightStats.excluded + rightStats.gc + rightStats.gc_clamp + rightStats.temp_min 
            + rightStats.temp_max + rightStats.compl_any + rightStats.compl_end + rightStats.poly_x + rightStats.stability + rightStats.ok;
    }

    res+="<table cellspacing='7'>";
    res += "<tr><th>Statistics</th></tr>\n";

    res += QString("<tr><th></th> <th>con</th> <th>too</th> <th>in</th> <th>in</th> <th></th> <th>no</th> <th>tm</th> <th>tm</th> <th>high</th> <th>high</th> <th></th> <th>high</th> <th></th></tr>");
    res += QString("<tr><th></th> <th>sid</th> <th>many</th> <th>tar</th> <th>excl</th> <th>bag</th> <th>GC</th> <th>too</th> <th>too</th> <th>any</th> <th>3'</th> <th>poly</th> <th>end</th> <th></th></tr>");
    res += QString("<tr><th></th> <th>ered</th> <th>Ns</th> <th>get</th> <th>reg</th> <th>GC&#37;</th> <th>clamp</th> <th>low</th> <th>high</th> <th>compl</th> <th>compl</th> <th>X</th> <th>stab</th> <th>ok</th></tr>");

    res += QString("<tr><th>Left</th><th> %1 </th><th> %2 </th><th> %3 </th><th> %4 </th><th> %5 </th><th> %6 </th><th> %7 </th><th> %8 </th><th> %9 </th><th> %10 </th><th> %11 </th><th> %12 </th><th> %13 </th></tr>")
        .arg(leftStats.considered).arg(leftStats.ns).arg(leftStats.target).arg(leftStats.excluded).arg(leftStats.gc).arg(leftStats.gc_clamp).arg(leftStats.temp_min).arg(leftStats.temp_max)
        .arg(leftStats.compl_any).arg(leftStats.compl_end).arg(leftStats.poly_x).arg(leftStats.stability).arg(leftStats.ok);
    res += QString("<tr><th>Right</th><th> %1 </th><th> %2 </th><th> %3 </th><th> %4 </th><th> %5 </th><th> %6 </th><th> %7 </th><th> %8 </th><th> %9 </th><th> %10 </th><th> %11 </th><th> %12 </th><th> %13 </th></tr>")
        .arg(rightStats.considered).arg(rightStats.ns).arg(rightStats.target).arg(rightStats.excluded).arg(rightStats.gc).arg(rightStats.gc_clamp).arg(rightStats.temp_min).arg(rightStats.temp_max)
        .arg(rightStats.compl_any).arg(rightStats.compl_end).arg(rightStats.poly_x).arg(rightStats.stability).arg(rightStats.ok);
    res+="</table>";
    res += "<br>Pair stats:<br>";
    res += QString("considered %1, unacceptable product size %2, high end compl %3, ok %4.")
        .arg(pairStats.considered).arg(pairStats.product).arg(pairStats.compl_end).arg(pairStats.ok);
    return res;

    return res;
}

Task::ReportResult Primer3ToAnnotationsTask::report()
{
    assert( searchTask );

    QList<PrimerPair> bestPairs = searchTask->getBestPairs();

    int index = 0;
    foreach(PrimerPair pair, bestPairs)
    {
        QList<SharedAnnotationData> annotations;
        if(NULL != pair.getLeftPrimer())
        {
            annotations.append(oligoToAnnotation("primer", *pair.getLeftPrimer(), pair.getProductSize(), U2Strand::Direct));
        }
        if(NULL != pair.getInternalOligo())
        {
            annotations.append(oligoToAnnotation("internalOligo", *pair.getInternalOligo(), pair.getProductSize(), U2Strand::Direct));
        }
        if(NULL != pair.getRightPrimer())
        {
            annotations.append(oligoToAnnotation("primer", *pair.getRightPrimer(), pair.getProductSize(), U2Strand::Complementary));
        }
        AppContext::getTaskScheduler()->registerTopLevelTask(
                new CreateAnnotationsTask(aobj, groupName + "/pair " + QString::number(index + 1), annotations));
        index++;
    }
    return ReportResult_Finished;
}

SharedAnnotationData Primer3ToAnnotationsTask::oligoToAnnotation(QString title, const Primer &primer, int productSize, U2Strand strand)
{
    SharedAnnotationData annotationData(new AnnotationData());
    annotationData->name = title;
    int start = primer.getStart();
    int length = primer.getLength();
    if (strand == U2Strand::Complementary) {
        start -= length - 1;
    }
    annotationData->setStrand(strand);
    annotationData->location->regions << U2Region(start, length);
    annotationData->qualifiers.append(U2Qualifier("tm", QString::number(primer.getMeltingTemperature())));
    annotationData->qualifiers.append(U2Qualifier("gc%", QString::number(primer.getGcContent())));
    annotationData->qualifiers.append(U2Qualifier("any", QString::number(0.01*primer.getSelfAny())));
    annotationData->qualifiers.append(U2Qualifier("3'", QString::number(0.01*primer.getSelfEnd())));
    annotationData->qualifiers.append(U2Qualifier("product_size", QString::number(productSize)));

    return annotationData;
}



} // namespace
