#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/Version.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/MultiTask.h>
#include <U2Core/FormatUtils.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Gui/OpenViewTask.h>

#include "FindEnzymesTask.h"
#include "CloningUtilTasks.h"

namespace U2 {

DigestSequenceTask::DigestSequenceTask(  const DNASequenceObject* so, AnnotationTableObject* source, 
                                       AnnotationTableObject* dest, const QList<SEnzymeData>& cutSites )
                                       :   Task("DigestSequenceTask", TaskFlags_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled), enzymeData(cutSites),
                                       searchForRestrictionSites(false), sourceObj(source), destObj(dest), dnaObj(so)
{
    GCOUNTER(cvar,tvar,"DigestSequenceIntoFragments");

    assert(sourceObj != NULL);
    assert(destObj != NULL);
    assert(dnaObj != NULL);


}

DigestSequenceTask::DigestSequenceTask( const DNASequenceObject* so, AnnotationTableObject* aobj, 
                                       const QList<SEnzymeData>& cutSites )
:   Task("DigestSequenceTask", TaskFlags_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled), enzymeData(cutSites),
    searchForRestrictionSites(true), sourceObj(aobj), destObj(aobj), dnaObj(so)
{
    GCOUNTER(cvar,tvar,"DigestSequenceIntoFragments");
    
    assert(sourceObj != NULL);
    assert(destObj != NULL);
    assert(dnaObj != NULL);
}

void DigestSequenceTask::prepare() {
    seqRange = dnaObj->getSequenceRange();
    isCircular = dnaObj->isCircular();
    
    if (searchForRestrictionSites) {
        assert(sourceObj == destObj);
        FindEnzymesTaskConfig cfg;
        cfg.circular = isCircular;
        cfg.groupName = ANNOTATION_GROUP_ENZYME;
        Task* t = new FindEnzymesToAnnotationsTask(sourceObj, dnaObj->getDNASequence(), enzymeData, cfg);
        addSubTask(t);
    }  
        
}

AnnotationData* DigestSequenceTask::createFragment( int pos1, const QString& enzymeId1, const QByteArray& leftOverhang, 
                                                   int pos2, const QString& enzymeId2, const QByteArray& rightOverhang )
{
    AnnotationData* ad = new AnnotationData();
    ad->location->regions.append(U2Region(pos1, pos2 - pos1 ));
    ad->qualifiers.append(U2Qualifier(QUALIFIER_LEFT_TERM, enzymeId1));
    ad->qualifiers.append(U2Qualifier(QUALIFIER_RIGHT_TERM, enzymeId2));
    ad->qualifiers.append(U2Qualifier(QUALIFIER_LEFT_OVERHANG, leftOverhang));
    ad->qualifiers.append(U2Qualifier(QUALIFIER_RIGHT_OVERHANG, rightOverhang));
    
    QString leftOverhangType = enzymeId1.isEmpty() ? OVERHANG_TYPE_BLUNT : OVERHANG_TYPE_STICKY;
    ad->qualifiers.append(U2Qualifier(QUALIFIER_LEFT_TYPE, leftOverhangType) );
    QString rightOverhangType = enzymeId2.isEmpty() ? OVERHANG_TYPE_BLUNT : OVERHANG_TYPE_STICKY;
    ad->qualifiers.append(U2Qualifier(QUALIFIER_RIGHT_TYPE, rightOverhangType) );

    ad->qualifiers.append(U2Qualifier(QUALIFIER_SOURCE, dnaObj->getGObjectName()));
    
    return ad;
}


Task::ReportResult DigestSequenceTask::report()
{
    if (hasErrors() || isCanceled()) {
        return ReportResult_Finished;
    }

    saveResults();
    
    return ReportResult_Finished;
}

void DigestSequenceTask::findCutSites()
{
    foreach (const SEnzymeData& enzyme, enzymeData) {
        
        if (enzyme->cutDirect == ENZYME_CUT_UNKNOWN || enzyme->cutComplement == ENZYME_CUT_UNKNOWN) {
            setError(tr("Can't use restriction site %1 for digestion,  cleavage site is unknown ").arg(enzyme->id));
            return;
        }

        QList<Annotation*> anns;
        foreach (Annotation* a, sourceObj->getAnnotations()) {
            if (a->getAnnotationName() == enzyme->id) {
                if (GObjectUtils::annotationHasNegativeSplit(a) ) {
                    continue;
                }
                anns.append(a);
            }
        }
                
        if (anns.isEmpty()) {
            stateInfo.setError(  QString("Restriction site %1 is not found").arg(enzyme->id) );
            continue;
        }
        
        foreach (Annotation * a, anns) {
            const QVector<U2Region>& location = a->getRegions();
            assert(location.size() == 1);
            int cutPos = location.first().startPos;
            cutSiteMap.insertMulti(cutPos, enzyme);
        }

    }
}

void DigestSequenceTask::run()
{
    if (hasErrors() || isCanceled()) {
        return;
    }

    findCutSites();

    if (cutSiteMap.isEmpty()) {
        return;
    }

    QMap<int,SEnzymeData>::const_iterator prev = cutSiteMap.constBegin(), current = cutSiteMap.constBegin();
    int count = 2;

    while ( (++current) != cutSiteMap.constEnd() )  {
        int pos1 = prev.key();
        int pos2 = current.key();
        const SEnzymeData& enzyme1 = prev.value();
        const SEnzymeData& enzyme2 = current.value();
        int len1= enzyme1->seq.length();
        int len2 = enzyme2->seq.length();

        {

            U2Region region1(pos1, len1);
            U2Region region2(pos2, len2);

            if (region1.intersects(region2)) {
                setError(tr("Unable to digest into fragments: intersecting enzymes %1 (%2..%3) and %4 (%5..%6)")
                    .arg(enzyme1->id).arg(region1.startPos).arg(region1.endPos())
                    .arg(enzyme2->id).arg(region2.startPos).arg(region2.endPos()));
                return;
            }

        }

        int diff1 = enzyme1->seq.length() - enzyme1->cutDirect;
        int offset1 = enzyme1->cutComplement > diff1 ? diff1 : enzyme1->cutComplement;
        pos1 = pos1 + len1 - offset1;
        int cutLen1 = qAbs(diff1 - enzyme1->cutComplement);
        QByteArray leftOverhang = dnaObj->getSequence().mid(pos1 - cutLen1, cutLen1);

        int diff2 = enzyme2->seq.length() - enzyme2->cutComplement;
        int offset2 = enzyme1->cutDirect > diff2 ? diff2 : enzyme2->cutDirect;
        pos2 += offset2;
        QByteArray rightOverhang = dnaObj->getSequence().mid(pos2, qAbs(diff2 - enzyme2->cutDirect));


        AnnotationData* ad = createFragment(pos1, enzyme1->id, leftOverhang,  pos2, enzyme2->id, rightOverhang);

        ad->name = QString("Fragment %1").arg(count);
        results.append(SharedAnnotationData(ad));
        ++count;
        ++prev;

    } 

    QMap<int,SEnzymeData>::const_iterator first = cutSiteMap.constBegin();

    const QByteArray& sourceSeq = dnaObj->getSequence();
    const SEnzymeData& firstCutter = first.value();
    const SEnzymeData& lastCutter = prev.value();
    int firstCutPos = first.key();
    int fdiff = firstCutter->seq.length() - firstCutter->cutComplement;
    firstCutPos += firstCutter->cutDirect > fdiff ? fdiff : firstCutter->cutDirect;
    QByteArray firstRightOverhang = dnaObj->getSequence().mid(firstCutPos, qAbs(fdiff - firstCutter->cutDirect));

    int lastCutPos = prev.key();
    int ldiff = lastCutter->seq.length() - lastCutter->cutDirect;
    int loffset = lastCutter->cutComplement > ldiff ? ldiff : lastCutter->cutComplement;

    lastCutPos = lastCutPos + lastCutter->seq.length() - loffset;

    if (lastCutPos >= sourceSeq.length()) {
        // last restriction site is situated between sequence start and end
        assert(isCircular);
        int leftCutPos = lastCutPos - sourceSeq.length();
        QByteArray leftOverhang = sourceSeq.mid(prev.key() + lastCutter->cutDirect) + sourceSeq.mid(0, leftCutPos);
        QByteArray rightOverhang = first == prev ? leftOverhang : firstRightOverhang;
        AnnotationData* ad1 = createFragment(leftCutPos, lastCutter->id, leftOverhang, firstCutPos, firstCutter->id, rightOverhang );
        ad1->name = QString("Fragment 1");
        results.append(SharedAnnotationData(ad1));
    } else {
        int lCutLen = qAbs(ldiff - lastCutter->cutComplement);
        QByteArray lastLeftOverhang = dnaObj->getSequence().mid(lastCutPos - lCutLen, lCutLen);
        AnnotationData* ad1 = createFragment(seqRange.startPos, QString(), QByteArray(), firstCutPos, firstCutter->id, firstRightOverhang );
        AnnotationData* ad2 = createFragment(lastCutPos, lastCutter->id, lastLeftOverhang, seqRange.endPos(), QString(), QByteArray());

        if (isCircular) {
            int split_size = (seqRange.length - lastCutPos) + firstCutPos; 

            // For splitted fragments only the one with positive split is valid
            ad1->qualifiers.append( U2Qualifier(QUALIFIER_SPLIT, QString("%1").arg(-split_size)) );
            ad1->name = QString("Fragment 1");

            ad2->qualifiers.append( U2Qualifier(QUALIFIER_SPLIT, QString("%1").arg(split_size)) );
            ad2->qualifiers.append( U2Qualifier(QUALIFIER_SPLIT_TERM, firstCutter->id) );
            ad2->qualifiers.append( U2Qualifier(QUALIFIER_SPLIT_OVERHANG, firstRightOverhang) );
            ad2->qualifiers.append( U2Qualifier(QUALIFIER_SPLIT_TYPE, OVERHANG_TYPE_STICKY) );
            ad2->name = ad1->name;
        } else {
            ad1->name = QString("Fragment 1");
            ad2->name = QString("Fragment %1").arg(count);
        }

        results.append(SharedAnnotationData(ad1));
        results.append(SharedAnnotationData(ad2));
    }

}


void DigestSequenceTask::saveResults()
{
    foreach (const SharedAnnotationData& data, results) {
        destObj->addAnnotation(new Annotation(data), ANNOTATION_GROUP_FRAGMENTS);
    }
}

QString DigestSequenceTask::generateReport() const
{
    QString res;
    QString topology = dnaObj->isCircular() ? tr("circular") : tr("linear");
    res+= tr("<h3><br>Digest into fragments %1 (%2)</h3>").arg(dnaObj->getDocument()->getName()).arg(topology);
    res+=tr("<br>Generated %1 fragments.").arg(results.count());
    int counter = 1;
    foreach (const SharedAnnotationData& sdata, results) {
        int startPos = sdata->location->regions.first().startPos;
        int endPos = sdata->location->regions.first().endPos();
        res+=tr("<br><br>&nbsp;&nbsp;&nbsp;&nbsp;%1:&nbsp;&nbsp;&nbsp;&nbsp;From %3 (%2) To %5 (%4) - %6 bp ").arg(counter)
                  .arg(startPos).arg(sdata->findFirstQualifierValue(QUALIFIER_LEFT_TERM))
                  .arg(endPos).arg(sdata->findFirstQualifierValue(QUALIFIER_RIGHT_TERM))
                   .arg(endPos - startPos + 1);   
        ++counter;
    }

    return res;

}


//////////////////////////////////////////////////////////////////////////

LigateFragmentsTask::LigateFragmentsTask( const QList<DNAFragment>& fragments, const LigateFragmentsTaskConfig& config )
: Task("LigateFragmentsTask", TaskFlags_NR_FOSCOE), fragmentList(fragments), cfg(config), 
  resultDoc(NULL), resultAlphabet(NULL)
{
    GCOUNTER(cvar,tvar,"LigateFragments");
}

static bool overhangsAreConsistent(const QByteArray& start, const QByteArray& end, DNAAlphabet* alphabet ) {
    DNATranslation* trans = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(alphabet);
    
    QByteArray compEnd(end);
    trans->translate(compEnd.data(), end.length());
    
    if (compEnd == start) {
       return true;
    }

    return false;
}


void LigateFragmentsTask::processOverhangs( const DNAFragment& prevFragment, const DNAFragment& curFragment, QByteArray& overhangAddition )
{
    QByteArray prevTermType = prevFragment.getRightTerminus().termType;
    QByteArray curTermType = curFragment.getLeftTerminus().termType;

    if (prevTermType != curTermType) {
        stateInfo.setError( tr("Fragments %1 and  %2 are inconsistent. Blunt and sticky ends incompatibility")
            .arg(prevFragment.getName()).arg(curFragment.getName()) );
        return;
    }
    
    QByteArray prevOverhang = prevFragment.getRightTerminus().overhang;
    QByteArray curOverhang = curFragment.getLeftTerminus().overhang;

    if (prevTermType == OVERHANG_TYPE_STICKY) {
        if (prevOverhang != curOverhang) {
            stateInfo.setError( tr("Right overhang from %1 and left overhang from %2 are inconsistent.")
                .arg(prevFragment.getName()).arg(curFragment.getName()) );
            return;
        } else {
            overhangAddition += curOverhang;
        }
    } else if (prevTermType == OVERHANG_TYPE_BLUNT) {
        overhangAddition += prevOverhang + curOverhang;
    } else {
        assert(0);
    }

}



void LigateFragmentsTask::prepare()
{
    QByteArray resultSeq;
    QVector<U2Region> fragmentRegions;

    DNAFragment prevFragment;
    assert(prevFragment.isEmpty());

    foreach (const DNAFragment& dnaFragment, fragmentList) {

        QVector<U2Region> location = dnaFragment.getFragmentRegions();
        assert(location.size() > 0);
        
        // check alphabet consistency
        DNAAlphabet* fragmentAlphabet = dnaFragment.getAlphabet();
        if (resultAlphabet == NULL ) {
            resultAlphabet = fragmentAlphabet;
        } else if (resultAlphabet != fragmentAlphabet) {
            if (fragmentAlphabet == NULL) {
                stateInfo.setError( tr("Unknown DNA alphabet in fragment %1 of %2")
                    .arg(dnaFragment.getName()).arg(dnaFragment.getSequenceName()) );
                return;
            }
            resultAlphabet = DNAAlphabet::deriveCommonAlphabet(resultAlphabet,fragmentAlphabet);
        }
        
        // check if overhangs are compatible
        QByteArray overhangAddition;
        if (cfg.checkOverhangs ) {
             if (!prevFragment.isEmpty()) {
                processOverhangs(prevFragment, dnaFragment, overhangAddition );
                if (stateInfo.hasErrors()) {
                    return;
                }
             }
             prevFragment = dnaFragment;
        }
        
        // handle fragment annotations
        int resultLen = resultSeq.length() + overhangAddition.length();
        foreach (AnnotationTableObject* aObj, dnaFragment.getRelatedAnnotations()) {
            QList<Annotation*> toSave = cloneAnnotationsInFragmentRegion(dnaFragment, aObj, resultLen);
            annotations.append(toSave);
        }
        
        if (cfg.annotateFragments) {
            Annotation* a = createFragmentAnnotation(dnaFragment, resultLen);
            annotations.append(a);
        }

        resultSeq.append(overhangAddition);
        resultSeq.append(dnaFragment.getSequence());
    }

    if (cfg.makeCircular && cfg.checkOverhangs) {
        const DNAFragment& first = fragmentList.first();
        const DNAFragment& last = fragmentList.last();
        QByteArray overhangAddition;
        processOverhangs(last, first, overhangAddition);
        if (stateInfo.hasErrors()) {
            return;
        }
        resultSeq.append(overhangAddition);
    }

    // create comment
    Annotation* sourceAnnot = createSourceAnnotation(resultSeq.length());
    annotations.append(sourceAnnot);

    createDocument(resultSeq,annotations);
    
    if (!cfg.addDocToProject) {
        return;
    }
    
    QList<Task*> tasks;
    tasks.append(new AddDocumentTask(resultDoc));

    if (cfg.openView) {
        tasks.append(new OpenViewTask(resultDoc));
    }
    if (cfg.saveDoc) {
        tasks.append(new SaveDocumentTask(resultDoc));
    }

    Task* multiTask = new MultiTask(tr("Add constructed molecule"), tasks );
    addSubTask(multiTask);
    

}


Annotation* LigateFragmentsTask::createSourceAnnotation( int regLen )
{
    Version v = Version::ugeneVersion();
    SharedAnnotationData sd( new AnnotationData);
    sd->name = "source";
    sd->location->regions << U2Region(0, regLen);
    sd->qualifiers.append( U2Qualifier("comment", QString("Molecule is created with Unipro UGENE v%1.%2").arg(v.major).arg(v.minor)) );
    return  new Annotation(sd);

}

Annotation* LigateFragmentsTask::createFragmentAnnotation( const DNAFragment& fragment, int startPos )
{
    SharedAnnotationData sd( new AnnotationData);
    sd->name = QString("%1 %2").arg(fragment.getSequenceName()).arg(fragment.getName());
    sd->location->regions << U2Region(startPos, fragment.getLength());
    sd->qualifiers.append(U2Qualifier("source_doc", fragment.getSequenceDocName()));

    return  new Annotation(sd);
}

QList<Annotation*> LigateFragmentsTask::cloneAnnotationsInRegion( const U2Region& fragmentRegion, AnnotationTableObject* source, int globalOffset )
{
    QList<Annotation*> results;
    
    // TODO: allow to cut annotations
    
    foreach(Annotation* a, source->getAnnotations()) {
        bool ok = true;
        const QVector<U2Region>& location = a->getRegions();
        foreach(const U2Region& region, location) {
            if (!fragmentRegion.contains(region) || fragmentRegion == region) {
                ok = false;
                break;
            }
        }
        if (ok) {
            int newPos = globalOffset + location.first().startPos - fragmentRegion.startPos;
            Annotation* cloned = new Annotation(a->data());
            QVector<U2Region> newLocation;
            foreach (const U2Region& region, a->getRegions()) {
                U2Region newRegion(region);
                newRegion.startPos = newPos;
                newLocation.append(newRegion);
            }
            cloned->replaceRegions(newLocation);
            results.append(cloned);
        }

    }

    return results;

}

static bool fragmentContainsRegion(const DNAFragment& fragment, const U2Region region) {
    
    QVector<U2Region> fragmentRegions = fragment.getFragmentRegions();

    bool result = false;
    foreach (const U2Region& fR, fragmentRegions) {
        if (fR.contains(region)) {
            result = true;
            break;
        }

    }
    
    return result;

} 


static int getRelativeStartPos(const DNAFragment& fragment, const U2Region region)
{
    
    QVector<U2Region> fragmentRegions = fragment.getFragmentRegions();

    int offset = 0;
    foreach (const U2Region& fR, fragmentRegions) {
        if (fR.contains(region)) {
            return offset + region.startPos - fR.startPos;
        }
        offset += fR.length;
    }
    
    // the fragment doesn't contain the region
    return -1;
}



QList<Annotation*> LigateFragmentsTask::cloneAnnotationsInFragmentRegion( const DNAFragment& fragment, AnnotationTableObject* source, int globalOffset )
{
    QList<Annotation*> results;

    // TODO: allow to remove annotations

    foreach(Annotation* a, source->getAnnotations()) {
        QVector<U2Region> location = a->getRegions();
        
        if (a->getAnnotationName().startsWith("Fragment")) {
            continue;
        }

        bool ok = true;
        foreach (const U2Region& r, location) {
            // sneaky annotations shall not pass!
            if (!fragmentContainsRegion(fragment, r)) {
                ok = false;
                break;
            }
        }

        if (ok) {
            Annotation* cloned = new Annotation(a->data());
            QVector<U2Region> newLocation;
            foreach (const U2Region& region, location) {
                int startPos = getRelativeStartPos(fragment, region);
                if (fragment.isInverted()) {
                    startPos = fragment.getLength() - startPos - region.length;
                    U2Strand strand = cloned->getStrand();
                    if (strand.isDirect()) {
                        cloned->setStrand(U2Strand::Complementary);
                    } else {
                        cloned->setStrand(U2Strand::Direct);
                    } 
                }
                assert(startPos != -1);
                int newPos = globalOffset + startPos;
                U2Region newRegion(region);
                newRegion.startPos = newPos;
                newLocation.append(newRegion);
            }
            
            cloned->replaceRegions(newLocation);
            results.append(cloned);
        }

    }

    return results;
}


void LigateFragmentsTask::createDocument( const QByteArray& seq, const QList<Annotation*> annotations )
{
    
    DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK);
    IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    QList<GObject*> objects;

