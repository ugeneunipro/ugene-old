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

#include "uhmm3QDActor.h"
#include <gobject/uHMMObject.h>
#include <search/uHMM3SearchTask.h>
#include <search/uHMM3SearchDialogImpl.h>

#include <U2Core/FailTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/DNASequenceObject.h>

#include <U2Misc/DialogUtils.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/BaseTypes.h>


namespace U2 {

static const QString PROFILE_ATTR("hmm-profile");
static const QString NSEQ_ATTR("seqs-num");
static const QString DOM_E_ATTR("e-val");
static const QString DOM_T_ATTR("score");
static const QString NO_BIAS_ATTR("no-bias-filter");
static const QString NO_NULL_ATTR("no-score-corrections");
static const QString DO_MAX_ATTR("do-max");
static const QString F1_ATTR("msv-filter-threshold");
static const QString F2_ATTR("viterbi-filter-threshold");
static const QString F3_ATTR("forward-filter-threshold");
static const QString SEED_ATTR("random-generator-seed");
static const QString USE_EVAL("use-e-val");
static const QString MIN_LEN("min-len");
static const QString MAX_LEN("max-len");

UHMM3QDActor::UHMM3QDActor( QDActorPrototype const* proto ) : QDActor(proto) {
    units["hmm"] = new QDSchemeUnit(this);
    cfg->setAnnotationKey("hmm_signal");
    PropertyDelegate* evpd = proto->getEditor()->getDelegate(DOM_E_ATTR);
    connect(evpd, SIGNAL(si_valueChanged(int)), SLOT(sl_evChanged(int)));
}

int UHMM3QDActor::getMinResultLen() const {
    return cfg->getParameter(MIN_LEN)->getAttributeValue<int>();
}

int UHMM3QDActor::getMaxResultLen() const {
    return cfg->getParameter(MAX_LEN)->getAttributeValue<int>();
}

QString UHMM3QDActor::getText() const {
    return tr("HMM3");
}

Task* UHMM3QDActor::getAlgorithmTask( const QVector<U2Region>& location ) {
    DNASequence dnaSeq = scheme->getDNA()->getDNASequence();
    const char* seq = dnaSeq.seq.constData();

    QMap<QString, Attribute*> params = cfg->getParameters();
    QString hmmFileStr = params.value(PROFILE_ATTR)->getAttributeValue<QString>();
    QStringList hmmFiles = hmmFileStr.split(QRegExp("\\s*;\\s*"));

    Task* t = new Task(tr("QD HMM3 search"), TaskFlag_NoRun);

    UHMM3SearchTaskSettings stngs;

    float domENum = cfg->getParameter(DOM_E_ATTR)->getAttributeValue<int>();
    bool filterByEvalue = cfg->getParameter(USE_EVAL)->getAttributeValue<bool>();
    if (filterByEvalue) {
        stngs.inner.domE = pow(10, domENum);
        stngs.inner.domT = OPTION_NOT_SET;
    } else {
        stngs.inner.domT = cfg->getParameter(DOM_T_ATTR)->getAttributeValue<double>();
    }
    
    stngs.inner.domZ = cfg->getParameter(NSEQ_ATTR)->getAttributeValue<double>();
    stngs.inner.noBiasFilter = cfg->getParameter(NO_BIAS_ATTR)->getAttributeValue<bool>();
    stngs.inner.noNull2 = cfg->getParameter(NO_NULL_ATTR)->getAttributeValue<bool>();
    stngs.inner.doMax = cfg->getParameter(DO_MAX_ATTR)->getAttributeValue<bool>();
    stngs.inner.f1 = (float)cfg->getParameter(F1_ATTR)->getAttributeValue<double>();
    stngs.inner.f2 = (float)cfg->getParameter(F2_ATTR)->getAttributeValue<double>();
    stngs.inner.f3 = (float)cfg->getParameter(F3_ATTR)->getAttributeValue<double>();
    stngs.inner.seed = cfg->getParameter(SEED_ATTR)->getAttributeValue<int>();

    foreach(QString hmmFile, hmmFiles) {
        foreach(U2Region r, location) {
            DNASequence sequence;
            sequence.seq = QByteArray(seq + r.startPos, r.length);
            sequence.alphabet = dnaSeq.alphabet;

            UHMM3SWSearchTask* st = new UHMM3SWSearchTask(hmmFile, sequence, stngs);
            t->addSubTask(st);
            offsets[st] = r.startPos;
        }
    }

    connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_onTaskFinished(Task*)));

    return t;
}

