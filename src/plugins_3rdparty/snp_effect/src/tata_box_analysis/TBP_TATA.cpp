#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

double sqrt(),  exp(),  log(); 

#include "TBP_TATA.h"

#define Recognition_Threshold >19.00000
#define Recognition_Defined   >-10.0000
#define MATR_TBP_Length 26
#define MATR_TBP_Point  13


/**************************************************************************/

double TBP_MeanFreq (char *S)
{
    double X, x;int N, Length, TBPLen=14, TBPStart=3;char *s;
    s=&S[0];Length=strlen(s);if (Length < TBPLen)return(-12345.);
    X=0.;N=0;s=&S[0];
    x=TBP_NatTATA_via_PWM_dsDNA_ssDNA(s);return(x);
}

/* TBP/dsDNA affinity increases with Minor Groove Width increase  */

double MinGrWd_for_TBPdsDNA (char *s)
{

    double  X; char *seq; size_t i,k, SiteLength=4;

    double DinucPar[16]={
    /*_AA.....AT.....AG.....AC.....TA.....TT.....TG.....TC.*/
    5.30,  5.31,  5.19,  6.04,  6.40,  5.30,  4.79,  4.71,
    /*.GA.....GT.....GG.....GC.....CA.....CT.....CG.....CC.*/
    4.71,  6.04,  4.62,  4.74,  4.79,  5.19,  5.16,  4.62};

    seq=&s[0]; if(strlen(seq) < SiteLength+1)return(-1001.);

    for (i=0, X=0.;i < SiteLength-1;i++) {
    switch (seq[i  ]) {
    case 'A': k= 0; break;case 'T': k= 4; break;
    case 'G': k= 8; break;case 'C': k=12; break;default : return(-1002.); }
    switch (seq[i+1]) {
    case 'A': k+=0; break;case 'T': k+=1; break;
    case 'G': k+=2; break;case 'C': k+=3; break;default : return(-1003.); }
    if (k > 15) return(-1004.); X+=DinucPar[k]; }

    return (X/(double)(SiteLength-1));
}

/* TBP/dsDNA affinity increases with TA-content increase */

double WeightTA_for_TBPdsDNA (char *s)
{
    double  X; char *seq; size_t i, SiteLength=15;

    double Weight [14]={
    /*.-5......-4......-3......-2......-1.....*/
    0.100,  0.106,  0.128,  0.180,  0.276,
    /*..0.......1.......2.......3.......4.....*/
    0.414,  0.594,  0.794,  0.942,  0.964,
    /*..5.......6.......7.......8.*/
    0.808,  0.480,  0.190,  0.100};

    seq=&s[0]; if(strlen(seq) < SiteLength)return(-101.);

    for (i=0, X=0.;i < SiteLength-1;i++)
    if(seq[i  ]=='T')
    if(seq[i+1]=='A') X+=Weight[i];

    return(X);
}

/* TBP/dsDNA affinity prediction via Minor Groove Width and TA-contents */

double TBPdsDNA_by_MinGrWd_TA (char *s){
    extern double MinGrWd_for_TBPdsDNA (char *);
    extern double WeightTA_for_TBPdsDNA (char *);

    double  x1, x2; char *seq; size_t s1=5, s2=0, SiteLength=15;

    seq=&s[0]; if(strlen(seq) < SiteLength)return(-102.);

    seq=&s[s1]; x1=MinGrWd_for_TBPdsDNA (seq); if(x1< -99.)return(x1);
    seq=&s[s2]; x2=WeightTA_for_TBPdsDNA (seq); if(x2< -99.)return(x2);

    return (-35.13+10.21*x1-0.75*x2);
}

/* MN Affinity between the yeast TBP and synthetic oligo-ssDNA TBP/ssDNA affinity increases with WR-content increase */

double WeightWR_for_TBPssDNA (char *s)
{
    double  X; char *seq; size_t i, SiteLength=15;
    double Weight [14] =  {
    /*.-5......-4......-3......-2......-1.....*/
    0.796,  0.386,  0.140,  0.140,  0.254,
    /*..0.......1.......2.......3.......4.....*/
    0.398,  0.538,  0.670,  0.774,  0.854,
    /*..5.......6.......7.......8.*/
    0.920,  0.966,  0.992,  1.000 };

    seq=&s[0];
    if ( strlen( seq ) < SiteLength ) {
        return(-201.);
    }

    for (i=0, X=0.;i < SiteLength-1;i++)
    if(seq[i  ]=='A' || seq[i  ]=='T')
    if(seq[i+1]=='A' || seq[i+1]=='G') X+=Weight[i];

    return(X);
}

