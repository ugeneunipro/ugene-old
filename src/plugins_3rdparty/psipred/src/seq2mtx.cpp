/* seq2mtx - convert single sequence to pseudo IMPALA mtx file */

/* Copyright (C) 2000 D.T. Jones */

#include <QtGlobal>
#include <QtCore/QTemporaryFile>
#include <QtCore/QTextStream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#ifdef Q_OS_WIN
#pragma warning(disable: 4996)
#endif

#include "sspred_utils.h"

#define MAXSEQLEN 65536

// #define FALSE 0
// #define TRUE 1

#define SQR(x) ((x)*(x))
#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))

const char *rescodes = "ARNDCQEGHILKMFPSTWYVBZX";

/*  BLOSUM 62 */
const short           aamat[23][23] =
{
    {4, -1, -2, -2, 0, -1, -1, 0, -2, -1, -1, -1, -1, -2, -1, 1, 0, -3, -2, 0, -2, -1, 0},
    {-1, 5, 0, -2, -3, 1, 0, -2, 0, -3, -2, 2, -1, -3, -2, -1, -1, -3, -2, -3, -1, 0, -1},
    {-2, 0, 6, 1, -3, 0, 0, 0, 1, -3, -3, 0, -2, -3, -2, 1, 0, -4, -2, -3, 3, 0, -1},
    {-2, -2, 1, 6, -3, 0, 2, -1, -1, -3, -4, -1, -3, -3, -1, 0, -1, -4,
     -3, -3, 4, 1, -1},
    {0, -3, -3, -3,10, -3, -4, -3, -3, -1, -1, -3, -1, -2, -3, -1, -1, -2,
     -2, -1, -3, -3, -2},
    {-1, 1, 0, 0, -3, 5, 2, -2, 0, -3, -2, 1, 0, -3, -1, 0, -1, -2,
     -1, -2, 0, 3, -1},
    {-1, 0, 0, 2, -4, 2, 5, -2, 0, -3, -3, 1, -2, -3, -1, 0, -1, -3,
     -2, -2, 1, 4, -1},
    {0, -2, 0, -1, -3, -2, -2, 6, -2, -4, -4, -2, -3, -3, -2, 0, -2, -2,
     -3, -3, -1, -2, -1},
    {-2, 0, 1, -1, -3, 0, 0, -2, 8, -3, -3, -1, -2, -1, -2, -1, -2, -2,
     2, -3, 0, 0, -1},
    {-1, -3, -3, -3, -1, -3, -3, -4, -3, 4, 2, -3, 1, 0, -3, -2, -1, -3,
     -1, 3, -3, -3, -1},
    {-1, -2, -3, -4, -1, -2, -3, -4, -3, 2, 4, -2, 2, 0, -3, -2, -1, -2,
     -1, 1, -4, -3, -1},
    {-1, 2, 0, -1, -3, 1, 1, -2, -1, -3, -2, 5, -1, -3, -1, 0, -1, -3,
     -2, -2, 0, 1, -1},
    {-1, -1, -2, -3, -1, 0, -2, -3, -2, 1, 2, -1, 5, 0, -2, -1, -1, -1,
     -1, 1, -3, -1, -1},
    {-2, -3, -3, -3, -2, -3, -3, -3, -1, 0, 0, -3, 0, 6, -4, -2, -2, 1,
     3, -1, -3, -3, -1},
    {-1, -2, -2, -1, -3, -1, -1, -2, -2, -3, -3, -1, -2, -4, 7, -1, -1, -4,
     -3, -2, -2, -1, -2},
    {1, -1, 1, 0, -1, 0, 0, 0, -1, -2, -2, 0, -1, -2, -1, 4, 1, -3,
     -2, -2, 0, 0, 0},
    {0, -1, 0, -1, -1, -1, -1, -2, -2, -1, -1, -1, -1, -2, -1, 1, 5, -2,
     -2, 0, -1, -1, 0},
    {-3, -3, -4, -4, -2, -2, -3, -2, -2, -3, -2, -3, -1, 1, -4, -3, -2, 11,
     2, -3, -4, -3, -2},
    {-2, -2, -2, -3, -2, -1, -2, -3, 2, -1, -1, -2, -1, 3, -3, -2, -2, 2,
     7, -1, -3, -2, -1},
    {0, -3, -3, -3, -1, -2, -2, -3, -3, 3, 1, -2, 1, -1, -2, -2, 0, -3,
     -1, 4, -3, -2, -1},
    {-2, -1, 3, 4, -3, 0, 1, -1, 0, -3, -4, 0, -3, -3, -2, 0, -1, -4,
     -3, -3, 4, 1, -1},
    {-1, 0, 0, 1, -3, 3, 4, -2, 0, -3, -3, 1, -1, -3, -1, 0, -1, -3,
     -2, -2, 1, 4, -1},
    {0, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2, 0, 0, -2,
     -1, -1, -1, -1, 4}
};

int seq2mtx(const char* seq, int seqlen, const char* outFileName)
{
    int i, j;
    const char *ncbicodes = "XAXCDEFGHIKLMNPQRSTVWXYXXX";
    
    if (seqlen < 5 || seqlen >= MAXSEQLEN)
        fail("Sequence length error!");
    
    
    FILE* pFile = fopen( outFileName, "w" );
    if (!pFile)
    {
        fail("open file for writing failed");
    }

    fprintf(pFile, "%d\n", seqlen);

    for (i=0; i<seqlen; i++)
        putc(seq[i], pFile);

    fprintf(pFile, "\n0\n0\n0\n0\n0\n0\n0\n0\n0\n0\n0\n0\n");

    for (i=0; i<seqlen; i++)
    {
        for (j=0; j<26; j++)
            if (ncbicodes[j] != 'X')
                fprintf(pFile, "%d  ", aamat[aanum(seq[i])][aanum(ncbicodes[j])]*100);
            else
                fprintf(pFile, "-32768  ");
        putc('\n', pFile);
    }
    fclose(pFile);
    
    return 0;
}

int seq2mtx( const char* seq, int seqlen, QTemporaryFile* tmpFile )
{
    int i, j;
    const char *ncbicodes = "XAXCDEFGHIKLMNPQRSTVWXYXXX";

    if (seqlen < 5 || seqlen >= MAXSEQLEN)
        fail("Sequence length error!");
    tmpFile->open();
    QTextStream stream(tmpFile);
    
    stream << seqlen << '\n';
    
    for (i=0; i<seqlen; i++)
        stream << seq[i];

    stream << "\n0\n0\n0\n0\n0\n0\n0\n0\n0\n0\n0\n0\n";

    for (i=0; i<seqlen; i++)
    {
        for (j=0; j<26; j++)
            if (ncbicodes[j] != 'X')
                stream << aamat[aanum(seq[i])][aanum(ncbicodes[j])]*100 << "  ";
                //fprintf(pFile, "%d  ", aamat[aanum(seq[i])][aanum(ncbicodes[j])]*100);
            else
                stream << "-32768 ";
                //fprintf(pFile, "-32768  ");
        stream << '\n';
        //putc('\n', pFile);
    }
   
    return 0;

}
