#include "CreateAnnotationDialog.h"
#include "CreateAnnotationWidgetController.h"

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/GObjectRelationRoles.h>


#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>

namespace U2 {

CreateAnnotationDialog::CreateAnnotationDialog(QWidget* p, CreateAnnotationModel& m) : QDialog(p), model(m){
    annWidgetController = new CreateAnnotationWidgetController(m, this);
    
    this->setObjectName("new_annotation_dialog");

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch(1);
    createButton = new QPushButton(tr("Create"), this);
    createButton->setObjectName("create_button");
    buttonsLayout->addWidget(createButton);
    cancelButton = new QPushButton(tr("Cancel"), this);
    cancelButton->setObjectName("cancel_button");
    buttonsLayout->addWidget(cancelButton);
    
    QVBoxLayout* topLayout = new QVBoxLayout();
    topLayout->setObjectName("new_annotation_dialog_layout");
    
    QWidget * annWidget = annWidgetController->getWidget();
    topLayout->addWidget(annWidget);
    annWidget->setMinimumSize(annWidget->layout()->minimumSize());
    annWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    topLayout->addLayout(buttonsLayout);
    setLayout(topLayout);
    setMaximumHeight(layout()->minimumSize().height());
    
    connect(createButton, SIGNAL(clicked(bool)), SLOT(sl_onCreateClicked(bool)));
    connect(cancelButton, SIGNAL(clicked(bool)), SLOT(sl_onCancelClicked(bool)));
    
    annWidgetController->setFocusToNameEdit();
    setWindowTitle(tr("Create annotation"));
}

void CreateAnnotationDialog::sl_onCreateClicked(bool) {
    QString err = annWidgetController->validate();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), err);
        return;
    } 
    annWidgetController->prepareAnnotationObject();
    model = annWidgetController->getModel();
    accept();
}

void CreateAnnotationDialog::sl_onCancelClicked(bool) {
    reject();
}



} // namespace
