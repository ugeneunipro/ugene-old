/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

namespace {

class CharacterStream {
public:
    CharacterStream(const QByteArray &input):
        input(input),
        position(0)
    {
    }

    char peek() {
        if(input.size() == position) {
            return '\0';
        }
        return input[position];
    }

    char next() {
        if(input.size() == position) {
            return '\0';
        }
        char result = input[position];
        position++;
        return result;
    }

private:
    QByteArray input;
    int position;
};

class Token {
public:
    enum Type {
        INVALID,
        END_OF_INPUT,
        LEFT_PARENTHESIS,
        RIGHT_PARENTHESIS,
        CARET,
        LESS,
        GREATER,
        COLON,
        COMMA,
        PERIOD,
        DOUBLE_PERIOD,
        JOIN,
        ORDER,
        COMPLEMENT,
        NUMBER,
        NAME
    };

    Token(const QByteArray &string, Type type):
        string(string),
        type(type)
    {
    }

    const QByteArray &getString() const {
        return string;
    }

    Type getType() const {
        return type;
    }

private:
    QByteArray string;
    Type type;
};

bool isNameCharacter(char c) {
    return (isalnum(c) || ('_' == c) || ('-' == c) || ('\'' == c) || ('*' == c));
}

class Lexer {
public:
    Lexer(const QByteArray &input):
        input(input),
        nextToken("", Token::INVALID),
        nextTokenValid(false)
    {
    }

    Token peek() {
        if(!nextTokenValid) {
            nextToken = readNext();
            nextTokenValid = true;
        }
        return nextToken;
    }

    Token next() {
        if(nextTokenValid) {
            nextTokenValid = false;
            return nextToken;
        }
        return readNext();
    }

private:
    Token readNext() {
        const QBitArray& WHITES = TextUtils::WHITES;
        char inputChar = input.peek();
        //while(isspace(inputChar)) {       //exclude the locale-specific function
        while(WHITES.testBit(inputChar)) {
            input.next();
            inputChar = input.peek();
        }
        switch(input.peek()) {
        case '\0':
            return Token("<end>", Token::END_OF_INPUT);
        case '(':
            return Token(QByteArray(1, input.next()), Token::LEFT_PARENTHESIS);
        case ')':
            return Token(QByteArray(1, input.next()), Token::RIGHT_PARENTHESIS);
        case '^':
            return Token(QByteArray(1, input.next()), Token::CARET);
        case '<':
            return Token(QByteArray(1, input.next()), Token::LESS);
        case '>':
            return Token(QByteArray(1, input.next()), Token::GREATER);
        case ':':
            return Token(QByteArray(1, input.next()), Token::COLON);
        case ',':
            return Token(QByteArray(1, input.next()), Token::COMMA);
        case '.':
            {
                QByteArray tokenString(1, input.next());
                if('.' == input.peek()) {
                    tokenString.append(input.next());
                    return Token(tokenString, Token::DOUBLE_PERIOD);
                }
                return Token(tokenString, Token::PERIOD);
            }
        default:
            {
                QByteArray tokenString;
                if(isdigit(input.peek())) {
                    while(isdigit(input.peek())) {
                        tokenString.append(input.next());
                    }
                    if(!isNameCharacter(input.peek())) {
                        return Token(tokenString, Token::NUMBER);
                    }
                }
                if(isNameCharacter(input.peek())) {
                    while(isNameCharacter(input.peek())) {
                        tokenString.append(input.next());
                    }
                    if("join" == tokenString) {
                        return Token(tokenString, Token::JOIN);
                    }
                    if("order" == tokenString) {
                        return Token(tokenString, Token::ORDER);
                    }
                    if("complement" == tokenString) {
                        return Token(tokenString, Token::COMPLEMENT);
                    }
                    return Token(tokenString, Token::NAME);
                }
                return Token(QByteArray(1, input.next()), Token::INVALID);
            }
        }
    }

private:
    CharacterStream input;
    Token nextToken;
    bool nextTokenValid;
};

U2Region toRegion(quint64 firstBase, quint64 secondBase) {
    quint64 minBase = qMin(firstBase, secondBase);
    quint64 maxBase = qMax(firstBase, secondBase);
    return U2Region(minBase - 1, maxBase - minBase + 1);
}

class Parser {
public:
    Parser(const QByteArray &input):
        lexer(input),
        join(false),
        order(false)
    {
        seqLenForCircular = -1;
    }

