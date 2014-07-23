/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <U2Core/Annotation.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/U2Type.h>

#include <QtCore/QSet>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QWidget>
#include <QtGui/QMenu>
#else
#include <QtWidgets/QWidget>
#include <QtWidgets/QMenu>
#endif

namespace U2 {

class AnnotatedDNAView;
class U2SequenceObject;
class DNAAlphabet;
class DNATranslation;
class DNASequenceSelection;
class ADVSequenceWidget;
class AnnotationSelection;
class GObject;
class Annotation;
class U2Region;

class U2VIEW_EXPORT ADVSequenceObjectContext : public QObject {
    Q_OBJECT
public:
    ADVSequenceObjectContext(AnnotatedDNAView* v, U2SequenceObject* obj);
    
    AnnotatedDNAView*   getAnnotatedDNAView() const {return view;}
    DNATranslation*     getComplementTT() const {return complTT;}
    DNATranslation*     getAminoTT() const {return aminoTT;}
    U2SequenceObject*   getSequenceObject() const {return seqObj;}
    GObject*            getSequenceGObject() const;
    
    qint64 getSequenceLength() const;
    const DNAAlphabet* getAlphabet() const;
    QByteArray getSequenceData(const U2Region& r) const;
    U2EntityRef getSequenceRef() const;
    bool        isRowChoosed();

    DNASequenceSelection*   getSequenceSelection() const {return selection;}
    
    QSet<AnnotationTableObject *> getAnnotationObjects(bool includeAutoAnnotations = false) const;
    QSet<AnnotationTableObject *> getAutoAnnotationObjects() const { return autoAnnotations; }
    QList<GObject*> getAnnotationGObjects() const;
    
    QMenu* createTranslationsMenu();
    void setAminoTranslation(const QString& tid);

    void addAnnotationObject(AnnotationTableObject *obj);
    void addAutoAnnotationObject(AnnotationTableObject *obj);
    void removeAnnotationObject(AnnotationTableObject *obj);

    AnnotationSelection * getAnnotationsSelection() const;
    
    const QList<ADVSequenceWidget*>& getSequenceWidgets() const {return seqWidgets;}
    void addSequenceWidget(ADVSequenceWidget* w);
    void removeSequenceWidget(ADVSequenceWidget* w);

    QList<Annotation> selectRelatedAnnotations(const QList<Annotation> &alist) const;
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
    void si_annotationObjectAdded(AnnotationTableObject *obj);
    void si_annotationObjectRemoved(AnnotationTableObject *obj);
    void si_translationRowsChanged();

private:
    void guessAminoTT(const AnnotationTableObject *ao);


    AnnotatedDNAView*               view;
    U2SequenceObject*               seqObj;
    DNATranslation*                 aminoTT;
    DNATranslation*                 complTT;
    DNASequenceSelection*           selection;
    QActionGroup*                   translations;
    QActionGroup*                   visibleFrames;
    QVector<QAction*>               translationRowsStatus;
    QList<ADVSequenceWidget*>       seqWidgets;
    QSet<AnnotationTableObject *>     annotations;
    QSet<AnnotationTableObject *>     autoAnnotations;
    bool                            clarifyAminoTT;
    bool                            rowChoosed;
};

} //namespace

#endif
