#include "muscle.h"
#include "msa.h"
#include "tree.h"
#include "profile.h"
#include <stdio.h>
#include "muscle_context.h"

#define TRACE	0

void RefineTreeE(MSA &msa, const SeqVect &v, Tree &tree, ProgNode *ProgNodes)
	{
    MuscleContext *ctx = getMuscleContext();
	const unsigned uSeqCount = msa.GetSeqCount();
	if (tree.GetLeafCount() != uSeqCount)
		Quit("Refine tree, tree has different number of nodes");

	if (uSeqCount < 3)
		return;

#if	DEBUG
	ValidateMuscleIds(msa);
	ValidateMuscleIds(tree);
#endif

	const unsigned uNodeCount = tree.GetNodeCount();
	unsigned *uNewNodeIndexToOldNodeIndex= new unsigned[uNodeCount];

	Tree Tree2;
	TreeFromMSA(msa, Tree2, ctx->params.g_Cluster2, ctx->params.g_Distance2, ctx->params.g_Root2, ctx->params.g_pstrDistMxFileName2);

#if	DEBUG
	ValidateMuscleIds(Tree2);
#endif

	DiffTreesE(Tree2, tree, uNewNodeIndexToOldNodeIndex);

	unsigned uRoot = Tree2.GetRootNodeIndex();
	if (NODE_CHANGED == uNewNodeIndexToOldNodeIndex[uRoot])
		{
		MSA msa2;
		RealignDiffsE(msa, v, Tree2, tree, uNewNodeIndexToOldNodeIndex, msa2, ProgNodes);
        if (!ctx->isCanceled()) {
            tree.Copy(Tree2);
		    msa.Copy(msa2);
#if	DEBUG
            ValidateMuscleIds(msa2);
#endif
        }
		}

	delete[] uNewNodeIndexToOldNodeIndex;

    if (ctx->isCanceled()) {
        throw MuscleException("Canceled");
    }

	SetCurrentAlignment(msa);
	ProgressStepsDone();

	}
