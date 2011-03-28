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
//     for(int i = 0; i < generatedSeq.size(); i++){
//         for (int j = 0; j < seqRowCount; j++){
//             delete[] generatedSeq[i][j];
//         }
//         delete generatedSeq[i];
//     }
//     generatedSeq.clear();
//     
}

QString SeqBoot::getTmpFileTemplate(){
    QString path = AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath();
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
        
        /*for (int j = 0; j < rowC; j++){
            //QByteArray* ba = new QByteArray('0' ,seqLen);
            QByteArray* ba = new QByteArray();
            MAlignmentRow* tmpR = new MAlignmentRow(malignment->getRowNames()[j], *ba);
            
            generatedSeq[i]->addRow(*tmpR);
        }*/
        //generatedSeq[i]->setLength(seqLen);
    }
    
}

void SeqBoot::generateDistMatrixes(const CreatePhyTreeSettings& settings){
    
}

void SeqBoot::consInit(){
    /* Initial settings */
    ibmpc          = IBMCRT;
    ansi           = ANSICRT;
    didreroot      = false;
    firsttree      = true;
    spp            = malignment->getNumRows() ;
    col            = 0 ;
    /* This is needed so functions in cons.c work */
    tree_pairing   = NO_PAIRING ;  
    
    strict = false;
    mr = false;
    mre = true;
    ml = false;
    mlfrac = 0.5;
    noroot = true;
    numopts = 0;
    outgrno_cons = 1;
    outgropt_cons = false;
    trout = false;
    prntsets = false;
    progress = false;
    treeprint_cons = false;

    ntrees = 0.0;
    maxgrp = 32767;   /* initial size of set hash table */
    lasti  = -1;

    int i, j, k;
    long tip_count = spp;
     /* do allocation first *****************************************/
    grouping  = (group_type **)  Malloc(maxgrp*sizeof(group_type *));
    lengths  = (double *)  Malloc(maxgrp*sizeof(double));

    timesseen_changes = (double*)Malloc(maxgrp*sizeof(double));
    for (i = 0; i < maxgrp; i++)
    timesseen_changes[i] = 0.0;

    for (i = 0; i < maxgrp; i++)
    grouping[i] = NULL;

    order     = (long **) Malloc(maxgrp*sizeof(long *));
    for (i = 0; i < maxgrp; i++)
    order[i] = NULL;

    timesseen = (double **)Malloc(maxgrp*sizeof(double *));
    for (i = 0; i < maxgrp; i++)
    timesseen[i] = NULL;

    nayme = (naym *)Malloc(tip_count*sizeof(naym));
    hashp = (hashtype)Malloc(sizeof(namenode) * NUM_BUCKETS);
    for (i=0;i<NUM_BUCKETS;i++) {
      hashp[i] = NULL;
    }
    setsz = (long)ceil((double)tip_count/(double)SETBITS);

    fullset = (group_type *)Malloc(setsz * sizeof(group_type));
    for (j = 0; j < setsz; j++)
    fullset[j] = 0L;
    k = 0;
    for (j = 1; j <= tip_count; j++) {
    if (j == ((k+1)*SETBITS+1)) k++;
    fullset[k] |= 1L << (j - k*SETBITS - 1);
    }
  /* end allocation **********************************************/

    firsttree = true;
    grbg = NULL;

}

void SeqBoot::consensus(){
    consInit();   
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

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    long inseed = qAbs(qrand());
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

    /*for(int i = 0; i < generatedSeq.size(); i++){
        for (int j = 0; j < generatedSeq[i]->getNumRows(); j++){
            for(int k = 0 ; k < generatedSeq[i]->getLength(); k++){
                putchar(generatedSeq[i]->getRow(j).chatAt(k));
            }
            putchar('\n');
        }
        putchar('\n');
        putchar('\n');
        putchar('\n');
    }*/

    freenewer();
    freenew();
    seq_freerest();

    if (nodep_boot)
        matrix_char_delete(nodep_boot, spp);
    if (nodef)
        matrix_double_delete(nodef, spp);

//     for(int i = 0; i < reps; i++){
//         for(int j = 0; j < generatedSeq[i]->getNumRows(); j++){
//             const MAlignmentRow& curR = generatedSeq[i]->getRow(j);
//             curR.setName("3");
//         }
//     }

    //clearGenratedSequences();
    
}

} //namespace