void UHMM3QDActor::sl_onTaskFinished(Task*) {
    QString aname = cfg->getAnnotationKey();
    foreach(UHMM3SWSearchTask* t, offsets.keys()) {
        QList<SharedAnnotationData> annotations = t->getResultsAsAnnotations(aname);
        int offset = offsets.value(t);
        foreach(SharedAnnotationData d, annotations) {
            U2Region r = d->location->regions.first();
            if (r.length < getMinResultLen() || r.length > getMaxResultLen()) {
                continue;
            }
            r.startPos+=offset;
            QDResultUnit ru(new QDResultUnitData);
            ru->strand = d->getStrand();
            ru->quals = d->qualifiers;
            ru->region = r;
            ru->owner = units.value("hmm");
            QDResultGroup::buildGroupFromSingleResult(ru, results);
        }
    }
    offsets.clear();
}

void UHMM3QDActor::sl_evChanged(int i) {
    PropertyDelegate* pd = proto->getEditor()->getDelegate(DOM_E_ATTR);
    SpinBoxDelegate* evpd = qobject_cast<SpinBoxDelegate*>(pd);
    assert(evpd);
    if (i>0) {
        evpd->setEditorProperty("prefix", "1e+");
    } else {
        evpd->setEditorProperty("prefix", "1e");
    }
}

void UHMM3QDActor::updateEditor() {
    int val = cfg->getParameter(DOM_E_ATTR)->getAttributeValue<int>();
    sl_evChanged(val);
}

//Prototype
//////////////////////////////////////////////////////////////////////////

