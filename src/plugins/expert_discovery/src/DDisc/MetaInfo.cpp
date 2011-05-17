#include "MetaInfo.h"
#include <stdexcept>
#include <string.h>
using namespace std;

namespace DDisc {

//////////////////////////////////////////////////////////////////////
// MetaInfo: метаинформация о сигнале из разметки
//////////////////////////////////////////////////////////////////////

MetaInfo::MetaInfo(void)
{

}

MetaInfo::~MetaInfo(void)
{

}

MetaInfo::MetaInfo(const MetaInfo& mi)
{
	*this = mi;
}

const MetaInfo& MetaInfo::operator =(const MetaInfo& mi)
{
	setNo(mi.getNo());
	setName(mi.getName());
	setMethodName(mi.getMethodName());
	return *this;
}

istream& MetaInfo::load(istream& in)
{
	char buf[BUF_SIZE];
	in >> ws;
	string eof = "</";
	string s = readTAG(in);
	eof += to_upper(s);
	eof += ">";
	int no;
	if (!parse(eof.c_str(),"</SIGNAL %d>", &no)) throw runtime_error("Invalid file format");
	setNo(no);
	in >> ws;
	in.getline(buf, BUF_SIZE);
	string loaded = strupr(buf);
	if (!parse(loaded.c_str(),"NAME %s", buf)) throw runtime_error("Invalid file format");
	if (!strlen(buf)) throw runtime_error("Invalid file format");
	setName(buf);

	in >> ws;
	in.getline(buf, BUF_SIZE);
	loaded = strupr(buf);
	if (!parse(loaded.c_str(),"METHOD_NAME %s", buf)) 
		throw runtime_error("Invalid file format");
	if (!strlen(buf)) throw runtime_error("Invalid file format");
	setMethodName(buf);
	bool err;
	do {
		in >> ws;
		in.getline(buf, BUF_SIZE);
		err = (strnicmp(buf, eof.c_str(), eof.length())!=0);
	} while (err && !in.eof());
	if (err) throw runtime_error("Invalid file format");
	return in;
}

ostream& MetaInfo::save(ostream& out) const
{
	runtime_error("MetaInfo::save() not implemented");
	return out;
}

//////////////////////////////////////////////////////////////////////
// Family: семейство сигналов, заданных метаинформацией
//////////////////////////////////////////////////////////////////////

Family::Family(void)
{

}

Family::~Family(void)
{

}

Family::Family(const Family& family)
{
	*this = family;
}

const Family& Family::operator =(const Family& family)
{
	setName( family.getName() );
	data = family.data;
	return *this;
}

istream& Family::load(istream& in)
{
	char buf[BUF_SIZE];
	string s = readTAG(in);
	string name = to_upper(s);	
	setName(name);
	in >> ws;
	in.getline(buf, BUF_SIZE);
	if (in.fail()) throw runtime_error("Invalid file format");
	int signumber;
	if (!parse(strupr(buf),"SIGNAL_NUMBER %d", &signumber))
		throw runtime_error("Invalid file format");
	MetaInfo metainfo;
	while (signumber--) {
		metainfo.load(in);
		data.push_back(metainfo);
		in >> ws;
	}
	in >> ws;
	in.getline(buf, BUF_SIZE);
	if (in.fail()) throw runtime_error("Invalid file format");
	string eof = "</" + name + ">";
	if (strnicmp(buf, eof.c_str(), eof.length())) throw runtime_error("Invalid file format");
	return in;
}

ostream& Family::save(ostream& out) const
{
	runtime_error("Family::save() not implemented");
	return out;
}

const MetaInfo& Family::getMetaInfo(int no) const
{
	return data[no];
}

const MetaInfo& Family::getMetaInfo(const string name) const
{
	int nSize = (int) data.size();
	for (int i=0; i<nSize; i++)
	{
		if (data[i].getName() == name)
			return data[i];
	}
	throw range_error("No such meta info");
}

MetaInfo& Family::getMetaInfo(int no)
{
	return data[no];
}

int Family::getSignalNumber() const
{
	return (int) data.size();
}	

void Family::AddInfo(const MetaInfo& info)
{
	data.push_back(info);
}

//////////////////////////////////////////////////////////////////////
// MetaInfoBase: База метаинформации, содержащая информацию о всех семействах
//////////////////////////////////////////////////////////////////////

MetaInfoBase::MetaInfoBase(void)
{

}

MetaInfoBase::~MetaInfoBase(void)
{

}

void MetaInfoBase::clear()
{
	data.clear();
}

istream& MetaInfoBase::load(istream& in)
{
	data.clear();
	in >> ws;
	while (!in.eof()) {
		Family family;
		family.load(in);
		data.push_back(family);
		in >> ws;
	}
	return in;
}

ostream& MetaInfoBase::save(ostream& out) const
{
	throw runtime_error("MetaInfoBase::save() not implemented");
	return out;
}

const Family& MetaInfoBase::getSignalFamily(int no) const
{
	return data[no];
}

Family& MetaInfoBase::getSignalFamily(int no)
{
	return data[no];
}

void MetaInfoBase::addFamily(const Family& rFamily)
{
	data.push_back(rFamily);
}

const Family& MetaInfoBase::getSignalFamily(const string name) const
{
	for (unsigned i=0; i<data.size(); i++) 
		if ( stricmp(name.c_str(),data[i].getName().c_str()) == 0 ) 
			return data[i];
	throw range_error("No such family");
}

int MetaInfoBase::getFamilyNumber() const
{
	return static_cast<int>(data.size());
}

void MetaInfoBase::insert(const char* family, MetaInfo& mi)
{
   try {
      Family& f = const_cast<Family&>(getSignalFamily(family));
      try { f.getMetaInfo(mi.getName()); }
      catch (...) {
         f.AddInfo(mi);
      }
   }
   catch (...) {
      Family f;
      f.AddInfo(mi);
      f.setName(family);
      addFamily(f);
   }
}


};


