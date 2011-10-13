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
#include "SeqBootAdapter.h"
#include "U2Core/global.h"
#include <U2Core/DNAAlphabet.h>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>

#include <QtCore/QByteArray>
#include <QtCore/QSharedData>
#include <QtCore/QTime>

#include <iostream>
#include <float.h>

namespace U2{

SeqBoot::SeqBoot() : malignment(NULL) {
    seqLen = 0;
    seqRowCount = 0;
}

SeqBoot::~SeqBoot(){
    clearGenratedSequences();
}

void SeqBoot::clearGenratedSequences(){
    for(int i = 0; i < generatedSeq.size(); i++){
            delete generatedSeq[i];
    }
    generatedSeq.clear();
    
}

QString SeqBoot::getTmpFileTemplate(){
    //QString path = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath("phylip");
    QString path = AppContext::getAppSettings()->getUserAppsSettings()->getUserTemporaryDirPath();
    if(path.isEmpty()){
        return path;
    }else{
        path+="/bootstrXXXXXX";
        return path;
    }
}


void SeqBoot::initGenerSeq(int reps, int rowC, int seqLen){
    generatedSeq = QVector<MAlignment*>(reps);
    this->seqLen = seqLen;
    seqRowCount = rowC;
        
    for(int i = 0; i < reps; i++){
        generatedSeq[i] = new MAlignment(QString("bootstrap %1").arg(reps), malignment->getAlphabet());
    }
    
}

const MAlignment& SeqBoot::getMSA(int pos) const {
    return *generatedSeq[pos];

}

void SeqBoot::generateSequencesFromAlignment( const MAlignment& ma, const CreatePhyTreeSettings& settings ){
    if(!settings.bootstrap){
        return ;
    }

    malignment = &ma;
    int replicates = settings.replicates;
    
    seqboot_getoptions();
    
    reps = replicates;

    spp = ma.getNumRows();
    sites = ma.getLength();

    initGenerSeq(replicates, spp, sites);
    loci = sites;
    maxalleles = 1;

    DNAAlphabetType alphabetType = ma.getAlphabet()->getType();

    seq_allocrest();
    seq_inputoptions();

    nodep_boot = matrix_char_new(spp, sites);
    for (int k=0; k<spp; k++){
        for(int j=0; j<sites; j++) {
            const MAlignmentRow& rowK = ma.getRow(k);
            nodep_boot[k][j] = rowK.chatAt(j);
        }
    }

    long inseed = settings.seed;
    inseed = inseed%2 != 0 ? inseed : inseed+1;
    for (int j = 0; j <= 5; j++)
        seed_boot[j] = 0;

    int i = 0;
    do {
        seed_boot[i] = inseed & 63;
        inseed /= 64;
        i++;
    } while (inseed != 0);

    
    bootwrite(generatedSeq, *malignment);

    freenewer();
    freenew();
    seq_freerest();

    if (nodep_boot)
        matrix_char_delete(nodep_boot, spp);
    if (nodef)
        matrix_double_delete(nodef, spp);

    //clearGenratedSequences();
}

} //namespace