    bool parse(U2Location &result) {
        result->regions.clear();
        result->strand = U2Strand::Direct;
        if(!parseLocation(result)) {
            return false;
        }
        // this causes the genbank parser to fail on some files
//        if(!match(Token::END_OF_INPUT)) {
//            return false;
//        }
        return true;
    }

    void setSeqLenForCircular(quint64 val) { seqLenForCircular = val; }
private:
    quint64 seqLenForCircular;
    

    bool parseNumber(quint64 &result) {
        if(lexer.peek().getType() != Token::NUMBER) {
            return false;
        }
        QByteArray string = lexer.next().getString();
        result = 0;
        foreach(char c, string) {
            result *= 10;
            result += (quint64)c - '0';
        }
        return true;
    }

    bool parseLocationDescriptor(U2Location &location) {
        bool remoteEntry = false;
        if(lexer.peek().getType() == Token::NAME) { // remote entries
            remoteEntry = true;
            QByteArray accession = lexer.next().getString();
            if(!match(Token::PERIOD)) {
                return false;
            }
            quint64 version = 0;
            if(!parseNumber(version)) {
                return false;
            }
            if(!match(Token::COLON)) {
                return false;
            }
            ioLog.info(LocationParser::tr("Ignoring remote entry: %1.%2").arg(QString(accession)).arg(version));
        }
        quint64 firstBase = 0;
        bool firstBaseIsFromRange = false;
        if(match(Token::LEFT_PARENTHESIS)) { // cases like (1.2)..
            firstBaseIsFromRange = true;
            if(!parseNumber(firstBase)) { // use the first number as a region boudary
                return false;
            }
            if(!match(Token::PERIOD)) {
                return false;
            }
            if(!match(Token::NUMBER)) { // ignore the second number
                return false;
            }
            if(!match(Token::RIGHT_PARENTHESIS)) {
                return false;
            }
            ioLog.info(LocationParser::tr("'a single base from a range' in combination with 'sequence span' is not supported"));
        } else {
            if(match(Token::LESS)) {
                ioLog.info(LocationParser::tr("Ignoring '<' at start position"));
            }
            if(!parseNumber(firstBase)) {
                return false;
            }
        }
        if(match(Token::PERIOD)) {
            if(firstBaseIsFromRange) { // ranges are only allowed in spans
                return false;
            }
            quint64 secondNumber = 0;
            if(!parseNumber(secondNumber)) {
                return false;
            }
            if(!location->isEmpty()) {
                return false;
            }
            if(!remoteEntry) { // ignore remote entries
                location->regions.append(toRegion(firstBase, secondNumber));
                location->regionType = U2LocationRegionType_SingleBase;
            }
        } else if(match(Token::DOUBLE_PERIOD)) {
            quint64 secondNumber = 0;
            if(match(Token::LEFT_PARENTHESIS)) { // cases like ..(1.2)
                if(!match(Token::NUMBER)) { // ignore the first number
                    return false;
                }
                if(!match(Token::PERIOD)) {
                    return false;
                }
                if(!parseNumber(secondNumber)) { // use the second number as a region boudary
                    return false;
                }
                if(!match(Token::RIGHT_PARENTHESIS)) {
                    return false;
                }
                ioLog.info(LocationParser::tr("'a single base from a range' in combination with 'sequence span' is not supported"));
            } else {
                if(match(Token::GREATER)) {
                    ioLog.info(LocationParser::tr("Ignoring '>' at end position"));
                }
                if(!parseNumber(secondNumber)) {
                    return false;
                }
            }
            if(!remoteEntry) { // ignore remote entries
                if(seqLenForCircular != -1 && firstBase > secondNumber){
                    location->regions.append(toRegion(1, secondNumber));
                    location->regions.append(toRegion(firstBase, seqLenForCircular));
                    location->regionType = U2LocationRegionType_Default;
                    location->op = U2LocationOperator_Join;
                }else{
                    location->regions.append(toRegion(firstBase, secondNumber));
                    location->regionType = U2LocationRegionType_Default;
                }
            }
        } else if(match(Token::CARET)) {
            if(firstBaseIsFromRange) { // ranges are only allowed in spans
                return false;
            }
            quint64 secondBase = 0;
            if(!parseNumber(secondBase)) {
                return false;
            }
            if(!location->isEmpty()) {
                return false;
            }
            if(!remoteEntry) { // ignore remote entries
                if(seqLenForCircular != -1 && firstBase > secondBase){
                    location->regions.append(toRegion(1, secondBase));
                    location->regions.append(toRegion(firstBase, seqLenForCircular));
                    location->regionType = U2LocationRegionType_Default;
                    location->op = U2LocationOperator_Join;
                }else{
                    location->regions.append(toRegion(firstBase, secondBase));
                    location->regionType = U2LocationRegionType_Site;
                }
            }
        } else {
            if(firstBaseIsFromRange) { // ranges are only allowed in spans
                return false;
            }
            if(!remoteEntry) { // ignore remote entries
                location->regions.append(toRegion(firstBase, firstBase));
                location->regionType = U2LocationRegionType_Default;
            }
        }
        return true;
    }

