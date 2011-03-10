#include "qscore.h"

// O(NL) computation of PREFAB Q score and Balibase TC score.
// Algorithm based on an idea due to Chuong (Tom) Do.
// Each position in the reference alignment is annotated with
// the column number C in the test alignment where the same 
// letter is found. A pair of identical Cs in the same reference
// column indicates a correctly aligned pair of letters.
void FastQ(const MSA_QScore &msaTest, const MSA_QScore &msaRef, double &Q, double &TC,
		   bool WarnIfNoRefAligned)
{
	unsigned CorrectPairCount = 0;
	unsigned RefAlignedPairCount = 0;

	const unsigned RefSeqCount = msaRef.GetSeqCount();
	const unsigned TestSeqCount = msaTest.GetSeqCount();

	const unsigned RefColCount = msaRef.GetColCount();
	const unsigned TestColCount = msaTest.GetColCount();

	StrToInt RefSeqNameToIndex;
	IntVec RefToTestSeqIndex(RefSeqCount);

	for (unsigned RefSeqIndex = 0; RefSeqIndex < RefSeqCount; ++RefSeqIndex)
	{
		const string SeqName = msaRef.GetSeqName(RefSeqIndex);
		RefToTestSeqIndex[RefSeqIndex] = UINT_MAX;
		RefSeqNameToIndex[SeqName] = RefSeqIndex;
	}

	for (unsigned TestSeqIndex = 0; TestSeqIndex < TestSeqCount; ++TestSeqIndex)
	{
		const string SeqName = msaTest.GetSeqName(TestSeqIndex);
		StrToInt::const_iterator p =
			RefSeqNameToIndex.find(SeqName);
		if (p != RefSeqNameToIndex.end())
		{
			unsigned RefSeqIndex = p->second;
			if (RefSeqIndex == UINT_MAX)
				Quit_Qscore("UINT_MAX");
			RefToTestSeqIndex[RefSeqIndex] = TestSeqIndex;
		}
	}

#ifdef DEBUG
	if (!FlagOpt_QScore("ignoremissingseqs"))
		for (unsigned RefSeqIndex = 0; RefSeqIndex < RefSeqCount; ++RefSeqIndex)
		{
			unsigned TestSeqIndex = RefToTestSeqIndex[RefSeqIndex];
			const char *RefSeqName = msaRef.GetSeqName(RefSeqIndex);
			if (TestSeqIndex == UINT_MAX)
			{
				//k_printf( "\n");
				//k_printf( "RefSeqIndex  RefSeqName\n");
				//k_printf( "===========  ==========\n");
				/*for (StrToInt::const_iterator p =
					RefSeqNameToIndex.begin(); p != RefSeqNameToIndex.end(); ++p)
					k_printf( "%11u  %s\n", p->second, (p->first).c_str());
				k_printf( "\n");*/
				Quit_Qscore("Ref seq %u=%.16s not found in test alignment", RefSeqIndex, RefSeqName);
			}
			else
			{
				const char *TestSeqName = msaTest.GetSeqName(TestSeqIndex);
				assert(!strcmp(RefSeqName, TestSeqName));
			}
		}
#endif

		// TestColIndex[i] is the one-based (not zero-based!) test column index
		// of the letter found in the current column of the reference alignment
		// (or the most recent letter if the reference column is gapped, or zero
		// if no letter has yet been found). Here, seq index i is for msaRef.
		IntVec TestColIndex(TestSeqCount, 0);

		// TestColIndexCount[i] is the number of times that a letter from test
		// column i (one-based!) appears in the current reference column.
		IntVec TestColIndexCount(TestColCount+1, 0);

		// TestColIndexes[i] is the column index in the test alignment of
		// the i'th non-gapped position in the current reference column.
		IntVec TestColIndexes;

		unsigned RefAlignedColCount = 0;
		unsigned CorrectColCount = 0;

		for (unsigned RefColIndex = 0; RefColIndex < RefColCount; RefColIndex++)
		{
			TestColIndexes.clear();
			TestColIndexes.reserve(RefSeqCount);

			// NonGappedCount is the number of non-gapped positions in the current
			// reference column.
			unsigned NonGappedCount = 0;
			unsigned FirstTestColIndex = UINT_MAX;
			bool RefColIsAligned = false;
			bool TestColAllCorrect = true;
			bool TestAllAligned = true;
			for (unsigned RefSeqIndex = 0; RefSeqIndex < RefSeqCount; RefSeqIndex++)
			{
				unsigned TestSeqIndex = RefToTestSeqIndex[RefSeqIndex];
				if (TestSeqIndex == UINT_MAX)
				{
					if (FlagOpt_QScore("ignoremissingseqs"))
						continue;
					Quit_Qscore("Test seq %.16s missing", msaRef.GetSeqName(RefSeqIndex));
				}

				char cRef = msaRef.GetChar(RefSeqIndex, RefColIndex);
				if (!IsGap(cRef))
				{
					char cTest = 0;
					unsigned Col = TestColIndex[TestSeqIndex];
					do
					cTest = msaTest.GetChar(TestSeqIndex, Col++);
					while (IsGap(cTest));
					if (toupper(cRef) != toupper(cTest))
					{
						//do nothing
					}
					if (isalpha(cRef) && isupper(cRef))
					{
						RefColIsAligned = true;
						++NonGappedCount;
						if (isupper(cTest))
						{
							TestColIndexes.push_back(Col);
							++(TestColIndexCount[Col]);
							if (FirstTestColIndex == UINT_MAX)
								FirstTestColIndex = Col;
							else
							{
								if (FirstTestColIndex != Col)
									TestColAllCorrect = false;
							}
						}
						else
							TestAllAligned = false;
					}
					else
					{
						if (RefColIsAligned)
						{
							/*k_printf( "\n");
							k_printf( "Ref col: ");
							for (unsigned RefSeqIndex = 0; RefSeqIndex < RefSeqCount; RefSeqIndex++)
								k_printf( "%c", msaRef.GetChar(RefSeqIndex, RefColIndex));
							k_printf( "\n");*/
							Quit_Qscore("Ref col %u has both upper- and lower-case letters",
								RefColIndex);
						}
					}
					TestColIndex[TestSeqIndex] = Col;
				}
			}

			if (RefColIsAligned && NonGappedCount > 1)
			{
				++RefAlignedColCount;
				if (TestColAllCorrect && TestAllAligned)
					++CorrectColCount;
			}

			unsigned ColPairCount = 0;
			for (IntVec::const_iterator p = TestColIndexes.begin(); p != TestColIndexes.end(); ++p)
			{
				unsigned Col = *p;
				unsigned Count = TestColIndexCount[Col];
				if (Count > 0)
					ColPairCount += Count*(Count - 1)/2;
				TestColIndexCount[Col] = 0;
			}

			CorrectPairCount += ColPairCount;
			RefAlignedPairCount += NonGappedCount*(NonGappedCount - 1)/2;
		}

		if (RefAlignedPairCount == 0)
			Q = 0;
		else
			Q = (double) CorrectPairCount / (double) RefAlignedPairCount;

		if (RefAlignedColCount == 0)
		{
			/*if (WarnIfNoRefAligned)
				k_printf(
				"Warning: reference alignment %s has no aligned (upper-case) columns\n",
				g_RefFileName);*/
			TC = 0;
		}
		else
			TC = (double) CorrectColCount / (double) RefAlignedColCount;
}
