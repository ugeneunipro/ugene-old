#ifndef _QSCORE_CONTEXT_
#define _QSCORE_CONTEXT_
class QScoreException {
public:
	QScoreException() {}
	QScoreException(const char* str);
	char str[4096];
};
#endif //_QSCORE_CONTEXT_
//
//struct QScoreContext {
//	const MSA_QScore *g_ptrmsaTest;
//	const MSA_QScore *g_ptrmsaRef;
//	unsigned g_TestSeqIndexA;
//	unsigned g_TestSeqIndexB;
//	unsigned g_RefSeqIndexA;
//	unsigned g_RefSeqIndexB;
//	bool g_Quiet;
//	bool g_Cline;
//	bool g_SeqDiffWarn;
//	bool g_Verbose;
//	bool g_StripX;
//	bool g_StripZ;
//	bool g_StripB;
//	bool g_Slow;
//	bool g_Modeler;
//	bool g_GC;
//
//	//qscore_fastq.cpp
//	unsigned g_SeqDiffCount;
//
//	QScoreContext();
//};
//

//extern const QScoreContext* getQScoreContext();