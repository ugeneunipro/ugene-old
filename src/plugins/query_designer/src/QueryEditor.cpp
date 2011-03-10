#include "QueryEditor.h"
#include "QueryProcCfgModel.h"
#include "QueryProcCfgDelegate.h"

#include "QueryViewController.h"
#include "QueryViewItems.h"

#include <U2Lang/Attribute.h>
#include <U2Lang/WorkflowUtils.h>

#include <QtGui/QValidator>


namespace U2 {

QValidator::State ProcessNameValidator::validate(QString& input, int& pos) const {
    //Q_UNUSED(input);
    if (input.contains(' ') || input.contains('-')) {
        return Invalid;
    }
    Q_UNUSED(pos);
    /*const QueryScene* scene = view->getScene();
    foreach(QGraphicsItem* item, scene->items()) {
        if (item->type()==QueryProcessItemType) {
            QuerySchemeItem* procItem = qgraphicsitem_cast<QuerySchemeItem*>(item);
            if(procItem->getProcess()->getNameLabel()==input) {
                QueryEditor* ed = qobject_cast<QueryEditor*>(parent());
                if(procItem->getProcess()->getNameLabel()!=ed->current->getNameLabel()) {
                    return Invalid;
                }
            }
        }
    }*/
    return Acceptable;
}

QueryEditor::QueryEditor(QWidget* parent/* =0 */) : QWidget(parent), current(NULL) {
    setupUi(this);
    caption->setMinimumHeight(nameEdit->sizeHint().height());

    QueryViewController* controller = qobject_cast<QueryViewController*>(parent);
    ProcessNameValidator* validator = new ProcessNameValidator(this, controller);
    nameEdit->setValidator(validator);
    directionCombo->insertItem(0, tr("Direct"));
    directionCombo->insertItem(1, tr("Complementary"));
    directionCombo->insertItem(2, tr("Any"));

    cfgModel = new QueryProcCfgModel(this);
    table->setModel(cfgModel);
    table->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->hide();
    table->verticalHeader()->setDefaultSectionSize(QFontMetrics(QFont()).height() + 6);
    table->setItemDelegate(new QueryProcCfgDelegate(this));
    table->setMinimumHeight(height()/2);

    reset();

    connect(table->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), SLOT(sl_showPropDoc()));
    connect(nameEdit, SIGNAL(editingFinished()), SLOT(sl_setLabel()));
    connect(keyEdit, SIGNAL(editingFinished()), SLOT(sl_setKey()));
    connect(directionCombo, SIGNAL(currentIndexChanged(int)), SLOT(sl_setDirection(int)));
    connect(cfgModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), SIGNAL(modified()));
}

void QueryEditor::setCurrentAttribute(const QString& id) {
    QModelIndex modelIndex = cfgModel->modelIndexById(id);

    QModelIndex prev = table->selectionModel()->currentIndex();

    if (modelIndex==prev) {
        table->selectionModel()->reset();
    }
    table->setCurrentIndex(modelIndex);
    //table->selectionModel()->setCurrentIndex(modelIndex, QItemSelectionModel::ClearAndSelect);
}

void QueryEditor::sl_setLabel() {
    if (current->getParameters()->getLabel()!=nameEdit->text()) {
        current->getParameters()->setLabel(nameEdit->text());
        emit modified();
    }
}

void QueryEditor::sl_setKey() {
    if (current->getParameters()->getAnnotationKey()!=keyEdit->text()) {
        current->getParameters()->setAnnotationKey(keyEdit->text());
        emit modified();
    }
}

void QueryEditor::sl_setDirection(int index) {
    QDStrandOption dir = QDStrandOption(index);
    if (current->getStrand()!=dir) {
        current->setStrand(dir);
        emit modified();
    }
}

void QueryEditor::sl_showPropDoc() {
    QModelIndex current = table->selectionModel()->currentIndex();
    if (current.isValid()) {
        propDoc->setText(WorkflowUtils::getRichDoc(current.data(DescriptorRole).value<Descriptor>()));
    } else {
        propDoc->setText("");
    }
}

void QueryEditor::setDescriptor(const Descriptor* d, const QString& hint) {
    QString text = d ? WorkflowUtils::getRichDoc(*d) + "<br><br>" + hint : hint;
    if (text.isEmpty()) {
        text = tr("Select an element to inspect.");
    }
    doc->setText(text);
}

void QueryEditor::showProto(QDActorPrototype* proto) {
    if (proto) {
        caption->setText(tr("Element name:"));
        caption->show();
        annotationLbl->setText(tr("Annotate as:"));
        annotationLbl->show();
        directionLbl->setText(tr("Strand:"));
        directionLbl->show();
        //nameEdit->setText(a->getParameters()->getLabel());
        nameEdit->show();
        nameEdit->setDisabled(true);
        //keyEdit->setText(a->getParameters()->getAnnotationKey());
        keyEdit->show();
        keyEdit->setDisabled(true);
        directionCombo->show();
        directionCombo->setDisabled(true);

        setDescriptor(&proto->getDescriptor(),
            tr("To configure the algorithm element parameters go to the \"Parameters\" area below."));
        cfgModel->setConfiguration(proto->getEditor(), proto->getParameters());
        tableSplitter->show();
        tableSplitter->setDisabled(true);
    } else {
        reset();
    }
}

void QueryEditor::edit(QDConstraint* constraint) {
    if(constraint) {
        caption->setText("");
        caption->hide();
        annotationLbl->setText("");
        annotationLbl->hide();
        directionLbl->setText(tr(""));
        directionLbl->hide();
        nameEdit->setText("");
        nameEdit->hide();
        keyEdit->setText("");
        keyEdit->hide();
        directionCombo->hide();
        doc->setText(tr("To configure the constraint element parameters go to the \"Parameters\" area below."));
        cfgModel->setConfiguration(constraint->getParameters());
        tableSplitter->show();
        tableSplitter->setDisabled(false);
    } else {
        reset();
    }
}

void QueryEditor::edit(QDActor* a) {
    current = a;
    if(a) {
        caption->setText(tr("Element name:"));
        caption->show();
        annotationLbl->setText(tr("Annotate as:"));
        annotationLbl->show();
        directionLbl->setText(tr("Strand:"));
        directionLbl->show();
        nameEdit->setText(a->getParameters()->getLabel());
        nameEdit->show();
        nameEdit->setDisabled(false);
        keyEdit->setText(a->getParameters()->getAnnotationKey());
        keyEdit->show();
        keyEdit->setDisabled(false);
        directionCombo->setCurrentIndex(a->getStrand());
        directionCombo->show();
        if (a->hasStrand()) {
            directionCombo->setDisabled(false);
        } else {
            directionCombo->setDisabled(true);
        }
        
        setDescriptor(&a->getProto()->getDescriptor(),
            tr("To configure the algorithm element parameters go to the \"Parameters\" area below."));
        cfgModel->setConfiguration(a->getParameters());
        tableSplitter->show();
        tableSplitter->setDisabled(false);
    } else {
        reset();
    }
}

void QueryEditor::reset() {
    caption->setText("");
    caption->hide();
    annotationLbl->setText("");
    annotationLbl->hide();
    directionLbl->setText(tr(""));
    directionLbl->hide();
    nameEdit->setText("");
    nameEdit->hide();
    keyEdit->setText("");
    keyEdit->hide();
    directionCombo->hide();
    setDescriptor(NULL);
    cfgModel->setConfiguration(NULL);
    tableSplitter->hide();
    propDoc->setText("");
}

}//namespace
