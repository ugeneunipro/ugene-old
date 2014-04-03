/* PSIPRED 2 - Neural Network Prediction of Secondary Structure */

/* Copyright (C) 2000 David T. Jones - Created : January 2000 */
/* Original Neural Network code Copyright (C) 1990 David T. Jones */

/* Average Prediction Module */

#include <QtGlobal>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTemporaryFile>
#include <QtCore/QTextStream>
#include <QtCore/QString>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#ifdef Q_OS_WIN
#pragma warning(disable: 4996)
#endif

#include "sspred_avpred.h"
#include "sspred_utils.h"
#include "sspred_net.h"


//void           *calloc(), *malloc();

// char           *wtfnm;
// 
// int             nwtsum, fwt_to[TOTAL], lwt_to[TOTAL];
// float           activation[TOTAL], bias[TOTAL], *weight[TOTAL];
// 
// int             profile[MAXSEQLEN][20];
// 
// int             seqlen;
// 
// char seq[MAXSEQLEN];

enum aacodes
{
    ALA, ARG, ASN, ASP, CYS,
    GLN, GLU, GLY, HIS, ILE,
    LEU, LYS, MET, PHE, PRO,
    SER, THR, TRP, TYR, VAL,
    UNK
};


PsiPassOne::PsiPassOne(QTemporaryFile* matFile, const QStringList& weightFiles) : matrixFile(matFile), weightFileNames(weightFiles) {
    
     fwt_to = (int*) malloc(TOTAL*sizeof(int));
     lwt_to = (int*) malloc(TOTAL*sizeof(int));
     activation = (float*) malloc(TOTAL*sizeof(float));
     bias = (float*) malloc(TOTAL*sizeof(float));
     weight = (float**) malloc(TOTAL*sizeof(float*));
};

PsiPassOne::~PsiPassOne()
{

    free(fwt_to);
    free(lwt_to);
    free(activation);
    free(bias);
    free(weight);


}

void PsiPassOne::compute_output(void)
{
    int             i, j;
    float           netinp;

    for (i = NUM_IN; i < TOTAL; i++)
    {
        netinp = bias[i];

        for (j = fwt_to[i]; j < lwt_to[i]; j++)
            netinp += activation[j] * weight[i][j];

        /* Trigger neuron */
        activation[i] = logistic(netinp);
    }
}

/*
* load weights - load all link weights from a disk file
*/
void PsiPassOne::load_wts( const char *fname )
{
    int             i, j;
    double          t;
    
    QFile weightFile(fname);
    if (!weightFile.open(QIODevice::ReadOnly)) {
             return;
    }

    QTextStream stream(&weightFile);

    /* Load input units to hidden layer weights */
    for (i = NUM_IN; i < NUM_IN + NUM_HID; i++)
        for (j = fwt_to[i]; j < lwt_to[i]; j++)
        {
            stream >> t;
            weight[i][j] = t;
        }

        /* Load hidden layer to output units weights */
        for (i = NUM_IN + NUM_HID; i < TOTAL; i++)
            for (j = fwt_to[i]; j < lwt_to[i]; j++)
            {
                stream >> t;
                weight[i][j] = t;
            }

            /* Load bias weights */
            for (j = NUM_IN; j < TOTAL; j++)
            {
                stream >> t;
                bias[j] = t;
            }
}

/* Initialize network */
void PsiPassOne::init(void)
{
    int             i;

    for (i = NUM_IN; i < TOTAL; i++)
        if (!(weight[i] = (float*) calloc(TOTAL - NUM_OUT, sizeof(float))))
            fail("init: Out of Memory!");

    /* Connect input units to hidden layer */
    for (i = NUM_IN; i < NUM_IN + NUM_HID; i++)
    {
        fwt_to[i] = 0;
        lwt_to[i] = NUM_IN;
    }

    /* Connect hidden units to output layer */
    for (i = NUM_IN + NUM_HID; i < TOTAL; i++)
    {
        fwt_to[i] = NUM_IN;
        lwt_to[i] = NUM_IN + NUM_HID;
    }
}


