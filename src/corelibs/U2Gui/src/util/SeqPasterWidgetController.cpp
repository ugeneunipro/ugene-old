/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include"SeqPasterWidgetController.h"
#include "ui/ui_SeqPasterWidget.h"

#include <U2Core/AppContext.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/DNAAlphabet.h>

namespace U2{

SeqPasterWidgetController::SeqPasterWidgetController( QWidget *p, const QByteArray& initText ): 
QWidget(p), preferred(0) 
{
    ui = new Ui_SeqPasterWidget;
    //TODO: add not null project checking
    ui->setupUi(this);
    if (!initText.isEmpty()) {
        ui->sequenceEdit->setPlainText(initText);
    }

    QList<DNAAlphabet*> alps = AppContext::getDNAAlphabetRegistry()->getRegisteredAlphabets();
    foreach(DNAAlphabet *a, alps){
        ui->alphabetBox->addItem(a->getName(), a->getId());
    }
    connect(ui->alphabetBox, SIGNAL(currentIndexChanged(const QString&)), SLOT(sl_currentindexChanged(const QString&)));
}
QByteArray SeqPasterWidgetController::getNormSequence(DNAAlphabet * alph, const QByteArray & seq, bool replace, QChar replaceChar) {
    assert(alph != NULL);
    QByteArray ret;
    if(alph->getId() == BaseDNAAlphabetIds::RAW()){
        foreach(QChar c, seq){
            QChar::Category cat = c.category();
            if((cat != QChar::Separator_Space) &&
                (cat != QChar::Other_Control)){
                    ret.append(c);
            }
        }
    }else{
        QByteArray alphabetSymbols = alph->getAlphabetChars(true);
        for(int i = 0; i < seq.size(); i++){
            QChar c = seq.at(i);
            if(alphabetSymbols.indexOf(c) > -1){
                ret.append(c);
            }else{
                if (replace){
                    ret.append(replaceChar);
                }
            }
        }
    }
    if(!alph->isCaseSensitive()){
        ret = ret.toUpper();
    }
    return ret;
}

QString SeqPasterWidgetController::validate(){
    if(ui->sequenceEdit->toPlainText().isEmpty()){
        return tr("Input sequence is empty");
    }
 
    QByteArray seq = ((ui->sequenceEdit->document())->toPlainText()).toUtf8();
    seq = QString(seq).remove(QRegExp("\\s")).toAscii();
    DNAAlphabet *alph = NULL;
    if(ui->groupBox->isChecked()){
        alph = U2AlphabetUtils::getById((ui->alphabetBox->itemData(ui->alphabetBox->currentIndex())).toString());
    }else{
        if (preferred != NULL){
            alph = preferred;
        } else {
            alph = U2AlphabetUtils::findBestAlphabet(seq);
        }
    }
    if(alph == NULL){
        return tr("Alphabet not detected");
    }

    bool replace = ui->replaceRB->isChecked();
    if(replace) {
        if(ui->symbolToReplaceEdit->text().isEmpty()){
            return tr("Replace symbol is empty");
        }
        char cc = ui->symbolToReplaceEdit->text().at(0).toAscii();
        if(!alph->contains(cc)){
            return tr("Replace symbol is not belongs to selected alphabet");
        }        
    }

    QChar replaceChar = replace ? ui->symbolToReplaceEdit->text().at(0) : QChar();
    QByteArray normSequence = getNormSequence(alph, seq, replace, replaceChar);
    resultSeq = DNASequence("New Sequence", normSequence, alph);
    return QString();
}

void SeqPasterWidgetController::sl_currentindexChanged( const QString& newText){
    DNAAlphabetRegistry *r = AppContext::getDNAAlphabetRegistry();
    if((r->findById(BaseDNAAlphabetIds::RAW()))->getName() == newText){
        ui->skipRB->setDisabled(true);
        ui->replaceRB->setDisabled(true);
        ui->symbolToReplaceEdit->setDisabled(true);
    }else{
        ui->skipRB->setEnabled(true);
        ui->replaceRB->setEnabled(true);
        ui->symbolToReplaceEdit->setEnabled(true);
    }
}

void SeqPasterWidgetController::disableCustomSettings(){
    ui->groupBox->setChecked(false);
    ui->groupBox->setDisabled(true);
	ui->groupBox->hide();
	//ui->groupBox->hide();
}

void SeqPasterWidgetController::setPreferredAlphabet( DNAAlphabet *alp ){
    preferred = alp;
}

SeqPasterWidgetController::~SeqPasterWidgetController()
{
    delete ui;
}

}//ns
