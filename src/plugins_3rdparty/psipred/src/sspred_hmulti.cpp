/* PSIPRED2 - Neural Network Prediction of Secondary Structure */

/* Copyright (C) 2000 David T. Jones - Created : January 2000 */
/* Original Neural Network code Copyright (C) 1990 David T. Jones */

/* 2nd Level Prediction Module */

#include <QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QString>
#include <QtCore/QDir>
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

#include "sspred_hmulti.h"
#include "sspred_net2.h"
#include "sspred_utils.h"


/* logistic 'squashing' function (+/- 1.0) */
//#define logistic(x) ((REAL)1.0 / ((REAL)1.0 + (REAL)exp(-(x))))

const char           *rnames[] =
{
    "ALA", "ARG", "ASN", "ASP", "CYS",
    "GLN", "GLU", "GLY", "HIS", "ILE",
    "LEU", "LYS", "MET", "PHE", "PRO",
    "SER", "THR", "TRP", "TYR", "VAL",
    "???"
};

// static char           *wtfnm;
// 
// static int             nwtsum, fwt_to[TOTAL], lwt_to[TOTAL];
// static REAL            activation[TOTAL], bias[TOTAL], *weight[TOTAL];
// 
// static float           profile[MAXSEQLEN][3];
// 
// static char            seq[MAXSEQLEN];
// 
// static int             seqlen, nprof;

PsiPassTwo::PsiPassTwo() {

    fwt_to = (int*) malloc(TOTAL*sizeof(int));
    lwt_to = (int*) malloc(TOTAL*sizeof(int));
    activation = (float*) malloc(TOTAL*sizeof(REAL));
    bias = (float*) malloc(TOTAL*sizeof(REAL));
    weight = (float**) malloc(TOTAL*sizeof(REAL*));
    nprof = 0; //must be initialized (at least for Fedora)
};

PsiPassTwo::~PsiPassTwo()
{

    free(fwt_to);
    free(lwt_to);
    free(activation);
    free(bias);
    free(weight);
}


