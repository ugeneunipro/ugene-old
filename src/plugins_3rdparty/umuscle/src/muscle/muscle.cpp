#include "muscle.h"
#include "msa.h"
#include "seqvect.h"
#include "msa.h"
#include "tree.h"
#include "profile.h"
#include "muscle_context.h"

void MUSCLE(SeqVect &v, MSA &msaOut)
	{
    MuscleContext *ctx = getMuscleContext();
	const unsigned uSeqCount = v.Length();

	if (0 == uSeqCount)
		Quit("No sequences in input file");

	ALPHA Alpha = ALPHA_Undefined;
	switch (ctx->params.g_SeqType)
		{
	case SEQTYPE_Auto:
		Alpha = v.GuessAlpha();
		break;

	case SEQTYPE_Protein:
		Alpha = ALPHA_Amino;
		break;

	case SEQTYPE_RNA:
		Alpha = ALPHA_RNA;
		break;

	case SEQTYPE_DNA:
		Alpha = ALPHA_DNA;
		break;

	default:
		Quit("Invalid seq type");
		}
	SetAlpha(Alpha);
	v.FixAlpha();

	if (ALPHA_DNA == Alpha || ALPHA_RNA == Alpha)
		{
		SetPPScore(PPSCORE_SPN);
		ctx->params.g_Distance1 = DISTANCE_Kmer4_6;
		}

	unsigned uMaxL = 0;
	unsigned uTotL = 0;
	for (unsigned uSeqIndex = 0; uSeqIndex < uSeqCount; ++uSeqIndex)
		{
		unsigned L = v.GetSeq(uSeqIndex).Length();
		uTotL += L;
		if (L > uMaxL)
			uMaxL = L;
		}

	SetIter(1);
	ctx->params.g_bDiags = ctx->params.g_bDiags1;
	SetSeqStats(uSeqCount, uMaxL, uTotL/uSeqCount);

	MSA::SetIdCount(uSeqCount);

//// Initialize sequence ids.
//// From this point on, ids must somehow propogate from here.
//	for (unsigned uSeqIndex = 0; uSeqIndex < uSeqCount; ++uSeqIndex)
//		v.SetSeqId(uSeqIndex, uSeqIndex);

	if (uSeqCount > 1)
		MHackStart(v);

	if (0 == uSeqCount)
		{
		msaOut.Clear();
		return;
		}

	if (1 == uSeqCount && ALPHA_Amino == Alpha)
		{
		const Seq &s = v.GetSeq(0);
		msaOut.FromSeq(s);
		return;
		}

// First iteration
	Tree GuideTree;
	TreeFromSeqVect(v, GuideTree, ctx->params.g_Cluster1, ctx->params.g_Distance1, ctx->params.g_Root1);

	SetMuscleTree(GuideTree);

	ProgNode *ProgNodes = 0;
	if (ctx->params.g_bLow)
		ProgNodes = ProgressiveAlignE(v, GuideTree, msaOut);
	else
		ProgressiveAlign(v, GuideTree, msaOut);
	SetCurrentAlignment(msaOut);

	if (1 == ctx->params.g_uMaxIters || 2 == uSeqCount)
		{
		MHackEnd(msaOut);
		return;
		}

	ctx->params.g_bDiags = ctx->params.g_bDiags2;
	SetIter(2);

	if (ctx->params.g_bLow)
		{
		if (0 != ctx->params.g_uMaxTreeRefineIters)
			RefineTreeE(msaOut, v, GuideTree, ProgNodes);
		}
	else
		RefineTree(msaOut, GuideTree);

	extern void DeleteProgNode(ProgNode &Node);
	const unsigned uNodeCount = GuideTree.GetNodeCount();
	for (unsigned uNodeIndex = 0; uNodeIndex < uNodeCount; ++uNodeIndex)
		DeleteProgNode(ProgNodes[uNodeIndex]);

	delete[] ProgNodes;
	ProgNodes = 0;

	SetSeqWeightMethod(ctx->params.g_SeqWeight2);
	SetMuscleTree(GuideTree);

	if (ctx->params.g_bAnchors)
		RefineVert(msaOut, GuideTree, ctx->params.g_uMaxIters - 2);
	else
		RefineHoriz(msaOut, GuideTree, ctx->params.g_uMaxIters - 2, false, false);

	MHackEnd(msaOut);
	}
