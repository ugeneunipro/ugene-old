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

#ifndef _U2_WORKFLOW_HR_SCHEMA_SERIALIZER_H_
#define _U2_WORKFLOW_HR_SCHEMA_SERIALIZER_H_

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>
#include <U2Core/global.h>
#include <U2Lang/Aliasing.h>
#include <U2Lang/Schema.h>
#include <U2Lang/Port.h>


namespace U2 {

using namespace Workflow;

class ExternalProcessConfig;
class DataConfig;
class AttributeConfig;
struct WorkflowSchemaReaderData;

class U2LANG_EXPORT HRSchemaSerializer : public QObject {
    Q_OBJECT
public:
    static const QString BLOCK_START;
    static const QString BLOCK_END;
    static const QString SERVICE_SYM;
    static const QString QUOTE;
    static const QString NEW_LINE;
    static const QString UNKNOWN_ERROR;
    static const QString NO_ERROR;
    static const QString HEADER_LINE;
    static const QString OLD_XML_HEADER;
    static const QString INCLUDE;
    static const QString INCLUDE_AS;
    static const QString BODY_START;
    static const QString META_START;
    static const QString DOT_ITERATION_START;
    static const QString ITERATION_START;
    static const QString DATAFLOW_SIGN;
    static const QString EQUALS_SIGN;
    static const QString UNDEFINED_CONSTRUCT;
    static const QString TYPE_ATTR;
    static const QString SCRIPT_ATTR;
    static const QString NAME_ATTR;
    static const QString ELEM_ID_ATTR;
    static const QString DOT;
    static const QString DASH;
    static const QString ITERATION_ID;
    static const QString PARAM_ALIASES_START;
    static const QString PORT_ALIASES_START;

    // -------------- backward compatibility --------------
        static const QString ALIASES_HELP_START;
        static const QString OLD_ALIASES_START;
    // ----------------------------------------------------

    static const QString VISUAL_START;
    static const QString UNDEFINED_META_BLOCK;
    static const QString TAB;
    static const QString NO_NAME;
    static const QString COLON;
    static const QString SEMICOLON;
    static const QString INPUT_START;
    static const QString OUTPUT_START;
    static const QString ATTRIBUTES_START;
    static const QString TYPE_PORT;
    static const QString FORMAT_PORT;
    static const QString CMDLINE;
    static const QString DESCRIPTION;
    static const QString PROMPTER;
    static const QString FUNCTION_START;
    static const QString COMMA;
    static const QString MARKER;
    static const QString MARKER_TYPE;
    static const QString MARKER_NAME;
    static const QString QUAL_NAME;
    static const QString ANN_NAME;
    static const QString ACTOR_BINDINGS;
    static const QString SOURCE_PORT;
    static const QString ALIAS;
    
public:
    struct U2LANG_EXPORT ReadFailed {
        ReadFailed(const QString & msg) : what(msg) {}
        QString what;
    }; // ReadFailed
    
    struct U2LANG_EXPORT Tokenizer {
        void tokenize(const QString & data);
        void tokenizeLine(const QString & line, QTextStream & s);
        void tokenizeBlock(const QString & line, QTextStream & s);
        void addToken(const QString & t);
        void appendToken(const QString & t, bool skipEmpty = true);
        void removeCommentTokens();
        void assertToken(const QString & etalon);
        
        QString take();
        QString look() const;
        bool notEmpty()const {return !tokens.isEmpty();}
        
        enum States {
            START_WORD
        }; // States
        
        QStringList tokens;
        int depth;
    }; // Tokenizer
    
    struct U2LANG_EXPORT ParsedPairs {
        ParsedPairs(Tokenizer & tokenizer);
        ParsedPairs() {}
        QMap<QString, QString> equalPairs;
        QMap<QString, QString> blockPairs;
        
        static QPair<QString, QString> parseOneEqual(Tokenizer & tokenizer);
    }; // ParsedPairs
    
    struct U2LANG_EXPORT FlowGraph {
        FlowGraph( const QList<QPair<Port*, Port*> >& d );
        bool findPath(Actor * from, Port * to) const;
        void removeDuplicates();
        void minimize();
        
        QMap<Port*, QList<Port*> > graph;
        QList<QPair<Port*, Port*> > dataflowLinks;
        int findRecursion;
    }; // FlowGraph
    
