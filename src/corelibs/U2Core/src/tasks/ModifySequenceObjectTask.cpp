/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <U2Core/AddDocumentTask.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/Counter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GenbankFeatures.h>
#include <U2Core/GObject.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/MultiTask.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/U1AnnotationUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>

#include "ModifySequenceObjectTask.h"

namespace U2 {

ModifySequenceContentTask::ModifySequenceContentTask(const DocumentFormatId &dfId, U2SequenceObject *seqObj, const U2Region &regionTodelete,
    const DNASequence &seq2Insert, bool recalculateQualifiers, U1AnnotationUtils::AnnotationStrategyForResize str, const GUrl &url, bool mergeAnnotations)
    : Task(tr("Modify sequence task"), TaskFlags(TaskFlag_NoRun) | TaskFlag_ReportingIsSupported), resultFormatId(dfId),
    mergeAnnotations(mergeAnnotations), recalculateQualifiers(recalculateQualifiers), curDoc(seqObj->getDocument()), newDoc(NULL), url(url), strat(str),
    seqObj(seqObj), regionToReplace(regionTodelete), sequence2Insert(seq2Insert)
{
    GCOUNTER(cvar, tvar, "Modify sequence task");
    inplaceMod = url == curDoc->getURL() || url.isEmpty();
}

Task::ReportResult ModifySequenceContentTask::report() {
    CHECK(!(regionToReplace.isEmpty() && sequence2Insert.seq.isEmpty()), ReportResult_Finished);
    CHECK_EXT(!curDoc->isStateLocked(), setError(tr("Document is locked")), ReportResult_Finished);

    U2Region seqRegion(0, seqObj->getSequenceLength());
    if (!seqRegion.contains(regionToReplace)) {
        algoLog.error(tr("Region to delete is larger than the whole sequence"));
        return ReportResult_Finished;
    }

    Project *p = AppContext::getProject();
    if (p != NULL) {
        if (p->isStateLocked()) {
            return ReportResult_CallMeAgain;
        }
        docs = p->getDocuments();
    }

    if (!docs.contains(curDoc)) {
        docs.append(curDoc);
    }

    if (!inplaceMod) {
        cloneSequenceAndAnnotations();
    }
    seqObj->replaceRegion(regionToReplace, sequence2Insert, stateInfo);
    CHECK_OP(stateInfo, ReportResult_Finished);

    fixAnnotations();

    if (!inplaceMod) {
        QList<Task*> tasks;
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
        tasks.append(new SaveDocumentTask(seqObj->getDocument(), iof, url.getURLString()));
        Project *p = AppContext::getProject();
        if (p != NULL) {
            tasks.append(new AddDocumentTask(newDoc));
        }
        AppContext::getTaskScheduler()->registerTopLevelTask(new MultiTask("Save document and add it to project (optional)", tasks));
    }
    return ReportResult_Finished;
}

typedef QPair<QString, QString> QStringPair;

namespace {

QString formatPairList(const QList<QStringPair> &pairList, bool useFirst) {
    QString result;
    const QString lineSeparator = "<br>";
    foreach (const QStringPair &pair, pairList) {
        result += useFirst ? pair.first : pair.second;
        result += lineSeparator;
    }
    result.chop(lineSeparator.length());
    return result;
}

}

QString ModifySequenceContentTask::generateReport() const {
    CHECK(!annotationForReport.isEmpty(), QString());

    QString report = tr("Some annotations have qualifiers referring a sequence region that has been removed during the sequence editing. "
        "You might want to change the qualifiers manually. Find them in the table below");
    report += "<br><table border=\"1\" cellpadding=\"1\">";
    report += "<tr><th>";
    report += tr("Annotation Name");
    report += "</th><th>";
    report += tr("Annotation Location");
    report += "</th><th>";
    report += tr("Qualifier Name");
    report += "</th><th>";
    report += tr("Referenced Region");
    report += "</th></tr>";

    foreach (Annotation *an, annotationForReport.keys()) {
        if (annotationForReport[an].isEmpty()) {
            coreLog.error(tr("Unexpected qualifiers count"));
            assert(false);
            continue;
        }

        report += QString("<tr><td>%1</td><td>%2</td>").arg(an->getName()).arg(U1AnnotationUtils::buildLocationString(*an->getLocation()));

        report += QString("<td>%1</td>").arg(formatPairList(annotationForReport[an], true));
        report += QString("<td>%1</td>").arg(formatPairList(annotationForReport[an], false));

        report += "</tr>";
    }
    report += "</table>";
    return report;
}

U2Qualifier ModifySequenceContentTask::getFixedTranslationQualifier(const SharedAnnotationData &ad) {
    QVector<U2Qualifier> translationQuals;
    ad->findQualifiers(GBFeatureUtils::QUALIFIER_TRANSLATION, translationQuals);
    CHECK(!translationQuals.empty(), U2Qualifier());

    DNATranslation *aminoTranslation = GObjectUtils::findAminoTT(seqObj, false);
    SAFE_POINT(NULL != aminoTranslation, L10N::nullPointerError("Amino translation"), U2Qualifier());

    QString completeTranslation;
    foreach (const U2Region &r, ad->getRegions()) {
        const QByteArray annotatedData = seqObj->getSequenceData(r, stateInfo);
        CHECK_OP(stateInfo, U2Qualifier());

        const DNAAlphabet *dstAlphabet = aminoTranslation->getDstAlphabet();
        QByteArray transContent(annotatedData.size() / 3, dstAlphabet->getDefaultSymbol());

        aminoTranslation->translate(annotatedData.constData(), annotatedData.length(), transContent.data(), transContent.length());
        completeTranslation.append(transContent);
    }

    return (completeTranslation != translationQuals.first().value) ? U2Qualifier(GBFeatureUtils::QUALIFIER_TRANSLATION, completeTranslation)
                                                                   : U2Qualifier();
}

void ModifySequenceContentTask::fixTranslationQualifier(SharedAnnotationData &ad) {
    CHECK(recalculateQualifiers, );

    const U2Qualifier translQual = getFixedTranslationQualifier(ad);
    CHECK(translQual.isValid(), );

    const QString existingTranslation = ad->findFirstQualifierValue(GBFeatureUtils::QUALIFIER_TRANSLATION);
    const U2Qualifier existingTranslQual(GBFeatureUtils::QUALIFIER_TRANSLATION, existingTranslation);
    for (int i = 0, n = ad->qualifiers.size(); i < n; ++i) {
        if (ad->qualifiers[i] == existingTranslQual) {
            ad->qualifiers.remove(i);
            break;
        }
    }
    ad->qualifiers.append(translQual);
}

void ModifySequenceContentTask::fixTranslationQualifier(Annotation *an) {
    CHECK(recalculateQualifiers, );

    const U2Qualifier newTranslQual = getFixedTranslationQualifier(an->getData());
    CHECK(newTranslQual.isValid(), );

    QList<U2Qualifier> translationQuals;
    an->findQualifiers(GBFeatureUtils::QUALIFIER_TRANSLATION, translationQuals);
    an->removeQualifier(translationQuals.first());
    an->addQualifier(newTranslQual);
}

QMap<QString, QList<SharedAnnotationData> > ModifySequenceContentTask::fixAnnotation(Annotation *an, bool &annIsRemoved) {
    QMap<QString, QList<SharedAnnotationData> > result;
    SAFE_POINT(NULL != an, L10N::nullPointerError("Annotation"), result);
    AnnotationTableObject *ato = an->getGObject();
    SAFE_POINT(NULL != ato, L10N::nullPointerError("Annotation table object"), result);

    QList<QVector<U2Region> > newRegions = U1AnnotationUtils::fixLocationsForReplacedRegion(regionToReplace,
        sequence2Insert.seq.length(), an->getRegions(), strat);

    if (newRegions[0].isEmpty()) {
        annIsRemoved = true;
    } else {
        fixAnnotationQualifiers(an);

        an->updateRegions(newRegions[0]);
        fixTranslationQualifier(an);
        for (int i = 1; i < newRegions.size(); i++) {
            SharedAnnotationData splittedAnnotation(new AnnotationData(*an->getData()));
            const QString groupName = an->getGroup()->getGroupPath();
            splittedAnnotation->location->regions = newRegions[i];
            fixTranslationQualifier(splittedAnnotation);
            result[groupName].append(splittedAnnotation);
        }
    }
    return result;
}

bool ModifySequenceContentTask::isRegionValid(const U2Region &region) const {
    return region.length > 0 && region.startPos < seqObj->getSequenceLength() - 1;
}

void ModifySequenceContentTask::fixAnnotationQualifiers(Annotation *an) {
    CHECK(recalculateQualifiers, );

    QRegExp locationMatcher("(\\d+)\\.\\.(\\d+)");
    foreach (const U2Qualifier &qual, an->getQualifiers()) {
        QString newQualifierValue = qual.value;

        int lastModifiedPos = 0;
        int lastFoundPos = 0;
        while ((lastFoundPos = locationMatcher.indexIn(qual.value, lastFoundPos)) != -1) {
            const QString matchedRegion = locationMatcher.cap();
            const qint64 start = locationMatcher.cap(1).toLongLong() - 1; // position starts with 0
            const qint64 end = locationMatcher.cap(2).toLongLong() - 1;

            U2Region referencedRegion(start, end - start + 1);
            if (isRegionValid(referencedRegion)) {
                QList<QVector<U2Region> > newRegions = U1AnnotationUtils::fixLocationsForReplacedRegion(regionToReplace,
                    sequence2Insert.seq.length(), QVector<U2Region>() << referencedRegion, U1AnnotationUtils::AnnotationStrategyForResize_Resize);

                if (!newRegions.isEmpty() && !newRegions[0].empty()) {
                    QString newRegionsStr;
                    foreach (const U2Region &region, newRegions[0]) {
                        newRegionsStr += QString("%1..%2,").arg(region.startPos + 1).arg(region.endPos()); // position starts with 1
                    }
                    newRegionsStr.chop(1); // remove last comma

                    const int oldRegionPos = newQualifierValue.indexOf(matchedRegion, lastModifiedPos);
                    SAFE_POINT(oldRegionPos != -1, "Unexpected region matched", );

                    newQualifierValue.replace(oldRegionPos, matchedRegion.length(), newRegionsStr);
                    lastModifiedPos = oldRegionPos + newRegionsStr.length();
                } else {
                    annotationForReport[an].append(QStringPair(qual.name, matchedRegion));
                    if (!isReportingEnabled()) {
                        setReportingEnabled(true);
                    }
                }
            }

            lastFoundPos += locationMatcher.matchedLength();
        }

        if (newQualifierValue != qual.value) {
            an->removeQualifier(qual);
            an->addQualifier(U2Qualifier(qual.name, newQualifierValue));
        }
    }
}

void ModifySequenceContentTask::fixAnnotations() {
    QList<GObject *> annotationTablesList;
    if (AppContext::getProject() != NULL) {
        annotationTablesList = GObjectUtils::findObjectsRelatedToObjectByRole(seqObj, GObjectTypes::ANNOTATION_TABLE,
            ObjectRole_Sequence, GObjectUtils::findAllObjects(UOF_LoadedOnly, GObjectTypes::ANNOTATION_TABLE), UOF_LoadedOnly);
    } else {
        foreach(Document *d, docs) {
            QList<GObject *> allAnnotationTables = d->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
            foreach(GObject *table, allAnnotationTables) {
                if (table->hasObjectRelation(seqObj, ObjectRole_Sequence)) {
                    annotationTablesList.append(table);
                }
            }
        }
    }

    foreach (GObject *table, annotationTablesList) {
        AnnotationTableObject *ato = qobject_cast<AnnotationTableObject *>(table);
        if (NULL != ato) {
            QMap<QString, QList<SharedAnnotationData> > group2AnnotationsToAdd;
            QList<Annotation *> annotationToRemove;
            foreach(Annotation *an, ato->getAnnotations()) {
                bool annIsToBeRemoved = false;
                QMap<QString, QList<SharedAnnotationData> > newAnnotations = fixAnnotation(an, annIsToBeRemoved);
                foreach (const QString &groupName, newAnnotations.keys()) {
                    group2AnnotationsToAdd[groupName].append(newAnnotations[groupName]);
                }
                if (annIsToBeRemoved) {
                    annotationToRemove.append(an);
                }
            }
            foreach (const QString &groupName, group2AnnotationsToAdd.keys()) {
                ato->addAnnotations(group2AnnotationsToAdd[groupName], groupName);
            }
            ato->removeAnnotations(annotationToRemove);
        } else {
            assert(false);
            coreLog.error(L10N::nullPointerError("Annotation table object"));
        }
    }
}

void ModifySequenceContentTask::cloneSequenceAndAnnotations() {
    IOAdapterRegistry *ioReg = AppContext::getIOAdapterRegistry();
    IOAdapterFactory* iof = ioReg->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
    CHECK(NULL != iof, );
    DocumentFormatRegistry *dfReg = AppContext::getDocumentFormatRegistry();
    DocumentFormat *df = dfReg->getFormatById(resultFormatId);
    SAFE_POINT(NULL != df, "Invalid document format!", );

    U2SequenceObject *oldSeqObj = seqObj;
    U2OpStatus2Log os;
    newDoc = df->createNewLoadedDocument(iof, url, os, curDoc->getGHintsMap());
    SAFE_POINT_EXT(df->isObjectOpSupported(newDoc, DocumentFormat::DocObjectOp_Add, GObjectTypes::SEQUENCE),
        stateInfo.setError("Failed to add sequence object to document!"), );
    U2Sequence clonedSeq = U2SequenceUtils::copySequence(oldSeqObj->getSequenceRef(), newDoc->getDbiRef(), U2ObjectDbi::ROOT_FOLDER, stateInfo);
    CHECK_OP(stateInfo, );
    seqObj = new U2SequenceObject(oldSeqObj->getGObjectName(), U2EntityRef(newDoc->getDbiRef(), clonedSeq.id), oldSeqObj->getGHintsMap());
    newDoc->addObject(seqObj);

    if (df->isObjectOpSupported(newDoc, DocumentFormat::DocObjectOp_Add, GObjectTypes::ANNOTATION_TABLE)) {
        if (mergeAnnotations) {
            AnnotationTableObject *newDocAto = new AnnotationTableObject("Annotations", newDoc->getDbiRef());
            newDocAto->addObjectRelation(seqObj, ObjectRole_Sequence);

            foreach (Document *d, docs) {
                QList<GObject *> annotationTablesList = d->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
                foreach (GObject *table, annotationTablesList) {
                    AnnotationTableObject *ato = qobject_cast<AnnotationTableObject *>(table);
                    if (ato->hasObjectRelation(oldSeqObj, ObjectRole_Sequence)) {
                        foreach (Annotation *ann, ato->getAnnotations()) {
                            newDocAto->addAnnotations(QList<SharedAnnotationData>() << ann->getData(), ann->getGroup()->getName());
                        }
                    }
                }
            }
            newDoc->addObject(newDocAto);
        } else {
            // use only sequence-doc annotations
            foreach (GObject *o, curDoc->getObjects()) {
                AnnotationTableObject *aObj = qobject_cast<AnnotationTableObject *>(o);
                if (NULL != aObj) {
                    U2OpStatus2Log os;
                    GObject *cl = aObj->clone(newDoc->getDbiRef(), os);
                    newDoc->addObject(cl);
                    GObjectUtils::updateRelationsURL(cl, curDoc->getURL(), newDoc->getURL());
                }
            }
        }
    }
    docs.append(newDoc);
}

} // namespace U2