/* TBP/ssDNA affinity increases with TV-content increase */

double WeightTV_for_TBPssDNA (char *s)
{
    double  X; char *seq; size_t i, SiteLength=15;

    double Weight [14]={
    /*.-5......-4......-3......-2......-1.....*/
    0.124,  0.234,  0.418,  0.634,  0.838,
    /*..0.......1.......2.......3.......4.....*/
    0.958,  0.994,  0.970,  0.882,  0.708,
    /*..5.......6.......7.......8.*/
    0.486,  0.280,  0.142,  0.100};

    seq=&s[0]; if(strlen(seq) < SiteLength)return(-202.);

    for (i=0, X=0.;i < SiteLength-1;i++)
    if(seq[i  ]=='T')
    if(seq[i+1]=='A' || seq[i+1]=='G' || seq[i+1]=='C') X+=Weight[i];

    return(X);
}

/* TBP/ssDNA affinity  prediction via WR- and TV-contents */

double TBPssDNA_by_WR_TV (char *s)
{
    extern double WeightWR_for_TBPssDNA (char *);
    extern double WeightTV_for_TBPssDNA (char *);

    double  x1, x2; char *seq; size_t s1=0, s2=0, SiteLength=15;

    seq=&s[0]; if(strlen(seq) < SiteLength)return(-2001.);

    seq=&s[s1]; x1=WeightWR_for_TBPssDNA (seq); if(x1< -999.)return(x1);
    seq=&s[s2]; x2=WeightTV_for_TBPssDNA (seq); if(x2< -999.)return(x2);

    return (14.5329+0.868652*x1+2.531143*x2);
}

/* Bucher, JMB'90,v212(4)p563-78. Table 3 (p579). */

double TBP_Bucher_PW_matrix (char *s)
{
    int A[  15]={
    -102,-305,   0,-461,   0,   0,   0,   0,  -1, -94, -54, -48, -48, -74, -62};
    int C[  15]={
     -28,-206,-522,-349,-517,-463,-412,-374,-113,  -5,   0,  -5, -11, -28, -40};
    int G[  15]={
       0,-274,-438,-461,-377,-473,-265,-150,   0,   0,  -9,   0,   0,   0,   0};
    int T[  15]={
    -168,   0,-228,   0,-234, -52,-365, -37,-140, -97,-140, -82, -66, -54, -61};
    double X, z=100.; char N, *seq; int len, i, SiteLength=15;
    seq=&s[-1]; len=strlen(seq); if(len < SiteLength) return (-100.-(double)len);
    for(i=0, X=0.;i<  SiteLength;i++)
     {N=s[i];switch(N){
     case 'A': X+=A[i]/z;break;
     case 'T': X+=T[i]/z;break;
     case 'G': X+=G[i]/z;break;
     case 'C': X+=C[i]/z;break;}}
    return(X);
} 

