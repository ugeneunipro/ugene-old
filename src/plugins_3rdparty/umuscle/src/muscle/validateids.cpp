#include "muscle.h"
#include "msa.h"
#include "tree.h"
#include "seqvect.h"
#include "muscle_context.h"

#if	DEBUG

void SetMuscleInputMSA(MSA &msa)
	{
	getMuscleContext()->validateids.MuscleInputMSA.Copy(msa);
	}

void SetMuscleSeqVect(SeqVect &v)
	{
	getMuscleContext()->validateids.g_ptrMuscleSeqVect = &v;
	}

void ValidateMuscleIdsSeqVect(const MSA &msa)
	{
    MuscleContext *ctx = getMuscleContext();
	const unsigned uSeqCount = msa.GetSeqCount();
	for (unsigned uSeqIndex = 0; uSeqIndex < uSeqCount; ++uSeqIndex)
		{
		const unsigned uId = msa.GetSeqId(uSeqIndex);
		const char *ptrNameMSA = msa.GetSeqName(uSeqIndex);
		const char *ptrName = ctx->validateids.g_ptrMuscleSeqVect->GetSeqName(uId);
		if (0 != strcmp(ptrNameMSA, ptrName))
			Quit("ValidateMuscleIdsSeqVect, names don't match");
		}
	}

void ValidateMuscleIdsMSA(const MSA &msa)
	{
    MuscleContext *ctx = getMuscleContext();
    MSA &MuscleInputMSA = ctx->validateids.MuscleInputMSA;
	const unsigned uSeqCount = msa.GetSeqCount();
	for (unsigned uSeqIndex = 0; uSeqIndex < uSeqCount; ++uSeqIndex)
		{
		const unsigned uId = msa.GetSeqId(uSeqIndex);
		const char *ptrNameMSA = msa.GetSeqName(uSeqIndex);
		const char *ptrName = ctx->validateids.MuscleInputMSA.GetSeqName(uId);
		if (0 != strcmp(ptrNameMSA, ptrName))
			{
			Log("Input MSA:\n");
			MuscleInputMSA.LogMe();
			Log("MSA being tested:\n");
			msa.LogMe();
			Log("Id=%u\n", uId);
			Log("Input name=%s\n", ptrName);
			Log("Test name=%s\n", ptrNameMSA);
			Quit("ValidateMuscleIdsMSA, names don't match");
			}
		}
	}

void ValidateMuscleIds(const MSA &msa)
	{
    MuscleContext *ctx = getMuscleContext();
	if (0 != ctx->validateids.g_ptrMuscleSeqVect)
		ValidateMuscleIdsSeqVect(msa);
	else if (0 != ctx->validateids.MuscleInputMSA.GetSeqCount())
		ValidateMuscleIdsMSA(msa);
	else
		Quit("ValidateMuscleIds, ptrMuscleSeqVect=0 && 0 == MuscleInputMSA.SeqCount()");

	}

void ValidateMuscleIdsSeqVect(const Tree &tree)
	{
    MuscleContext *ctx = getMuscleContext();
	const unsigned uSeqCount = ctx->validateids.g_ptrMuscleSeqVect->GetSeqCount();
	const unsigned uNodeCount = tree.GetNodeCount();
	for (unsigned uNodeIndex = 0; uNodeIndex < uNodeCount; ++uNodeIndex)
		{
		if (!tree.IsLeaf(uNodeIndex))
			continue;
		const unsigned uId = tree.GetLeafId(uNodeIndex);
		if (uId >= uSeqCount)
			{
			tree.LogMe();
			Quit("Leaf with node index %u has id=%u, there are %u seqs",
			  uNodeIndex, uId, uSeqCount);
			}
		const char *ptrNameTree = tree.GetLeafName(uNodeIndex);
		const char *ptrName = ctx->validateids.g_ptrMuscleSeqVect->GetSeqName(uId);
		if (0 != strcmp(ptrNameTree, ptrName))
			Quit("ValidateMuscleIds: names don't match");
		}
	}

void ValidateMuscleIdsMSA(const Tree &tree)
	{
    MuscleContext *ctx = getMuscleContext();
	const unsigned uNodeCount = tree.GetNodeCount();
	for (unsigned uNodeIndex = 0; uNodeIndex < uNodeCount; ++uNodeIndex)
		{
		if (!tree.IsLeaf(uNodeIndex))
			continue;
		const unsigned uId = tree.GetLeafId(uNodeIndex);
		const char *ptrNameTree = tree.GetLeafName(uNodeIndex);
		const char *ptrName = ctx->validateids.MuscleInputMSA.GetSeqName(uId);
		if (0 != strcmp(ptrNameTree, ptrName))
			Quit("ValidateMuscleIds: names don't match");
		}
	}

void ValidateMuscleIds(const Tree &tree)
	{
    MuscleContext *ctx = getMuscleContext();
   	if (0 != ctx->validateids.g_ptrMuscleSeqVect)
		ValidateMuscleIdsSeqVect(tree);
	else if (0 != ctx->validateids.MuscleInputMSA.GetSeqCount())
		ValidateMuscleIdsMSA(tree);
	else
		Quit("ValidateMuscleIds, ptrMuscleSeqVect=0 && 0 == MuscleInputMSA.SeqCount");
	}
#endif
