#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <QtGlobal>
#include <QtCore/QTextStream>

#include "nrutil.h"
#include "gor.h"


//disable "unsafe functions" deprecation warnings on MS VS
#ifdef Q_OS_WIN
#pragma warning(disable: 4996)
#endif


#define LSIZE 500
#define BUFSIZE 10000
#define MAXRES 12000
#define MAXLINE 150
#define DISLOCATION 8
#define OFFSET 9        /* DISLOCATION+1 */
#define WINSIZ 17       /* 2*DISLOCATION+1 */
#define NPAIRS 136      /* WINSIZ*(WINSIZ-1)/2 */
#define BLANK 21        /* The index of the character '^' in array amino */
#define interpol_coeff 0.75
#define MINFREQ 10.
#define SKIP_BLANK 0
#define Nterm 3
#define Cterm 2



/*
 * External variables
 */


const float ExpInv = 1.0f / (float) WINSIZ;
const char conf[5] = {' ','H','E','C','S'};



//TODO: get rid of static data
double infopair[3][NPAIRS+1][23][23];
double infodir[3][WINSIZ+1][23];
float nS[4], pS[4];


int runGORIV( QFile& seqDb, QFile& strucDb , char* inputSeq, int numResidues, char* outputSeq )
{
 
/***************************************************************************/
/*                                                                         */
/* GOR secondary structure prediction method version IV                    */
/* J. Garnier, J.-F. Gibrat, B. Robson, Methods in Enzymology,             */
/* R.F. Doolittle Ed., vol 266, 540-553, (1996)                            */
/*                                                                         */
/* For any information please contact: J. Garnier or J.-F. Gibrat          */
/* Unite de Bio-informatique, Batiment des Biotechnologies, I.N.R.A.,      */
/* 78351, Jouy-en-Josas, FRANCE.                                           */
/* tel: 33 (1) 34 65 25 67                                                 */
/* fax: 33 (1) 34 65 22 73                                                 */
/* Email: gibrat@proline.jouy.inra.fr                                      */
/*                                                                         */
/* This program gets its input from the command line                       */
/*                                                                         */
/* This program provides 64.4% residues correctly predicted for a 3 states */
/* prediction (H, E, C) for the 267 proteins of the database using a jack- */
/* knife.                                                                  */
/* Last modification: 07/10/97                                             */
/*                                                                         */
/***************************************************************************/

  char **obs, **seq;
  char **title_obs, **title_seq;
  int nprot_dbase;
  int *temp, *sequence;
  int i;
  int nerr;
  char *predi;
  float **probai;
 
 /*
 * Determine the number of proteinin the Kabsch-Sander data base
 */

  nprot_dbase = 0;
  Q_ASSERT(seqDb.isOpen());
  QTextStream stream(&seqDb);

  while(!stream.atEnd()) {
    QByteArray buf = stream.readLine().toAscii();
    if (buf[0] == '>' || buf[0] == '!') {
        nprot_dbase++;
    }
  }
  seqDb.reset();

  printf("There are %d proteins in Kabsch-Sander database\n\n",nprot_dbase);

/*
 * Memory allocations
 */

  seq = cmatrix(1,nprot_dbase,1,MAXRES);
  obs = cmatrix(1,nprot_dbase,1,MAXRES);
  title_obs = cmatrix(1,nprot_dbase,1,MAXLINE);
  title_seq = cmatrix(1,nprot_dbase,1,MAXLINE);
  temp = ivector(1,nprot_dbase);
  sequence = ivector(1,nprot_dbase);
  predi = cvector(1,MAXRES);
  probai = matrix(1,MAXRES,1,3);
 

/*
 * Input the sequences and observed secondary structures for the data base
 */

  readFile(seqDb,nprot_dbase,seq,title_seq,temp);
  readFile(strucDb,nprot_dbase,obs,title_obs,sequence);

/*
 * Check that the data are consistent in the two files
 */

  nerr = 0;
  for(i = 1; i <= nprot_dbase; i++)
    if(temp[i] != sequence[i]) {
      printf("%dth protein temp= %d nres= %d\n",i,temp[i],sequence[i]);
      printf("%s\n%s\n\n",title_seq[i],title_obs[i]);
      nerr++;
    }
  
  for(i = 1; i <= nprot_dbase; i++)
    if(strncmp(title_seq[i],title_obs[i],100) != 0) {
      printf("\n%dth data base protein\n %s \n %s \n",i,title_seq[i],title_obs[i]);
      nerr++;
    }

  if(nerr > 0) {
    printf("%d errors\n",nerr);
    exit(1);
  }

  /*
  * Calculate the parameters
  */

  Parameters(nprot_dbase,sequence,obs,seq);

  /*
  * Predict the secondary structure of protein pro.
  */

  predic(numResidues,inputSeq,predi,probai);
  First_Pass(numResidues,probai,predi);
  Second_Pass(numResidues,probai,predi);

  /*
  * Print the results for the protein
  */

  strncpy(outputSeq, predi + sizeof(char), numResidues);
    
  /*
  * Free memory
  */

  free_cmatrix(seq,1,nprot_dbase,1,MAXRES);
  free_cmatrix(obs,1,nprot_dbase,1,MAXRES);
  free_cmatrix(title_obs,1,nprot_dbase,1,MAXLINE);
  free_cmatrix(title_seq,1,nprot_dbase,1,MAXLINE);
  free_ivector(temp,1,nprot_dbase);
  free_ivector(sequence,1,nprot_dbase);
  free_cvector(predi,1,MAXRES);
  free_matrix(probai,1,MAXRES,1,3);


  return(0);

}