void PsiPassTwo::compute_output(void)
{
    int             i, j;
    REAL            netinp;

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
void PsiPassTwo::load_wts( const char *fname )
{
    int             i, j;
    
    QFile weightFile(fname);
    if (!weightFile.open(QIODevice::ReadOnly)) {
        fail("cannot open weights file");
    }

    QTextStream stream(&weightFile);     
    
    /* Load input units to hidden layer weights */
    for (i = NUM_IN; i < NUM_IN + NUM_HID; i++)
	for (j = fwt_to[i]; j < lwt_to[i]; j++)
	{
	    stream >> weight[i][j];
	}

    /* Load hidden layer to output units weights */
    for (i = NUM_IN + NUM_HID; i < TOTAL; i++)
	for (j = fwt_to[i]; j < lwt_to[i]; j++)
	{
	    stream >> weight[i][j];
	}

    /* Load bias weights */
    for (j = NUM_IN; j < TOTAL; j++)
    {
	    stream >> bias[j];
    }

}

void PsiPassTwo::init(void)
{
    int             i;

    for (i = NUM_IN; i < TOTAL; i++)
	if (!(weight[i] = (float*) calloc(TOTAL - NUM_OUT, sizeof(REAL))))
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

/* Main prediction routine */
QByteArray PsiPassTwo::predict( int niters, float dca, float dcb, const char *outname )
{
    //    char            pred, predsst[MAXSEQLEN], lastpreds[MAXSEQLEN], *che = "CHE";
    //   float           score_c[MAXSEQLEN], score_h[MAXSEQLEN], score_e[MAXSEQLEN], bestsc, score, conf[MAXSEQLEN], predq3, av_c, av_h, av_e;
    //  int             aa, a, b, nb, i, j, k, n, winpos;

    int             aa,b, nb, i, j, winpos;
    char            pred, *predsst, *lastpreds;
    float           *score_c, *score_h, *score_e, *conf, av_c, av_h, av_e;

    // Allocate buffers
    predsst = (char*) malloc(seqlen*sizeof(char));
    lastpreds = (char*) malloc(seqlen*sizeof(char));
    score_c = (float*) malloc(seqlen*sizeof(float));
    score_h = (float*) malloc(seqlen*sizeof(float));
    score_e = (float*) malloc(seqlen*sizeof(float));
    conf = (float*) malloc(seqlen*sizeof(float));
    
    FILE *ofp;

    ofp = fopen(outname, "w");
    if (!ofp)
      fail("Cannot open output file!");

    fputs("# PSIPRED VFORMAT (PSIPRED V2.6 by David Jones)\n\n", ofp);
    
    if (niters < 1)
      niters = 1;

    do {
	memcpy(lastpreds, predsst, seqlen);
	av_c = av_h = av_e = 0.0;
	for (winpos = 0; winpos < seqlen; winpos++)
	{
	    av_c += profile[winpos][0];
	    av_h += profile[winpos][1];
	    av_e += profile[winpos][2];
	}
	av_c /= seqlen;
	av_h /= seqlen;
	av_e /= seqlen;
	for (winpos = 0; winpos < seqlen; winpos++)
	{
	    for (j = 0; j < NUM_IN; j++)
		activation[j] = 0.0;
	    activation[(WINR - WINL + 1) * IPERGRP] = av_c;
	    activation[(WINR - WINL + 1) * IPERGRP + 1] = av_h;
	    activation[(WINR - WINL + 1) * IPERGRP + 2] = av_e;
	    activation[(WINR - WINL + 1) * IPERGRP + 3] = logistic((seqlen-150)/100.0);
	    for (j = WINL; j <= WINR; j++)
	    {
		if (j + winpos >= 0 && j + winpos < seqlen)
		{
		    for (aa = 0; aa < 3; aa++)
			activation[(j - WINL) * IPERGRP + aa] = profile[j + winpos][aa];
		}
		else
		    activation[(j - WINL) * IPERGRP + 3] = 1.0;
	    }
	    compute_output();
	    if (activation[TOTAL - NUM_OUT] > (dca * activation[TOTAL - NUM_OUT + 1]) && activation[TOTAL - NUM_OUT] > (dcb * activation[TOTAL - NUM_OUT + 2]))
		pred = 'C';
	    else if (dca * activation[TOTAL - NUM_OUT + 1] > activation[TOTAL - NUM_OUT] && dca*activation[TOTAL - NUM_OUT + 1] > dcb * activation[TOTAL - NUM_OUT + 2])
		pred = 'H';
	    else
		pred = 'E';
	    predsst[winpos] = pred;
	    score_c[winpos] = activation[TOTAL - NUM_OUT];
	    score_h[winpos] = activation[TOTAL - NUM_OUT + 1];
	    score_e[winpos] = activation[TOTAL - NUM_OUT + 2];
	}
	
	for (winpos = 0; winpos < seqlen; winpos++)
	{
	    profile[winpos][0] = score_c[winpos];
	    profile[winpos][1] = score_h[winpos];
	    profile[winpos][2] = score_e[winpos];
	}
    } while (memcmp(predsst, lastpreds, seqlen) && --niters);
    
    for (winpos = 0; winpos < seqlen; winpos++)
	conf[winpos] = (2*MAX(MAX(score_c[winpos], score_h[winpos]), score_e[winpos])-(score_c[winpos]+score_h[winpos]+score_e[winpos])+MIN(MIN(score_c[winpos], score_h[winpos]), score_e[winpos]));
    
    for (winpos = 0; winpos < seqlen; winpos++)
	if (winpos && winpos < seqlen - 1 && predsst[winpos - 1] == predsst[winpos + 1] && conf[winpos] < 0.5*(conf[winpos-1]+conf[winpos+1]))
	    predsst[winpos] = predsst[winpos - 1];
    
    for (winpos = 0; winpos < seqlen; winpos++)
    {
	if (winpos && winpos < seqlen - 1 && predsst[winpos - 1] == 'C' && predsst[winpos] != predsst[winpos + 1])
	    predsst[winpos] = 'C';
	if (winpos && winpos < seqlen - 1 && predsst[winpos + 1] == 'C' && predsst[winpos] != predsst[winpos - 1])
	    predsst[winpos] = 'C';
    }
    
    for (winpos=0; winpos<seqlen; winpos++)
	fprintf(ofp, "%4d %c %c  %6.3f %6.3f %6.3f\n", winpos + 1, seq[winpos], predsst[winpos], score_c[winpos], score_h[winpos], score_e[winpos]);
    
    fclose(ofp);

//     FILE* pFile = fopen( "header.out", "w" );
//     if (!pFile)
//     {
//         fail("open file for writing failed");
//     }
    QByteArray result;
    nb = seqlen / 60 + 1;
    j = 1;
    for (b = 0; b < nb; b++)
    {
        //fprintf(pFile, "\nConf: ");
        for (i = 0; i < 60; i++)
        {
            if (b * 60 + i >= seqlen)
                break;
            j = b * 60 + i + 1;
        //    putc(MIN((char)(10.0*conf[j-1]+'0'), '9'), pFile);
        }

        //fprintf("\nPred: ");

        for (i = 0; i < 60; i++)
        {
            if (b * 60 + i >= seqlen)
                break;
            j = b * 60 + i + 1;
          //  putc(predsst[j - 1], pFile);
            result.append(predsst[j-1]);
        }

        //fprintf(pFile, "\n  AA: ");

        for (i = 0; i < 60; i++)
        {
            if (b * 60 + i >= seqlen)
                break;
            j = b * 60 + i + 1;
      //      putc(seq[j - 1], pFile);
           
        }

//        fprintf(pFile, "\n      ");

        for (i = 0; i < 58; i++)
        {
            if (b * 60 + i + 3 > seqlen)
                break;
            j = b * 60 + i + 3;
            if (!(j % 10)) {
        //        fprintf(pFile, "%3d", j);
                i += 2;
            }
            else {
          //      fprintf(pFile, " ");
            }
        }
       // putc('\n', pFile);

       // putc('\n', pFile);
    }

    //fclose(pFile);

    //Deallocate buffers
    free(predsst);
    free(lastpreds);
    free(score_c);
    free(score_h);
    free(score_e);
    free(conf);
    
    return result;
}

/* Read PSI AA frequency data */
int PsiPassTwo::getss(FILE * lfil)
{
    int             naa;
    float pv[3];
    char            buf[256];

    naa = 0;
    while (!feof(lfil))
    {
	if (!fgets(buf, 255, lfil))
	    break;
    seq[naa] = buf[5];
    //char c = buf[5];
	//seq.insert(naa, c);
	if (sscanf(buf + 11, "%f%f%f", &pv[0], &pv[1], &pv[2]) != 3)
	    break;

	if (!nprof)
	  {
	    profile[naa][0] = pv[0];
	    profile[naa][1] = pv[1];
	    profile[naa][2] = pv[2];
	  }
	else
	  {
	    profile[naa][0] += pv[0];
	    profile[naa][1] += pv[1];
	    profile[naa][2] += pv[2];
	  }

	naa++;
    }

    nprof++;
    
    if (!naa)
	fail("Bad format!");

    return naa;
}

int PsiPassTwo::runPsiPass( int argc, const char *argv[], QByteArray& result )
{
    int             i;
    FILE           *ifp;




    /* malloc_debug(3); */
    if (argc < 5)
	fail("usage : psipass2 weight-file itercount DCA DCB outputfile ss-infile ...");

    init();
    load_wts(wtfnm = argv[1]);
    QString outputFileName=U2::AppContext::getAppSettings()->getUserAppsSettings()->getUserTemporaryDirPath() + QDir::separator() + "output.ss"; //File created at sspred_avpred.cpp in method predict
    ifp = fopen(outputFileName.toLatin1().constData(), "r");
    if (!ifp) {
        fail("failed opening file for reading");
//	    exit(1);
    }
    seqlen = getss(ifp);
    fclose(ifp);
    
    for (i=0; i<seqlen; i++)
    {
      profile[i][0] /= nprof;
      profile[i][1] /= nprof;
      profile[i][2] /= nprof;
    }
    
    //puts("# PSIPRED HFORMAT (PSIPRED V2.6 by David Jones)");
    QString outputFileName2=U2::AppContext::getAppSettings()->getUserAppsSettings()->getUserTemporaryDirPath() + QDir::separator() + "output.ss2";
    result = predict(atoi(argv[2]), (float)atof(argv[3]), (float)atof(argv[4]), outputFileName2.toLatin1().constData());

    return 0;
}
