#include "muscle.h"
#include "textfile.h"
#include "msa.h"
#include "tree.h"
#include "profile.h"
#include "objscore.h"
#include "muscle_context.h"

bool TreeNeededForWeighting(SEQWEIGHT s)
	{
	switch (s)
		{
	case SEQWEIGHT_ClustalW:
	case SEQWEIGHT_ThreeWay:
		return true;
	default:
		return false;
		}
	}

static ProfPos *ProfileFromMSALocal(MSA &msa, Tree &tree)
	{
    MuscleContext *ctx = getMuscleContext();
	const unsigned uSeqCount = msa.GetSeqCount();
	for (unsigned uSeqIndex = 0; uSeqIndex < uSeqCount; ++uSeqIndex)
		msa.SetSeqId(uSeqIndex, uSeqIndex);

	if (TreeNeededForWeighting(ctx->params.g_SeqWeight2))
		{
		TreeFromMSA(msa, tree, ctx->params.g_Cluster2, ctx->params.g_Distance2, ctx->params.g_Root1);
		SetMuscleTree(tree);
		}
	return ProfileFromMSA(msa);
	}

void ProfileProfile(MSA &msa1, MSA &msa2, MSA &msaOut)
	{
	//ALPHA Alpha = ALPHA_Undefined;
	//switch (g_SeqType)
	//	{
	//case SEQTYPE_Auto:
	//	Alpha = msa1.GuessAlpha();
	//	break;

	//case SEQTYPE_Protein:
	//	Alpha = ALPHA_Amino;
	//	break;

	//case SEQTYPE_DNA:
	//	Alpha = ALPHA_DNA;
	//	break;

	//case SEQTYPE_RNA:
	//	Alpha = ALPHA_RNA;
	//	break;

	//default:
	//	Quit("Invalid SeqType");
	//	}
	//SetAlpha(Alpha);

	//msa1.FixAlpha();
	//msa2.FixAlpha();

	unsigned uLength1;
	unsigned uLength2;

	uLength1 = msa1.GetColCount();
	uLength2 = msa2.GetColCount();

	Tree tree1;
	Tree tree2;
	ProfPos *Prof1 = ProfileFromMSALocal(msa1, tree1);
	ProfPos *Prof2 = ProfileFromMSALocal(msa2, tree2);

	PWPath Path;
	ProfPos *ProfOut;
	unsigned uLengthOut;
	Progress("Aligning profiles");
	AlignTwoProfs(Prof1, uLength1, 1.0, Prof2, uLength2, 1.0, Path, &ProfOut, &uLengthOut);

	Progress("Building output");
	AlignTwoMSAsGivenPath(Path, msa1, msa2, msaOut);
	}

// Do profile-profile alignment
void Profile()
	{
    MuscleContext *ctx = getMuscleContext();
    
	if (0 == ctx->params.g_pstrFileName1 || 0 == ctx->params.g_pstrFileName2)
		Quit("-profile needs -in1 and -in2");

	SetSeqWeightMethod(ctx->params.g_SeqWeight1);

	TextFile file1(ctx->params.g_pstrFileName1);
	TextFile file2(ctx->params.g_pstrFileName2);

	MSA msa1;
	MSA msa2;
	MSA msaOut;

	Progress("Reading %s", ctx->params.g_pstrFileName1);
	msa1.FromFile(file1);
	Progress("%u seqs %u cols", msa1.GetSeqCount(), msa1.GetColCount());

	Progress("Reading %s", ctx->params.g_pstrFileName2);
	msa2.FromFile(file2);
	Progress("%u seqs %u cols", msa2.GetSeqCount(), msa2.GetColCount());

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
		Quit("Invalid seq type");
		}
	SetAlpha(Alpha);

	msa1.FixAlpha();
	msa2.FixAlpha();

	SetPPScore();
	if (ALPHA_DNA == Alpha || ALPHA_RNA == Alpha)
		SetPPScore(PPSCORE_SPN);

	const unsigned uSeqCount1 = msa1.GetSeqCount();
	const unsigned uSeqCount2 = msa2.GetSeqCount();
	const unsigned uSumSeqCount = uSeqCount1 + uSeqCount2;
	MSA::SetIdCount(uSumSeqCount);

	ProfileProfile(msa1, msa2, msaOut);

	Progress("Writing output");
	MuscleOutput(msaOut);
	}