/*****************************************************************************/
/*                                                                           */
/* This routine reads the sequence and observed secondary structures for all */
/* the proteins in the data base.                                            */
/*                                                                           */
/*****************************************************************************/
void readFile(QFile& file, int nprot, char **obs, char **title, int *pnter)
{
  
    Q_ASSERT(file.isOpen());
    int ip, nres, i;
    char c;
    char *keep;

    QTextStream stream(&file);

    keep = (char *) malloc((size_t) MAXRES*sizeof(char));

    for(ip = 1; ip <= nprot; ip++) {

        QByteArray title_ip = stream.readLine().toAscii();
        title_ip.push_back("\n");
        strncpy(title[ip], title_ip.constData(), title_ip.length() + 1);
        nres = 0;
        while(!stream.atEnd()) {
            stream >> c;
            if (c == '@') {
                break;
            }
            if(c == '\n' || c == ' ' || c =='\t') continue;
            nres++;
            if(nres > MAXRES) {
                printf("The value of MAXRES should be increased: %d",MAXRES);
                exit(1);
            }
            if((c >= 'A' && c < 'Z') && c != 'B' && c != 'J' && c != 'O' && c != 'U') {
                keep[nres] = c;
            }
            else {
                printf("protein: %d residue: %d\n",ip,nres);
                printf("Invalid amino acid type or secondary structure state: ==>%c<==\n",c);
                exit(1);
            }
        }
        
        while(!stream.atEnd()) {
            stream >> c;
            if (c == '\n') {
                break;
            }
        }
        
        for(i = 1; i <= nres; i++)
            obs[ip][i] = keep[i];
        pnter[ip] = nres;
    }

    free(keep);
  
 
}
/*****************************************************************************/
/*                                                                           */
/* This function returns an integer for each amino acid type.                */
/*                                                                           */
/*****************************************************************************/
int seq_indx(int c)
{

  switch(c) {
  case 'A': return(1);
  case 'C': return(2);
  case 'D': return(3);
  case 'E': return(4);
  case 'F': return(5);
  case 'G': return(6);
  case 'H': return(7);
  case 'I': return(8);
  case 'K': return(9);
  case 'L': return(10);
  case 'M': return(11);
  case 'N': return(12);
  case 'P': return(13);
  case 'Q': return(14);
  case 'R': return(15);
  case 'S': return(16);
  case 'T': return(17);
  case 'V': return(18);
  case 'W': return(19);
  case 'Y': return(20);
  case '^': return(21);
  case '-': return(22);
  default : return(23);
  }

}
/*****************************************************************************/
/*                                                                           */
/* This function returns an integer for each secondary structure type.       */
/*                                                                           */
/*****************************************************************************/
int obs_indx(int c)
{

  switch(c) {
  case 'H': 
      return(1);
  case 'E': 
      return(2);
  case 'C': 
      return(3);
  case 'X': 
      return(0);
  default:
      Q_ASSERT(0);
      return -1;
  }

}
/***********************************************************************************************************/
/*                                                                                                         */
/* This routine performs the prediction of the current protein                                             */
/*                                                                                                         */
/***********************************************************************************************************/
void Normalize(float proba[], double v[]);

