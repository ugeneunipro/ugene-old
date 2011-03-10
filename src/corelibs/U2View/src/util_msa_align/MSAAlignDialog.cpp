#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QKeyEvent>

#include <U2Algorithm/MSAAlignAlgRegistry.h>
#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Misc/DialogUtils.h>

#include "MSAAlignDialog.h"
#include "MSAAlignGUIExtension.h"

namespace U2 {

MSAAlignDialog::MSAAlignDialog(const QString& _algorithmName, QWidget* p)
    : QDialog(p), algorithmName(_algorithmName), customGUI(NULL), algoEnv(NULL)
{
    setupUi(this);        
    addGuiExtension();        
    connect(selectFileNameButton, SIGNAL(clicked()), SLOT(sl_onFileNameButtonClicked()));
}

void MSAAlignDialog::setupUi(QDialog *msaAlignDialog) {
        
    if (msaAlignDialog->objectName().isEmpty()) {
        msaAlignDialog->setObjectName(QString::fromUtf8("MSAAlignDialog"));
    }
    msaAlignDialog->resize(430, 130);
    msaAlignDialog->setMinimumSize(QSize(430, 130));
    verticalLayout = new QVBoxLayout(msaAlignDialog);
    fileSelectBox = new QGroupBox(msaAlignDialog);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(fileSelectBox->sizePolicy().hasHeightForWidth());
    fileSelectBox->setSizePolicy(sizePolicy);
    horizontalLayout = new QHBoxLayout(fileSelectBox);
    resultFileNameEdit = new QLineEdit(fileSelectBox);
    horizontalLayout->addWidget(resultFileNameEdit);

    selectFileNameButton = new QPushButton(fileSelectBox);
    QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(selectFileNameButton->sizePolicy().hasHeightForWidth());
    selectFileNameButton->setSizePolicy(sizePolicy1);
    selectFileNameButton->setMinimumSize(QSize(25, 19));
    selectFileNameButton->setMaximumSize(QSize(25, 19));

    horizontalLayout->addWidget(selectFileNameButton);

    horizontalLayout->setStretch(0, 1);

    verticalLayout->addWidget(fileSelectBox);

    verticalSpacer = new QSpacerItem(20, 15, QSizePolicy::Minimum, QSizePolicy::Fixed);

    verticalLayout->addItem(verticalSpacer);

    buttonFrame = new QFrame(msaAlignDialog);
    sizePolicy.setHeightForWidth(buttonFrame->sizePolicy().hasHeightForWidth());
    buttonFrame->setSizePolicy(sizePolicy);
    horizontalLayout_2 = new QHBoxLayout(buttonFrame);
    horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_2->addItem(horizontalSpacer);
    alignButton = new QPushButton(buttonFrame);
    horizontalLayout_2->addWidget(alignButton);
    cancelButton = new QPushButton(buttonFrame);
    horizontalLayout_2->addWidget(cancelButton);

    verticalLayout->addWidget(buttonFrame);

    msaAlignDialog->setWindowTitle(tr("Align MSA"));
    fileSelectBox->setTitle(tr("Select input file"));
    selectFileNameButton->setText(tr("..."));
    alignButton->setText(tr("Align"));
    cancelButton->setText(tr("Cancel"));

    QObject::connect(alignButton, SIGNAL(clicked()), msaAlignDialog, SLOT(accept()));
    QObject::connect(cancelButton, SIGNAL(clicked()), msaAlignDialog, SLOT(reject()));
}

void MSAAlignDialog::sl_onFileNameButtonClicked() {
    LastOpenDirHelper lod;
    lod.url = QFileDialog::getOpenFileName(this, tr("Set result alignment file name"), lod.dir, 
        DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, true));
    if (!lod.url.isEmpty()) {
        GUrl result = lod.url;
        if (result.lastFileSuffix().isEmpty()) {
            result = QString( "%1.srfa" ).arg( result.getURLString() );
        }
        resultFileNameEdit->setText(result.getURLString());
    }
}

const QString MSAAlignDialog::getResultFileName() {
    return resultFileNameEdit->text();
}

const QString MSAAlignDialog::getAlgorithmName() {
    return algorithmName;
}

QMap<QString, QVariant> MSAAlignDialog::getCustomSettings() {
    if (customGUI != NULL) {
        return customGUI->getMSAAlignCustomSettings();
    } else {
        return QMap<QString, QVariant>();
    }
}

void MSAAlignDialog::addGuiExtension() {
    static const int insertPos = verticalLayout->count() - 2;

    // cleanup previous extension
    if (customGUI != NULL) {
        layout()->removeWidget(customGUI);         
        setMinimumHeight(minimumHeight() - customGUI->minimumHeight());
        delete customGUI;
        customGUI = NULL;
    }

    MSAAlignAlgRegistry* registry = AppContext::getMSAAlignAlgRegistry();

    // insert new extension widget
    MSAAlignAlgorithmEnv* env = registry->getAlgorithm(algorithmName);
    if (env == NULL) {
        adjustSize();
        return;
    }
    MSAAlignGUIExtensionsFactory* gui = env->getGUIExtFactory();
    if (gui!=NULL && gui->hasMainWidget()) {
        customGUI = gui->createMainWidget(this);
        int extensionMinWidth = customGUI->sizeHint().width();
        int extensionMinHeight = customGUI->sizeHint().height();
        customGUI->setMinimumWidth(extensionMinWidth);
        customGUI->setMinimumHeight(extensionMinHeight);
        verticalLayout->insertWidget(insertPos, customGUI);
        // adjust sizes
        setMinimumHeight(customGUI->minimumHeight() + minimumHeight());
        if (minimumWidth() < customGUI->minimumWidth()) {
            setMinimumWidth(customGUI->minimumWidth());
        };
        if (!customGUI->windowTitle().isEmpty()) {
            setWindowTitle(customGUI->windowTitle());
        }
        customGUI->show();
    } 
    adjustSize();
}

void MSAAlignDialog::accept() {    
    if (resultFileNameEdit->text().isEmpty() ) {
        QMessageBox::information(this, tr("MSA Align"),
            tr("Result alignment file name is not set!") );
    } else {
        QDialog::accept();
    }
}

} // U2
