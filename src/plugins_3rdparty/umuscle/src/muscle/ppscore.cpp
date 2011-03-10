#include "muscle.h"
#include "textfile.h"
#include "msa.h"
#include "tree.h"
#include "profile.h"
#include "objscore.h"
#include "muscle_context.h"

static ProfPos *ProfileFromMSALocal(MSA &msa, Tree &tree)
    {
    MuscleContext *ctx = getMuscleContext();
	const unsigned uSeqCount = msa.GetSeqCount();
	for (unsigned uSeqIndex = 0; uSeqIndex < uSeqCount; ++uSeqIndex)
		msa.SetSeqId(uSeqIndex, uSeqIndex);

	TreeFromMSA(msa, tree, ctx->params.g_Cluster2, ctx->params.g_Distance2, ctx->params.g_Root1);
	SetMuscleTree(tree);
	return ProfileFromMSA(msa);
	}

void PPScore()
	{
    MuscleContext *ctx = getMuscleContext();
	if (0 == ctx->params.g_pstrFileName1 || 0 == ctx->params.g_pstrFileName2)
		Quit("-ppscore needs -in1 and -in2");

	SetSeqWeightMethod(ctx->params.g_SeqWeight1);

	TextFile file1(ctx->params.g_pstrFileName1);
	TextFile file2(ctx->params.g_pstrFileName2);

	MSA msa1;
	MSA msa2;

	msa1.FromFile(file1);
	msa2.FromFile(file2);

	const unsigned uLength1 = msa1.GetColCount();
	const unsigned uLength2 = msa2.GetColCount();

	if (uLength1 != uLength2)
		Quit("Profiles must have the same length");

	ALPHA Alpha = ALPHA_Undefined;
	switch (ctx->params.g_SeqType)
		{
	case SEQTYPE_Auto:
		Alpha = msa1.GuessAlpha();
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

	msa1.FixAlpha();
	msa2.FixAlpha();

	if (ALPHA_DNA == Alpha || ALPHA_RNA == Alpha)
		SetPPScore(PPSCORE_SPN);

	const unsigned uSeqCount1 = msa1.GetSeqCount();
	const unsigned uSeqCount2 = msa2.GetSeqCount();
	const unsigned uMaxSeqCount = (uSeqCount1 > uSeqCount2 ? uSeqCount1 : uSeqCount2);
	MSA::SetIdCount(uMaxSeqCount);

	Tree tree1;
	Tree tree2;
	ProfPos *Prof1 = ProfileFromMSALocal(msa1, tree1);
	ProfPos *Prof2 = ProfileFromMSALocal(msa2, tree2);

	ctx->ppscore.g_bTracePPScore = true;
	ctx->ppscore.g_ptrPPScoreMSA1 = &msa1;
	ctx->ppscore.g_ptrPPScoreMSA2 = &msa2;

	SCORE Score = ObjScoreDP_Profs(Prof1, Prof2, uLength1);

	Log("Score=%.4g\n", Score);
	printf("Score=%.4g\n", Score);
	}
