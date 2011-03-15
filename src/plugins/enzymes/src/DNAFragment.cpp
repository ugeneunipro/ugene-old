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

#include <U2Core/DocumentModel.h>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/AppContext.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNATranslation.h>


#include "EnzymeModel.h"
#include "DNAFragment.h"

namespace U2 {

DNAFragment::DNAFragment( Annotation* fragment, DNASequenceObject* sObj, const QList<AnnotationTableObject*> relatedAnns )
    : annotatedFragment(fragment), dnaObj(sObj), relatedAnnotations(relatedAnns), reverseCompl(false)
{   
    assert(fragment != NULL);
    assert(sObj != NULL);
    updateTerms();
}

DNAFragment::DNAFragment( const DNAFragment& other )
{
    annotatedFragment = other.annotatedFragment;
    dnaObj = other.dnaObj;
    relatedAnnotations = other.relatedAnnotations;
    reverseCompl = other.reverseCompl;
    updateTerms();
}


DNAFragment& DNAFragment::operator=( const DNAFragment& other )
{
    annotatedFragment = other.annotatedFragment;
    dnaObj = other.dnaObj;
    relatedAnnotations = other.relatedAnnotations;
    reverseCompl = other.reverseCompl;
    updateTerms();
    return *this;
}


bool static isDNAFragment( const Annotation* a )
{
    QString aName = a->getAnnotationName();
    if (aName.startsWith("Fragment")) {
        return true;
    } else {
        return false;
    }
}

QList<DNAFragment> DNAFragment::findAvailableFragments()
{
    QList<GObject*> aObjects = GObjectUtils::findAllObjects(UOF_LoadedOnly,GObjectTypes::ANNOTATION_TABLE);
    QList<GObject*> sObjects = GObjectUtils::findAllObjects(UOF_LoadedOnly,GObjectTypes::SEQUENCE);
    
    return findAvailableFragments(aObjects,sObjects);
}

QList<DNAFragment> DNAFragment::findAvailableFragments( const QList<GObject*>& aObjects, const QList<GObject*>& sObjects )
{
    QList<DNAFragment> fragments;
    foreach (GObject* obj , aObjects) {
        AnnotationTableObject* aObj = qobject_cast<AnnotationTableObject*>(obj);
        assert(aObj != NULL);
        QList<Annotation*> annotations = aObj->getAnnotations();
        foreach (Annotation* a, annotations) {
            if (isDNAFragment(a)) {
                // Skip negative split
                if (GObjectUtils::annotationHasNegativeSplit(a)) {
                    continue;
                }
                // Find related sequence object
                DNASequenceObject* dnaObj = NULL;
                QList<GObjectRelation> relations = aObj->getObjectRelations();
                foreach (const GObjectRelation& relation, relations ) {
                    if (relation.role != GObjectRelationRole::SEQUENCE) {
                        continue;
                    }
                    GObject* relatedObj = GObjectUtils::selectObjectByReference(relation.ref, sObjects, UOF_LoadedOnly);
                    dnaObj = qobject_cast<DNASequenceObject*>(relatedObj);
                }
                if (dnaObj == NULL) {
                    continue;
                }
                // Find related annotation tables 
                QList<GObject*> relatedAnns = GObjectUtils::findObjectsRelatedToObjectByRole(dnaObj, GObjectTypes::ANNOTATION_TABLE, GObjectRelationRole::SEQUENCE, aObjects, UOF_LoadedOnly);
                // Add fragment
                DNAFragment fragment;
                fragment.annotatedFragment = a;
                fragment.dnaObj = dnaObj;
                foreach(GObject* relAnn, relatedAnns) {
                    AnnotationTableObject* related = qobject_cast<AnnotationTableObject*>(relAnn);
                    fragment.relatedAnnotations.append(related);    
                }
                fragments.append(fragment);
            }
        }
    }
    return fragments;
}

QVector<U2Region> DNAFragment::getFragmentRegions() const
{
    assert(!isEmpty());
    QVector<U2Region> result = annotatedFragment->getRegions();
    QString split = annotatedFragment->findFirstQualifierValue(QUALIFIER_SPLIT);
    if (!split.isEmpty()) {
        int splitVal = split.toInt();
        U2Region oldRegion = result.first();
        U2Region addRegion(0, splitVal - oldRegion.length);
        result.append(addRegion);
    }

    return result;
}

QString DNAFragment::getName() const
{
    assert(!isEmpty());
    return annotatedFragment->getAnnotationName();
}

DNAAlphabet* DNAFragment::getAlphabet() const
{
    assert(!isEmpty());
    return dnaObj->getAlphabet();
}

QString DNAFragment::getSequenceName() const
{
    return dnaObj->getGObjectName();
}

QByteArray DNAFragment::getSequence() const
{
    assert(!isEmpty());
    const U2Region& region = annotatedFragment->getLocation()->regions[0];
    QByteArray seq = dnaObj->getSequence().mid(region.startPos, region.length);
    QString split = annotatedFragment->findFirstQualifierValue(QUALIFIER_SPLIT);
    if (!split.isEmpty()) {
        int splitVal = split.toInt();
        QByteArray addSeq = dnaObj->getSequence().mid(0, splitVal - region.length);
        seq.append(addSeq);
    }

    if (reverseCompl) {
        DNAAlphabet* al = dnaObj->getAlphabet();
        DNATranslation* transl = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(al);
        transl->translate(seq.data(), seq.size());
        TextUtils::reverse(seq.data(), seq.size());
    }

    return seq;
}


QString DNAFragment::getSequenceDocName() const
{
    return dnaObj->getDocument()->getName();
}

const DNAFragmentTerminus& DNAFragment::getLeftTerminus() const
{
    return leftTerm;
}

const DNAFragmentTerminus& DNAFragment::getRightTerminus() const
{
    return rightTerm;
}

bool DNAFragment::isSplitted() const
{
    return !annotatedFragment->findFirstQualifierValue(QUALIFIER_SPLIT).isEmpty();
}

void DNAFragment::setInverted( bool inverted /*= true*/ )
{
    QString val = inverted ? "yes" : "no";
    GObjectUtils::replaceAnnotationQualfier(annotatedFragment, QUALIFIER_INVERTED, val, true);
    updateTerms();
}

void DNAFragment::setLeftTermType( const QByteArray& termType )
{
    QString qName = QUALIFIER_LEFT_TYPE;
    if (reverseCompl) {
        qName = isSplitted() ?  QUALIFIER_SPLIT_TYPE : QUALIFIER_RIGHT_TYPE;
    }
    GObjectUtils::replaceAnnotationQualfier(annotatedFragment, qName, termType);
    updateLeftTerm();
}

void DNAFragment::setRightTermType( const QByteArray& termType )
{
    QString qName = isSplitted() ?  QUALIFIER_SPLIT_TYPE : QUALIFIER_RIGHT_TYPE;
    if (reverseCompl) {
        qName = QUALIFIER_LEFT_TYPE;
    }
    GObjectUtils::replaceAnnotationQualfier(annotatedFragment, qName, termType);
    updateTerms();
}

void DNAFragment::setOverhang( const QByteArray& qName, const QByteArray& overhang )
{   
    GObjectUtils::replaceAnnotationQualfier(annotatedFragment, qName, overhang);
    updateTerms();
}

void DNAFragment::setLeftOverhang( const QByteArray& overhang )
{
     QByteArray buf(overhang);
     QByteArray qName(QUALIFIER_LEFT_OVERHANG);
     if (reverseCompl) {
         toRevCompl(buf);
         qName = isSplitted() ?  QUALIFIER_SPLIT_OVERHANG : QUALIFIER_RIGHT_OVERHANG;
     } 
     
     setOverhang(qName, buf);
        
}

void DNAFragment::setRightOverhang( const QByteArray& overhang )
{
    QByteArray buf(overhang);
    QByteArray qName(isSplitted() ?  QUALIFIER_SPLIT_OVERHANG : QUALIFIER_RIGHT_OVERHANG);
    if (reverseCompl) {
        toRevCompl(buf);
        qName = QUALIFIER_LEFT_OVERHANG;
    } 

    setOverhang(qName, buf);
}


int DNAFragment::getLength() const
{
    assert(!isEmpty());
    QVector<U2Region> regions = getFragmentRegions();
    int len = 0;
    foreach (const U2Region& r, regions) {
        len += r.length;
    }
    return len;
}

const QByteArray& DNAFragment::getSourceSequence() const
{
    assert(!isEmpty());
    return dnaObj->getSequence(); 
}

void DNAFragment::updateTerms()
{
    if (annotatedFragment == NULL) {
        return;
    }
    updateLeftTerm();
    updateRightTerm();
    
    QString buf = annotatedFragment->findFirstQualifierValue(QUALIFIER_INVERTED);
    reverseCompl = buf == "yes" ? true : false;

    if (reverseCompl) {
        toRevCompl(leftTerm.overhang);
        toRevCompl(rightTerm.overhang);
        qSwap(leftTerm, rightTerm);
    }
}

void DNAFragment::updateLeftTerm()
{
    assert(annotatedFragment != NULL);
    leftTerm.enzymeId = annotatedFragment->findFirstQualifierValue(QUALIFIER_LEFT_TERM).toAscii();
    leftTerm.overhang = annotatedFragment->findFirstQualifierValue(QUALIFIER_LEFT_OVERHANG).toAscii();
    leftTerm.termType = annotatedFragment->findFirstQualifierValue(QUALIFIER_LEFT_TYPE).toAscii();
}

void DNAFragment::updateRightTerm()
{
    assert(annotatedFragment != NULL);
    if (isSplitted()) {
        rightTerm.enzymeId = annotatedFragment->findFirstQualifierValue(QUALIFIER_SPLIT_TERM).toAscii();
        rightTerm.overhang = annotatedFragment->findFirstQualifierValue(QUALIFIER_SPLIT_OVERHANG).toAscii();
        rightTerm.termType = annotatedFragment->findFirstQualifierValue(QUALIFIER_SPLIT_TYPE).toAscii();
    } else {
        rightTerm.enzymeId = annotatedFragment->findFirstQualifierValue(QUALIFIER_RIGHT_TERM).toAscii();
        rightTerm.overhang = annotatedFragment->findFirstQualifierValue(QUALIFIER_RIGHT_OVERHANG).toAscii();
        rightTerm.termType = annotatedFragment->findFirstQualifierValue(QUALIFIER_RIGHT_TYPE).toAscii();
    }
}

void DNAFragment::toRevCompl( QByteArray& seq )
{
    DNAAlphabet* al = dnaObj->getAlphabet();
    DNATranslation* transl = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(al);
    transl->translate(seq.data(), seq.size());
    TextUtils::reverse(seq.data(), seq.size());
}






} // namespace
