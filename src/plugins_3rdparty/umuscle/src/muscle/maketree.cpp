#include "muscle.h"
#include "msa.h"
#include "textfile.h"
#include "tree.h"
#include "muscle_context.h"

void DoMakeTree()
	{
    MuscleContext *ctx = getMuscleContext();
	if (ctx->params.g_pstrInFileName == 0 || ctx->params.g_pstrOutFileName == 0)
		Quit("-maketree requires -in <msa> and -out <treefile>");

	SetStartTime();

	SetSeqWeightMethod(ctx->params.g_SeqWeight1);

	TextFile MSAFile(ctx->params.g_pstrInFileName);

	MSA msa;
	msa.FromFile(MSAFile);

	unsigned uSeqCount = msa.GetSeqCount();
	MSA::SetIdCount(uSeqCount);

// Initialize sequence ids.
// From this point on, ids must somehow propogate from here.
	for (unsigned uSeqIndex = 0; uSeqIndex < uSeqCount; ++uSeqIndex)
		msa.SetSeqId(uSeqIndex, uSeqIndex);
	SetMuscleInputMSA(msa);

	Progress("%u sequences", uSeqCount);

	Tree tree;
	TreeFromMSA(msa, tree, ctx->params.g_Cluster2, ctx->params.g_Distance2, ctx->params.g_Root2);

	TextFile TreeFile(ctx->params.g_pstrOutFileName, true);
	tree.ToFile(TreeFile);

	Progress("Tree created");
	}
