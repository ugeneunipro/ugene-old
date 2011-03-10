#include "BaseDocumentFormatConfigurators.h"

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/Settings.h>
#include <U2Formats/GenbankPlainTextFormat.h>
#include <U2Formats/FastaFormat.h>
#include <U2Formats/EMBLPlainTextFormat.h>
#include <U2Formats/DocumentFormatUtils.h>

#include <ui/ui_FormatSettingsDialog.h>
#include <ui/ui_MultipartDocFormatConfiguratorWidget.h>

#include <QtGui/QGroupBox>

namespace U2 {


void BaseDocumentFormatConfigurators::initBuiltInConfigurators() {
    AppContext::getDocumentFormatConfigurators()->registerConfigurator(new MultiPartDocFormatConfigurator(BaseDocumentFormats::PLAIN_FASTA));
    AppContext::getDocumentFormatConfigurators()->registerConfigurator(new MultiPartDocFormatConfigurator(BaseDocumentFormats::PLAIN_GENBANK));
    AppContext::getDocumentFormatConfigurators()->registerConfigurator(new MultiPartDocFormatConfigurator(BaseDocumentFormats::PLAIN_EMBL));
    AppContext::getDocumentFormatConfigurators()->registerConfigurator(new MultiPartDocFormatConfigurator(BaseDocumentFormats::FASTQ));
}

#define SETTINGS_ROOT "format_settings/"

void BaseDocumentFormatConfigurators::loadDefaultFormatSettings(const DocumentFormatId& format, QVariantMap& formatSettings) {
    QVariant v = AppContext::getSettings()->getValue(SETTINGS_ROOT + format);
    if (v.type() != QVariant::Map) {
        return;
    }
    formatSettings = v.toMap();
}

void BaseDocumentFormatConfigurators::saveDefaultFormatSettings(const DocumentFormatId& format, const QVariantMap& formatSettings) {
    AppContext::getSettings()->setValue(SETTINGS_ROOT + format, formatSettings);
}


void MultiPartDocFormatConfigurator::configure(QVariantMap& info) {
    DocumentFormat* f = AppContext::getDocumentFormatRegistry()->getFormatById(formatId);
    
    // create GUI
    QDialog configDialog;
    Ui_FormatSettingsDialog dModel;
	configDialog.setWindowIcon(QIcon(":/ugene/images/ugene_16.png"));
    dModel.setupUi(&configDialog);

    Ui_MultipartDocFormatConfiguratorWidget wModel;
    QGroupBox * w = new QGroupBox();
    w->setTitle(tr("%1 format settings").arg(f->getFormatName()));
    wModel.setupUi(w);
    
    dModel.verticalLayout->insertWidget(0, w);

    // fill GUI with default settings
    BaseDocumentFormatConfigurators::loadDefaultFormatSettings(formatId, info);
    int mergeGap = -1;
    QString mergeToken = MERGE_MULTI_DOC_GAP_SIZE_SETTINGS;
    QVariant v  = info.value(mergeToken);    
    if (v.isValid()) {
        mergeGap = v.toInt();
    }
    if (mergeGap == -1) {
        wModel.separateRB->setChecked(true);
    } else {
        wModel.mergeRB->setChecked(true);
        wModel.gapSpin->setValue(mergeGap);
    }

    //execute dialog
    int r = configDialog.exec();
    if (r != QDialog::Accepted) {
        return;
    }
    //save settings to model
    if (wModel.separateRB->isChecked()) {
        info.clear();
    } else {
        info[mergeToken] = wModel.gapSpin->value();
    }
    //save settings to user-settings if needed
    if (dModel.saveCB->isChecked()) {
        BaseDocumentFormatConfigurators::saveDefaultFormatSettings(formatId, info);
    }

}


} //namespace