    QString seqName = cfg.seqName.isEmpty() ? cfg.docUrl.baseFileName() : cfg.seqName;
    DNASequence dna(seqName, seq, resultAlphabet);
    dna.circular = cfg.makeCircular;
    
    // set Genbank header
    DNALocusInfo loi;
    loi.name = seqName;
    loi.topology = cfg.makeCircular ? "circular" : "linear";
    loi.molecule = "DNA";
    loi.division = "SYN";
    QDate date = QDate::currentDate();
    loi.date = QString("%1-%2-%3").arg(date.toString("dd"))
        .arg( FormatUtils::getShortMonthName(date.month()) )
        .arg( date.toString("yyyy"));
                    
    dna.info.insert(DNAInfo::LOCUS, qVariantFromValue<DNALocusInfo>(loi));

    DNASequenceObject* dnaObj = new DNASequenceObject(seqName, dna);
    objects.append(dnaObj);
   
    AnnotationTableObject* aObj = new AnnotationTableObject(QString("%1 annotations").arg(seqName));
    aObj->addAnnotations(annotations);
    objects.append(aObj);
    
    resultDoc = new Document(df, iof, cfg.docUrl, objects);
    resultDoc->setModified(true);
    aObj->addObjectRelation(dnaObj,GObjectRelationRole::SEQUENCE);
    

    
}

void LigateFragmentsTask::cleanup()
{
    if (stateInfo.hasErrors()) {
        qDeleteAll(annotations);    
    }
}





} // U2

