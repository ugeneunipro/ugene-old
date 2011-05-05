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

#include <QtGui/QMessageBox>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2Misc/DialogUtils.h>

#include <U2Algorithm/EnzymeModel.h>
#include "CreateFragmentDialog.h"

namespace U2 {

CreateFragmentDialog::CreateFragmentDialog( ADVSequenceObjectContext* ctx,  QWidget* p )
: QDialog(p)
{
    
    setupUi(this);
        
    seqObj = ctx->getSequenceObject();
    seqSelection = ctx->getSequenceSelection();
    relatedAnnotations = ctx->getAnnotationObjects(true).toList();
    
    startBox->setMaximum(seqObj->getSequenceLen());
    endBox->setMaximum(seqObj->getSequenceLen());
    
    setupAnnotationsWidget();

}

CreateFragmentDialog::CreateFragmentDialog( DNASequenceObject* obj, QWidget* p )
: QDialog(p)
{
    setupUi(this);
    seqObj = obj;
    seqSelection = NULL;

    QList<GObject*> aObjects = GObjectUtils::findAllObjects(UOF_LoadedOnly,GObjectTypes::ANNOTATION_TABLE);
    QList<GObject*> related = GObjectUtils::findObjectsRelatedToObjectByRole(seqObj, GObjectTypes::ANNOTATION_TABLE, 
        GObjectRelationRole::SEQUENCE, aObjects, UOF_LoadedOnly);
    
    foreach (GObject* obj, related) {
        AnnotationTableObject* aObj = qobject_cast<AnnotationTableObject*>(obj);
        assert(aObj != NULL);
        relatedAnnotations.append(aObj);
    }

    startBox->setMaximum(seqObj->getSequenceLen());
    endBox->setMaximum(seqObj->getSequenceLen());
    seqSelectionButton->setEnabled(false);

    setupAnnotationsWidget();

}

void CreateFragmentDialog::accept()
{
    
    QString leftOverhang, rightOverhang;

    if (leftEndBox->isChecked()) {
        leftOverhang = lCustomOverhangEdit->text();
        DNAAlphabet* alph = AppContext::getDNAAlphabetRegistry()->findAlphabet(leftOverhang.toAscii());
        if (!alph->isNucleic()) {
            QMessageBox::warning(this, windowTitle(),tr("5'overhang cotnains unsupported symbols!"));
            return;
        }

    } 

    if (rightEndBox->isChecked()) {
        rightOverhang = rCustomOverhangEdit->text();
        DNAAlphabet* alph = AppContext::getDNAAlphabetRegistry()->findAlphabet(rightOverhang.toAscii());
        if (!alph->isNucleic()) {
            QMessageBox::warning(this, windowTitle(),tr("3'overhang contains unsupported symbols!"));
            return;
        }
    }    
    
    U2Region reg;
    
    if (wholeSeqButton->isChecked()) {
        reg = seqObj->getSequenceRange();
    } else if (seqSelectionButton->isChecked()) {
        assert(seqSelection != NULL);
        QVector<U2Region> selection = seqSelection->getSelectedRegions();
        if (selection.isEmpty()) {
            QMessageBox::warning(this, windowTitle(),tr("Sequence selection is empty!\nChoose another region."));
            return;
        }
        
        reg = selection.first();
    } else {
        reg.startPos = startBox->value() - 1;
        reg.length = endBox->value() - reg.startPos;

        if (reg.length <= 0) {
            QMessageBox::warning(this, windowTitle(),tr("Invalid fragment region!\nChoose another region."));
            return;
        }
    }

    ac->prepareAnnotationObject();
    const CreateAnnotationModel& m = ac->getModel();
    AnnotationTableObject* obj = m.getAnnotationObject();
    QString groupName = m.groupName;
    assert(!groupName.isEmpty() && obj!=NULL);

    AnnotationData* ad = new AnnotationData();
    
    ad->location->regions.append(reg);

    ad->qualifiers.append(U2Qualifier(QUALIFIER_LEFT_TERM, QString()));
    ad->qualifiers.append(U2Qualifier(QUALIFIER_RIGHT_TERM, QString()));
    ad->qualifiers.append(U2Qualifier(QUALIFIER_LEFT_OVERHANG, leftOverhang) );
    ad->qualifiers.append(U2Qualifier(QUALIFIER_RIGHT_OVERHANG, rightOverhang) );
    QString leftOverhangType = leftOverhang.isEmpty() ? OVERHANG_TYPE_BLUNT : OVERHANG_TYPE_STICKY;
    ad->qualifiers.append(U2Qualifier(QUALIFIER_LEFT_TYPE, leftOverhangType) );
    QString rightOverhangType = rightOverhang.isEmpty() ? OVERHANG_TYPE_BLUNT : OVERHANG_TYPE_STICKY;
    ad->qualifiers.append(U2Qualifier(QUALIFIER_RIGHT_TYPE, rightOverhangType) );
    ad->qualifiers.append(U2Qualifier(QUALIFIER_SOURCE, seqObj->getGObjectName()));
    ad->name = QString("Fragment (%1-%2)").arg(reg.startPos+1).arg(reg.endPos());

    Annotation* annotatedFragment = new Annotation(SharedAnnotationData(ad));
    obj->addAnnotation(annotatedFragment,groupName);
    
    dnaFragment = DNAFragment(annotatedFragment,seqObj, relatedAnnotations);

    QDialog::accept();
}


void CreateFragmentDialog::setupAnnotationsWidget()
{
    CreateAnnotationModel acm;
    acm.sequenceObjectRef = GObjectReference(seqObj);
    acm.hideAnnotationName = true;
    acm.hideLocation = true;
    acm.data->name = ANNOTATION_GROUP_FRAGMENTS;
    acm.sequenceLen = seqObj->getSequenceLen();
    ac = new CreateAnnotationWidgetController(acm, this);
    QWidget* caw = ac->getWidget();    
    QVBoxLayout* l = new QVBoxLayout();
    l->setMargin(0);
    l->addWidget(caw);
    annotationsWidget->setLayout(l);
    annotationsWidget->setMinimumSize(caw->layout()->minimumSize());
}


} // namespace