/* Make 1st level prediction averaged over specified weight sets */
void PsiPassOne::predict()
{
    //int             aa, i, j, k, n, winpos,ws;
    int             aa, j, winpos;
    //char fname[80], predsst[MAXSEQLEN];
    char *predsst;
    //float   avout[MAXSEQLEN][3], conf, confsum[MAXSEQLEN];
    float   **avout, conf, *confsum;

    // Allocate buffers
    // TODO: not good, memory is allocated in small chunks for avout
    predsst = (char*) malloc(seqlen*sizeof(char));
    avout = (float**) malloc(seqlen*sizeof(float*));
    for (int i = 0; i < seqlen; ++i) {
        avout[i] = (float*) malloc(3*sizeof(float));
    }
    confsum = (float*) malloc(seqlen*sizeof(float));
    


    for (winpos = 0; winpos < seqlen; winpos++)
        avout[winpos][0] = avout[winpos][1] = avout[winpos][2] = confsum[winpos] = 0.0F;

    foreach (const QString& wfName, weightFileNames)
    {
        load_wts(qPrintable(wfName));

        for (winpos = 0; winpos < seqlen; winpos++)
        {
            for (j = 0; j < NUM_IN; j++)
                activation[j] = 0.0;
            for (j = WINL; j <= WINR; j++)
            {
                if (j + winpos >= 0 && j + winpos < seqlen)
                    for (aa=0; aa<20; aa++)
                        activation[(j - WINL) * 21 + aa] = profile[j+winpos][aa]/1000.0;
                else
                    activation[(j - WINL) * 21 + 20] = 1.0;
            }

            compute_output();

            conf = (2*MAX(MAX(activation[TOTAL - NUM_OUT], activation[TOTAL - NUM_OUT+1]), activation[TOTAL - NUM_OUT+2])-(activation[TOTAL - NUM_OUT]+activation[TOTAL - NUM_OUT+1]+activation[TOTAL - NUM_OUT+2])+MIN(MIN(activation[TOTAL - NUM_OUT], activation[TOTAL - NUM_OUT+1]), activation[TOTAL - NUM_OUT+2]));

            avout[winpos][0] += conf * activation[TOTAL - NUM_OUT];
            avout[winpos][1] += conf * activation[TOTAL - NUM_OUT+1];
            avout[winpos][2] += conf * activation[TOTAL - NUM_OUT+2];
            confsum[winpos] += conf;
        }
    }

    for (winpos = 0; winpos < seqlen; winpos++)
    {
        avout[winpos][0] /= confsum[winpos];
        avout[winpos][1] /= confsum[winpos];
        avout[winpos][2] /= confsum[winpos];
        if (avout[winpos][0] >= MAX(avout[winpos][1], avout[winpos][2]))
            predsst[winpos] = 'C';
        else if (avout[winpos][2] >= MAX(avout[winpos][0], avout[winpos][1]))
            predsst[winpos] = 'E';
        else
            predsst[winpos] = 'H';
    }
    QString pFilePath = U2::AppContext::getAppSettings()->getUserAppsSettings()->getUserTemporaryDirPath() + QDir::separator() + "output.ss";
    FILE* pFile = fopen(pFilePath.toLatin1().constData(), "w");
    if (!pFile) {
        fail("failed opening file for writing");
    }

    for (winpos = 0; winpos < seqlen; winpos++)
        fprintf(pFile, "%4d %c %c  %6.3f %6.3f %6.3f\n", winpos + 1, seq.constData()[winpos], predsst[winpos], avout[winpos][0], avout[winpos][1], avout[winpos][2]);
    fclose(pFile);

    // Deallocate buffers
    free(predsst);
    for (int i = 0; i < seqlen; ++i) {
        free(avout[i]);
    }
    free(avout);
    free(confsum);

}

#define BUFSIZE 256

/* Read PSI AA frequency data */
int PsiPassOne::getmtx()
{
    int             j, naa;
    
    QTextStream stream(matrixFile);
    qDebug("%s", qPrintable(matrixFile->fileName()));

    stream >> naa;
    if (naa == 0) {
        fail("Bad mtx file - no sequence length!");
    }

    if (naa > MAXSEQLEN)
        fail("Input sequence too long!");
    
    stream >> seq;
    if (seq.size() == 0)
    {   fail("Bad mtx file - no sequence!");
    }
    
    while (!stream.atEnd())
    {
        QByteArray line;
        line = stream.readLine().toLatin1();
        char* buf = line.data();
        if (!strncmp(buf, "-32768 ", 7))
        {
            for (j=0; j<naa; j++)
            {
                if (sscanf(buf, "%*d%d%*d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%*d%d", &profile[j][ALA],  &profile[j][CYS], &profile[j][ASP],  &profile[j][GLU],  &profile[j][PHE],  &profile[j][GLY],  &profile[j][HIS],  &profile[j][ILE],  &profile[j][LYS],  &profile[j][LEU],  &profile[j][MET],  &profile[j][ASN],  &profile[j][PRO],  &profile[j][GLN],  &profile[j][ARG],  &profile[j][SER],  &profile[j][THR],  &profile[j][VAL],  &profile[j][TRP],  &profile[j][TYR]) != 20)
                    fail("Bad mtx format!");
                line = stream.readLine().toLatin1();
                if (line.size() == 0)
                    break;
                buf = line.data();

            }
        }
    }

    return naa;
}

int PsiPassOne::runPsiPass()
{

    seqlen = getmtx();

    init();

    predict();

    return 0;
} 
