#include "qscore.h"

/***
Strip trailing whitespace from FASTA annotation and
truncate at first whitespace or comma (-truncname option).
***/
static void FixLabel(std::string &Label)
	{
// Truncate trailing whitespace
	const char *WhiteSpace = " \t\n\r";
	size_t n = Label.find_last_of(WhiteSpace);
	if (n != std::string::npos)
		Label.erase(n);

	if (FlagOpt_QScore("truncname"))
		{
		n = Label.find_first_of(" \t\r\n,");
		if (n != std::string::npos)
			Label.erase(n);
		}
	}

static bool DoStrip(char c)
	{
	/*const QScoreContext *ctx = getQScoreContext();
	c = toupper(c);
	if (ctx->g_StripX && c == 'X')
		return true;
	if (ctx->g_StripZ && c == 'Z')
		return true;
	if (ctx->g_StripB && c == 'B')
		return true;*/
	return false;
	}

#define APPEND_CHAR(c)							\
	{											\
	if (Pos >= BufferSize)						\
		Quit_Qscore("ReadMFA: buffer too small");		\
	if (!DoStrip(c))							\
		Buffer[Pos++] = (c);					\
	}

#define APPEND_SEQ(Label, Start, Length, UngappedLength) \
	{												\
	if (0 == m_uSeqCount)							\
		m_uColCount = Length;						\
	else if (Length != m_uColCount)					\
		Quit_Qscore("Sequence lengths differ %s=%d, %s=%d",\
		  Label.c_str(), m_uColCount, Label.c_str(), Length);\
	if (m_uSeqCount >= m_uCacheSeqCount)			\
		{											\
		m_uCacheSeqCount += 128;					\
		char **Seqs = all(char *, m_uCacheSeqCount);\
		unsigned *SeqLengths = all(unsigned, m_uCacheSeqCount);\
		if (m_uSeqCount > 0)						\
			{										\
			memcpy(Seqs, m_szSeqs, m_uSeqCount*sizeof(char **));\
			memcpy(SeqLengths, m_SeqLengths, m_uSeqCount*sizeof(unsigned));\
			delete[] m_szSeqs;						\
			delete[] m_SeqLengths;					\
			}										\
		m_szSeqs = Seqs;							\
		m_SeqLengths = SeqLengths;					\
		}											\
	m_szSeqs[m_uSeqCount] = m_SeqBuffer + Start;	\
	FixLabel(Label);								\
	m_SeqNames.push_back(Label);					\
	m_SeqNameToIndex[Label] = m_uSeqCount;			\
	Label.clear();									\
	m_SeqLengths[m_uSeqCount] = UngappedLength;		\
	++m_uSeqCount;									\
	}

void MSA_QScore::FromFASTAFile(FILE *f)
	{
	Clear();

	rewind(f);
	int FileSize = GetFileSize(f);
	int BufferSize = FileSize;
	char *Buffer = all(char, BufferSize);
	m_SeqBuffer = Buffer;

	char prev_c = '\n';
	bool InLabel = false;
	//int ContigFrom = 0;
	std::string Label;
	int UngappedSeqLength = 0;
	int Pos = 0;
	int ContigStart = 0;

	for (;;)
		{
		int c = fgetc(f);
		if (EOF == c)
			{
			if (feof(f))
				break;
			Quit_Qscore("Stream error");
			}
		if (InLabel)
			{
			if (c == '\r')
				continue;
			if ('\n' == c)
				{
				UngappedSeqLength = 0;
				InLabel = false;
				}
			else
				{
				Label.push_back(c);
				}
			}
		else
			{
			if ('>' == c && '\n' == prev_c)
				{
				unsigned ContigLength = Pos - ContigStart;
				if (ContigLength > 0)
					{
					APPEND_SEQ(Label, ContigStart, ContigLength, UngappedSeqLength)
					}

				ContigStart = Pos;
				InLabel = true;
				}
			else if (!isspace(c))
				{
				APPEND_CHAR(c)
					if (!::IsGap(c))
						++UngappedSeqLength;
				}
			}
		prev_c = c;
		}

	unsigned ContigLength = Pos - ContigStart;
	if (ContigLength > 0)
		{
		APPEND_SEQ(Label, ContigStart, ContigLength, UngappedSeqLength);
		}
	MakeGapMap();
	MakeUngapMap();
	}
