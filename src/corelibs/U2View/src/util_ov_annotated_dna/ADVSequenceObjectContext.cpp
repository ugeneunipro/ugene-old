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

#include "ADVSequenceObjectContext.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/GHints.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/DNASequenceSelection.h>

#include "AnnotatedDNAView.h"

namespace U2 {

ADVSequenceObjectContext::ADVSequenceObjectContext(AnnotatedDNAView* v, DNASequenceObject* obj) 
: QObject(v), view(v), seqObj(obj), aminoTT(NULL), complTT(NULL), selection(NULL), translations(NULL), visibleFrames(NULL)
{
    selection = new DNASequenceSelection(seqObj, this);
    clarifyAminoTT = false;
    DNAAlphabet* al  = getAlphabet();
    if (al->isNucleic()) {
        DNATranslationRegistry* tr = AppContext::getDNATranslationRegistry();
        complTT = GObjectUtils::findComplementTT(seqObj);
        aminoTT = GObjectUtils::findAminoTT(seqObj, true);
        clarifyAminoTT = aminoTT == NULL;
        
        QList<DNATranslation*> aminoTs = tr->lookupTranslation(al, DNATranslationType_NUCL_2_AMINO);
        if (!aminoTs.empty()) {
            aminoTT = aminoTT == NULL ? aminoTs.first() : aminoTT;
            translations = new QActionGroup(this);
            foreach(DNATranslation* t, aminoTs) {
                QAction* a = translations->addAction(t->getTranslationName());
                a->setCheckable(true);
                a->setChecked(aminoTT == t);
                a->setData(QVariant(t->getTranslationId()));
                connect(a, SIGNAL(triggered()), SLOT(sl_setAminoTranslation()));
            }
            visibleFrames = new QActionGroup(this);
            visibleFrames->setExclusive(false);
            for(int i = 0; i < 6; i++){
                QAction* a;
                if(i < 3){
                    a = visibleFrames->addAction(QString("%1 direct translation frame").arg(i+1));
                }else{
                    a = visibleFrames->addAction(QString("%1 complement translation frame").arg(i+1-3));
                }
                a->setCheckable(true);
                a->setChecked(true);
                //set row id
                a->setData(i);
                connect(a, SIGNAL(triggered()), SLOT(sl_toggleTranslations()));
            }
        }
    }
}

void ADVSequenceObjectContext::guessAminoTT(AnnotationTableObject* ao) {
    DNAAlphabet* al  = getAlphabet();
    assert(al->isNucleic());
    DNATranslation* res = NULL;    
    DNATranslationRegistry* tr = AppContext::getDNATranslationRegistry();
    // try to guess relevant translation from a CDS feature (if any)
    foreach(Annotation* ann, ao->getAnnotations()) {
        if (ann->getAnnotationName() == "CDS") {
            QVector<U2Qualifier> ql;
            ann->findQualifiers("transl_table", ql);
            if (ql.size() > 0) {
                QString guess = "NCBI-GenBank #"+ql.first().value;
                res = tr->lookupTranslation(al, DNATranslationType_NUCL_2_AMINO, guess);
                if (res !=NULL) {
                    break;
                }
            }
        }
    }
    if (res != NULL) {
        clarifyAminoTT = false;
        setAminoTranslation(res->getTranslationId());
    }
}


const QByteArray& ADVSequenceObjectContext::getSequenceData() const {
    return seqObj->getSequence();
}

int ADVSequenceObjectContext::getSequenceLen() const {
    return seqObj->getSequenceLen();
}

DNAAlphabet* ADVSequenceObjectContext::getAlphabet() const {
    return seqObj->getAlphabet();
}

QList<GObject*> ADVSequenceObjectContext::getAnnotationGObjects() const {
    QList<GObject*> res;
    foreach(AnnotationTableObject* ao, annotations) {
        res.append(ao);
    }
    return res;
}


QMenu* ADVSequenceObjectContext::createTranslationsMenu() {
    QMenu* m = NULL, *frames = NULL;
    if (translations) {
        m = new QMenu(tr("Amino translation"));
        frames = new QMenu(tr("Translation frames"));

        SAFE_POINT(visibleFrames != NULL, "ADVSequenceObjectContext: visibleFrames is NULL ?!", m);
        foreach(QAction* a, visibleFrames->actions()) {
            frames->addAction(a);
        }
        m->addMenu(frames);
        m->addSeparator();

        m->setIcon(QIcon(":core/images/tt_switch.png"));
        foreach(QAction* a, translations->actions()) {
            m->addAction(a);
        }
    }
    return m;
}

void ADVSequenceObjectContext::setAminoTranslation(const QString& tid) {
    DNAAlphabet* al = getAlphabet();
    DNATranslation* aTT = AppContext::getDNATranslationRegistry()->lookupTranslation(al, DNATranslationType_NUCL_2_AMINO, tid);
    assert(aTT!=NULL);
    if (aTT == aminoTT) {
        return;
    }
    aminoTT = aTT;
    foreach(QAction* a, translations->actions()) {
        if (a->data().toString() == tid) {
            a->setChecked(true);
            break;
        }
    }
    seqObj->getGHints()->set(AMINO_TT_GOBJECT_HINT, tid);
    emit si_aminoTranslationChanged();
}

void ADVSequenceObjectContext::sl_setAminoTranslation() {
    QAction* a = qobject_cast<QAction*>(sender());
    QString tid = a->data().toString();
    setAminoTranslation(tid);
}

AnnotationSelection* ADVSequenceObjectContext::getAnnotationsSelection() const {
    return view->getAnnotationsSelection();
}

void ADVSequenceObjectContext::removeSequenceWidget(ADVSequenceWidget* w) {
    assert(seqWidgets.contains(w));
    seqWidgets.removeOne(w);
}

void ADVSequenceObjectContext::addSequenceWidget(ADVSequenceWidget* w) {
    assert(!seqWidgets.contains(w));
    seqWidgets.append(w);
}

void ADVSequenceObjectContext::addAnnotationObject(AnnotationTableObject* obj) {
    assert(!annotations.contains(obj));
    assert(obj->hasObjectRelation(seqObj, GObjectRelationRole::SEQUENCE));
    annotations.insert(obj);
    emit si_annotationObjectAdded(obj);
    if (clarifyAminoTT) {
        guessAminoTT(obj);
    }
}

void ADVSequenceObjectContext::removeAnnotationObject(AnnotationTableObject* obj) {
    assert(annotations.contains(obj));
    annotations.remove(obj);
    emit si_annotationObjectRemoved(obj);
}


QList<Annotation*> ADVSequenceObjectContext::selectRelatedAnnotations(const QList<Annotation*>& alist) const {
    QList<Annotation*> res;
    foreach(Annotation* a, alist) {
        AnnotationTableObject* o = a->getGObject();
        if (annotations.contains(o) || autoAnnotations.contains(o)) {
            res.append(a);
        }
    }
    return res;
}


GObject* ADVSequenceObjectContext::getSequenceGObject() const {
    return seqObj;
}

void ADVSequenceObjectContext::addAutoAnnotationObject( AnnotationTableObject* obj )
{
    autoAnnotations.insert(obj);
    emit si_annotationObjectAdded(obj);
}

QSet<AnnotationTableObject*> ADVSequenceObjectContext::getAnnotationObjects(bool includeAutoAnnotations) const
{
    QSet<AnnotationTableObject*> result = annotations;
    if (includeAutoAnnotations) {
        result += autoAnnotations;
    }

    return result;

}

void ADVSequenceObjectContext::sl_toggleTranslations(){
    emit si_translationRowsChanged();
}

QVector<bool> ADVSequenceObjectContext::getTranslationRowsVisibleStatus(){
    QVector<bool> result;
    if (visibleFrames != NULL) {
        foreach(QAction* a, visibleFrames->actions()) {
            result.append(a->isChecked());
        }
    }
    return result;
}

}//namespace
