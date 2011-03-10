#include "CMDLineRegistry.h"
#include "CMDLineHelpProvider.h"
#include "CMDLineCoreOptions.h"

namespace U2 {

/***************************************************
 * CMDLineRegistry
 ***************************************************/

const QString SINGLE_DASH   = "-";
const QString DOUBLE_DASH   = "--";
const QString EQUALS        = "=";

static bool isDoubleDashParameter(const QString& val) {
    return val.startsWith( DOUBLE_DASH ) && val.length() > 2 && val.at(2).isLetter();
}

static bool isSingleDashParameter(const QString& val) {
    return val.startsWith( SINGLE_DASH ) && val.length() > 1 && val.at(1).isLetter();
}

static bool tryParseDoubleDashParameter(const QString& argument, QString& paramName, QString &paramValue) {
    if (!isDoubleDashParameter(argument)) {
        return false;
    }
    int nameEndIdx = argument.indexOf( EQUALS );
    if (nameEndIdx == -1) {
        paramName = argument.mid(2);
    } else {
        paramName = argument.mid(2, nameEndIdx - 2);
        paramValue = argument.mid(nameEndIdx+1);
    }
    return true;
}

static bool tryParseSingleDashParameter(const QString& argument, const QString& nextArgument, QString& paramName, QString &paramValue) {
    if (!isSingleDashParameter(argument)) {
        return false;
    }
    paramName = argument.mid(1);
    if (!isDoubleDashParameter(nextArgument) && !isSingleDashParameter(nextArgument)) {
        paramValue = nextArgument;    
    }    
    return true;
}

CMDLineRegistry::CMDLineRegistry(const QStringList& arguments) {
    int sz = arguments.size();
    for( int i = 0; i < sz; i++ ) {
        const QString& argument = arguments.at( i );
        StringPair pair;
        if ( !tryParseDoubleDashParameter(argument, pair.first, pair.second) ) {
            QString nextArgument;
            if (i < sz - 1) {
                nextArgument = arguments.at(i + 1);
            }
            if ( tryParseSingleDashParameter(argument, nextArgument, pair.first, pair.second) ) {
                if (!pair.second.isEmpty()) {
                    i++;
                }
            } else {
                pair.second = argument;
            }
        }
        params << pair;
    }
}

CMDLineRegistry::~CMDLineRegistry() {
    qDeleteAll( helpProviders );
}

const QList<StringPair> & CMDLineRegistry::getParameters() const {
    return params;
}

QStringList CMDLineRegistry::getOrderedParameterNames() const {
    QStringList res;
    QList<StringPair>::const_iterator it = params.constBegin();
    while( it != params.constEnd() ) {
        res << it->first;
        ++it;
    }
    return res;
}

bool CMDLineRegistry::hasParameter( const QString & paramName, int startWithIdx ) const {
    int sz = params.size();
    for( int i = qMax(0, startWithIdx); i < sz; ++i ) {
        const StringPair& param = params[i];
        if( param.first == paramName ) {
            return true;
        }
    }
    return false;
}

QString CMDLineRegistry::getParameterValue( const QString & paramName, int startWithIdx ) const {
    int sz = params.size();
    for( int i = qMax(0, startWithIdx); i < sz; ++i ) {
        const StringPair& param = params[i];
        if( param.first == paramName ) {
            return param.second;
        }
    }
    return QString();
}

static bool providerNameComparator(const CMDLineHelpProvider* p1, const CMDLineHelpProvider* p2) {
    return p1->getHelpSectionName().compare(p2->getHelpSectionName()) > 0;
}

void CMDLineRegistry::registerCMDLineHelpProvider(CMDLineHelpProvider* provider) {
    helpProviders.append(provider);
    qStableSort(helpProviders.begin(), helpProviders.end(), providerNameComparator); 
}

void CMDLineRegistry::unregisterCMDLineHelpProvider(CMDLineHelpProvider* provider) {
    helpProviders.removeOne(provider);
}

} //namespace
