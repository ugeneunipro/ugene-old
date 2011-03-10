#ifndef _U2_WORKFLOW_HR_SCHEMA_SERIALIZER_H_
#define _U2_WORKFLOW_HR_SCHEMA_SERIALIZER_H_

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>
#include <U2Core/global.h>
#include <U2Lang/Schema.h>
#include <U2Lang/Port.h>

namespace U2 {

using namespace Workflow;

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
    static const QString BODY_START;
    static const QString META_START;
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
    static const QString ALIASES_START;
    static const QString ALIASES_HELP_START;
    static const QString VISUAL_START;
    static const QString UNDEFINED_META_BLOCK;
    static const QString TAB;
    static const QString NO_NAME;
    static const QString COLON;
    static const QString SEMICOLON;
    
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
    static void parseBodyHeader(Tokenizer & tokenizer, Metadata * meta, bool needName = true);
    static Actor* parseElementsDefinition(Tokenizer & tokenizer, const QString & actorName, QMap<QString, Actor*> & actorMap, 
                                            QMap<ActorId, ActorId>* idMap = NULL);
    static QPair<Port*, Port*> parseDataflow(Tokenizer & tokenizer, const QString & srcTok, const QMap<QString, Actor*> & actorMap);
    static QString parseAt(const QString & dottedStr, int ind);
    static Iteration parseIteration(Tokenizer & tokenizer, const QString & iterationName, 
                                        const QMap<QString, Actor*> & actorMap, bool pasteMode = false);
    static void parseAliases(Tokenizer & tokenizer, const QMap<QString, Actor*> & actorMap);
    static void parseAliasesHelp(Tokenizer & tokenizer, const QList<Actor*> & procs);
    // if slot has no val-> add it to binding
    static void addEmptyValsToBindings(const QList<Actor*> & procs);
    // idMap not null in copy mode
    static QString string2Schema(const QString & data, Schema * schema, Metadata * meta = NULL, QMap<ActorId, ActorId>* idMap = NULL);
    
    static void addPart( QString & to, const QString & w);
    static QString header2String(const Metadata * meta);
    static QString makeBlock(const QString & title, const QString & name, const QString & blockItself, int tabsNum = 1, bool nl = false, bool sc = false);
    static QString makeEqualsPair(const QString & key, const QString & value, int tabsNum = 2);
    static QString makeArrowPair( const QString & left, const QString & right, int tabsNum = 1 );
    static QString scriptBlock(const QString & scriptText, int tabsNum = 3);
    static QString elementsDefinition(const QList<Actor*> & procs, const NamesMap & nmap, bool copyMode = false);
    static QString dataflowDefinition(const QList<Actor*> & procs, const NamesMap & nmap);
    static QString iterationsDefinition(const QList<Iteration> & iterations, const NamesMap & nmap, bool checkDummyIteration = true);
    static QString schemaAliases(const QList<Actor*> & procs, const NamesMap& nmap);
    static QString aliasesHelp(const QList<Actor*> & procs);
    static NamesMap generateElementNames(const QList<Actor*>& procs);
    static QString schema2String(const Schema & schema, const Metadata * meta, bool copyMode = false);
    
    static QMap<ActorId, ActorId> deepCopy(const Schema& from, Schema* to);
};

} // U2

#endif // _U2_WORKFLOW_HR_SCHEMA_SERIALIZER_H_
