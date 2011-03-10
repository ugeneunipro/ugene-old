#ifndef Seq_h
#define Seq_h

#include <vector>

class TextFile;
class MSA_QScore;

typedef std::vector<char> CharVect;

class Seq : public CharVect
	{
public:
	Seq()
		{
		m_ptrName = 0;
	// Start with moderate size to avoid
	// thrashing the heap.
		reserve(200);
		}
	virtual ~Seq()
		{
		delete[] m_ptrName;
		}

private:
// Not implemented; prevent use of copy c'tor and assignment.
	Seq(const Seq &);
	Seq &operator=(const Seq &);

public:
	void Clear()
		{
		clear();
		delete[] m_ptrName;
		m_ptrName = 0;
		m_uId = uInsane;
		}
	const char *GetName() const
		{
		return m_ptrName;
		}
	unsigned GetId() const
		{
		if (uInsane == m_uId)
			Quit_Qscore("Seq::GetId, id not set");
		return m_uId;
		}
	void SetId(unsigned uId) { m_uId = uId; }

	bool FromFASTAFile(TextFile &File);
	void ToFASTAFile(TextFile &File) const;
	void ExtractUngapped(MSA_QScore &msa) const;

	void FromString(const char *pstrSeq, const char *pstrName);
	void Copy(const Seq &rhs);
	void CopyReversed(const Seq &rhs);
	void StripGaps();
	void ToUpper();
	void SetName(const char *ptrName);
	unsigned GetLetter(unsigned uIndex) const;
	unsigned Length() const { return (unsigned) size(); }
	bool EqIgnoreCase(const Seq &s) const;
	bool EqIgnoreCaseAndGaps(const Seq &s) const;
	bool HasGap() const;
	unsigned GetUngappedLength() const;
	void ListMe() const;
	char GetChar(unsigned uIndex) const { return operator[](uIndex); }
	void SetChar(unsigned uIndex, char c) { operator[](uIndex) = c; }

#ifndef	_WIN32
	reference at(size_type i) { return operator[](i); }
	const_reference at(size_type i) const { return operator[](i); }
#endif

private:
	char *m_ptrName;
	unsigned m_uId;
	};

#endif	// Seq.h
