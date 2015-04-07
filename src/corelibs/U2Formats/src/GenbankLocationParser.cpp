/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <U2Core/Log.h>
#include <U2Core/TextUtils.h>

#include "GenbankLocationParser.h"

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
    char prev() {
        if(0 == position) {
            return '\0';
        }
        char result = input[position];
        position--;
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
        BOND,
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
    const QBitArray& digitOrAlpha = TextUtils::ALPHA_NUMS;
    return (digitOrAlpha.testBit(c) || ('_' == c) || ('-' == c) || ('\'' == c) || ('*' == c));
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
            ioLog.trace(QString("GENBANK LOCATION PARSER: Space token (ascii code): %1").arg(static_cast<int>(input.peek())));
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
                const QBitArray& NUMS = TextUtils::NUMS;
                QByteArray tokenString;
                if(NUMS.testBit(input.peek()) || '-' == input.peek()) {
                    if('-' == input.peek()) {
                        tokenString.append(input.next());
                    }
                    while(NUMS.testBit(input.peek())) {
                        tokenString.append(input.next());
                    }
                    if("-" == QString(tokenString)) {
                        tokenString = "";
                        input.prev();
                    }
                    else if(!isNameCharacter(input.peek())) {
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
                    if("bond" == tokenString) {
                        return Token(tokenString, Token::BOND);
                    }
                    return Token(tokenString, Token::NAME);
                }
                ioLog.trace(QString("GENBANK LOCATION PARSER: Invalid token (ascii code): %1, next token (ascii)").arg(static_cast<int>(input.peek())));
                char nextChar = input.next();
                ioLog.trace(QString("GENBANK LOCATION PARSER: Next token after invalid (ascii code)").arg(static_cast<int>(nextChar)));
                return Token(QByteArray(1, nextChar), Token::INVALID);
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

//ioLog added to trace an error which occurred on user's OS only
class Parser {
public:
    Parser(const QByteArray &input):
        lexer(input),
        join(false),
        order(false),
        bond(false)
    {
        seqLenForCircular = -1;
    }

    bool parse(U2Location &result, QString &errorReport) {
        result->regions.clear();
        result->strand = U2Strand::Direct;
        if(!parseLocation(result, errorReport)) {
            return false;
        }
        // this causes the genbank parser to fail on some files
//        if(!match(Token::END_OF_INPUT)) {
//            return false;
//        }
        return true;
    }

    void setSeqLenForCircular(qint64 val) { seqLenForCircular = val; }
private:
    qint64 seqLenForCircular;


    bool parseNumber(qint64 &result) {
        if(lexer.peek().getType() != Token::NUMBER) {
            return false;
        }
        QByteArray string = lexer.next().getString();
        result = 0;

        int sign = 1;
        if('-' == string.at(0)) {
            sign = -1;
            string = string.right(1);
        }
        foreach(char c, string) {
            result *= 10;
            result += (quint64)c - '0';
        }
        result = result * sign;
        return true;
    }

    bool parseLocationDescriptor(U2Location &location, QString& errorReport) {
        bool remoteEntry = false;
        if(lexer.peek().getType() == Token::NAME) { // remote entries
            remoteEntry = true;
            QByteArray accession = lexer.next().getString();
            QString data = accession;
            if(!match(Token::PERIOD)) {
                errorReport = QString("GENBANK LOCATION PARSER: Must be PERIOD instead of %1").arg(lexer.peek().getString().data());
                ioLog.trace(errorReport);
                return false;
            }
            qint64 version = 0;
            if(!parseNumber(version)) {
                errorReport = QString("GENBANK LOCATION PARSER: can't parse Number. Token: %1").arg(lexer.peek().getString().data());
                ioLog.trace(errorReport);
                return false;
            }
            if(!match(Token::COLON)) {
                errorReport = QString("GENBANK LOCATION PARSER: Must be COLON instead of %1").arg(lexer.peek().getString().data());
                ioLog.trace(errorReport);
                return false;
            }
            ioLog.info(LocationParser::tr("Ignoring remote entry: %1.%2").arg(QString(accession)).arg(version));
        }
        qint64 firstBase = 0;
        bool firstBaseIsFromRange = false;
        if(match(Token::LEFT_PARENTHESIS)) { // cases like (1.2)..
            firstBaseIsFromRange = true;
            if(!parseNumber(firstBase)) { // use the first number as a region boundary
                errorReport = QString("GENBANK LOCATION PARSER: can't parse Number. Token: %1").arg(lexer.peek().getString().data());
                ioLog.trace(errorReport);
                return false;
            }
            else if(firstBase < 0) {
                errorReport = QString("GENBANK LOCATION PARSER: region boundary can not be less then zero. Token: %1%2").arg(firstBase).arg(lexer.peek().getString().data());
                ioLog.trace(errorReport);
                return false;
            }
            if(!match(Token::PERIOD)) {
                errorReport = QString("GENBANK LOCATION PARSER: Must be PERIOD instead of %1").arg(lexer.peek().getString().data());
                ioLog.trace(errorReport);
                return false;
            }
            if(!match(Token::NUMBER)) { // ignore the second number
                errorReport = QString("GENBANK LOCATION PARSER: Must be NUMBER instead of %1").arg(lexer.peek().getString().data());
                ioLog.trace(errorReport);
                return false;
            }
            if(!match(Token::RIGHT_PARENTHESIS)) {
                errorReport = QString("GENBANK LOCATION PARSER: Must be RIGHT_PARENTHESIS instead of %1").arg(lexer.peek().getString().data());
                ioLog.trace(errorReport);
                return false;
            }
            ioLog.info(LocationParser::tr("'a single base from a range' in combination with 'sequence span' is not supported"));
        } else {
            if(match(Token::LESS)) {
                ioLog.info(LocationParser::tr("Ignoring '<' at start position"));
            }
            if(!parseNumber(firstBase)) {
                errorReport = QString("GENBANK LOCATION PARSER: can't parse Number. Token: %1").arg(lexer.peek().getString().data());
                ioLog.trace(errorReport);
                return false;
            }
            else if(firstBase < 0) {
                errorReport = QString("Region boundary can not be less then zero: %1%2").arg(firstBase).arg(lexer.peek().getString().data());
                ioLog.trace("GENBANK LOCATION PARSER:" + errorReport);
                return false;
            }
        }
        if(match(Token::PERIOD)) {
            if(firstBaseIsFromRange) { // ranges are only allowed in spans
                errorReport = QString("GENBANK LOCATION PARSER: ranges are only allowed in spans. Token: %1").arg(lexer.peek().getString().data());
                ioLog.trace(errorReport);
                return false;
            }
            qint64 secondNumber = 0;
            if(!parseNumber(secondNumber)) {
                errorReport = QString("GENBANK LOCATION PARSER: can't parse Number. Token: %1").arg(lexer.peek().getString().data());
                ioLog.trace(errorReport);
                return false;
            }
            else if(secondNumber < 0) {
                errorReport = QString("Region boundary can not be less then zero: %1%2").arg(secondNumber).arg(lexer.peek().getString().data());
                ioLog.trace("GENBANK LOCATION PARSER:" + errorReport);
                return false;
            }
            if(!location->isEmpty()) {
                errorReport = QString("GENBANK LOCATION PARSER: location is not empty. Token: %1").arg(lexer.peek().getString().data());
                ioLog.trace(errorReport);
                return false;
            }
            if(!remoteEntry) { // ignore remote entries
                location->regions.append(toRegion(firstBase, secondNumber));
                location->regionType = U2LocationRegionType_SingleBase;
            }
        } else if(match(Token::DOUBLE_PERIOD)) {
            qint64 secondNumber = 0;
            if(match(Token::LEFT_PARENTHESIS)) { // cases like ..(1.2)
                if(!match(Token::NUMBER)) { // ignore the first number
                    errorReport = QString("GENBANK LOCATION PARSER: Must be NUMBER instead of %1").arg(lexer.peek().getString().data());
                    ioLog.trace(errorReport);
                    return false;
                }
                if(!match(Token::PERIOD)) {
                    errorReport = QString("GENBANK LOCATION PARSER: Must be PERIOD instead of %1").arg(lexer.peek().getString().data());
                    ioLog.trace(errorReport);
                    return false;
                }
                if(!parseNumber(secondNumber)) { // use the second number as a region boudary
                    errorReport = QString("GENBANK LOCATION PARSER: can't parse Number. Token: %1").arg(lexer.peek().getString().data());
                    ioLog.trace(errorReport);
                    return false;
                }
                else if(secondNumber < 0) {
                    errorReport = QString("Region boundary can not be less then zero: %1%2").arg(secondNumber).arg(lexer.peek().getString().data());
                    ioLog.trace("GENBANK LOCATION PARSER:" + errorReport);
                    return false;
                }
                if(!match(Token::RIGHT_PARENTHESIS)) {
                    errorReport = QString("GENBANK LOCATION PARSER: Must be RIGHT_PARENTHESIS instead of %1").arg(lexer.peek().getString().data());
                    ioLog.trace(errorReport);
                    return false;
                }
                ioLog.info(LocationParser::tr("'a single base from a range' in combination with 'sequence span' is not supported"));
            } else {
                if(match(Token::GREATER)) {
                    ioLog.info(LocationParser::tr("Ignoring '>' at end position"));
                }
                if(!parseNumber(secondNumber)) {
                    errorReport = QString("GENBANK LOCATION PARSER: can't parse Number. Token: %1").arg(lexer.peek().getString().data());
                    ioLog.trace(errorReport);
                    return false;
                }
                else if(secondNumber < 0) {
                    errorReport = QString("Region boundary can not be less then zero: %1%2").arg(secondNumber).arg(lexer.peek().getString().data());
                    ioLog.trace("GENBANK LOCATION PARSER:" + errorReport);
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
            qint64 secondBase = 0;
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

    bool parseLocation(U2Location &location, QString &errorReport) {
        if(match(Token::JOIN)) {
            if(!match(Token::LEFT_PARENTHESIS)) {
                ioLog.trace(QString("GENBANK LOCATION PARSER: Wrong token after JOIN %1").arg(lexer.peek().getString().data()));
                errorReport = LocationParser::tr("Wrong token after JOIN %1").arg(lexer.peek().getString().data());
                return false;
            }
            if(order) {
                ioLog.trace(QString("GENBANK LOCATION PARSER: Wrong token after JOIN  - order %1").arg(lexer.peek().getString().data()));
                errorReport = LocationParser::tr("Wrong token after JOIN  - order %1").arg(lexer.peek().getString().data());
                return false;
            }
            join = true;
            location->op = U2LocationOperator_Join;
            do {
                if(!parseLocation(location, errorReport)) {
                    ioLog.trace(QString("GENBANK LOCATION PARSER: Can't parse location on JOIN"));
                    errorReport = LocationParser::tr("Can't parse location on JOIN");
                    return false;
                }
            } while (match(Token::COMMA));
            if(!match(Token::RIGHT_PARENTHESIS)) {
                ioLog.trace(QString("GENBANK LOCATION PARSER: Must be RIGHT_PARENTHESIS instead of %1").arg(lexer.peek().getString().data()));
                errorReport = LocationParser::tr("Must be RIGHT_PARENTHESIS instead of %1").arg(lexer.peek().getString().data());
                return false;
            }
        } else if(match(Token::ORDER)) {
            if(!match(Token::LEFT_PARENTHESIS)) {
                ioLog.trace(QString("GENBANK LOCATION PARSER: Wrong token after ORDER %1").arg(lexer.peek().getString().data()));
                errorReport = LocationParser::tr("Wrong token after ORDER %1").arg(lexer.peek().getString().data());
                return false;
            }
            if(join) {
                ioLog.trace(QString("GENBANK LOCATION PARSER: Wrong token after ORDER - join %1").arg(lexer.peek().getString().data()));
                return false;
            }
            order = true;
            location->op = U2LocationOperator_Order;
            do {
                if(!parseLocation(location, errorReport)) {
                    ioLog.trace(QString("GENBANK LOCATION PARSER: Can't parse location on ORDER"));
                    errorReport = LocationParser::tr("Can't parse location on ORDER");
                    return false;
                }
            } while (match(Token::COMMA));
            if(!match(Token::RIGHT_PARENTHESIS)) {
                ioLog.trace(QString("GENBANK LOCATION PARSER: Must be RIGHT_PARENTHESIS instead of %1").arg(lexer.peek().getString().data()));
                errorReport = LocationParser::tr("Must be RIGHT_PARENTHESIS instead of %1").arg(lexer.peek().getString().data());
                return false;
            }
        } else if (match(Token::BOND)) {
            if(!match(Token::LEFT_PARENTHESIS)) {
                ioLog.trace(QString("GENBANK LOCATION PARSER: Wrong token after BOND %1").arg(lexer.peek().getString().data()));
                errorReport = LocationParser::tr("Wrong token after BOND %1").arg(lexer.peek().getString().data());
                return false;
            }
            bond = true;
            location->op = U2LocationOperator_Bond;
            do {
                if(!parseLocation(location, errorReport)) {
                    ioLog.trace(QString("GENBANK LOCATION PARSER: Can't parse location on BOND"));
                    errorReport = LocationParser::tr("Can't parse location on BONDs");
                    return false;
                }
            } while (match(Token::COMMA));
            if(!match(Token::RIGHT_PARENTHESIS)) {
                ioLog.trace(QString("GENBANK LOCATION PARSER: Must be RIGHT_PARENTHESIS instead of %1").arg(lexer.peek().getString().data()));
                errorReport = LocationParser::tr("Must be RIGHT_PARENTHESIS instead of %1").arg(lexer.peek().getString().data());
                return false;
            }

        }else if(match(Token::COMPLEMENT)) {
            if(!match(Token::LEFT_PARENTHESIS)) {
                ioLog.trace(QString("GENBANK LOCATION PARSER: Must be LEFT_PARENTHESIS instead of %1").arg(lexer.peek().getString().data()));
                errorReport = LocationParser::tr("Must be LEFT_PARENTHESIS instead of %1").arg(lexer.peek().getString().data());
                return false;
            }
            location->strand = U2Strand::Complementary;
            // the following doesn't match the specification
            do {
                if(!parseLocation(location, errorReport)) {
                    ioLog.trace(QString("GENBANK LOCATION PARSER: Can't parse location on COMPLEMENT"));
                    errorReport = LocationParser::tr("Can't parse location on COMPLEMENT");
                    return false;
                }
            } while (match(Token::COMMA));
            if(!match(Token::RIGHT_PARENTHESIS)) {
                ioLog.trace(QString("GENBANK LOCATION PARSER: Must be RIGHT_PARENTHESIS instead of %1").arg(lexer.peek().getString().data()));
                errorReport = LocationParser::tr("Must be RIGHT_PARENTHESIS instead of %1").arg(lexer.peek().getString().data());
                return false;
            }
        } else {
            do {
                if(!parseLocationDescriptor(location, errorReport)) {
                    ioLog.trace(QString("GENBANK LOCATION PARSER: Can't parse location descriptor"));
                    return false;
                }
            } while (match(Token::COMMA));
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
    bool bond;
};

}

QString LocationParser::parseLocation( const char* _str, int _len, U2Location& location, qint64 seqlenForCircular )
{
    Parser parser(QByteArray(_str, _len));
    QString errorReport;
    parser.setSeqLenForCircular(seqlenForCircular);
    if(!parser.parse(location, errorReport)) {
        location->regions.clear();
    }
    return errorReport;
}

QString LocationParser::buildLocationString(const SharedAnnotationData &d) {
    QVector<U2Region> location = d->getRegions();
    bool complement = d->getStrand().isCompementary();
    bool multi = location.size() > 1;
    QString locationStr = complement ? "complement(" : "";
    if (!location.empty()) {
        if (multi) {
            locationStr += d->isOrder() ? "order(" :
                                          (d->isBond() ? "bond(" : "join(");
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

}

}//namespace
