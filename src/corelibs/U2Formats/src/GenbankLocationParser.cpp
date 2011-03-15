/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "GenbankLocationParser.h"

#include <U2Core/TextUtils.h>
#include <U2Core/AnnotationTableObject.h>

namespace U2 {
namespace Genbank {

static int parseNum(const char* str, int len) {
    int num = 0;
    for (int i=0; i<len; i++) {
        int n = str[i]-'0';
        if (n < 0 || n > 9) {
            return -1;
        }
        num = num*10 + n;
    }
    return num;
}

static void checkNumberEnd(U2Location& location , int& x1, int& x2, const char* str, int len, bool addToResultsIfOne) {
    if (x1 < 0) {
        x1 = parseNum(str, len);
    } else {
        x2 = parseNum(str, len);
    }
    if (x2!=-1 || addToResultsIfOne) {
        x2 = qMax(x1, x2); //if x2 == -1 -> make it == x2
        if (x1 > 0 && x1 <= x2) {
            int lLen = x2 - x1 + 1; //both x1 and x2 are inclusive
            location->regions << U2Region(x1-1, lLen); //-1 because in gb files location starts from 1
        }

    }
}

#define STARTS_WITH(_str, _strlen, _token, _tokenlen) \
    *(_str) == *(_token) && (_strlen) > (_tokenlen) && !qstrncmp((_token), (_str), (_tokenlen))


void LocationParser::parseLocation(const char* _str, int _len, U2Location& location) {
    //get a copy;
    QByteArray lineBuff(_str, _len);
    char* line = lineBuff.data();
    //remove all white spaces
    int len = TextUtils::remove(_str, _len, line, TextUtils::WHITES);


    enum states {SKIPPING, READ_NUMBER, READ_DOT};
/** Init...*/
    location->reset();
    int state = SKIPPING;
    int x1 = -1;
    int x2 = -1;
    int numStart = 0;
    int dots = 0;

/*----------------------------------*/
    int i = 0;
    for (; i<len; i++) {
        char c = line[i];
        const char* strStart = line+i;
        int strLen = len-i;
        if (STARTS_WITH(strStart, strLen, "join(", 5)) {
            i+=4;
            location->op = U2LocationOperator_Join;
        } else if (STARTS_WITH(strStart, strLen, "order(", 6)) {
            i+=5;
            location->op = U2LocationOperator_Order;
        } else if (STARTS_WITH(strStart, strLen, "complement(", 11)){
            location->strand = U2Strand::Complementary;
            i+=10;
        } else if (c >= '0' && c <= '9') {
            if (state == READ_DOT && dots == 1 ) {
                if (x2 < 0) { //(a.b)..c ->skip b
                    int j = i;
                    for(; i < len && c >= '0' && c <= '9'; i++){
                        c = line[i];
                    }
                    if (len-i > 2 && line[i]!=',') {
                        i--;
                        state = SKIPPING;
                        continue;
                    } else { // full location is something like (1.15)-> save '15' as x2
                        i=j;
                    }
                } else { // a..(b.c) -> skip b
                    x2=-1;
                }
            }
            if (state == READ_NUMBER) {
                continue;
            }
            state = READ_NUMBER;
            numStart = i;
        } else if (c=='.' || c=='^') {
            if (state==READ_NUMBER) {
                checkNumberEnd(location, x1, x2, line+numStart, i-numStart, false);
            }
            if (state!=READ_DOT) {
                state = READ_DOT;
                dots = c=='.' ? 1 : 2; //assume '^' == '..'
            } else {
                dots++;
            }
        } else if (c == ',' || c==')') {
            if (state==READ_NUMBER) {
                checkNumberEnd(location, x1, x2, line+numStart, i-numStart, true);
            }
            state = SKIPPING;
            x1=-1;
            x2=-1;
        } else if (c=='(' || c=='<' || c == '>'){
            state = SKIPPING;
        } else {
            break;//code to parse braces and operators is not ready.
        }
    }
    if (state == READ_NUMBER) {
        checkNumberEnd(location, x1, x2, line+numStart, i-numStart, true);
    }
}

QString LocationParser::buildLocationString(const AnnotationData* d) {
    QVector<U2Region> location = d->getRegions();
    bool complement = d->getStrand().isCompementary();
    bool multi = location.size() > 1;
    QString locationStr = complement ? "complement(" : "";
    if (!location.empty()) {
        if (multi) {
            locationStr += d->isOrder() ? "order(" : "join(";
        }
        locationStr += buildLocationString(location);
    }
    if (multi) {
        locationStr += ")";
    }
    if (complement) {
        locationStr.append(")");
    }
    return locationStr;
}

QString LocationParser::buildLocationString( const QVector<U2Region>& regions )
{
    QString locationStr;
    bool first = true;

    foreach (const U2Region& r, regions) {
        if (!first) {
            locationStr += ",";
        }  else {
            first = false;
        }
        locationStr.append(QString::number(r.startPos+1).append("..").append(QString::number(r.endPos())));
    }
    return locationStr;
}

}}//namespace
