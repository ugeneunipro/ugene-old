#include "CreateObjectRelationDialogController.h"
#include <ui/ui_CreateObjectRelationDialog.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectTypes.h>

#include <QtGui/QIcon>
#include <QtGui/QMessageBox>

namespace U2 {

CreateObjectRelationDialogController::CreateObjectRelationDialogController(GObject* _assObj, const QList<GObject*>& _objects, 
                                                                           const QString& _role, bool rd, const QString& relationHint, QWidget* p)
: QDialog(p), selectedObject(NULL), assObj(_assObj), objects(_objects), role(_role), removeDuplicates(rd)
{
    assert(!objects.isEmpty());
    assert(!role.isEmpty());
    assert(assObj!=NULL);
    ui = new Ui_CreateObjectRelationDialog;

    ui->setupUi(this);

    QIcon objectIcon(":/core/images/gobject.png");
    foreach(GObject* obj, objects) {
        ui->listWidget->addItem(new QListWidgetItem(objectIcon, obj->getGObjectName()));        
    }
    ui->listWidget->setItemSelected(ui->listWidget->item(0), true);
    ui->relationHintLabel->setText(relationHint);
}

void CreateObjectRelationDialogController::accept() {
    int idx = ui->listWidget->currentRow();
    assert(idx>=0 && idx < objects.size());
    
    GObject* selObj = objects[idx];
    if (role == GObjectRelationRole::SEQUENCE && assObj->getGObjectType() == GObjectTypes::ANNOTATION_TABLE) {
        DNASequenceObject* dnaObj = qobject_cast<DNASequenceObject*>(selObj);
        AnnotationTableObject* ao = qobject_cast<AnnotationTableObject*>(assObj);
        AnnotationTableObjectConstraints c;
        c.sequenceSizeToFit = dnaObj->getSequenceLen();
        bool res = ao->checkConstraints(&c);
        if (!res) {
            int rc = QMessageBox::question(this, tr("Warning"), 
                tr("Found annotations that are out of the sequence range, continue?"), QMessageBox::Yes, QMessageBox::No);
            if (rc == QMessageBox::No) {
                return;
            }
        }
        if (removeDuplicates) {
            QList<GObjectRelation> oldRel = assObj->findRelatedObjectsByRole(role);
            foreach(const GObjectRelation& r, oldRel) {
                assObj->removeObjectRelation(r);
            }
        }
        assObj->addObjectRelation(selObj, role);
    }

    selectedObject = selObj;
    QDialog::accept();
}

CreateObjectRelationDialogController::~CreateObjectRelationDialogController()
{
    delete ui;
}
}//namespace