    typedef QMap<ActorId, QString> NamesMap;
    
    static void parseHeader(Tokenizer & tokenizer, Metadata * meta);
    static void parseIncludes(Tokenizer &tokenizer, QList<QString> includedUrls);
    static void parseBodyHeader(Tokenizer & tokenizer, Metadata * meta, bool needName = true);
    static Actor* parseElementsDefinition(Tokenizer & tokenizer, const QString & actorName, QMap<QString, Actor*> & actorMap, 
                                            QMap<ActorId, ActorId>* idMap = NULL);
    static void parseFunctionDefinition(Tokenizer & tokenizer, QMap<QString, Actor*> & actorMap);
    static void parseMarkerDefinition(Actor *proc, const QString &markerId, ParsedPairs &pairs);
    static QPair<Port*, Port*> parseDataflow(Tokenizer & tokenizer, const QString & srcTok, const QMap<QString, Actor*> & actorMap);
    static QString parseAt(const QString & dottedStr, int ind);
    static Iteration parseIteration(Tokenizer & tokenizer, const QString & iterationName, 
                                        const QMap<QString, Actor*> & actorMap, bool pasteMode = false);
    static ActorBindingsGraph parseActorBindings(Tokenizer &tokenizer, const QMap<QString, Actor*> &actorMap, QList<QPair<Port*, Port*> > &links);
    static void parseParameterAliases(Tokenizer & tokenizer, const QMap<QString, Actor*> & actorMap);
    static void parsePortAliases(Tokenizer & tokenizer, const QMap<QString, Actor*> & actorMap, QList<PortAlias> &portAliases);

    // -------------- backward compatibility --------------
        static void parseOldAliases(Tokenizer & tokenizer, const QMap<QString, Actor*> & actorMap);
        static void parseAliasesHelp(Tokenizer & tokenizer, const QList<Actor*> & procs);
    // ----------------------------------------------------

    // if slot has no val-> add it to binding
    static void addEmptyValsToBindings(const QList<Actor*> & procs);
    // idMap not null in copy mode
    static QString string2Schema(const QString & data, Schema * schema, Metadata * meta = NULL, QMap<ActorId, ActorId>* idMap = NULL, QList<QString> includedUrls = QList<QString>());
    
    static void addPart( QString & to, const QString & w);
    static QString header2String(const Metadata * meta);
    static QString makeBlock(const QString & title, const QString & name, const QString & blockItself, int tabsNum = 1, bool nl = false, bool sc = false);
    static QString makeEqualsPair(const QString & key, const QString & value, int tabsNum = 2);
    static QString makeArrowPair( const QString & left, const QString & right, int tabsNum = 1 );
    static QString scriptBlock(const QString & scriptText, int tabsNum = 3);
    static QString includesDefinition(const QList<Actor*> & procs);
    static QString elementsDefinition(const QList<Actor*> & procs, const NamesMap & nmap, bool copyMode = false);
    static QString markersDefinition(const QList<Actor*> & procs, const NamesMap & nmap, bool copyMode = false);
    static QString actorBindings(const ActorBindingsGraph *graph, const NamesMap &nmap, bool copyMode = false);
    static QString dataflowDefinition(const QList<Actor*> & procs, const NamesMap & nmap);
    static QString iterationsDefinition(const QList<Iteration> & iterations, const NamesMap & nmap, bool checkDummyIteration = true);
    static QString schemaParameterAliases(const QList<Actor*> & procs, const NamesMap& nmap);
    static QString schemaPortAliases(const NamesMap &nmap, const QList<PortAlias> &portAliases);
    static NamesMap generateElementNames(const QList<Actor*>& procs);
    static QString schema2String(const Schema & schema, const Metadata * meta, bool copyMode = false);

    static ExternalProcessConfig* string2Actor(const QString & bytes);
    static QString actor2String(ExternalProcessConfig *cfg);
    static ExternalProcessConfig *parseActorBody(Tokenizer & tokenizer);
    static void parsePorts(Tokenizer & tokenizer, QList<DataConfig>& ports);
    static void parseAttributes(Tokenizer & tokenizer, QList<AttributeConfig>& attrs);
    
    static QMap<ActorId, ActorId> deepCopy(const Schema& from, Schema* to);
};

} // U2

#endif // _U2_WORKFLOW_HR_SCHEMA_SERIALIZER_H_