double TBP_NatTATA_via_PWM_dsDNA_ssDNA (char *s)
{
    extern double TBPdsDNA_by_MinGrWd_TA (char *);
    extern double TBPssDNA_by_WR_TV      (char *);
    extern double TBP_Bucher_PW_matrix   (char *);

    double x0,y0, x1,y1, x2,y2, s1,s2, Bucher, ssDNA, dsDNA, Score;
    char *seq, nucl, dir[26], comp[26];
    int i,j,k,kk, seq_len, Olig_Length, TATA_Core;

    Olig_Length=26; TATA_Core=20;
    x0=y0=x1=y1=x2=y2=Bucher=ssDNA=dsDNA=Score=-10000.;

    seq=&s[0]; seq_len=strlen(seq);

    if(seq_len > Olig_Length) seq_len=Olig_Length;

    if(seq_len < TATA_Core+1)return(-103.);
    seq_len-=TATA_Core-1;
    for(i=0;i<Olig_Length;i++){dir[i]='\0';comp[i]='\0';}

    for (i=0, k=0,kk=1, s1=0.,s2=0.; i<seq_len;i++) {
        seq=&s[i];
        for (j=0;j<TATA_Core;j++) {
            nucl='\0'; dir[j]='\0'; comp[TATA_Core-j-1]='\0';
            switch (seq[j]){
            case 'a': case 'A': nucl='T'; break;
            case 't': case 'T': case 'u': case 'U': nucl='A'; break;
            case 'g': case 'G': nucl='C'; break;
            case 'c': case 'C': nucl='G'; break;
            default: printf ("\n j=%d ",j); nucl='G'; break;}
            dir[j]=seq[j]; comp[TATA_Core-j-1]=nucl;
        }

        seq=&dir [6]; x0=TBP_Bucher_PW_matrix (seq); if(x0<-99.)return(-91.);
        seq=&comp[6]; y0=TBP_Bucher_PW_matrix (seq); if(y0<-99.)return(y0/10.);

        seq=&dir [4]; x1=TBPssDNA_by_WR_TV(seq); if(x1<-99.)return(-93.);
        seq=&comp[4]; y1=TBPssDNA_by_WR_TV(seq); if(y1<-99.)return(-94.);

        seq=&dir [1]; x2=TBPdsDNA_by_MinGrWd_TA(seq); if(x2<-99.)return(-95.);
        seq=&comp[1]; y2=TBPdsDNA_by_MinGrWd_TA(seq); if(y2<-99.)return(-96.);
        s1+=x2; s2+=y2; k++;

        if(Bucher<x0){Bucher=x0; ssDNA=(x1+y1)/2.; kk=1;}
        if(Bucher<y0){Bucher=y0; ssDNA=(x1+y1)/2.; kk=2;}
    }

    if(k<1)return(-95.); s1/=k; s2/=k;  dsDNA=s1; if(kk>1)dsDNA=s2;

    Score=10.902344 + 0.231397*dsDNA + 0.1535*Bucher + 0.2007953333333*ssDNA;

    return (Score);
}

double TBP_NatTATA_95conf_interval (char *s)
{
    extern double TBP_NatTATA_via_PWM_dsDNA_ssDNA (char *);

    extern double sqrt (double);

    double x,y, z;
    char *seq, nat_nucl, mut_nucl, mut[26];
    int i,j,k;

    seq=&s[0]; x=TBP_NatTATA_via_PWM_dsDNA_ssDNA(seq); if(x<-50.) return (x);

    for (i=0, z=0.; i<26;i++) for(k=0;k<3;k++) {seq=&s[0]; nat_nucl=seq[i];
    switch (nat_nucl){
        case 'a': case 'A':
        switch(k){
        case 0: mut_nucl='T'; break;
        case 1: mut_nucl='G'; break;
        case 2: mut_nucl='C'; break;} break;
        case 't': case 'T': case 'u': case 'U':
        switch(k){
        case 0: mut_nucl='A'; break;
        case 1: mut_nucl='G'; break;
        case 2: mut_nucl='C'; break;} break;
        case 'g': case 'G':
        switch(k){
        case 0: mut_nucl='A'; break;
        case 1: mut_nucl='T'; break;
        case 2: mut_nucl='C'; break;} break;
        case 'c': case 'C':
        switch(k){
        case 0: mut_nucl='A'; break;
        case 1: mut_nucl='T'; break;
        case 2: mut_nucl='G'; break;} break;}

    for (j=0; j<26; j++) {seq=&s[0]; mut[j]=seq[j];} mut[i]=mut_nucl;

    seq=&mut[0]; y=TBP_NatTATA_via_PWM_dsDNA_ssDNA(seq); if(y<-50.) return (y);

    z+=(x-y)*(x-y); }

    z/=(3.*26.)*(3.*26.-1.);
    z=1.6652*sqrt(z);

    return (z);
}


