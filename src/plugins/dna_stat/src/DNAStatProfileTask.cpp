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

#include <U2Core/GObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/WebWindow.h>

#include "DNAStatProfileTask.h"

namespace U2 {


static QMap<char,double> createProteinMWMap(){
    QMap<char,double> mwMap;

    mwMap.insert('A', 89.09); // ALA
    mwMap.insert('R', 174.20); // ARG
    mwMap.insert('N', 132.12); // ASN
    mwMap.insert('D', 133.10); // ASP
    mwMap.insert('B', 132.61); // ASX
    mwMap.insert('C', 121.15); // CYS
    mwMap.insert('Q', 146.15); // GLN
    mwMap.insert('E', 147.13); // GLU
    mwMap.insert('Z', 146.64); // GLX
    mwMap.insert( 'G', 75.07); // GLY
    mwMap.insert( 'H', 155.16); // HIS
    mwMap.insert( 'I',  131.17); // ILE
    mwMap.insert( 'L', 131.17); // LEU
    mwMap.insert( 'K', 146.19); // LYS
    mwMap.insert( 'M', 149.21); // MET
    mwMap.insert( 'F', 165.19); // PHE
    mwMap.insert( 'P', 115.13); //PRO
    mwMap.insert( 'S', 105.09); // SER
    mwMap.insert( 'T', 119.12); // THR
    mwMap.insert( 'W', 204.23); // TRP
    mwMap.insert( 'Y', 181.19); // TYR
    mwMap.insert( 'V', 117.15); // VAL
    
    return mwMap;
}

static QMap<char,double> createPKAMap() {
    QMap<char,double> res;
    
    res.insert('D', 4.0);
    res.insert('C', 8.5);
    res.insert('E', 4.4);
    res.insert('Y', 10.0);
    res.insert('c', 3.1); // CTERM
    res.insert('R', 12.0);
    res.insert('H', 6.5);
    res.insert('K', 10.4);
    res.insert('n',8.0); // NTERM

    return res;
} 

static QMap<char,int> createChargeMap() {
    QMap<char,int> res;

    res.insert('D', -1);
    res.insert('C', -1);
    res.insert('E', -1);
    res.insert('Y', -1);
    res.insert('c', -1); // CTERM
    res.insert('R', 1);
    res.insert('H', 1);
    res.insert('K', 1);
    res.insert('n', 1); // NTERM

    return res;
} 


QMap<char,double> DNAStatProfileTask::pMWMap = createProteinMWMap();
QMap<char,double> DNAStatProfileTask::pKaMap = createPKAMap();
QMap<char,int> DNAStatProfileTask::pChargeMap = createChargeMap();

DNAStatProfileTask::DNAStatProfileTask(ADVSequenceObjectContext* context):Task(tr("Generate sequence statistics profile"), TaskFlag_None){
    ctx = context;
    contentCounter.resize(256);
    contentCounter.fill(0);
    nA = nC = nG = nT = 0;
}

void DNAStatProfileTask::run(){
    computeStats();
    QString seqName = ctx->getSequenceGObject()->getGObjectName(), url = ctx->getSequenceGObject()->getDocument()->getURL().getURLString();   
    U2SequenceObject* dnaObj = ctx->getSequenceObject();

    //setup style
    resultText= "<STYLE TYPE=\"text/css\"><!-- \n";
    resultText+="table.tbl   {\n border-width: 1px;\n border-style: solid;\n border-spacing: 0;\n border-collapse: collapse;\n}\n";
    resultText+="table.tbl td{\n max-width: 200px;\n min-width: 20px;\n text-align: center;\n border-width: 1px;\n ";
    resultText+="border-style: solid;\n margin:0px;\n padding: 0px;\n}\n";
    resultText+="--></STYLE>\n";

    //header
    resultText+="<h2>" + DNAStatProfileTask::tr("Sequence Statistics") + "</h2><br>\n";
    
    resultText+="<table>\n";
    resultText+="<tr><td><b>" + DNAStatProfileTask::tr("Sequence file:") + "</b></td><td>" + url + "@" + seqName + "</td></tr><tr><td><b>" + 
        DNAStatProfileTask::tr("Sequence length:") + "</b></td><td>" + QString::number(seqLen) + "</td></tr>\n";
    resultText += "<tr><td><b>Molecule Type:</b></td><td>" + ctx->getSequenceObject()->getAlphabet()->getName() + "</td></tr>\n";

    DNAAlphabet* al = dnaObj->getAlphabet();
    if (al->isNucleic()) {

        float gcContent = 100.0 * (nG + nC) / (float) seqLen;
        resultText += "<tr><td><b>GC content:</b></td><td>" + QString("%1 %").arg(gcContent, 0, 'f', 2) + "</td></tr>\n";

        // Calculating molar weight
        // Source: http://www.basic.northwestern.edu/biotools/oligocalc.html
        bool isRna = al->getId() == BaseDNAAlphabetIds::NUCL_RNA_DEFAULT() ||
            al->getId() == BaseDNAAlphabetIds::NUCL_RNA_EXTENDED();

        float molarWeight = 0;
        if (isRna) {
            molarWeight = nA * 329.21 + nT * 306.17 + nC * 305.18 + nG * 345.21 + 159.0;
        } else {
            molarWeight = nA * 313.21 + nT * 304.2 + nC * 289.18 + nG * 329.21 + 17.04;
        }

        int molarAbsCoef = nA*15400 + nT*8800 + nC*7300 + nG*11700;

        float meltingTm = 0;
        if (seqLen < 15) {
            meltingTm = (nA+nT) * 2 + (nG + nC) * 4;
        } else {
            meltingTm = 64.9 + 41*(nG + nC-16.4)/(float)(nA+nT+nG+nC);
        }

        resultText += "<tr><td><b>Molar Weight:</b></td><td>" + QString("%1 Da").arg(molarWeight, 0, 'f', 2) + "</td></tr>\n";
        resultText += "<tr><td><b>Molar ext. coef.:</b></td><td>" + QString("%1 I/mol (at 260 nm)").arg(molarAbsCoef) + "</td></tr>\n";
        resultText += "<tr><td><b>Melting Tm:</b></td><td>" + QString("%1 C (at salt CC 50 mM, primer CC 50 mM, pH 7.0)").arg(meltingTm, 0, 'f', 2) + "</td></tr>\n";
    } else if (al->isAmino()) {
        
        // it's ok to use whole sequence since the proteins in 95% cases are not bigger than 2000 residues
        QByteArray seq = dnaObj->getWholeSequence().constSequence();
    
        // Source: http://code.google.com/p/pdb-tools/
        
        // calculate molecular weight
        
        static const double MWH2O = 18.0;
        double mw = 0;
        for (int i = 0; i < seqLen; ++i ) {
            mw += pMWMap.value( seq.at(i) );
        }
        mw = mw - (seqLen - 1)*MWH2O;
        resultText += "<tr><td><b>Molecular Weight:</b></td><td>" + QString("%1").arg(mw, 0, 'f', 2) + "</td></tr>\n";
       
        // calculate pI
        
        double pI = calcPi(seq);
        resultText += "<tr><td><b>Isoelectric Point (pI):</b></td><td>" + QString("%1").arg(pI, 0, 'f', 2) + "</td></tr>\n";

        // calculate  groups

    }

    resultText+="</table>\n";
    resultText+="<br><br>\n";

    resultText+="<table class=tbl>";

    resultText+="<table class=tbl>";
    resultText+="<tr><td></td><td>" + 
        DNAStatProfileTask::tr("Symbol counts") + "</td><td>" +
        DNAStatProfileTask::tr("Symbol percents %") + "</td></tr>";

    for (int i = 0; i < 256; i++) {
        char symbol = char(i);
        qint64 cnt = contentCounter[i];
        if (cnt == 0) {
            continue;
        }
        float percentage = cnt/(float)seqLen * 100;
        resultText+=QString("<tr><td><b>%1</b></td><td>%2</td><td>%3</td></tr>").arg(symbol).arg(QString::number(cnt)).arg(QString::number(percentage, 'g', 4));
    }
    resultText+= "</table>\n";
    if(!diNuclCounter.isEmpty()){
        resultText+="<br>";
        resultText+="<table class=tbl>";
        resultText+="<tr><td></td><td>" + 
            DNAStatProfileTask::tr("Dinucleotide counts") + "</td><td>" +
            DNAStatProfileTask::tr("Dinucleotide percents %") + "</td></tr>";
        QMap<QByteArray, int>::const_iterator it(diNuclCounter.begin());
        for(;it != diNuclCounter.end(); it++){
            const QByteArray diNucl = it.key();
            const int cnt = it.value();
            float percentage = cnt/(float)seqLen * 100;
            resultText+=QString("<tr><td><b>%1</b></td><td>%2</td><td>%3</td></tr>").arg(QString(diNucl)).arg(QString::number(cnt)).arg(QString::number(percentage, 'g', 4));
        }
        resultText+= "</table>\n";
    }
}

Task::ReportResult DNAStatProfileTask::report(){
    assert(!resultText.isEmpty());
    
    return ReportResult_Finished;
}

void DNAStatProfileTask::computeStats(){
    seqLen = ctx->getSequenceLength();
    QByteArray alphabetChars = ctx->getAlphabet()->getAlphabetChars();
    
    U2Region wholeSeqReg(0, ctx->getSequenceLength());
    qint64 blockSize = 1024*1024;
    qint64 prevEnd = 0;
    bool dinucl = ctx->getAlphabet()->getId() == BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
    QByteArray nucPair = "NN";

    do {
        U2Region r = wholeSeqReg.intersect(U2Region(prevEnd, blockSize));
        prevEnd += blockSize;
        QByteArray seqBlock = ctx->getSequenceData(r);
        foreach(char c, seqBlock){
            if (c == 'A') {
                nA++;
            } else if (c == 'G') {
                nG++;
            } else if (c == 'T' || c == 'U') {
                nT++;
            } else if (c == 'C') {
                nC++;
            }
            contentCounter[uchar(c)]++;
            if (!dinucl) {
                continue;
            }
            nucPair[0] = nucPair[1];
            nucPair[1] = c;
            if (!nucPair.contains("-") && !nucPair.contains("N")) {
                continue;
            }
            if (diNuclCounter.contains(nucPair)){
                diNuclCounter[nucPair]++;
            } else {
                diNuclCounter.insert(nucPair, 1);
            }
        }
    } while (prevEnd < wholeSeqReg.endPos());

        
}

double DNAStatProfileTask::calcPi( const QByteArray& seq )
{
    QMap<char,int> countMap;
    for (int i = 0; i < seqLen; ++i) {
        char r = seq.at(i);
        if ( pKaMap.contains( r ) ) {
            countMap[r]++;
        }
    }
    countMap['c'] = 1;
    countMap['n'] = 1;
    
    static const double CUTOFF = 0.001;
    static const double INITIAL_CUTOFF = 2.0;

    double step = INITIAL_CUTOFF;
    double pH = 0;
    while (step > CUTOFF) {
        if ( calcChargeState(countMap,pH) > 0 ){
            pH += step;
        } else {
            step *= 0.5;
            pH -= step;
        }
    }

    return pH;

}

double DNAStatProfileTask::calcChargeState(const QMap<char,int>& countMap, double pH )
{
    double chargeState = 0.;
    QList<char> counts = countMap.keys();
    foreach(char r, counts) {
        double pKa = pKaMap.value(r);
        double charge = pChargeMap.value(r);
        chargeState += countMap.value(r)*charge/( 1 + pow(10.0, charge*(pH-pKa)) );
    } 

    return chargeState;
}

} //namespace

