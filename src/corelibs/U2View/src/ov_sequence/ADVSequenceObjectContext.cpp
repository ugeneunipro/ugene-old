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

#include "ADVSequenceObjectContext.h"

#include "AnnotatedDNAView.h"

#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/GHints.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2View/CodonTable.h>


namespace U2 {

ADVSequenceObjectContext::ADVSequenceObjectContext(AnnotatedDNAView* v, U2SequenceObject* obj)
: QObject(v), view(v), seqObj(obj), aminoTT(NULL), complTT(NULL), selection(NULL), translations(NULL), visibleFrames(NULL),rowChoosed(false)
{
    selection = new DNASequenceSelection(seqObj, this);
    clarifyAminoTT = false;
    const DNAAlphabet* al  = getAlphabet();
    if (al->isNucleic()) {
        DNATranslationRegistry* tr = AppContext::getDNATranslationRegistry();
        complTT = GObjectUtils::findComplementTT(seqObj->getAlphabet());
        aminoTT = GObjectUtils::findAminoTT(seqObj, true);
        clarifyAminoTT = aminoTT == NULL;

        QList<DNATranslation*> aminoTs = tr->lookupTranslation(al, DNATranslationType_NUCL_2_AMINO);
        if (!aminoTs.empty()) {
            aminoTT = aminoTT == NULL ? tr->getStandardGeneticCodeTranslation(al) : aminoTT;
            translations = new QActionGroup(this);
            const CodonTableView *ct = v->getCodonTableView();
            foreach(DNATranslation* t, aminoTs) {
                QAction* a = translations->addAction(t->getTranslationName());
                a->setObjectName(t->getTranslationName());
                a->setCheckable(true);
                a->setChecked(aminoTT == t);
                a->setData(QVariant(t->getTranslationId()));
                connect(a, SIGNAL(triggered()), SLOT(sl_setAminoTranslation()));
                connect(a, SIGNAL(triggered()), ct, SLOT(sl_setAminoTranslation()));
            }
            visibleFrames = new QActionGroup(this);
            visibleFrames->setExclusive(false);
            for(int i = 0; i < 6; i++){
                QAction* a;
                if(i < 3){
                    a = visibleFrames->addAction(QString("%1 direct translation frame").arg(i+1));
                }else{
                    a = visibleFrames->addAction(QString("%1 complementary translation frame").arg(i+1-3));
                }
                a->setCheckable(true);
                a->setChecked(true);
                //set row id
                a->setData(i);
                //save status
                translationRowsStatus.append(a);
                connect(a, SIGNAL(triggered()), SLOT(sl_toggleTranslations()));
            }
        }
    }
}

void ADVSequenceObjectContext::guessAminoTT(const AnnotationTableObject *ao) {
    const DNAAlphabet *al  = getAlphabet();
    SAFE_POINT(al->isNucleic(), "Unexpected DNA alphabet detected!",);
    DNATranslation *res = NULL;
    DNATranslationRegistry *tr = AppContext::getDNATranslationRegistry();
    // try to guess relevant translation from a CDS feature (if any)
    foreach (Annotation *ann, ao->getAnnotationsByName("CDS")) {
        QList<U2Qualifier> ql;
        ann->findQualifiers("transl_table", ql);
        if (ql.size() > 0) {
            QString guess = "NCBI-GenBank #"+ql.first().value;
            res = tr->lookupTranslation(al, DNATranslationType_NUCL_2_AMINO, guess);
            if (res !=NULL) {
                break;
            }
        }
    }
    if (res != NULL) {
        clarifyAminoTT = false;
        setAminoTranslation(res->getTranslationId());
    }
}


qint64 ADVSequenceObjectContext::getSequenceLength() const {
    return seqObj->getSequenceLength();
}

const DNAAlphabet* ADVSequenceObjectContext::getAlphabet() const {
    return seqObj->getAlphabet();
}

QByteArray ADVSequenceObjectContext::getSequenceData(const U2Region &r, U2OpStatus &os) const {
    return seqObj->getSequenceData(r, os);
}

U2EntityRef ADVSequenceObjectContext::getSequenceRef() const {
    return seqObj->getSequenceRef();
}

QList<GObject *> ADVSequenceObjectContext::getAnnotationGObjects() const {
    QList<GObject *> res;
    foreach (AnnotationTableObject *ao, annotations) {
        res.append(ao);
    }
    return res;
}

void ADVSequenceObjectContext::sl_showDirectOnly(){
    GCOUNTER( cvar, tvar, "SequenceView::DetView::ShowDirectTranslationsOnly" );
    bool needUpdate = false;
    QList<QAction*> actionList = visibleFrames->actions();
    translationRowsStatus.clear();
    int i = 0;
    for(; i < 3; i++){
        QAction *a = actionList[i];
        if(!a->isChecked()){
            needUpdate = true;
            a->setChecked(true);
            translationRowsStatus.append(a);
        }
    }
    for(; i < 6; i++){
        QAction *a = actionList[i];
        if(a->isChecked()){
            needUpdate = true;
            a->setChecked(false);
        }
    }
    if(needUpdate){
        emit si_translationRowsChanged();
    }
}

void ADVSequenceObjectContext::sl_showComplOnly(){
    GCOUNTER( cvar, tvar, "SequenceView::DetView::ShowComplementTranslationsOnly" );
    bool needUpdate = false;
    QList<QAction*> actionList = visibleFrames->actions();
    translationRowsStatus.clear();
    int i = 0;
    for(; i < 3; i++){
        QAction *a = actionList[i];
        if(a->isChecked()){
            needUpdate = true;
            a->setChecked(false);
        }
    }
    for(; i < 6; i++){
        QAction *a = actionList[i];
        if(!a->isChecked()){
            needUpdate = true;
            a->setChecked(true);
            translationRowsStatus.append(a);
        }
    }
    if(needUpdate){
        emit si_translationRowsChanged();
    }
}

void ADVSequenceObjectContext::sl_showShowAll(){
    GCOUNTER( cvar, tvar, "SequenceView::DetView::ShowAllTranslations" );
    bool needUpdate = false;
    translationRowsStatus.clear();
    foreach(QAction* a, visibleFrames->actions()){
        if(!a->isChecked()) {
            needUpdate = true;
            a->setChecked(true);
            translationRowsStatus.append(a);
        }
    }
    if(needUpdate){
        emit si_translationRowsChanged();
    }
}

void ADVSequenceObjectContext::sl_onAnnotationRelationChange() {
    AnnotationTableObject* obj = qobject_cast<AnnotationTableObject*>(sender());
    SAFE_POINT(obj != NULL, tr("Incorrect signal sender!"),);

    if (!obj->hasObjectRelation(seqObj, ObjectRole_Sequence)) {
        disconnect(obj, SIGNAL(si_relationChanged()), this, SLOT(sl_onAnnotationRelationChange()));
        view->removeObject(obj);
    }
}

QMenu * ADVSequenceObjectContext::createGeneticCodeMenu() {
    CHECK(NULL != translations, NULL);
    QMenu *menu = new QMenu(tr("Select genetic code"));
    menu->setIcon(QIcon(":core/images/tt_switch.png"));
    menu->menuAction()->setObjectName("AminoTranslationAction");

    foreach (QAction *a, translations->actions()) {
        menu->addAction(a);
    }
    return menu;
}

QMenu * ADVSequenceObjectContext::createTranslationFramesMenu(QAction *showTranslationAction) {
    SAFE_POINT(visibleFrames != NULL, "ADVSequenceObjectContext: visibleFrames is NULL ?!", NULL);
    QMenu *menu = new QMenu(tr("Show/hide amino acid translations"));
    menu->setIcon(QIcon(":core/images/show_trans.png"));
    menu->menuAction()->setObjectName("Translation frames");

    menu->addAction(showTranslationAction);
    menu->addSeparator();

    menu->addAction(tr("Show direct only"), this, SLOT(sl_showDirectOnly()));
    menu->addAction(tr("Show complementary only"), this, SLOT(sl_showComplOnly()));
    menu->addAction(tr("Show all"), this, SLOT(sl_showShowAll()));
    menu->addSeparator();

    foreach(QAction* a, visibleFrames->actions()) {
        menu->addAction(a);
    }
    return menu;
}

void ADVSequenceObjectContext::setAminoTranslation(const QString& tid) {
    const DNAAlphabet* al = getAlphabet();
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
    GCOUNTER( cvar, tvar, "DetView_SetAminoTranslation" );
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

void ADVSequenceObjectContext::addAnnotationObject(AnnotationTableObject *obj) {
    SAFE_POINT(!annotations.contains(obj), "Unexpected annotation table!",);
    SAFE_POINT(obj->hasObjectRelation(seqObj, ObjectRole_Sequence), "Annotation table relates to unexpected sequence!",);
    connect(obj, SIGNAL(si_relationChanged()), SLOT(sl_onAnnotationRelationChange()));
    annotations.insert(obj);
    emit si_annotationObjectAdded(obj);
    if (clarifyAminoTT) {
        guessAminoTT(obj);
    }
}

void ADVSequenceObjectContext::removeAnnotationObject(AnnotationTableObject *obj) {
    SAFE_POINT(annotations.contains(obj), "Unexpected annotation table!",);
    annotations.remove(obj);
    emit si_annotationObjectRemoved(obj);
}

QList<Annotation *> ADVSequenceObjectContext::selectRelatedAnnotations(const QList<Annotation *> &alist) const {
    QList<Annotation *> res;
    foreach (Annotation *a, alist) {
        AnnotationTableObject* o = a->getGObject();
        if (annotations.contains(o) || autoAnnotations.contains(o)) {
            res.append(a);
        }
    }
    return res;
}

GObject * ADVSequenceObjectContext::getSequenceGObject() const {
    return seqObj;
}

void ADVSequenceObjectContext::addAutoAnnotationObject(AnnotationTableObject *obj) {
    autoAnnotations.insert(obj);
    emit si_annotationObjectAdded(obj);
}

QSet<AnnotationTableObject *> ADVSequenceObjectContext::getAnnotationObjects(
    bool includeAutoAnnotations) const
{
    QSet<AnnotationTableObject *> result = annotations;
    if (includeAutoAnnotations) {
        result += autoAnnotations;
    }

    return result;
}

void ADVSequenceObjectContext::sl_toggleTranslations(){
    translationRowsStatus.clear();
    rowChoosed = true;
    emit si_translationRowsChanged();
    rowChoosed = false;
}

bool ADVSequenceObjectContext::isRowChoosed(){
    return rowChoosed;
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
void ADVSequenceObjectContext::setTranslationsVisible(bool enable){
    bool needUpdate = false;
    if(!enable){
        translationRowsStatus.clear();
    }
    foreach(QAction* a, visibleFrames->actions()){
        if(!enable){//hide
            if(a->isChecked()) {
                needUpdate = true;
                a->setChecked(false);
                translationRowsStatus.append(a);
            }
        }else{//show
            if(!a->isChecked() && (translationRowsStatus.contains(a) || translationRowsStatus.isEmpty())) {
                needUpdate = true;
                a->setChecked(true);
            }
        }
    }
    if(needUpdate){
        emit si_translationRowsChanged();
    }
}

} // namespace U2
