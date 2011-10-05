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

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>

#include <U2Core/GObjectReference.h>
#include <U2Core/GObjectRelationRoles.h>

#include "SendSelectionDialog.h"



namespace U2 {

#define SETTINGS_ROOT           QString("remote_blast_plugin/")
#define SHORT_SETTINGS          QString("short")
#define EXPECT_SETTINGS         QString("espect_value")
#define HITS_SETTINGS           QString("max_hits")
#define LOW_COMPLEX_SETTINGS    QString("low_complexity_filter")
#define REPEATS_SETTINGS        QString("human_repeats_filter")
#define LOOKUP_SETTINGS         QString("lookup_mask")
#define LOWCASE_SETTINGS        QString("lowcase_mask")
#define RETRY_SETTINGS          QString("retry")
#define FILTER_SETTINGS         QString("filter")

void SendSelectionDialog::setUpSettings() {
    Settings *s = AppContext::getSettings();
    shortSequenceCheckBox->setChecked(s->getValue(SETTINGS_ROOT + SHORT_SETTINGS, false).toBool());
    evalueSpinBox->setValue(s->getValue(SETTINGS_ROOT + EXPECT_SETTINGS, 10).toDouble());
    quantitySpinBox->setValue(s->getValue(SETTINGS_ROOT + HITS_SETTINGS, 20).toInt());
    lowComplexityFilterCheckBox->setChecked(s->getValue(SETTINGS_ROOT + LOW_COMPLEX_SETTINGS, true).toBool());
    repeatsCheckBox->setChecked(s->getValue(SETTINGS_ROOT + REPEATS_SETTINGS, false).toBool());
    lookupMaskCheckBox->setChecked(s->getValue(SETTINGS_ROOT + LOOKUP_SETTINGS, false).toBool());
    lowerCaseCheckBox->setChecked(s->getValue(SETTINGS_ROOT + LOWCASE_SETTINGS, false).toBool());
    retrySpinBox->setValue(s->getValue(SETTINGS_ROOT + RETRY_SETTINGS, 2).toInt());
    evalueRadioButton->setChecked(s->getValue(SETTINGS_ROOT + FILTER_SETTINGS, true).toBool());
    scoreRadioButton->setChecked(!s->getValue(SETTINGS_ROOT + FILTER_SETTINGS, true).toBool());
}

void SendSelectionDialog::saveSettings() {
    Settings *s = AppContext::getSettings();
    s->setValue(SETTINGS_ROOT + SHORT_SETTINGS, shortSequenceCheckBox->isChecked());
    s->setValue(SETTINGS_ROOT + EXPECT_SETTINGS, evalueSpinBox->value());
    s->setValue(SETTINGS_ROOT + HITS_SETTINGS, quantitySpinBox->value());
    s->setValue(SETTINGS_ROOT + LOW_COMPLEX_SETTINGS, lowComplexityFilterCheckBox->isChecked());
    s->setValue(SETTINGS_ROOT + REPEATS_SETTINGS, repeatsCheckBox->isChecked());
    s->setValue(SETTINGS_ROOT + LOOKUP_SETTINGS, lookupMaskCheckBox->isChecked());
    s->setValue(SETTINGS_ROOT + LOWCASE_SETTINGS, lowerCaseCheckBox->isChecked());
    s->setValue(SETTINGS_ROOT + RETRY_SETTINGS, retrySpinBox->value());
    s->setValue(SETTINGS_ROOT + FILTER_SETTINGS, evalueRadioButton->isChecked());
}

void SendSelectionDialog::alignComboBoxes() {
    int count = dataBase->count();
    dataBase->setEditable(true);
    dataBase->lineEdit()->setReadOnly(true);
    dataBase->lineEdit()->setAlignment(Qt::AlignRight);
    for(int i = 0; i < count; i++) {
        dataBase->setItemData(i,Qt::AlignRight, Qt::TextAlignmentRole);
    }

    count = wordSizeComboBox->count();
    wordSizeComboBox->setEditable(true);
    wordSizeComboBox->lineEdit()->setReadOnly(true);
    wordSizeComboBox->lineEdit()->setAlignment(Qt::AlignRight);
    for(int i = 0; i < count; i++) {
        wordSizeComboBox->setItemData(i,Qt::AlignRight, Qt::TextAlignmentRole);
    }

    count = scoresComboBox->count();
    scoresComboBox->setEditable(true);
    scoresComboBox->lineEdit()->setReadOnly(true);
    scoresComboBox->lineEdit()->setAlignment(Qt::AlignRight);
    for(int i = 0; i < count; i++) {
        scoresComboBox->setItemData(i,Qt::AlignRight, Qt::TextAlignmentRole);
    }

    count = costsComboBox->count();
    costsComboBox->setEditable(true);
    costsComboBox->lineEdit()->setReadOnly(true);
    costsComboBox->lineEdit()->setAlignment(Qt::AlignRight);
    for(int i = 0; i < count; i++) {
        costsComboBox->setItemData(i,Qt::AlignRight, Qt::TextAlignmentRole);
    }

    count = matrixComboBox->count();
    matrixComboBox->setEditable(true);
    matrixComboBox->lineEdit()->setReadOnly(true);
    matrixComboBox->lineEdit()->setAlignment(Qt::AlignRight);
    for(int i = 0; i < count; i++) {
        matrixComboBox->setItemData(i,Qt::AlignRight, Qt::TextAlignmentRole);
    }

    count = serviceComboBox->count();
    serviceComboBox->setEditable(true);
    serviceComboBox->lineEdit()->setReadOnly(true);
    serviceComboBox->lineEdit()->setAlignment(Qt::AlignRight);
    for(int i = 0; i < count; i++) {
        serviceComboBox->setItemData(i,Qt::AlignRight, Qt::TextAlignmentRole);
    }
}

SendSelectionDialog::SendSelectionDialog(const U2SequenceObject* dnaso, bool _isAminoSeq, QWidget *p):QDialog(p), translateToAmino(false), isAminoSeq(_isAminoSeq), extImported(false) {
    CreateAnnotationModel ca_m;
    ca_m.data->name = "misc_feature";
    ca_m.hideAnnotationName = true;
    ca_m.hideLocation = true;
    ca_m.sequenceObjectRef = GObjectReference(dnaso);
    ca_m.sequenceLen = dnaso->getSequenceLength();
    ca_c = new CreateAnnotationWidgetController(ca_m, this);
    setupUi(this);
    int idx = 2;
    QWidget *wdgt;
    wdgt = ca_c->getWidget();
    wdgt->setMinimumHeight(150);
    verticalLayout_4->insertWidget(idx, wdgt);

    matrixComboBox->addItems(ParametersLists::blastp_matrix);
    matrixComboBox->hide();
    label_10->hide();
    label_12->hide();
    phiPatternEdit->hide();
    label_7->hide();
    textEdit->setMaximumHeight(100);

    setupDataBaseList();
    setUpSettings();
    megablastCheckBox->setEnabled(false);
    alignComboBoxes();

    connect( dataBase, SIGNAL(currentIndexChanged(int)), SLOT(sl_scriptSelected(int)) );
    connect( okButton, SIGNAL(clicked()), SLOT(sl_OK()) );
    connect( cancelButton, SIGNAL(clicked()), SLOT(sl_Cancel()) );
    connect( megablastCheckBox, SIGNAL(stateChanged(int)),SLOT(sl_megablastChecked(int)) );
    connect( serviceComboBox,SIGNAL(currentIndexChanged(int)),SLOT(sl_serviceChanged(int)) );
    sl_scriptSelected( 0 );
}

void SendSelectionDialog::sl_serviceChanged(int) {
    if(serviceComboBox->currentText()=="phi") {
        label_12->show();
        phiPatternEdit->show();
    }
    else {
        label_12->hide();
        phiPatternEdit->hide();
    }
}

void SendSelectionDialog::sl_megablastChecked(int state) {
    if(state==Qt::Checked) {
        wordSizeComboBox->clear();
        wordSizeComboBox->addItems(ParametersLists::megablast_wordSize);
        wordSizeComboBox->setCurrentIndex(3);
    }
    else {
        wordSizeComboBox->clear();
        wordSizeComboBox->addItems(ParametersLists::blastn_wordSize);
    }
}

QString SendSelectionDialog::getGroupName() const {
    return ca_c->getModel().groupName;
}

const CreateAnnotationModel *SendSelectionDialog::getModel() const {
    return &(ca_c->getModel());
}

AnnotationTableObject* SendSelectionDialog::getAnnotationObject() const {
    if(ca_c->isNewObject()) {
        AnnotationTableObject* aobj = new AnnotationTableObject("Annotations");
        aobj->addObjectRelation(GObjectRelation(ca_c->getModel().sequenceObjectRef, GObjectRelationRole::SEQUENCE));
        return aobj;
    }
    else {
        ca_c->prepareAnnotationObject();
        return ca_c->getModel().getAnnotationObject();
    }
    
}

QString SendSelectionDialog::getUrl() const{
    return ca_c->getModel().newDocUrl;
}



void SendSelectionDialog::setupDataBaseList() {
    //cannot analyze amino sequences using nucleotide databases
    if( isAminoSeq ) {
        dataBase->removeItem(0);
    }
}

void SendSelectionDialog::sl_scriptSelected( int index ) {
    Q_UNUSED(index);
    //this->adjustSize();
    QString descr = "";
    if(dataBase->currentText()=="cdd") {
        optionsTab->setTabEnabled(1,0);
        descr.append(CDD_DESCRIPTION);
        shortSequenceCheckBox->setEnabled(false);
        megablastCheckBox->setEnabled(false);
        matrixComboBox->hide();
        label_10->hide();
        dbComboBox->clear();
        dbComboBox->addItems(ParametersLists::cdd_dataBase);
    }
    else {
        optionsTab->setTabEnabled(1,1);
        descr.append(BLAST_DESCRIPTION);
        shortSequenceCheckBox->setEnabled(true);
        megablastCheckBox->setEnabled(true);
        if(dataBase->currentText()=="blastn") {

            phiPatternEdit->hide();
            label_12->hide();

            megablastCheckBox->setEnabled(true);

            wordSizeComboBox->clear();
            wordSizeComboBox->addItems(ParametersLists::blastn_wordSize);
            wordSizeComboBox->setCurrentIndex(1);

            costsComboBox->clear();
            costsComboBox->addItems(ParametersLists::blastn_gapCost);
            costsComboBox->setCurrentIndex(4);

            scoresComboBox->clear();
            scoresComboBox->addItems(ParametersLists::blastn_scores);
            scoresComboBox->setCurrentIndex(3);
            scoresComboBox->show();
            label_5->show();

            dbComboBox->clear();
            dbComboBox->addItems(ParametersLists::blastn_dataBase);
            dbComboBox->setCurrentIndex(2);

            matrixComboBox->hide();
            label_10->hide();

            serviceComboBox->hide();
            label_7->hide();
        }
        else {
            megablastCheckBox->setEnabled(false);

            wordSizeComboBox->clear();
            wordSizeComboBox->addItems(ParametersLists::blastp_wordSize);
            wordSizeComboBox->setCurrentIndex(1);

            costsComboBox->clear();
            costsComboBox->addItems(ParametersLists::blastp_gapCost);
            costsComboBox->setCurrentIndex(4);
            
            dbComboBox->clear();
            dbComboBox->addItems(ParametersLists::blastp_dataBase);
            
            matrixComboBox->show();
            matrixComboBox->setCurrentIndex(3);
            label_10->show();
            
            scoresComboBox->hide();
            label_5->hide();
            
            serviceComboBox->show();
            label_7->show();
        }
    }
    textEdit->setPlainText(descr);
    alignComboBoxes();
}

void SendSelectionDialog::sl_OK() {
    QString error = ca_c->validate();
    if (!error.isEmpty()) {
        QMessageBox::critical(NULL, tr("Error"), error);
        return;
    }
    retries = retrySpinBox->value();
    db = dataBase->currentText();
    if(db!="cdd") {
        requestParameters = "CMD=Put";
        addParametr(requestParameters,ReqParams::program,db);

        double eValue = evalueSpinBox->value();
        if(shortSequenceCheckBox->isChecked())
            eValue = 1000;
        addParametr(requestParameters,ReqParams::expect,eValue);    

        int maxHit = quantitySpinBox->value();
        addParametr(requestParameters,ReqParams::hits,maxHit);

        if(megablastCheckBox->isChecked()) {
            addParametr(requestParameters,ReqParams::megablast,"yes");
        }
        
        addParametr(requestParameters,ReqParams::database,dbComboBox->currentText().split(" ").last());

        QString filter="";
        if(lowComplexityFilterCheckBox->isChecked()&&!shortSequenceCheckBox->isChecked()) {
            filter.append("L");
        }
        if(repeatsCheckBox->isChecked()) {
            filter.append("R");
        }
        if(lookupMaskCheckBox->isChecked()) {
            filter.append("m");
        }
        if(!filter.isEmpty()) {
            addParametr(requestParameters,ReqParams::filter,filter);
        }

        addParametr(requestParameters,ReqParams::gapCost,costsComboBox->currentText());
        addParametr(requestParameters,ReqParams::matchScore,scoresComboBox->currentText().split(" ").first());
        addParametr(requestParameters,ReqParams::mismatchScore,scoresComboBox->currentText().split(" ").last());
        
        if(shortSequenceCheckBox->isChecked()) {
            QString wordSize = wordSizeComboBox->currentText().toInt()>7 ? "7" : wordSizeComboBox->currentText();
            addParametr(requestParameters,ReqParams::wordSize, wordSize);
        }
        else {
            addParametr(requestParameters,ReqParams::wordSize,wordSizeComboBox->currentText());
        }
        
        if(lowerCaseCheckBox->isChecked()) {
            addParametr(requestParameters,ReqParams::lowCaseMask,"yes");
        }

        if(db=="blastp") {
            if(!isAminoSeq) {
                translateToAmino = true;
            }

            addParametr(requestParameters,ReqParams::matrix,matrixComboBox->currentText());
            addParametr(requestParameters,ReqParams::service,serviceComboBox->currentText());
            if(serviceComboBox->currentText()=="phi") {
                addParametr(requestParameters,ReqParams::phiPattern,phiPatternEdit->text());
            }
        }
    }

    else { //CDD
        if(!isAminoSeq) {
            translateToAmino = true;
        }

        requestParameters = "filter=true";
        addParametr(requestParameters,ReqParams::cdd_db,dbComboBox->currentText());
        addParametr(requestParameters,ReqParams::cdd_hits,quantitySpinBox->value());
        addParametr(requestParameters,ReqParams::cdd_eValue,evalueSpinBox->value());    
    }

    if(translateToAmino) {
        QMessageBox msg(this);
        msg.setText(tr("You chose to search nucleotide sequence in protein database. This sequence will be converted into 6 sequences(3 translations for both strands)."
            "Therefore this search may take some time. Continue?"));
        msg.setWindowTitle(windowTitle());
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msg.setDefaultButton(QMessageBox::Yes);
        if(msg.exec() == QMessageBox::Cancel) {
            return;
        }
    }

    int filterResults = 0;
    filterResults |= (int)accessionCheckBox->isChecked();
    filterResults |= (int)defCheckBox->isChecked() << 1;
    filterResults |= (int)idCheckBox->isChecked() << 2;
    cfg.filterResult = filterResults;
    cfg.useEval = evalueRadioButton->isChecked();
    cfg.retries = retrySpinBox->value();
    cfg.params = requestParameters;
    cfg.dbChoosen = db;

    saveSettings();
    accept();
}

void SendSelectionDialog::sl_Cancel() {
    reject();
}

}
