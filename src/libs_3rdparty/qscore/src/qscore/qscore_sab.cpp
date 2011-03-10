#include "qscore.h"
#include "sab_ref2.h"
#include "sab_ids.h"
#ifdef _MSC_VER
#include <direct.h>
#undef chdir
#define chdir _chdir
#else
#include <unistd.h>
#endif

#define array_n(x)	(sizeof(x)/sizeof(x[0]))

const unsigned nrefs = array_n(SAB_Ref2Filenames);
const unsigned nids = array_n(SAB_Ids);
static map<string, MSA_QScore *> RefMSAs;

MSA_QScore &GetRef(const string &Label1, const string &Label2)
	{
	string Name = Label1 + string("-") + Label2;
	map<string, MSA_QScore *>::iterator p = RefMSAs.find(Name);
	if (p != RefMSAs.end())
		return *p->second;
	Name = Label2 + string("-") + Label1;
	p = RefMSAs.find(Name);
	if (p == RefMSAs.end())
		Quit_Qscore("SAB ref %s not found", Name.c_str());
	return *p->second;
	}

static double SAB1(const char *FileName, MSA_QScore &TestMSA)
	{
	double SumQ = 0.0;
	unsigned PairCount = 0;
	const unsigned SeqCount = TestMSA.GetSeqCount();
	for (unsigned i = 0; i < SeqCount; ++i)
		{
		const string Label1 = TestMSA.GetSeqName(i);
		for (unsigned j = i + 1; j < SeqCount; ++j)
			{
			const string Label2 = TestMSA.GetSeqName(j);
			MSA_QScore &RefMSA = GetRef(Label1, Label2);

			double Q;
			double TC;
			FastQ(TestMSA, RefMSA, Q, TC);
			SumQ += Q;
			++PairCount;
			}
		}
	return PairCount == 0 ? 0 : SumQ/PairCount;
	}

void SAB()
	{
	const char *TestDir = ValueOpt_QScore("sab_test");
	const char *RefDir = ValueOpt_QScore("sab_ref");

	chdir(RefDir);
	for (unsigned i = 0; i < nrefs; ++i)
		{
		//if (i%100 == 0)
			//k_printf( "Reading refs %u/%u\r", i, nrefs);
		MSA_QScore *msa = new MSA_QScore;
		const char *FileName = SAB_Ref2Filenames[i];
		FILE *f = OpenStdioFile(FileName);
		msa->FromFASTAFile(f);
		fclose(f);
		RefMSAs[FileName] = msa;
		}
	//k_printf( "\n");

	chdir(TestDir);
	double SumQ = 0.0;
	for (unsigned i = 0; i < nids; ++i)
		{
		const char *FileName = SAB_Ids[i];
		FILE *f = OpenStdioFile(FileName);
		MSA_QScore TestMSA;
		TestMSA.FromFASTAFile(f);
		fclose(f);

		double Q = SAB1(FileName, TestMSA);
		//printf("Ref=SAB;Test=%s;Q=%.4f\n", FileName, Q);
		SumQ += Q;
		}
	//k_printf( "\n");
	//k_printf( "Q=%7.4f  %s\n", RefDir, SumQ/nids);
	}
