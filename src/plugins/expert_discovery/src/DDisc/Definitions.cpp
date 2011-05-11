#include "definitions.h"

#include <stdarg.h>

#include <strstream>
#include <stdexcept>
#include <stdio.h>

namespace DDisc {

using namespace std;

/**
 * Converts integer to string
 */
string to_string(int v) {
	if (v==PINF || v==MINF) return "UNL";
	char buffer[MAX_STR_INT];
	sprintf(buffer,"%d",v);
	return string(buffer);
}

/**
* Converts double to string
*/
string to_string(double v) {
	if (v==PINF || v==MINF) return "UNL";
	char buffer[MAX_STR_INT];
	sprintf(buffer,"%f",v);
	return string(buffer);
}

string readTAG(istream& in) {
	char buf[BUF_SIZE];
	string result;
	in >> ws;
	in.clear();
	if (in.get() != '<' || in.fail()) 
		throw runtime_error("Invalid file format");
	do {
		try {
			in.clear();
			in.getline(buf, BUF_SIZE, '>');
			result += buf;
		}
		catch (exception &ex) {	
			cout << ex.what();
			exit(0);
		}
	} while (in.fail() && !in.eof());
	return result;
}

string& to_upper(string &str) {
	for (unsigned i=0; i<str.length(); i++) str[i] = toupper(str[i]);
	return str;
}

bool parse(const char* source, const char* format,...) {
	va_list marker;
	va_start(marker, format);
	strstream s;
	strstream f;
	char cs, cf;
	s << source;
	f << format;
	s >> ws;
	f >> ws;
	while (s.good() && f.good()) {
		s.get(cs);
		f.get(cf);
		if (!f.good()) continue;
		if (cf == '%') {
			s.putback(cs);
			f.get(cf);
			switch (cf) {
			case 'd': { 
					int &i = *va_arg(marker, int*);
					s >> i;
					if (s.fail()) {
						s.clear();
						char buf[4];
						buf[3] = 0;
						s.read(buf,3);
						if (strnicmp(buf,"UNL",3)) {
							va_end(marker);
							return false;
						}
						i = INF;
					}
					if (s.eof()) f.putback(f.get());
				}
				break;
			case 's': {
					char *str = va_arg(marker, char*);
					f.get(cf);
					if (f.eof()) cf = ' ';
					if (isspace(cf)) s >> str;
					else s.getline(str,BUF_SIZE,cf);
					if (!f.eof()) {
						f.putback(cf);
						s.putback(cf);
					}
					s.clear();
				}
				break;
			default: 
				va_end(marker);
				return false;
			}
		} else
		if (isspace(cf)) {
			f >> ws;
			s >> ws;
		} else 
		if (toupper(cf) != toupper(cs)) {
			va_end(marker);
			return false;
		}
	}
	va_end(marker);
	if (!f.eof()) {
	    return false;
	}
	return true;
}

ostream& writeInt(ostream& out, int v) {
	if ( v==MINF || v==PINF) out << "UNL";
	else out << v;
	return out;
}


bool isValidWord15(const char* strWord) {
	static const char complementator[26] = {
	// 'E' = error
	/* 'A','B','C','D','E','F','G','H','I','J','K','L','M', */
	   'T','V','G','H','E','E','C','D','E','E','M','E','K',
	/* 'N','O','P','Q','R','S','T','U','V','W','X','Y','Z   */
	   'N','E','E','E','Y','S','A','U','B','W','E','R','E'
	};

	const char *pChar = strWord;
	while (*pChar) {
		char ch = toupper(*pChar);
		if (ch<'A' || ch >'Z' || complementator[ch-'A'] == 'E')
			return false;
		pChar++;
	}
	return true;
}

#ifdef __TARGET_UNIX
char *strupr(char *s) {
    int length = strlen(s);
    for (int i=0; i<length; i++) s[i] = toupper(s[i]);
    return s;
}
#endif

};