void predic(int nres, char *seq, char *pred, float **proba)
{
  double it[3];
  int aa1, aa2;
  int konf, ires;
  int dis1, dis2, np;

/*
 * Calculate sum of information values for each secondary structure type (konf)
 */

  for(ires = 1; ires <= nres; ires++) {  
    it[1] = it[2] = 0.0;
    for(dis1 = -DISLOCATION; dis1 <= +DISLOCATION; dis1++) {
      if(ires+dis1 < 1 || ires +dis1 > nres) {
        if(SKIP_BLANK) continue;                 /* If SKIP_BLANK "amino acid" of type ' ', i.e., */
        aa1 = 21;                                /* aa1 = 21 are not included in the calculation  */
      } else {
        aa1 = seq_indx(seq[ires+dis1]);
      }
      for(dis2 = dis1+1; dis2 <= +DISLOCATION; dis2++) {
	if(ires+dis2 < 1 || ires +dis2 > nres) {
	  if(SKIP_BLANK) continue;                 
	  aa2 = 21;                                
	} else {
	  aa2 = seq_indx(seq[ires+dis2]);
	}
	np = (dis1+8) * (WINSIZ-1) - ((dis1+8)*(dis1+9)/2) + (dis2+8);
	for(konf = 1; konf <= 2; konf++) 
	  it[konf] = it[konf] + infopair[konf][np][aa1][aa2];
      }
    }
    for(dis1 = -DISLOCATION; dis1 <= +DISLOCATION; dis1++) {
      if(ires+dis1 < 1 || ires +dis1 > nres) {
        if(SKIP_BLANK) continue;                 
        aa1 = 21;                                
      } else {
        aa1 = seq_indx(seq[ires+dis1]);
      }
      for(konf = 1; konf <= 2; konf++) {
	it[konf] = it[konf] + infodir[konf][dis1+9][aa1];
      }
    }

    Normalize(proba[ires],it);

    pred[ires] = conf[INDMAXVAL(proba[ires],1,3)];

  }

/*
 * If "blank residues" are not included the first Nterm and the last Cterm residues are predicted as coils
 */
    
  if(SKIP_BLANK) {                              
    for(ires = 1; ires <= Nterm; ires++)        
      pred[ires] = 'C';
    for(ires = nres-Cterm+1; ires <= nres; ires++)
      pred[ires] = 'C';
  }

}
/***************************************************************************/
/*                                                                         */
/*                           Routine Parameters                            */
/*                                                                         */
/***************************************************************************/
int seq_indx(int c);
int obs_indx(int c);
void Indices(int np, int *dis1, int *dis2);

