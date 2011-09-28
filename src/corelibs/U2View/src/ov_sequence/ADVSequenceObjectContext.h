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

#ifndef _U2_ADV_SEQUENCE_OBJECT_CONTEXT_H_
#define _U2_ADV_SEQUENCE_OBJECT_CONTEXT_H_

#include  <U2Core/U2Type.h>

#include <QtCore/QSet>
#include <QtGui/QWidget>
#include <QtGui/QMenu>

namespace U2 {

class AnnotatedDNAView;
class DNASequenceObject;
class DNAAlphabet;
class DNATranslation;
class DNASequenceSelection;
class ADVSequenceWidget;
class AnnotationTableObject;
class AnnotationSelection;
class GObject;
class Annotation;
class U2SequenceDbi;

class U2VIEW_EXPORT ADVSequenceObjectContext : public QObject {
    Q_OBJECT
public:
    ADVSequenceObjectContext(AnnotatedDNAView* v, DNASequenceObject* obj);
    
    AnnotatedDNAView*   getAnnotatedDNAView() const {return view;}
    DNATranslation*     getComplementTT() const {return complTT;}
    DNATranslation*     getAminoTT() const {return aminoTT;}
    DNASequenceObject*  getSequenceObject() const {return seqObj;}
    GObject*            getSequenceGObject() const;
    U2SequenceDbi*      getSequenceDbi() const;
    U2DataId            getSequenceId() const;

    const QByteArray& getSequenceData() const;
    int getSequenceLen() const;
    DNAAlphabet* getAlphabet() const;

    DNASequenceSelection*   getSequenceSelection() const {return selection;}
    
    QSet<AnnotationTableObject*> getAnnotationObjects(bool includeAutoAnnotations = false) const;
    QSet<AnnotationTableObject*> getAutoAnnotationObjects() const { return autoAnnotations; }
    QList<GObject*> getAnnotationGObjects() const;
    
    QMenu* createTranslationsMenu();
    void setAminoTranslation(const QString& tid);

    void addAnnotationObject(AnnotationTableObject* obj);
    void addAutoAnnotationObject(AnnotationTableObject* obj);
    void removeAnnotationObject(AnnotationTableObject* obj);

    AnnotationSelection* getAnnotationsSelection() const;
    
    const QList<ADVSequenceWidget*>& getSequenceWidgets() const {return seqWidgets;}
    void addSequenceWidget(ADVSequenceWidget* w);
    void removeSequenceWidget(ADVSequenceWidget* w);

    QList<Annotation*> selectRelatedAnnotations(const QList<Annotation*>& alist) const;
    QVector<bool> getTranslationRowsVisibleStatus();
    void setTranslationsVisible(bool enable);

private slots:
    void sl_setAminoTranslation();
    void sl_toggleTranslations();
    void sl_showDirectOnly();
    void sl_showComplOnly();
    void sl_showShowAll();
signals:
    void si_aminoTranslationChanged();
    void si_annotationObjectAdded(AnnotationTableObject* obj);
    void si_annotationObjectRemoved(AnnotationTableObject* obj);
    void si_translationRowsChanged();

private:
    void guessAminoTT(AnnotationTableObject* ao);


    AnnotatedDNAView*               view;
    DNASequenceObject*              seqObj;
    DNATranslation*                 aminoTT;
    DNATranslation*                 complTT;
    DNASequenceSelection*           selection;
    QActionGroup*                   translations;
    QActionGroup*                   visibleFrames;
    QVector<QAction*>               translationRowsStatus;
    QList<ADVSequenceWidget*>       seqWidgets;
    QSet<AnnotationTableObject*>    annotations;
    QSet<AnnotationTableObject*>    autoAnnotations;
    bool                            clarifyAminoTT;
};

} //namespace

#endif