int MATRIX_TBP /* Output: Status, 1 means O'K, otherwise Error # */
(                  
 char *Seq, float* directScores, float* complementScores, int* lenSeq
 )                  
{
    char *seq, nucl_seq, nucl_dir, nucl_comp, direct[50],complement[50];
    int i,j,k,m, len, new_nucl;
    double z, Score_dir, Score_comp;

    if (Seq == NULL || directScores == NULL || complementScores == NULL){
        return -1;
    }

    seq=&Seq[0];
    len=strlen(seq);
    if(len< MATR_TBP_Length){
        return -99;
    }

    for(i=0;i<50;i++){
        direct[i]='\0';
        complement[i]='\0';
    }

    for(j=0,i=0,k=0;j<len;j++){
        nucl_seq=Seq[j]; new_nucl=0;
        if (nucl_seq ==   10){
            continue;
        }
        if (nucl_seq == '\0'){
            continue;
        }
        if (nucl_seq == '\t'){
            continue;
        }
        if (nucl_seq == '\b'){
            continue;
        }
        /*if (nucl_seq == NULL) continue;*/
        if (nucl_seq == 0){
            continue; 
        }
        if (nucl_seq ==  ' '){
            continue;
        }
        if (nucl_seq == '\n'){
            continue;
        }
        switch (nucl_seq){
            case 'a': case 'A': nucl_dir='A'; nucl_comp='T'; new_nucl=1; break;
            case 't': case 'T':
            case 'u': case 'U': nucl_dir='T'; nucl_comp='A'; new_nucl=1; break;
            case 'g': case 'G': nucl_dir='G'; nucl_comp='C'; new_nucl=1; break;
            case 'c': case 'C': nucl_dir='C'; nucl_comp='G'; new_nucl=1; break;
            case 'm': case 'M':
            case 'k': case 'K':
            case 'r': case 'R':
            case 'y': case 'Y':
            case 'w': case 'W':
            case 's': case 'S':
            case 'x': case 'X':
            case 'n': case 'N': nucl_dir='N'; nucl_comp='N'; new_nucl=1; break;
        }
        if(new_nucl){
            i++;
            Score_dir =-999.;
            Score_comp=-999.;
            if (k>MATR_TBP_Length-1){
                k--;
                for (m=0;m<k;m++){
                    direct[m]=direct[m+1];
                    complement[k-m]=complement[k-m-1];
                }
            }
            direct[k]=nucl_dir;
            complement[MATR_TBP_Length-k-1]=nucl_comp;
            k++;
            if(k==MATR_TBP_Length){
                seq=&direct[0];
                z=TBP_MeanFreq (seq);
                Score_dir =z;
                seq=&complement[0];
                z=TBP_MeanFreq (seq);
                Score_comp=z;
                /*    if (abs(Score_dir-Score_comp) > 0.005) return (-987); */
                seq=&direct[0];
                z=TBP_NatTATA_95conf_interval (seq);
                Score_comp=z;
            }
            if(i>MATR_TBP_Point-1){ // i>12
                directScores[i-MATR_TBP_Length+MATR_TBP_Point] = (float)Score_dir;
                complementScores[i-MATR_TBP_Length+MATR_TBP_Point] = (float)Score_comp;
                //fprintf (Out_Score,"%d \t %f\t %f\n",
                //i - 12
                //i-MATR_TBP_Length+MATR_TBP_Point+1, Score_dir, Score_comp);
            }
            //if(Score_dir Recognition_Defined){ 
            //    if(Score_dir Recognition_Threshold){
            //        fprintf (Out_Table," %d.. %d / TATA-box (+)-chain: %s Score=%f (%f)\n",
            //        i-MATR_TBP_Length+1, i, direct, Score_dir, Score_comp);
            //        fprintf (Out_Table, " %d.. %d / TATA-box (-)-chain: %s Score=%f (%f)\n",
            //       i-MATR_TBP_Length+1, i, complement, Score_dir, Score_comp);
            //    }
            //}
        }
    }
    //j=i-11
    for (j=i-MATR_TBP_Length+MATR_TBP_Point+1;j<i;j++){
        directScores[j] = -999.;
        complementScores[j] = -999.;
        //fprintf (Out_Score,"%d \t %6.3f\t %6.3f\n", j, -999., -999.);
    }
    
    //fclose (Out_Score);
    //fclose (Out_Table);
    if (i<MATR_TBP_Length){
        return -9999;
    }

    *lenSeq = i;

    return 1;
}