void Parameters(int nprot_dbase, int *nres, char **obs, char **seq)
{
/*
 * Compute the frequencies from proteins in the data base.
 *
 */

  int pro;
  int ires;
  int konf, dis, aa1, aa2, np;
  int dis1, dis2;
  static float Singlet[4][WINSIZ+1][23];
  static float Doublet[4][NPAIRS+1][23][23];
  double C1, C2;
  float f1, f2, f3;

  C1 = 2. * ExpInv;
  C2 = 1. - C1;

/*
 * Initialisation
 */

  for(konf = 0; konf < 4; konf++)
    for(dis = 0; dis < WINSIZ+1; dis++)
      for(aa1 = 0; aa1 < 23; aa1++) 
	Singlet[konf][dis][aa1] = 0.0;


  for(konf = 0; konf < 4; konf++)
    for(np = 0; np < NPAIRS+1; np++) 
      for(aa1 = 0; aa1 < 23; aa1++)
	for(aa2 = 0; aa2 < 23; aa2++)
	  Doublet[konf][np][aa1][aa2] = 0.0;

  nS[0] = nS[1] = nS[2] = nS[3] = 0;
  
/*
 * Loop over all the proteins of the data base. 
 */

  for(pro = 1; pro <= nprot_dbase; pro++) {

/*
 * Determine frequencies related to the sequence of the query protein (the 1st row in the alignment)
 */

    for(ires = 1; ires <= nres[pro]; ires++) {

      konf = obs_indx(obs[pro][ires]);
      if(konf == 0)                    /* Skip X conformations, i.e., residues for */
	continue;                      /* which the secondary structure is unknown */
	  
      nS[konf]++;   

      for(dis = -DISLOCATION; dis <= DISLOCATION; dis++) {
	if(ires+dis < 1 || ires+dis > nres[pro])
	  aa1 = BLANK;
	else
	  aa1 = seq_indx(seq[pro][ires+dis]);
	Singlet[konf][dis+OFFSET][aa1] += 1.0;
      }
      
      np = 0;
      for(dis1 = -DISLOCATION; dis1 <= DISLOCATION; dis1++) {
	if(ires+dis1 < 1 || ires+dis1 > nres[pro])
	  aa1 = BLANK;
	else
	  aa1 = seq_indx(seq[pro][ires+dis1]);
	for(dis2 = dis1+1; dis2 <= DISLOCATION; dis2++) {
	  if(ires+dis2 < 1 || ires+dis2 > nres[pro])
	    aa2 = BLANK;
	  else
	    aa2 = seq_indx(seq[pro][ires+dis2]);
	  np++;
	  Doublet[konf][np][aa1][aa2] += 1.0;
	}
      }

    }

  } /* End of loop over the proteins in the data base index pro */

/*
 * Calculate probabilities for the 3 secondary structures, H, E and C.
 */

  nS[0] = nS[1] + nS[2] + nS[3];

  for(konf = 1; konf <= 3; konf++)
    pS[konf] = (float) nS[konf] / (float) nS[0];

/*
 * Calculate information parameters (sort of)
 */

  for(konf = 1; konf <= 2; konf++) {
    for(np = 1; np <= NPAIRS; np++) {
      for(aa1 = 1; aa1 <= 21; aa1++) {
	for(aa2 = 1; aa2 <= 21; aa2++) {
	  f1 = Doublet[konf][np][aa1][aa2];
	  f2 = Doublet[3][np][aa1][aa2];
	  if(f1 < MINFREQ) {
	    Indices(np,&dis1,&dis2);
	    f3 = Singlet[konf][dis1][aa1] * Singlet[konf][dis2][aa1] / (float) nS[konf];
	    f1 = (f3 - f1) * (float) interpol_coeff + f1;
	    if(f1 < 1.e-6) f1 = 1.0;
	  }
	  if(f2 < MINFREQ) {
	    Indices(np,&dis1,&dis2);
	    f3 = Singlet[3][dis1][aa1] * Singlet[3][dis2][aa1] / (float) nS[3];
	    f2 = (f3 - f2) * (float) interpol_coeff + f2;
	    if(f2 < 1.e-6) f2 = 1.0;
	  }
	  infopair[konf][np][aa1][aa2] = C1 * (log(f1)-log(f2));
	}
      }
    }
  }

  for(konf = 1; konf <= 2; konf++) {
    for(dis = 1; dis <= WINSIZ; dis++) {
      for(aa1 = 1; aa1 <= 21; aa1++) {
	f1 = Singlet[konf][dis][aa1];
	f2 = Singlet[3][dis][aa1];
	if(f1 < 1.e-6) f1 = 1.0;
	if(f2 < 1.e-6) f2 = 1.0;
	infodir[konf][dis][aa1] = C2 * (log(f2)- log(f1));
      }
    }
  }
  
}

