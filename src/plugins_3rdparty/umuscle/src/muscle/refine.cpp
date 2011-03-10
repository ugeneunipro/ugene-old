#include "muscle.h"
#include "textfile.h"
#include "seqvect.h"
#include "distfunc.h"
#include "msa.h"
#include "tree.h"
#include "clust.h"
#include "profile.h"
#include "clustsetmsa.h"
#include "muscle_context.h"

void Refine()
	{
    MuscleContext *ctx = getMuscleContext();

	SetOutputFileName(ctx->params.g_pstrOutFileName);
	SetInputFileName(ctx->params.g_pstrInFileName);
	SetStartTime();

	SetMaxIters(ctx->params.g_uMaxIters);
	SetSeqWeightMethod(ctx->params.g_SeqWeight1);

	TextFile fileIn(ctx->params.g_pstrInFileName);
	MSA msa;
	msa.FromFile(fileIn);

	const unsigned uSeqCount = msa.GetSeqCount();
	if (0 == uSeqCount)
		Quit("No sequences in input file");

	ALPHA Alpha = ALPHA_Undefined;
	switch (ctx->params.g_SeqType)
		{
	case SEQTYPE_Auto:
		Alpha = msa.GuessAlpha();
		break;

	case SEQTYPE_Protein:
		Alpha = ALPHA_Amino;
		break;

	case SEQTYPE_DNA:
		Alpha = ALPHA_DNA;
		break;

	case SEQTYPE_RNA:
		Alpha = ALPHA_RNA;
		break;

	default:
		Quit("Invalid SeqType");
		}
	SetAlpha(Alpha);
	msa.FixAlpha();

	SetPPScore();
	if (ALPHA_DNA == Alpha || ALPHA_RNA == Alpha)
		SetPPScore(PPSCORE_SPN);

	MSA::SetIdCount(uSeqCount);

// Initialize sequence ids.
// From this point on, ids must somehow propogate from here.
	for (unsigned uSeqIndex = 0; uSeqIndex < uSeqCount; ++uSeqIndex)
		msa.SetSeqId(uSeqIndex, uSeqIndex);
	SetMuscleInputMSA(msa);

	Tree GuideTree;
	TreeFromMSA(msa, GuideTree, ctx->params.g_Cluster2, ctx->params.g_Distance2, ctx->params.g_Root2);
	SetMuscleTree(GuideTree);

	if (ctx->params.g_bAnchors)
		RefineVert(msa, GuideTree, ctx->params.g_uMaxIters);
	else
		RefineHoriz(msa, GuideTree, ctx->params.g_uMaxIters, false, false);

	ValidateMuscleIds(msa);
	ValidateMuscleIds(GuideTree);

//	TextFile fileOut(g_pstrOutFileName, true);
//	msa.ToFile(fileOut);
	MuscleOutput(msa);
	}
