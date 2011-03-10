#include "KalignAdapter.h"
#include "KalignException.h"
#include "KalignConstants.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/Task.h>
#include <U2Core/GAutoDeleteList.h>
#include <U2Core/MAlignment.h>

#include <algorithm>
#include <cctype>

#include <QtCore/QVector>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

extern "C" {
#include "kalign2/kalign2.h"
}

namespace U2 {

//////////////////////////////////////////////////////////////////////////
void KalignAdapter::align(const MAlignment& ma, MAlignment& res, TaskStateInfo& ti) {
	if(ti.cancelFlag)  {
		return;
	}
	try { 
		alignUnsafe(ma, res, ti);
	} catch (KalignException e) {
		if (!ti.cancelFlag) {
			ti.setError(  tr("Internal Kalign error: %1").arg(e.str) );
		}
	}
}

void KalignAdapter::alignUnsafe(const MAlignment& ma, MAlignment& res, TaskStateInfo& ti) {
	ti.progress = 0;
	int* tree = 0;
	quint32 a, b, c;

	struct alignment* aln = 0;
	struct parameters* param = 0;
	struct aln_tree_node* tree2 = 0;

	
	param = (parameters*)malloc(sizeof(struct parameters));

	param =  interface(param,0,0);

	kalign_context *ctx = get_kalign_context();
	unsigned int &numseq = ctx->numseq;
	unsigned int &numprofiles = ctx->numprofiles;

	if (ma.getNumRows() < 2){
		if (!numseq){
			k_printf("No sequences found.\n\n");
		} else {
			k_printf("Only one sequence found.\n\n");
		}
		free_param(param);
	    throw KalignException("Can't align less then 2 sequences");
	}

	if(ctx->gpo != -1) {
		param->gpo = ctx->gpo;
	}
	if(ctx->gpe != -1) {
		param->gpe = ctx->gpe;
	}
	if(ctx->tgpe != -1) {
		param->tgpe = ctx->tgpe;
	}
	if(ctx->secret != -1) {
		param->secret = ctx->secret;
	}

	/************************************************************************/
	/* Convert MA to aln                                                    */
	/************************************************************************/
	k_printf("Prepare data");
	numseq = ma.getNumRows();
	numprofiles = (numseq << 1) - 1;
	aln = aln_alloc(aln);
	for(quint32 i = 0 ; i < numseq; i++) {
		const MAlignmentRow& row= ma.getRow(i);
		aln->sl[i] = row.getCoreLength() - row.getCore().count('-');
		aln->lsn[i] = row.getName().length();
	}

	for (quint32 i = 0; i < numseq;i++){
		aln->s[i] = (int*) malloc(sizeof(int)*(aln->sl[i]+1));
		aln->seq[i] = (char*)malloc(sizeof(char)*(aln->sl[i]+1));
		aln->sn[i] = (char*)malloc(sizeof(char)*(aln->lsn[i]+1));
	}

	int aacode[26] = {0,1,2,3,4,5,6,7,8,-1,9,10,11,12,23,13,14,15,16,17,17,18,19,20,21,22};
	for(quint32 i = 0; i < numseq; i++) {
		const MAlignmentRow& row= ma.getRow(i);
		qstrncpy(aln->sn[i], row.getName().toAscii(), row.getName().length() + 1); //+1 to include '\0'
		QString gapless = QString(row.getCore()).remove('-');
		qstrncpy(aln->seq[i], gapless.toAscii(), gapless.length() + 1);	//+1 to include '\0'
		for (quint32 j = 0; j < aln->sl[i]; j++) {
			if (isalpha((int)aln->seq[i][j])){
				aln->s[i][j] = aacode[toupper(aln->seq[i][j])-65];
			} else {
				aln->s[i][j] = -1;
			}
		}
		aln->s[i][aln->sl[i]] = 0;
		aln->seq[i][aln->sl[i]] = 0;
		aln->sn[i][aln->lsn[i]] = 0;
	}

	/*for(int i=0;i<numseq;i++) {
		for(int j=0;j<aln->sl[i];j++)
			printf("%d  ", aln->s[i][j]);
	}*/

	//aln_dump(aln);

	//aln = detect_and_read_sequences(aln,param);

	if(param->ntree > (int)numseq){
		param->ntree = (int)numseq;
	}

	//DETECT DNA
	if(param->dna == -1){
		for (quint32 i = 0; i < numseq;i++){
			param->dna = byg_detect(aln->s[i],aln->sl[i]);
			if(param->dna){
				break;
			}
		}
	}
	//param->dna = 0;
	//k_printf("DNA:%d\n",param->dna);
	//exit(0);

	if(param->dna == 1){
		//brief sanity check...
		for (quint32 i = 0; i < numseq;i++){
			if(aln->sl[i] < 6){
				//k_printf("Dna/Rna alignments are only supported for sequences longer than 6.");
				free(param);
				free_aln(aln);
				throw KalignException("Dna/Rna alignments are only supported for sequences longer than 6.");
			}
		}
		aln =  make_dna(aln);
	}

	//int j;

	//fast distance calculation;
	float** submatrix = 0;
	submatrix = read_matrix(submatrix,param); // sets gap penalties as well.....

	//if(byg_start(param->alignment_type,"profPROFprofilePROFILE") != -1){
	//	profile_alignment_main(aln,param,submatrix);
	//}

	float** dm = 0;
	if(param->ntree > 1){
		//if(byg_start(param->distance,"pairclustalPAIRCLUSTAL") != -1){
		//	if(byg_start(param->tree,"njNJ") != -1){
		//		dm = protein_pairwise_alignment_distance(aln,dm,param,submatrix,1);
		//	}else{
		//		dm = protein_pairwise_alignment_distance(aln,dm,param,submatrix,0);
		//	}
		//}else if(byg_start("wu",param->alignment_type) != -1){
		//	dm =  protein_wu_distance2(aln,dm,param);
		//	//	param->feature_type = "wumanber";
		if(param->dna == 1){
		//	if(byg_start(param->tree,"njNJ") != -1){
		//		dm =  dna_distance(aln,dm,param,1);
		//	}else{
				dm =  dna_distance(aln,dm,param,0);
		//	}
		}else{
			//if(byg_start(param->tree,"njNJ") != -1){
			//	dm =  protein_wu_distance(aln,dm,param,1);
			//}else{
				dm =  protein_wu_distance(aln,dm,param,0);
			//}
		}
		if(check_task_canceled(ctx)) {
			for (int i = 32;i--;){
				free(submatrix[i]);
			}
			free(submatrix);
			for (int i = numseq;i--;){
				free(dm[i]);
			}
			free(dm);
			free_aln(aln);
			free_param(param);
			throwKalignException("Canceled");
		}
		/*int j; 
		for (int i = 0; i< numseq;i++){
		for (j = 0; j< numseq;j++){
		k_printf("%f	",dm[i][j]);
		}
		k_printf("\n");
		}*/

		//if(byg_start(param->tree,"njNJ") != -1){
		//	tree2 = real_nj(dm,param->ntree);
		//}else{
			tree2 = real_upgma(dm,param->ntree);
		//}
		//if(param->print_tree){
		//	print_tree(tree2,aln,param->print_tree);
		//}
	}

	tree = (int*) malloc(sizeof(int)*(numseq*3+1));
	for (quint32 i = 1; i < (numseq*3)+1;i++){
		tree[i] = 0;
	}
	tree[0] = 1; 

	if(param->ntree < 2){
		tree[0] = 0;
		tree[1] = 1;

		c = numseq;
		tree[2] = c;
		a = 2;
		for (quint32 i = 3; i < (numseq-1)*3;i+=3){
			tree[i] = c;
			tree[i+1] = a;
			c++;
			tree[i+2] = c;
			a++;
		}
	}else if(param->ntree > 2){
		ntreeify(tree2,param->ntree);
	}else{
		tree = readtree(tree2,tree);
		for (quint32 i = 0; i < (numseq*3);i++){
			tree[i] = tree[i+1];
		}
		free(tree2->links);
		free(tree2->internal_lables);
		free(tree2);
	}




	//get matrices... 
	//struct feature_matrix* fm = 0;

	//struct ntree_data* ntree_data = 0;

	int** map = 0;
	//if(param->ntree > 2){
	//	ntree_data = (struct ntree_data*)malloc(sizeof(struct ntree_data));
	//	ntree_data->realtree = tree2;
	//	ntree_data->aln = aln;
	//	ntree_data->profile = 0;
	//	ntree_data->map = 0;
	//	ntree_data->ntree = param->ntree;
	//	ntree_data->submatrix = submatrix;
	//	ntree_data->tree = tree; 

	//	ntree_data = ntree_alignment(ntree_data);
	//	map = ntree_data->map;
	//	tree = ntree_data->tree;
	//	for (int i = 0; i < (numseq*3);i++){
	//		tree[i] = tree[i+1];
	//	}
	//	free(ntree_data);
	//}else if (param->feature_type){
	//	fm = get_feature_matrix(fm,aln,param);
	//	if(!fm){
	//		for (int i = 32;i--;){
	//			free(submatrix[i]);
	//		}
	//		free(submatrix);
	//		free_param(param);
	//		free(map);
	//		free(tree);
	//		throw KalignException("getting feature matrix error");
	//	}

	//	map = feature_hirschberg_alignment(aln,tree,submatrix,map,fm);
	//	//exit(0);
	//	//map =  feature_alignment(aln,tree,submatrix, map,fm);

	//}else if (byg_start("pairwise",param->alignment_type) != -1){
	//	if(param->dna == 1){
	//		map = dna_alignment_against_a(aln,tree,submatrix, map,param->gap_inc);
	//	}else{
	//		map = hirschberg_alignment_against_a(aln,tree,submatrix, map,param->smooth_window,param->gap_inc);
	//	}
	//	//map =  default_alignment(aln,tree,submatrix, map);
	//}else if (byg_start("fast",param->alignment_type) != -1){
	//	map =  default_alignment(aln,tree,submatrix, map);
	if(param->dna == 1){
		map =  dna_alignment(aln,tree,submatrix, map,param->gap_inc);
	//	/*}else if (byg_start("test",param->alignment_type) != -1){
	//	map =  test_alignment(aln,tree,submatrix, map,param->internal_gap_weight,param->smooth_window,param->gap_inc);
	//	}else if (param->aa){
	//	map =  aa_alignment(aln,tree,submatrix, map,param->aa);
	//	}else if (param->alter_gaps){
	//	map = alter_gaps_alignment(aln,tree,submatrix,map,param->alter_gaps,param->alter_range,param->alter_weight);
	//	}else if (byg_start("altergaps",param->alignment_type) != -1){
	//	map = alter_gaps_alignment(aln,tree,submatrix,map,param->alter_gaps,param->alter_range,param->alter_weight);
	//	}else if(byg_start("simple",param->alignment_type) != -1){
	//	map =  simple_hirschberg_alignment(aln,tree,submatrix, map);*/
	//}else if(byg_start("advanced",param->alignment_type) != -1){
	//	map =  advanced_hirschberg_alignment(aln,tree,submatrix, map,param->smooth_window,param->gap_inc,param->internal_gap_weight);
	}else{
		map =  hirschberg_alignment(aln,tree,submatrix, map,param->smooth_window,param->gap_inc);
	}
	if(check_task_canceled(ctx)) {
		free_param(param);
		free_aln(aln);
		free(map);
		free(tree);
		throwKalignException("Canceled");
	}


	//clear up sequence array to be reused as gap array....
	int *p = 0;
	for (quint32 i = 0; i < numseq;i++){
		p = aln->s[i];
		for (a = 0; a < aln->sl[i];a++){
			p[a] = 0;
		}
	}
	//clear up

	for (quint32 i = 0; i < (numseq-1)*3;i +=3){
		a = tree[i];
		b = tree[i+1];
		aln = make_seq(aln,a,b,map[tree[i+2]]);
	}

	//for (int i = 0; i < numseq;i++){
	//	k_printf("%s	%d\n",aln->sn[i],aln->nsip[i]);
	//}


	for (quint32 i = 0; i < numseq;i++){
		aln->nsip[i] = 0;
	}



	aln =  sort_sequences(aln,tree,param->sort);

	//for (int i = 0; i < numseq;i++){
	//	k_printf("%d	%d	%d\n",i,aln->nsip[i],aln->sip[i][0]);
	//}

	/************************************************************************/
	/* Convert aln to MA                                                    */
	/************************************************************************/
	res.setAlphabet(ma.getAlphabet());
	for (quint32 i = 0; i < numseq;i++){
		int f = aln->nsip[i];
		QString seq;
		for(quint32 j=0;j<aln->sl[f];j++) {
			seq += QString(aln->s[f][j],'-') + aln->seq[f][j];
		}
		seq += QString(aln->s[f][aln->sl[f]],'-');
		res.addRow(MAlignmentRow(QString(aln->sn[f]), seq.toAscii()));
	}




	//output(aln,param);
	/*	if(!param->format){
	fasta_output(aln,param->outfile);
	}else{
	if (byg_start("msf",param->format) != -1){
	msf_output(aln,param->outfile);
	}else if (byg_start("clustal",param->format) != -1){
	clustal_output(aln,param->outfile);
	}else if (byg_start("macsim",param->format) != -1){
	macsim_output(aln,param->outfile,param->infile[0]);
	}
	}
	*/
	free_param(param);
	free_aln(aln);
	free(map);
	free(tree);
	//KalignContext* ctx = getKalignContext();

}

} //namespace