/*****************************************************************************/
/*                                                                           */
/* Determine indices dis1 dis2 as a function of np                           */
/*                                                                           */
/*****************************************************************************/
void Indices(int np, int *dis1, int *dis2)
{
  int i, j, k;

  k = 0;
  for(i = -DISLOCATION; i <= DISLOCATION; i++) {
    for(j= i+1; j <= DISLOCATION; j++) {
      k++;
      if(k == np) {
	*dis1 = i;
	*dis2 = j;
	return;
      }
    }
  }
  printf("Error invalid value of np= %d\n",np);
  exit(1);
}
/*********************************************************************************/
/*                                                                               */
/*                          Normalize the probabilities                          */
/*                                                                               */
/*********************************************************************************/
void Normalize(float proba[], double v[])
{
  double denom;
  
  denom = 1.0 / (1.0 + exp(v[1]) + exp(v[2]));
  proba[1] = (float) ( exp(v[1]) * denom );
  proba[2] = (float)  ( exp(v[2]) * denom );
  proba[3] = (float) denom;
}
/**********************************************************************************/
/*                                                                                */
/* Print out the results for the current protein.                                 */
/*                                                                                */
/**********************************************************************************/
void printout(int nres, char *seq, char *predi, char *title, float **proba, FILE *fp2)
{
  int ires;
  int nlines, nl;

/*
 * Print the results for the current protein
 */

  printf("\n\n>%s\n",title+1);
  nlines = nres / 50 + 1;

  for(nl = 1; nl < nlines; nl++) {

    for(ires = (nl-1)*50+1; ires <= nl*50; ires++) {
      printf("%c",seq[ires]);
      if(ires % 10 == 0) printf("%c",' ');
    }
    printf("    %s\n","Sequence");

    for(ires = (nl-1)*50+1; ires <= nl*50; ires++) {
      printf("%c",predi[ires]);
      if(ires % 10 == 0) printf("%c",' ');
    }
    printf("    %s\n","Predicted Sec. Struct.");

    printf("\n");

  }

  for(ires = (nlines-1)*50+1; ires <= nlines*50; ires++) { /* last -likely incomplete- line */
    if(ires <= nres) {
      printf("%c",seq[ires]);
    } else {
      printf("%c",' ');
    }
    if(ires % 10 == 0) printf("%c",' ');
  }
  printf("    %s\n","Sequence");

  for(ires = (nlines-1)*50+1; ires <= nlines*50; ires++) {
    if(ires <= nres) {
      printf("%c",predi[ires]);
    } else {
      printf("%c",' ');
    }
    if(ires % 10 == 0) printf("%c",' ');
  }
  printf("    %s\n","Predicted Sec. Struct.");

  printf("\n\n");

  if(fp2 != NULL) {
    fprintf(fp2,"\n\n%s\n%d\n",title+1,nres);
    fprintf(fp2,"SEQ PRD   H     E     C\n");
    for(ires = 1; ires <= nres; ires++)
      fprintf(fp2," %c   %c  %5.3f %5.3f %5.3f\n",seq[ires],predi[ires],proba[ires][1],proba[ires][2],proba[ires][3]);
      
  }

}
/***************************************************************************/
/*                                                                         */
/*                           Routine First_Pass                            */
/*                                                                         */
/***************************************************************************/
int obs_indx(int c);

void First_Pass(int nres, float **proba, char *pred)
{
/*
 * 1) Look for areas that are a mixture of Es and Hs.
 * 2) When such an area is isolated check whether Es and Hs occurs in two blocks.
 * If yes and number of Hs > 4 and number of Es > 3 do nothing
 * In all other cases compute the product of probabilities for all residues in the area
 * and assign to this area the conformation having the highest probability over the area.
 *
 */

  int ires;
  int lim1 = 0;
  int open;
  int kk;
  int type;
  int block[3];
  int nseg;
  int size[3] = {0,4,3};
  double ptot[3];

  pred[1] = pred[nres] = 'C';
  open = 0;
  for(ires = 1; ires <= nres; ires++) {
    if(pred[ires] != 'C') {
      if(!open) {
	open = 1;
	lim1 = ires;
      }
    } else {
      if(open) {
	open = 0;
	int lim2 = ires - 1;
	type = obs_indx(pred[lim1]);
	block[1] = block[2] = 0;
	nseg = 1;
	block[nseg]++;
	for(kk = lim1+1; kk <= lim2; kk++) {
	  if(obs_indx(pred[kk]) != type)
	    nseg++;
	  if(nseg <= 2) block[nseg]++;
	  type = obs_indx(pred[kk]);
	}
	if(nseg > 2 || block[1] < size[obs_indx(pred[lim1])] || block[2] < size[obs_indx(pred[lim2])]) {
	  ptot[1] = ptot[2] = 1.0;
	  for(kk = lim1; kk <= lim2; kk++) {
	    ptot[1] = ptot[1] * proba[kk][1];
	    ptot[2] = ptot[2] * proba[kk][2];
	  }
	  if(ptot[1] > ptot[2]) {
	    for(kk = lim1; kk <= lim2; kk++)
	      pred[kk] = 'H';
	  } else {
	    for(kk = lim1; kk <= lim2; kk++)
	      pred[kk] = 'E';
	  }
	}
      }
    }
  }

}
/***************************************************************************/
/*                                                                         */
/*                           Routine Second_Pass                           */
/*                                                                         */
/***************************************************************************/
int obs_indx(int c);