    bool parseLocation(U2Location &location) {
        if(match(Token::JOIN)) {
            if(!match(Token::LEFT_PARENTHESIS)) {
                return false;
            }
            if(order) {
                return false;
            }
            join = true;
            location->op = U2LocationOperator_Join;
            while(true) {
                if(!parseLocation(location)) {
                    return false;
                }
                if(!match(Token::COMMA)) {
                    break;
                }
            }
            if(!match(Token::RIGHT_PARENTHESIS)) {
                return false;
            }
        } else if(match(Token::ORDER)) {
            if(!match(Token::LEFT_PARENTHESIS)) {
                return false;
            }
            if(join) {
                return false;
            }
            order = true;
            location->op = U2LocationOperator_Order;
            while(true) {
                if(!parseLocation(location)) {
                    return false;
                }
                if(!match(Token::COMMA)) {
                    break;
                }
            }
            if(!match(Token::RIGHT_PARENTHESIS)) {
                return false;
            }
        } else if(match(Token::COMPLEMENT)) {
            if(!match(Token::LEFT_PARENTHESIS)) {
                return false;
            }
            location->strand = U2Strand::Complementary;
            // the following doesn't match the specification
            while(true) {
                if(!parseLocation(location)) {
                    return false;
                }
                if(!match(Token::COMMA)) {
                    break;
                }
            }
            if(!match(Token::RIGHT_PARENTHESIS)) {
                return false;
            }
        } else {
            while(true) {
                if(!parseLocationDescriptor(location)) {
                    return false;
                }
                if(!match(Token::COMMA)) {
                    break;
                }
            }
        }
        return true;
    }

    bool match(Token::Type type) {
        if(lexer.peek().getType() == type) {
            lexer.next();
            return true;
        }
        return false;
    }

private:
    Lexer lexer;
    bool join;
    bool order;
};

}

void LocationParser::parseLocation( const char* _str, int _len, U2Location& location, qint64 seqlenForCircular )
{
    Parser parser(QByteArray(_str, _len));
    parser.setSeqLenForCircular(seqlenForCircular);
    if(!parser.parse(location)) {
        location->regions.clear();
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
