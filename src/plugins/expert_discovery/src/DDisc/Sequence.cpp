// Sequence.cpp: implementation of the Sequence class.
//
//////////////////////////////////////////////////////////////////////

#include "Sequence.h"
#include <stdexcept>
#include <algorithm>
#include <string.h>
using namespace std;

namespace DDisc {

//////////////////////////////////////////////////////////////////////
// Marking: разметка символьной последовательности
//////////////////////////////////////////////////////////////////////

Marking::Marking()
{

}

Marking::~Marking()
{

}

Marking::Marking(const Marking& m)
{
        *this = m;
}

const Marking& Marking::operator = (const Marking& m)
{
        marking = m.marking;
        return *this;
}


bool Marking::Comparator::operator ()(const Interval& a, const Interval& b) const {
        if (a.getFrom()<b.getFrom()) return true;
        else 
        if (a.getFrom()>b.getFrom()) return false;
        else 
        if (a.getTo()<b.getTo()) return true;
        else return false;
}

Interval Marking::hasSignalAt(Interval interval, string name, string family) const
{
    to_upper(name);
    to_upper(family);

	if(marking.empty() || marking.count(family) == 0){
		return Interval();
	}
	MarkingData::const_iterator family_pos = marking.find(family);
	if (family_pos==marking.end()) return Interval();
	const FamilyMarking& family_marking = family_pos->second;
	FamilyMarking::const_iterator signal_pos = family_marking.find(name);
	if (signal_pos == family_marking.end()) return Interval();
	const IntervalSet& set = signal_pos->second;

	IntervalSet::const_iterator iter = set.begin();
	while (iter != set.end()) {
		const Interval& sig = *iter;
		if (interval.isInInterval(sig.getFrom()) && interval.isInInterval(sig.getTo()))
			return sig;
		iter++;
	}
	return Interval();
}

void Marking::clear()
{
        marking.clear();
}

void Marking::set(string name, string family, Interval interval)
{
	to_upper(name);
	to_upper(family);
	IntervalSet& set = marking[family][name];
	set.insert(interval);
}


//////////////////////////////////////////////////////////////////////
// Sequence: символьная последовательность
//////////////////////////////////////////////////////////////////////

Sequence::Sequence()
{
	pMarking = NULL;
	setHasScore(false);
	setScore(0);
}

Sequence::~Sequence()
{

}

Sequence::Sequence(const Sequence& seq)
{
        *this = seq;
}

Sequence::Sequence (const string & name, const string& seq){
	setName(name);
	setSequence(seq);
}

const Sequence& Sequence::operator = (const Sequence& seq)
{
        setName(seq.getName());
        setSequence(seq.getSequence());
		setHasScore(seq.isHasScore());
		setScore(seq.getScore());
        pMarking = seq.pMarking;
        return *this;
}

Marking& Sequence::getSequenceMarking()
{
	if (!pMarking) throw runtime_error("No marking for this sequence");
	return *const_cast<Marking*>(pMarking);
}

const Marking& Sequence::getSequenceMarking() const 
{
   if (!pMarking) throw runtime_error("No marking for this sequence");
   return *pMarking;
}

void Sequence::setSequenceMarking(const Marking& rMarking)
{
	pMarking = &rMarking;
}

istream& Sequence::load(istream& in)
{
        char buf[BUF_SIZE];
        in >> ws;
        if (in.get() != '>' || in.fail()) throw runtime_error("Invalid file format");
        in >> ws;
        in.getline(buf,BUF_SIZE);
        int end = strlen(buf) - 1;
        while (end>=0 &&isspace(buf[end])) {
           buf[end] = 0;
           end--;
        }
        if (end == 0) throw runtime_error("Invalid file format");

        setName(buf);
        setSequence("");
        do {
                in.clear();
                in.getline(buf,BUF_SIZE,'>');
                char *token = strtok(buf, " \n\t");
                while (token) {
                        setSequence( getSequence() + strupr(token) );
                        token = strtok(NULL, " \n\t");
                }
        } while (in.fail() && !in.eof());
        if (!in.eof()) in.putback('>');
        return in;
}

ostream& Sequence::save(ostream& out) const
{
        out << "> " << getName().c_str() << endl;
        out << getSequence().c_str() << endl;
        return out;
}

size_t Sequence::getSize() const
{
	return getSequence().size();
}

void DDisc::Sequence::clearMarking(void)
{
	pMarking = NULL;
}

//////////////////////////////////////////////////////////////////////
// MarkingBase: база разметок символьных последовательностьей
//////////////////////////////////////////////////////////////////////

MarkingBase::MarkingBase()
{

}

MarkingBase::~MarkingBase()
{

}

const Marking& MarkingBase::getMarking(int objno) const
{
   Data::const_iterator i = data.find(objno);
   if (i == data.end()) throw range_error("No marking for this index");
   return i->second;
}

void MarkingBase::clear()
{
   data.clear();
}

Marking& MarkingBase::getMarking(int objno)
{
   Data::iterator i = data.find(objno);
   if (i == data.end()) throw range_error("No marking for this index");
   return i->second;
}

void MarkingBase::setMarking(int objno, const Marking& rMrk)
{
	data[objno] =  rMrk;
}

istream& MarkingBase::load(istream& in)
{
	data.clear();
    in >> ws;
    while (!in.eof()) {
            string s = readTAG(in);
            string family = to_upper(s);
            string family_eof = "/" + family;
            string object;
            s = readTAG(in);
            object = to_upper(s);
            while (strnicmp(object.c_str(), family_eof.c_str(), family_eof.length())!=0) {
                    int objno;
                    if (!parse(object.c_str(),"OBJECT %d",&objno)) 
                            runtime_error("Invalid file format");
                    objno--;
                    string object_eof = "</" + object + ">";
                    char item[BUF_SIZE];
                    in >> ws;
                    in.clear();
                    in.getline(item, BUF_SIZE);
                    while (strnicmp(item, object_eof.c_str(), object_eof.length())!=0) {
                            int from, to;
                            char name[BUF_SIZE];
                            strupr(item);
                            if (!parse(item,"%d..%d %s ", &from, &to, &name[0]))
							{
                                    throw runtime_error("Invalid file format");
                            }       
                            strupr(name);
                            data[objno].set(name, family, Interval(from-1, to-1));
                            in >> ws;
                            in.clear();
                            in.getline(item, BUF_SIZE);
                    }
                    in >> ws;
                    s = readTAG(in);
                    object = to_upper(s);
            }
            in >> ws;
            if (in.eof()) continue;
            s = readTAG(in);
            object = to_upper(s);
    }
    return in;
}

ostream& MarkingBase::save(ostream& out) const
{
        throw logic_error("Marking::save() not implemented");
        return out;
}

//////////////////////////////////////////////////////////////////////
// SequenceBase: база символьных последовательностьей
//////////////////////////////////////////////////////////////////////

SequenceBase::SequenceBase()
{

}

SequenceBase::~SequenceBase()
{

}

void SequenceBase::setMarking(const MarkingBase& rBase)
{
   for (int i=0; i<getSize(); i++) {
      try {
         data[i].setSequenceMarking( rBase.getMarking(i) );
      }
      catch (range_error) {
         data[i].clearMarking();
      }
   }
}

void SequenceBase::clearMarking(void)
{
   for (int i=0; i<getSize(); i++)
      data[i].clearMarking();
}


const Sequence& SequenceBase::getSequence(int objno) const
{
        return data[objno];
}

int SequenceBase::getSize() const
{
        return static_cast<int>(data.size());
}

void SequenceBase::clear()
{
	data.clear();
}

void SequenceBase::clearScores()
{
	for (int i=0; i<(int)data.size(); i++)
	{
		data[i].setHasScore(false);
		data[i].setScore(0);
	}
}

istream& SequenceBase::load(istream& in)
{
  data.clear();
  in.clear();
  Sequence seq;
  do {
    seq.load(in);
    data.push_back(seq);
  } while (!in.eof());
  return in;
}

ostream& SequenceBase::save(ostream& out) const
{
        for (unsigned i=0; i<data.size(); i++) {
                data[i].save(out);
        }
        return out;
}

Sequence& SequenceBase::getSequence(int objno)
{
	return data[objno];
}

int SequenceBase::addSequence(const Sequence& rSeq)
{
	data.push_back(rSeq);
   return (int)data.size() - 1;
}

vector<double> SequenceBase::getScores()
{
	int nSize = getSize();
	vector<double> vScores;
	vScores.resize(nSize);
	for (int i=0; i<nSize; i++)
	{
		Sequence& rSeq = getSequence(i);
		if (rSeq.isHasScore())
		{
			vScores[i] = rSeq.getScore();
		}
		else
		{
			vScores[i] = 0;
		}
	}
	return vScores;
}

int SequenceBase::findSequence(std::string code)
{
   for (int i=0; i<(int)data.size(); i++)
      if (data[i].getSequence() == code)
         return i;
   return -1;
}

int SequenceBase::getObjNo(const char* strId) const
{
   for (int i=0; i<(int)data.size(); i++)
      if (stricmp(data[i].getName().c_str(), strId) == 0)
         return i;
   return -1;
}


};