void Second_Pass(int nres, float **proba, char *pred)
{
/*
 * Correct strands having less than 2 and helices having less than 4 residues.
 * Either the secondary structure element is suppressed or additional
 * residues are recruted to reach the required number.
 * 
 */

  int ires, ires1;
  int len;
  int standard[4] = {0,4,2,0};
  int missing;
  int k;
  int lim1, lim2, lim3, lim4, Lim1 = 0, Lim2 = 0, Lim3 = 0, Lim4 = 0, KeepNterm = 0, KeepCterm = 0;
  float cost, costmax;
  int type;
  int type_Cterm, type_Nterm;

  len = 0;
  type = obs_indx(pred[1]);
  for(ires = 2; ires <= nres; ires++) {
    if(type != obs_indx(pred[ires])) {
      if(len < standard[type]) { /* Check all possibilities */
	costmax = 0.0;
	missing = standard[type] - len;
/*
 * Check the cost of increasing the secondary structure element
 */
	lim1 = ires - len - missing;
	for(k = 1; k <= missing+1; k++) {
	  lim2 = lim1 + standard[type] - 1;
	  if(lim1 < 1 || lim2 > nres) {
	    lim1++;
	    continue;
	  }
	  cost = 1.0;
	  for(ires1 = lim1; ires1 <= lim2; ires1++)
	    cost *= proba[ires1][type];
	  if(cost > costmax) {
	    costmax = cost;
	    Lim1 = lim1;
	    Lim2 = lim2;
	    KeepNterm = type;
	    Lim3 = 0;
	    Lim4 = -1;
	  }
	  lim1++;
	}
/*
 * Check the cost of suppressing the secondary structure element using the same segments as previously
 */
	type_Nterm = obs_indx(pred[ires-len-1]);
	type_Cterm = obs_indx(pred[ires]);
	lim1 = ires - len - missing;
	for(k = 1; k <= missing+1; k++) {
	  lim4 = lim1 + standard[type] - 1;
	  if(lim1 < 1 || lim4 > nres) {
	    lim1++;
	    continue;
	  }
	  lim2 = ires - 1;
	  lim3 = lim2 + 1;
	  while(lim3 >= ires - len) {
	    cost = 1.0;
	    for(ires1 = lim1; ires1 <= lim2; ires1++)
	      cost *= proba[ires1][type_Nterm];
	    for(ires1 = lim3; ires1 <= lim4; ires1++)
	      cost *= proba[ires][type_Cterm];
	    if(cost > costmax) {
	      costmax = cost;
	      Lim1 = lim1;
	      Lim2 = lim2;
	      Lim3 = lim3;
	      Lim4 = lim4;
	      KeepNterm = type_Nterm;
	      KeepCterm = type_Cterm;
	    }
	    lim2--;
	    lim3--;
	  }
	  lim1++;
	}
/*
 * Modify pred accordingly
 */
	for(ires1 = Lim1; ires1 <= Lim2; ires1++)
	  pred[ires1] = conf[KeepNterm];
	for(ires1 = Lim3; ires1 <= Lim4; ires1++)
	  pred[ires1] = conf[KeepCterm];
/*
 * Move to the end of the modified segment if necessary
 */
	if(Lim2 > ires || Lim4 > ires) {
	  if(Lim2 > Lim4)
	    ires = Lim2;
	  else 
	    ires = Lim4;
	}

      } /* End of segment correction */
 
      len = 1;
    } else {
      len++;
    }
    type = obs_indx(pred[ires]);
  }

}