UHMM3QDActorPrototype::UHMM3QDActorPrototype() {
    descriptor.setId("hmm3");
    descriptor.setDisplayName(UHMM3QDActor::tr("HMM3"));
    descriptor.setDocumentation(UHMM3QDActor::tr("Finds HMM signals in supplied sequence, stores found regions as annotations."));

    {
        Descriptor pd(PROFILE_ATTR, UHMM3QDActor::tr("HMM profile"), UHMM3QDActor::tr("HMM profile(s) to search with"));
        Descriptor nsd(NSEQ_ATTR, UHMM3QDActor::tr("Number of seqs"), QApplication::translate("HMMSearchDialog", "e_value_as_nsec_tip", 0, QApplication::UnicodeUTF8));
        Descriptor ded(DOM_E_ATTR, UHMM3QDActor::tr("Filter by high E-value"), QApplication::translate("HMMSearchDialog", "results_evalue_cutoff_tip", 0, QApplication::UnicodeUTF8));
        Descriptor dtd(DOM_T_ATTR, UHMM3QDActor::tr("Filter by low score"), QApplication::translate("HMMSearchDialog", "results_score_cutoff_tip", 0, QApplication::UnicodeUTF8));
        Descriptor bd(NO_BIAS_ATTR, UHMM3QDActor::tr("No bias filter"), UHMM3QDActor::tr("Turns off composition bias filter."));
        Descriptor nd(NO_NULL_ATTR, UHMM3QDActor::tr("Turn off biased composition score corrections"), UHMM3QDActor::tr("Turns off biased composition score corrections."));
        Descriptor md(DO_MAX_ATTR, UHMM3QDActor::tr("Do max"), UHMM3QDActor::tr("Turns all heuristic filters off (less speed, more power)."));
        Descriptor f1d(F1_ATTR, UHMM3QDActor::tr("MSV filter threshold"), UHMM3QDActor::tr("MSV filter treshold."));
        Descriptor f2d(F2_ATTR, UHMM3QDActor::tr("Viterbi filter threshold"), UHMM3QDActor::tr("Viterbi filter treshold."));
        Descriptor f3d(F3_ATTR, UHMM3QDActor::tr("Forward filter threshold"), UHMM3QDActor::tr("Forward filter treshold."));
        Descriptor sd(SEED_ATTR, UHMM3QDActor::tr("Random generator seed"), UHMM3QDActor::tr("Random generator seed."));
        Descriptor useEvd(USE_EVAL, UHMM3QDActor::tr("Use E-value"), UHMM3QDActor::tr("Filters by E-value if true. Otherwise filters by score."));
        Descriptor mind(MIN_LEN, UHMM3QDActor::tr("Min len"), UHMM3QDActor::tr("Min result length."));
        Descriptor maxd(MAX_LEN, UHMM3QDActor::tr("Max len"), UHMM3QDActor::tr("Maximum result length."));

        attributes << new Attribute(pd, BaseTypes::STRING_TYPE(), true);
        attributes << new Attribute(nsd, BaseTypes::NUM_TYPE(), false, QVariant(1));
        attributes << new Attribute(useEvd, BaseTypes::BOOL_TYPE(), false, true);
        attributes << new Attribute(ded, BaseTypes::NUM_TYPE(), false, QVariant(1));
        attributes << new Attribute(dtd, BaseTypes::NUM_TYPE(), false, QVariant(0.01));
        attributes << new Attribute(bd, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attributes << new Attribute(nd, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attributes << new Attribute(md, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attributes << new Attribute(f1d, BaseTypes::NUM_TYPE(), false, QVariant(0.02));
        attributes << new Attribute(f2d, BaseTypes::NUM_TYPE(), false, QVariant(0.001));
        attributes << new Attribute(f3d, BaseTypes::NUM_TYPE(), false, QVariant(0.00001));
        attributes << new Attribute(sd, BaseTypes::NUM_TYPE(), false, QVariant(42));
        attributes << new Attribute(mind, BaseTypes::NUM_TYPE(), false, QVariant(30));
        attributes << new Attribute(maxd, BaseTypes::NUM_TYPE(), false, QVariant(5000));
    }

    QMap<QString, PropertyDelegate*> delegates;

    {
        delegates[PROFILE_ATTR] = new URLDelegate(
            DialogUtils::prepareDocumentsFileFilterByObjType(UHMMObject::UHMM_OT, true),
            UHMM3SearchDialogImpl::HMM_FILES_DIR_ID, true/*multi*/);
        
        QVariantMap eMap; eMap["prefix"]= ("1e+"); eMap["minimum"] = (-99); eMap["maximum"] = (1);
        delegates[DOM_E_ATTR] = new SpinBoxDelegate(eMap);

        QVariantMap nMap; nMap["maximum"] = 999999999.00; nMap["minimum"] = 0.00;
        delegates[NSEQ_ATTR] = new DoubleSpinBoxDelegate(nMap);

        QVariantMap tMap; tMap["decimals"]= (2); tMap["minimum"] = 0.01; tMap["maximum"] = 99.99;
        tMap["singleStep"] = (1.0);
        delegates[DOM_T_ATTR] = new DoubleSpinBoxDelegate(tMap);

        QVariantMap sMap; sMap["minimum"] = 0; sMap["maximum"] = INT_MAX;
        delegates[SEED_ATTR] = new SpinBoxDelegate(sMap);

        QVariantMap lenMap; lenMap["minimum"] = 2; lenMap["maximum"] = INT_MAX;
        delegates[MIN_LEN] = new SpinBoxDelegate(lenMap);
        delegates[MAX_LEN] = new SpinBoxDelegate(lenMap);
    }

    editor = new DelegateEditor(delegates);
}

} //namespace
