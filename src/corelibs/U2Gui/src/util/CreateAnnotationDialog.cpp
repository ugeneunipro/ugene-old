/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "CreateAnnotationDialog.h"
#include "CreateAnnotationWidgetController.h"

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/GObjectRelationRoles.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#else
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#endif

#include <U2Gui/HelpButton.h>


namespace U2 {

CreateAnnotationDialog::CreateAnnotationDialog(QWidget* p, CreateAnnotationModel& m) : QDialog(p), model(m){
    annWidgetController = new CreateAnnotationWidgetController(m, this);
    
    this->setObjectName("new_annotation_dialog");

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch(1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    buttonsLayout->addWidget(buttonBox);
    buttonBox->setObjectName("buttonBox");

    createButton = buttonBox->button(QDialogButtonBox::Ok);
    cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

    new HelpButton(this, buttonBox, "14058955");
    createButton->setText(tr("Create"));
    cancelButton->setText(tr("Cancel"));

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
    setWindowTitle(tr("Create Annotation"));
}

void CreateAnnotationDialog::sl_onCreateClicked(bool) {
    QString err = annWidgetController->validate();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), err);
        return;
    } 
    bool objectPrepared = annWidgetController->prepareAnnotationObject();
    if (!objectPrepared){
        QMessageBox::warning(this, tr("Error"), tr("Cannot create an annotation object. Please check settings"));
        return;
    }
    model = annWidgetController->getModel();
    accept();
}

void CreateAnnotationDialog::sl_onCancelClicked(bool) {
    reject();
}



} // namespace
