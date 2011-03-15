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

#ifndef _U2_ANNOTATED_DNA_VIEW_CLIPBOARD_H_
#define _U2_ANNOTATED_DNA_VIEW_CLIPBOARD_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include <QtGui/QAction>

namespace U2 {

class AnnotatedDNAView;
class LRegionsSelection;
class Annotation;
class AnnotationSelection;
class ADVSequenceObjectContext;
class ADVSequenceWidget;

class U2VIEW_EXPORT ADVClipboard: public QObject {
    Q_OBJECT
public:
    ADVClipboard(AnnotatedDNAView* ctx);

    QAction* getCopySequenceAction() const {return copySequenceAction;}
    QAction* getCopyTranslationAction() const {return copyTranslationAction;}
    QAction* getCopyComplementAction() const {return copyComplementSequenceAction;}
    QAction* getCopyComplementTranslationAction() const {return copyComplementTranslationAction;}

    QAction* getCopyAnnotationSequenceAction() const {return copyAnnotationSequenceAction;}
    QAction* getCopyAnnotationSequenceTranslationAction() const {return copyAnnotationSequenceTranslationAction;}

    void addCopyMenu(QMenu* m);

public slots:
    
    void sl_onDNASelectionChanged(LRegionsSelection* s, const QVector<U2Region>& added, const QVector<U2Region>& removed);
    void sl_onAnnotationSelectionChanged(AnnotationSelection* s, const QList<Annotation*>& added, const QList<Annotation*>& removed);
    void sl_onFocusedSequenceWidgetChanged(ADVSequenceWidget*, ADVSequenceWidget*);

    void sl_copySequence();
    void sl_copyTranslation();
    void sl_copyComplementSequence();
    void sl_copyComplementTranslation();
    void sl_copyAnnotationSequence();
    void sl_copyAnnotationSequenceTranslation();

    ADVSequenceObjectContext* getSequenceContext() const;

private:
    void connectSequence(ADVSequenceObjectContext* s);
    void updateActions();
    void copySequenceSelection(bool complement, bool amino);

    AnnotatedDNAView* ctx;
    QAction* copySequenceAction;
    QAction* copyTranslationAction;
    QAction* copyComplementSequenceAction;
    QAction* copyComplementTranslationAction;
    QAction* copyAnnotationSequenceAction;
    QAction* copyAnnotationSequenceTranslationAction;
};

}//namespace

#endif
