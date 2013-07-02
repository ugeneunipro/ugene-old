/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <QtCore/QTextStream>

#include <U2Core/GUrl.h>
#include <U2Core/Log.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/ActorModel.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/Dataset.h>
#include <U2Lang/ExternalToolCfg.h>
#include <U2Lang/GrouperSlotAttribute.h>
#include <U2Lang/HRWizardSerializer.h>
#include <U2Lang/IncludedProtoFactory.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/Marker.h>
#include <U2Lang/MarkerAttribute.h>
#include <U2Lang/SchemaSerializer.h>
#include <U2Lang/ScriptWorkerSerializer.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowSettings.h>
#include <U2Lang/WorkflowUtils.h>

#include "HRVisualSerializer.h"

#include "HRSchemaSerializer.h"

namespace U2 {

const QString HRSchemaSerializer::BLOCK_START           = "{";
const QString HRSchemaSerializer::BLOCK_END             = "}";
const QString HRSchemaSerializer::SERVICE_SYM           = "#";
const QString HRSchemaSerializer::QUOTE                 = "\"";
const QString HRSchemaSerializer::NEW_LINE              = "\n";
const QString HRSchemaSerializer::UNKNOWN_ERROR         = HRSchemaSerializer::tr("Error: unknown exception caught");
const QString HRSchemaSerializer::NO_ERROR              = "";
const QString HRSchemaSerializer::HEADER_LINE           = "#@UGENE_WORKFLOW";
const QString HRSchemaSerializer::DEPRECATED_HEADER_LINE= "#!UGENE_WORKFLOW";
const QString HRSchemaSerializer::OLD_XML_HEADER        = "<!DOCTYPE GB2WORKFLOW>";
const QString HRSchemaSerializer::INCLUDE               = "include";
const QString HRSchemaSerializer::INCLUDE_AS            = "as";
const QString HRSchemaSerializer::BODY_START            = "workflow";
const QString HRSchemaSerializer::META_START            = ".meta";
const QString HRSchemaSerializer::DOT_ITERATION_START   = ".iteration";
const QString HRSchemaSerializer::ITERATION_START       = "iteration";
const QString HRSchemaSerializer::DATAFLOW_SIGN         = "->";
const QString HRSchemaSerializer::EQUALS_SIGN           = ":";
const QString HRSchemaSerializer::UNDEFINED_CONSTRUCT   = HRSchemaSerializer::tr("Undefined construct at '%1 %2'");
const QString HRSchemaSerializer::TYPE_ATTR             = "type";
const QString HRSchemaSerializer::SCRIPT_ATTR           = "script";
const QString HRSchemaSerializer::NAME_ATTR             = "name";
const QString HRSchemaSerializer::ELEM_ID_ATTR          = "elem-id";
const QString HRSchemaSerializer::DOT                   = ".";
const QString HRSchemaSerializer::DASH                  = "-";
const QString HRSchemaSerializer::ITERATION_ID          = "id";
const QString HRSchemaSerializer::PARAM_ALIASES_START   = "parameter-aliases";
const QString HRSchemaSerializer::PORT_ALIASES_START    = "port-aliases";
const QString HRSchemaSerializer::PATH_THROUGH          = "path-through";

// -------------- backward compatibility --------------
    const QString HRSchemaSerializer::ALIASES_HELP_START    = "help";
    const QString HRSchemaSerializer::OLD_ALIASES_START     = "aliases";
// ----------------------------------------------------

const QString HRSchemaSerializer::VISUAL_START          = "visual";
const QString HRSchemaSerializer::UNDEFINED_META_BLOCK  = HRSchemaSerializer::tr("Undefined block in .meta: '%1'");
const QString HRSchemaSerializer::TAB                   = "    ";
const QString HRSchemaSerializer::NO_NAME               = "";
const QString HRSchemaSerializer::COLON                 = ":";
const QString HRSchemaSerializer::SEMICOLON             = ";";
const QString HRSchemaSerializer::INPUT_START           = ".inputs";
const QString HRSchemaSerializer::OUTPUT_START          = ".outputs";
const QString HRSchemaSerializer::ATTRIBUTES_START      = ".attributes";
const QString HRSchemaSerializer::TYPE_PORT             = "type";
const QString HRSchemaSerializer::FORMAT_PORT           = "format";
const QString HRSchemaSerializer::CMDLINE               = "cmdline";
const QString HRSchemaSerializer::DESCRIPTION           = "description";
const QString HRSchemaSerializer::PROMPTER              = "templatedescription";
const QString HRSchemaSerializer::FUNCTION_START        = "@";
const QString HRSchemaSerializer::COMMA                 = ",";
const QString HRSchemaSerializer::MARKER                = "marker";
const QString HRSchemaSerializer::MARKER_TYPE           = "marker-type";
const QString HRSchemaSerializer::MARKER_NAME           = "marker-name";
const QString HRSchemaSerializer::QUAL_NAME             = "qualifier-name";
const QString HRSchemaSerializer::ANN_NAME              = "annotation-name";
const QString HRSchemaSerializer::ACTOR_BINDINGS        = ".actor-bindings";
const QString HRSchemaSerializer::SOURCE_PORT           = "source";
const QString HRSchemaSerializer::ALIAS                 = "alias";
const QString HRSchemaSerializer::IN_SLOT               = "in-slot";
const QString HRSchemaSerializer::ACTION                = "action";
const QString HRSchemaSerializer::OUT_SLOT_ATTR         = "out-slot";
const QString HRSchemaSerializer::DATASET_NAME          = "dataset";
const QString HRSchemaSerializer::DIRECTORY_URL         = "dir";
const QString HRSchemaSerializer::FILE_URL              = "file";
const QString HRSchemaSerializer::PATH                  = "path";
const QString HRSchemaSerializer::EXC_FILTER            = "exclude-name-filter";
const QString HRSchemaSerializer::INC_FILTER            = "include-name-filter";
const QString HRSchemaSerializer::RECURSIVE             = "recursive";

template <class T>
static void setIfNotNull(const T & what, T * to) {
    if(to != NULL) {
        *to = what;
    }
}

static Attribute* getAttribute(Actor * proc, const QString & attrId) {
    assert(proc != NULL);
    if( proc->hasParameter(attrId) ) {
        return proc->getParameter(attrId);
    } else if( proc->hasParameter(BaseAttributes::URL_IN_ATTRIBUTE().getId()) && attrId == BaseAttributes::URL_LOCATION_ATTRIBUTE().getId() ) {
        Attribute * attr = new Attribute(BaseAttributes::URL_LOCATION_ATTRIBUTE(), BaseTypes::BOOL_TYPE(), false, true);
        proc->addParameter( BaseAttributes::URL_LOCATION_ATTRIBUTE().getId(), attr);
        return attr;
    }
    return NULL;
}

static QVariant getAttrValue(Actor * proc, const QString & attrId, const QString & valueStr) {
    Attribute * attr = getAttribute(proc, attrId);
    if(attr == NULL) {
        throw HRSchemaSerializer::ReadFailed(HRSchemaSerializer::tr("Parameter '%1' undefined for element '%2'").
            arg(attrId).arg(proc->getLabel()));
    }
    DataTypeValueFactory * valueFactory = WorkflowEnv::getDataTypeValueFactoryRegistry()->getById(attr->getAttributeType()->getId());
    if( valueFactory == NULL ) {
        throw HRSchemaSerializer::ReadFailed(HRSchemaSerializer::tr("Cannot parse value from '%1': no value factory").arg(valueStr));
    }
    bool ok = false;
    QVariant value = valueFactory->getValueFromString(valueStr, &ok);
    if(!ok) {
        throw HRSchemaSerializer::ReadFailed(HRSchemaSerializer::tr("Cannot parse value from '%1'").arg(valueStr));
    }
    return value;
}

static QString makeIndent(int tabsNum) {
    tabsNum = tabsNum <= 0 ? 0 : tabsNum;
    QString res;
    for(int i = 0; i < tabsNum; ++i) {
        res += HRSchemaSerializer::TAB;
    }
    return res;
}

static QString quotedString(const QString & str) {
    return HRSchemaSerializer::QUOTE + str + HRSchemaSerializer::QUOTE;
}

QString HRSchemaSerializer::valueString(const QString & s) {
    QString str = s;
    str.replace("\"", "'");
    if( str.contains(QRegExp("\\s") ) || str.contains(HRSchemaSerializer::SEMICOLON) || 
        str.contains(HRSchemaSerializer::EQUALS_SIGN) || str.contains(HRSchemaSerializer::DATAFLOW_SIGN) || 
        str.contains(HRSchemaSerializer::BLOCK_START) || str.contains(HRSchemaSerializer::BLOCK_END) ||
        str.contains(HRSchemaSerializer::FUNCTION_START)) {
        return quotedString(str);
    } else {
        return str;
    }
}

QString HRSchemaSerializer::Tokenizer::take() {
    if(tokens.isEmpty()) {
        throw ReadFailed(HRSchemaSerializer::tr("Unexpected end of file"));
    }
    return tokens.takeFirst();
}

QString HRSchemaSerializer::Tokenizer::look() const {
    if(tokens.isEmpty()) {
        throw ReadFailed(HRSchemaSerializer::tr("Unexpected end of file"));
    }
    return tokens.first();
}

void HRSchemaSerializer::Tokenizer::appendToken(const QString & t, bool skipEmpty) {
    if(t.isEmpty() && skipEmpty) {
        return;
    }
    if(t == BLOCK_START) {
        depth++;
    }
    if(t == BLOCK_END) {
        depth--;
    }
    tokens.append(t);
}

void HRSchemaSerializer::Tokenizer::addToken(const QString & t) { 
    QString tok = t.trimmed().replace("'", "\"");
    if(tok.isEmpty() || tok == SEMICOLON) {
        return;
    }
    if(tok.contains(EQUALS_SIGN) && tok != EQUALS_SIGN) {
        int ind = tok.indexOf(EQUALS_SIGN);
        assert(ind != -1);
        appendToken(tok.mid(0, ind));
        appendToken(EQUALS_SIGN);
        appendToken(tok.mid(ind + 1));
        return;
    }
    if(tok.contains(DATAFLOW_SIGN) && tok != DATAFLOW_SIGN) {
        QStringList splitted = tok.split(DATAFLOW_SIGN);
        assert(splitted.size() == 2);
        appendToken(splitted.at(0));
        appendToken(DATAFLOW_SIGN);
        appendToken(splitted.at(1));
        return;
    }
    if(tok.endsWith(BLOCK_START) && tok != BLOCK_START) {
        appendToken(tok.mid(0, tok.size() - BLOCK_START.size()));
        appendToken(BLOCK_START);
        return;
    }
    if( tok.startsWith(BLOCK_START) && tok != BLOCK_START ) {
        appendToken(BLOCK_START);
        appendToken(tok.mid(1));
        return;
    }
    if(tok.startsWith(BLOCK_END) && tok != BLOCK_END) {
        appendToken(BLOCK_END);
        appendToken(tok.mid(1));
        return;
    }
    if(tok.endsWith(BLOCK_END) && tok != BLOCK_END) {
        appendToken(tok.mid(0, tok.size() - BLOCK_END.size()));
        appendToken(BLOCK_END);
        return;
    }
    appendToken(tok);
}

void HRSchemaSerializer::Tokenizer::removeCommentTokens() {
    foreach(const QString & t, tokens) {
        if(t.startsWith(SERVICE_SYM)) {
            tokens.removeAll(t);
        }
    }
}

void HRSchemaSerializer::Tokenizer::assertToken(const QString & etalon) {
    QString candidate = take();
    if( candidate != etalon ) {
        throw ReadFailed(HRSchemaSerializer::tr("Expected '%1', got %2").arg(etalon).arg(candidate));
    }
}

static bool isBlockLine(const QString & str) {
    int bInd = str.indexOf(HRSchemaSerializer::BLOCK_START);
    int eInd = str.indexOf(HRSchemaSerializer::EQUALS_SIGN);
    if(bInd == -1) {
        return false;
    } else {
        if(eInd == -1) {
            return true;
        } else {
            return bInd < eInd;
        }
    }
}

static const int WIZARD_PAGE_DEPTH = 3;
static const int ELEMENT_DEPTH = 1;
void HRSchemaSerializer::Tokenizer::tokenizeSchema(const QString & d) {
    depth = 0;
    QString data = d;
    QTextStream stream(&data);
    bool isElemDef = false;
    bool elemDefHeader = false;
    bool pageDef = false;
    bool pageDefHeader = false;
    do {
        QString line = stream.readLine().trimmed();
        if(line.isEmpty()) {
            continue;
        }
        if( line.startsWith(SERVICE_SYM) ) {
            appendToken(line);
            continue;
        }
        if (ELEMENT_DEPTH == depth) {
            isElemDef = !line.startsWith(META_START) && !line.startsWith(DOT_ITERATION_START) && !line.contains(DATAFLOW_SIGN) 
                && !line.startsWith(INPUT_START) && !line.startsWith(OUTPUT_START) && !line.startsWith(ATTRIBUTES_START);
            elemDefHeader = true;
        } else {
            elemDefHeader = false;
        }
        if (WIZARD_PAGE_DEPTH == depth) {
            pageDef = line.startsWith(HRWizardParser::PAGE);
            pageDefHeader = true;
        } else {
            pageDefHeader = false;
        }
        if(isBlockLine(line) &&
            ((pageDef && !pageDefHeader) || (isElemDef && !elemDefHeader))) {
            tokenizeBlock(line, stream);
            continue;
        }
        tokenizeLine(line, stream);
    } while (!stream.atEnd());
}

void HRSchemaSerializer::Tokenizer::tokenize(const QString &d, int unparseableBlockDepth) {
    depth = 0;
    QString data = d;
    QTextStream stream(&data);
    do {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }
        if (line.startsWith(SERVICE_SYM)) {
            appendToken(line);
            continue;
        }
        if (isBlockLine(line) && depth >= unparseableBlockDepth) {
            tokenizeBlock(line, stream);
        } else {
            tokenizeLine(line, stream);
        }
    } while (!stream.atEnd());
}

static void skipDelimiters(QTextStream & s) {
    while(!s.atEnd()) {
        qint64 curPos = s.pos();
        QChar ch; s >> ch;
        if(ch.isSpace() || ch == HRSchemaSerializer::NEW_LINE.at(0) || ch == HRSchemaSerializer::SEMICOLON.at(0)) {
            continue;
        }
        s.seek(curPos);
        break;
    }
}

void HRSchemaSerializer::Tokenizer::tokenizeBlock(const QString & line, QTextStream & s) {
    if(!line.contains(BLOCK_START)) {
        throw ReadFailed(HRSchemaSerializer::tr("Expected '%1', near '%2'").arg(BLOCK_START).arg(line));
    }
    QString tok = line.mid(0, line.indexOf(BLOCK_START)).trimmed();
    appendToken(tok);
    appendToken(BLOCK_START);
    QString blockTok;
    QString ln = line.mid(line.indexOf(BLOCK_START) + 1);
    if(ln.isEmpty()) {
        ln = s.readLine();
    }
    ln += NEW_LINE;
    QTextStream stream(&ln);
    int level = 0;
    while(!stream.atEnd()) {
        QChar ch; stream >> ch;
        if(ch == BLOCK_START.at(0)) {
            level++;
        }
        if(ch == BLOCK_END.at(0)) {
            if(level-- == 0) {
                appendToken(blockTok.trimmed(), false);
                appendToken(BLOCK_END);
                skipDelimiters(stream);
                if(!stream.atEnd()) {
                    tokenizeBlock(stream.readAll(), s);
                }
                return;
            }
        }
        blockTok.append(ch);
        if(stream.atEnd()) {
            ln = s.readLine() + NEW_LINE;
            stream.setString(&ln);
        }
    }
}

void HRSchemaSerializer::Tokenizer::tokenizeLine(const QString & l, QTextStream & s) {
    QString line = l;
    QTextStream stream(&line);
    QString curToken;
    bool finishAtQuote = false;
    while(!stream.atEnd()) {
        QChar ch; stream >> ch;
        if( stream.atEnd() && finishAtQuote && ch != QUOTE.at(0) ) {
            line = s.readLine();
            stream.setString(&line);
        }
        if(ch.isSpace() || ch == SEMICOLON.at(0)) {
            if(!finishAtQuote) {
                addToken(curToken);
                curToken.clear();
                continue;
            } else {
                curToken.append(ch);
            }
        } else if(ch == QUOTE.at(0)) {
            if( finishAtQuote ) {
                appendToken(curToken);
                curToken.clear();
                finishAtQuote = false;
            } else {
                addToken(curToken);
                curToken.clear();
                finishAtQuote = true;
            }
            continue;
        } else if (ch == FUNCTION_START.at(0)) {
            if (finishAtQuote) {
                curToken.append(ch);
            } else {
                addToken(curToken);
                curToken.clear();
                addToken(ch);
            }
        } else {
            curToken.append(ch);
        }
    }
    addToken(curToken);
}

static QString skipBlock(HRSchemaSerializer::Tokenizer & tokenizer) {
    QString skipped;
    while(tokenizer.look() != HRSchemaSerializer::BLOCK_END) {
        QString tok = tokenizer.take();
        skipped += "\n" + HRSchemaSerializer::valueString(tok);;
        if( tok == HRSchemaSerializer::BLOCK_START ) {
            skipped += skipBlock(tokenizer);
            skipped += "\n" + HRSchemaSerializer::BLOCK_END;
        }
    }
    tokenizer.take();
    return skipped;
}

HRSchemaSerializer::ParsedPairs::ParsedPairs(HRSchemaSerializer::Tokenizer & tokenizer, bool bigBlocks) {
    init(tokenizer, bigBlocks);
}

HRSchemaSerializer::ParsedPairs::ParsedPairs(const QString & data, int unparseableBlockDepth) {
    HRSchemaSerializer::Tokenizer tokenizer;
    tokenizer.tokenize(data, unparseableBlockDepth);
    init(tokenizer, false);
}

void HRSchemaSerializer::ParsedPairs::init(Tokenizer & tokenizer, bool bigBlocks) {
    while(tokenizer.notEmpty() && tokenizer.look() != BLOCK_END) {
        QString tok = tokenizer.take();
        QString next = tokenizer.take();
        if( next == EQUALS_SIGN ) {
            QString value = tokenizer.take();
            equalPairs[tok] = value;
            equalPairsList << StringPair(tok, value);
        }
        else if(next == BLOCK_START) {
            QString value;
            if (bigBlocks) {
                value = skipBlock(tokenizer);
            } else {
                value = tokenizer.take();
                tokenizer.assertToken(BLOCK_END);
            }
            blockPairs.insertMulti(tok, value);
            blockPairsList << StringPair(tok, value);
        }
        else {
            throw ReadFailed(HRSchemaSerializer::tr("Expected %3 or %1 after %2").arg(BLOCK_START).arg(tok).arg(EQUALS_SIGN));
        }
    }
}

QPair<QString, QString> HRSchemaSerializer::ParsedPairs::parseOneEqual(Tokenizer & tokenizer) {
    QPair<QString, QString> res;
    res.first = tokenizer.take();
    if(tokenizer.take() != EQUALS_SIGN) {
        throw ReadFailed(HRSchemaSerializer::tr("%2 expected after %1").arg(res.first).arg(EQUALS_SIGN));
    }
    res.second = tokenizer.take();
    return res;
}

HRSchemaSerializer::FlowGraph::FlowGraph( const QList<QPair<Port*, Port*> >& d ) : dataflowLinks(d) {
    removeDuplicates();
    for(int i = 0; i < dataflowLinks.size(); ++i) {
        const QPair<Port*, Port*> & pair = dataflowLinks.at(i);
        if(!graph.contains(pair.first)) {
            graph[pair.first] = QList<Port*>();
        }
        graph.find(pair.first)->append(pair.second);
    }
}

void HRSchemaSerializer::FlowGraph::removeDuplicates() {
    QList<QPair<Port*, Port*> > links;
    for(int i = 0; i < dataflowLinks.size(); ++i) {
        const QPair<Port*, Port*> & p = dataflowLinks.at(i);
        bool found = false;
        for(int j = 0; j < links.size(); ++j) {
            const QPair<Port*, Port*> & pair = links.at(j);
            if(pair.first == p.first && pair.second == p.second) {
                found = true;
                break;
            }
        }
        if(!found) {
            links << p;
        }
    }
    dataflowLinks = links;
}

bool HRSchemaSerializer::FlowGraph::findPath(Actor * from, Port * to) const {
    static const int RECURSION_MAX = 100;
    if(findRecursion == RECURSION_MAX) {
        throw ReadFailed(tr("Cannot create flow graph"));
    }
    foreach(Port * p, from->getOutputPorts()) {
        if(graph[p].contains(to) ) {
            return true;
        }
        foreach(Port * connection, graph[p]) {
            if(findPath(connection->owner(), to)) {
                return true;
            }
        }
    }
    return false;
}

void HRSchemaSerializer::FlowGraph::minimize() {
    for(int i = 0; i < dataflowLinks.size(); ++i) {
        Port * src = dataflowLinks.at(i).first;
        Port * dst = dataflowLinks.at(i).second;
        foreach(Port * p, graph[src]) {
            findRecursion = 0;
            if(findPath(p->owner(), dst) ){
                graph.find(src)->removeAll(dst);
                break;
            }
        }
    }
}

bool HRSchemaSerializer::isHeaderLine(const QString &line) {
    return (line.startsWith(HEADER_LINE) ||
        line.startsWith(DEPRECATED_HEADER_LINE));
}

void HRSchemaSerializer::checkHeaderLine(const QString &line, Tokenizer &tokenizer) {
    if(!isHeaderLine(line)) {
        if( tokenizer.notEmpty() && line + " " + tokenizer.take() == OLD_XML_HEADER ) {
             throw ReadFailed(tr("XML schema format is obsolete and not supported"));
        }
        throw ReadFailed(tr("Bad header: expected '%1', got '%2'").arg(HEADER_LINE).arg(line));
     }
}

void HRSchemaSerializer::parseHeader(Tokenizer & tokenizer, Metadata * meta) {
    QString head = tokenizer.take();
    checkHeaderLine(head, tokenizer);
    QString desc;
    while(tokenizer.look().startsWith(SERVICE_SYM)) {
        desc += tokenizer.take().mid(SERVICE_SYM.size()) + HRSchemaSerializer::NEW_LINE;
    }
    setIfNotNull<QString>(desc, meta == NULL ? NULL : &meta->comment);
}

enum IncludeElementType {
    SCHEMA,
    EXTERNAL_TOOL,
    SCRIPT
};

static bool getAbsoluteIncludePath(QString &path) {
    if (QFileInfo(path).isAbsolute()) {
        return QFile::exists(path);
    }
    
    QString absPath;
    absPath = WorkflowSettings::getExternalToolDirectory() + path;
    if (QFile::exists(absPath)) {
        path = absPath;
        return true;
    }

    absPath = WorkflowSettings::getUserDirectory() + path;
    if (QFile::exists(absPath)) {
        path = absPath;
        return true;
    }

    absPath = WorkflowSettings::getIncludedElementsDirectory() + path;
    if (QFile::exists(absPath)) {
        path = absPath;
        return true;
    }

    return false;
}

void HRSchemaSerializer::parseIncludes(Tokenizer &tokenizer, QList<QString> includedUrls) {
    tokenizer.assertToken(INCLUDE);
    QString path = tokenizer.take();
    QString actorName;
    bool includeAs = false;
    QString tok = tokenizer.look();
    if (INCLUDE_AS == tok) {
        tokenizer.assertToken(INCLUDE_AS);
        includeAs = true;
        actorName = tokenizer.take();
    }

    if (!getAbsoluteIncludePath(path)) {
        throw ReadFailed(tr("The included file '%1' doesn't exists").arg(path));
    }
    
    // read the file content
    QString ext = GUrl(path).lastFileSuffix();
    QString rawData;
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            throw ReadFailed(tr("Can't open '%1'").arg(path));
        }
        QTextStream in(&file);
        in.setCodec("UTF-8");
        rawData = in.readAll();
        rawData = rawData.trimmed();
    }

    IncludeElementType includeType;
    ActorPrototype *proto = NULL;
    ExternalProcessConfig *cfg = NULL;
    Schema *schema = NULL;
    QString error;

    // construct the needed proto from the file content
    if(isHeaderLine(rawData)) {
        if ("etc" == ext) {
            includeType = EXTERNAL_TOOL;
            cfg = string2Actor(rawData);
            if (NULL == cfg) {
                throw ReadFailed(tr("File '%1' contains mistakes").arg(path));
            }
            if (includeAs) {
                cfg->name = actorName;
            } else {
                actorName = cfg->name;
            }
            cfg->filePath = path;
            proto = IncludedProtoFactory::getExternalToolProto(cfg);
        } else {
            includeType = SCHEMA;
            if (includedUrls.contains(path)) {
                throw ReadFailed(tr("There is recursive including of the file: '%1'").arg(path));
            }
            QList<QString> newUrlList = includedUrls;
            newUrlList.append(path);

            schema = new Schema();
            QMap<ActorId, ActorId> procMap;
            error = string2Schema(rawData, schema, NULL, &procMap, newUrlList);
            if (NULL != schema && error.isEmpty()) {
                if (includeAs) {
                    schema->setTypeName(actorName);
                } else {
                    actorName = schema->getTypeName();
                }
                proto = IncludedProtoFactory::getSchemaActorProto(schema, actorName, path);
            }
        }
    } else if(rawData.startsWith(HRSchemaSerializer::OLD_XML_HEADER)) {
        includeType = SCRIPT;
        proto = ScriptWorkerSerializer::string2actor(rawData, actorName, error, path);
        if (!includeAs && NULL != proto) {
            actorName = proto->getDisplayName();
        }
    } else {
        throw ReadFailed(tr("Unknown file format: '%1'").arg(path));
    }
    if (NULL == proto || !error.isEmpty()) {
        throw ReadFailed( QString("Included element \"%1\" contains a error: %2").arg(path).arg(error) );
    }

    // register the new proto
    if (IncludedProtoFactory::isRegistered(actorName)) {
        bool isEqualProtos = IncludedProtoFactory::isRegisteredTheSameProto(actorName, proto);
        if (!isEqualProtos) {
            throw ReadFailed( QString("Another worker with this name is already registered: %1").arg(actorName) );
        }
    } else {
        WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_INCLUDES(), proto);
        if (EXTERNAL_TOOL == includeType) {
            WorkflowEnv::getExternalCfgRegistry()->registerExternalTool(cfg);
            IncludedProtoFactory::registerExternalToolWorker(cfg);
        } else if (SCRIPT == includeType) {
            IncludedProtoFactory::registerScriptWorker(actorName);
        } else if (SCHEMA == includeType) {
            WorkflowEnv::getSchemaActorsRegistry()->registerSchema(actorName, schema);
        }
    }
}

void HRSchemaSerializer::parseBodyHeader(Tokenizer & tokenizer, Metadata * meta, bool needName) {
    QString bodyStart = tokenizer.take();
    if( bodyStart != BODY_START ) {
        throw ReadFailed(tr("Bad header: expected '%1', got '%2'").arg(BODY_START).arg(bodyStart));
    }
    
    if(tokenizer.look() == BLOCK_START) {
        if(needName) {
            coreLog.details(tr("Schema name not specified"));
        }
    } else {
        setIfNotNull<QString>(tokenizer.take(), meta == NULL ? NULL : &meta->name);
    }
}

void HRSchemaSerializer::deprecatedUrlAttribute(Actor *proc, const QString &urls) {
    QStringList urlList = urls.split(SEMICOLON);
    Dataset dSet;
    foreach (const QString &url, urlList) {
        dSet.addUrl(new FileUrlContainer(url));
    }
    Attribute *a = proc->getParameter(BaseAttributes::URL_IN_ATTRIBUTE().getId());
    if (NULL != a) {
        QList<Dataset> sets;
        sets << dSet;
        a->setAttributeValue(qVariantFromValue< QList<Dataset> >(sets));
    }
}

QList<Dataset> HRSchemaSerializer::parseUrlAttribute(const QString attrId, QList<StringPair> &blockPairs) {
    QList<Dataset> sets;
    QStringList setBlocks;
    foreach (const StringPair &pair, blockPairs) {
        if (attrId == pair.first) {
            setBlocks << pair.second;
            blockPairs.removeOne(pair);
        }
    }
    foreach (const QString &block, setBlocks) {
        Tokenizer tokenizer;
        tokenizer.tokenize(block);

        QString name;
        QList<URLContainer*> urls;
        try {
            while (tokenizer.notEmpty()) {
                QString tok = tokenizer.take();
                if (DATASET_NAME == tok) {
                    tokenizer.assertToken(EQUALS_SIGN);
                    name = tokenizer.take();
                } else if (FILE_URL == tok) {
                    tokenizer.assertToken(EQUALS_SIGN);
                    urls << new FileUrlContainer(tokenizer.take());
                } else if (DIRECTORY_URL == tok) {
                    urls << parseDirectoryUrl(tokenizer);
                }
            }

            if (name.isEmpty()) {
                throw ReadFailed(tr("Url definition does not contain dataset name"));
            }
        } catch (ReadFailed ex) {
            foreach (URLContainer *url, urls) {
                delete url;
            }
            throw ReadFailed(ex.what);
        }
        Dataset dSet(name);
        foreach (URLContainer *url, urls) {
            dSet.addUrl(url);
        }
        sets << dSet;
    }

    return sets;
}

URLContainer * HRSchemaSerializer::parseDirectoryUrl(Tokenizer &tokenizer) {
    QString sign = tokenizer.take();
    if (EQUALS_SIGN == sign) {
        return new DirUrlContainer(tokenizer.take());
    } else if (BLOCK_START == sign) {
        ParsedPairs pairs(tokenizer);
        tokenizer.assertToken(BLOCK_END);
        QString path = pairs.equalPairs.value(PATH, "");
        QString incFilter = pairs.equalPairs.value(INC_FILTER, "");
        QString excFilter = pairs.equalPairs.value(EXC_FILTER, "");
        QString recursiveStr = pairs.equalPairs.value(RECURSIVE, "false");
        DataTypeValueFactory * valueFactory = WorkflowEnv::getDataTypeValueFactoryRegistry()->getById(BaseTypes::BOOL_TYPE()->getId());
        bool recursive = false;
        bool ok = false;
        QVariant v = valueFactory->getValueFromString(recursiveStr, &ok);
        if (ok) {
            recursive = v.toBool();
        }

        return new DirUrlContainer(path, incFilter, excFilter, recursive);
    } else {
        throw new ReadFailed(tr("Directory url definition: '%1' or '%2' are expected, '%3' is found")
            .arg(BLOCK_START).arg(EQUALS_SIGN).arg(sign));
    }
}

Actor* HRSchemaSerializer::parseElementsDefinition(Tokenizer & tokenizer, const QString & actorName, 
                                                   QMap<QString, Actor*> & actorMap, QMap<ActorId, ActorId>* idMap) {
    if( actorName.contains(QRegExp("\\s")) ) {
        throw ReadFailed(tr("Element name cannot contain whitespaces: '%1'").arg(actorName));
    }
    if(actorName.contains(DOT)) {
        throw ReadFailed(tr("Element name cannot contain dots: '%1'").arg(actorName));
    }
    if(actorMap.contains(actorName)) {
        throw ReadFailed(tr("Element '%1' already defined").arg(actorName));
    }
    
    ParsedPairs pairs(tokenizer);
    QString procType = pairs.equalPairs.take(TYPE_ATTR);
    if( procType.isEmpty() ) {
        throw ReadFailed(tr("Type attribute not set for %1 element").arg(actorName));
    }
    ActorPrototype * proto = WorkflowEnv::getProtoRegistry()->getProto(SchemaSerializer::getElemType(procType));
    
    QString procScriptText = pairs.blockPairs.take(SCRIPT_ATTR);
    Actor * proc = NULL;
    proc = deprecatedActorsReplacer(actorName, procType, pairs); //AttributeScript always empty for replaced actors
    if(proto == NULL && proc == NULL) {
        throw ReadFailed(tr("Unknown type of %1 element: %2").arg(actorName).arg(procType));
    }
    if(proc == NULL){
        proc = proto->createInstance(actorName, procScriptText.isEmpty() ? NULL : new AttributeScript(procScriptText));
    }
    actorMap[actorName] = proc;

    QString procName = pairs.equalPairs.take(NAME_ATTR);
    if( procName.isEmpty() ) {
        throw ReadFailed(tr("Name attribute not set for %1 element").arg(actorName));
    }
    proc->setLabel(procName);
    
    ActorId oldId = str2aid(pairs.equalPairs.take(ELEM_ID_ATTR));
    if(idMap != NULL && !oldId.isEmpty()) {
        idMap->insert(oldId, proc->getId());
    }

    foreach(const QString & key, pairs.blockPairs.uniqueKeys()) {
        Attribute *a = proc->getParameter(key);
        if (NULL == a) {
            continue;
        }
        if (GROUPER_SLOT_GROUP == a->getGroup()) {
            parseGrouperOutSlots(proc, pairs.blockPairs.values(key), key);
        } else if (NULL != dynamic_cast<URLAttribute*>(a)) {
            QList<Dataset> sets = parseUrlAttribute(a->getId(), pairs.blockPairsList);
            a->setAttributeValue(qVariantFromValue< QList<Dataset> >(sets));
        } else {
            proc->getParameter(key)->getAttributeScript().setScriptText(pairs.blockPairs.value(key));
        }
    }
    
    foreach( const QString & key, pairs.equalPairs.keys() ) {
        proc->getParameter(key)->setAttributeValue(getAttrValue(proc, key, pairs.equalPairs.value(key)));
    }

    return proc;
}

GrouperSlotAction HRSchemaSerializer::parseAction(Tokenizer &tokenizer) {
    ParsedPairs pairs(tokenizer);
    tokenizer.assertToken(BLOCK_END);

    QString type = pairs.equalPairs.value(TYPE_ATTR, "");
    if (type.isEmpty()) {
        throw ReadFailed(tr("Grouper out slot action: empty type"));
    } else if (!ActionTypes::isValidType(type)) {
        throw ReadFailed(tr("Grouper out slot action: invalid type: %1").arg(type));
    }
    pairs.equalPairs.take(TYPE_ATTR);

    GrouperSlotAction result(type);

    foreach (const QString &paramId, pairs.equalPairs.keys()) {
        QString param = pairs.equalPairs.take(paramId);
        if (!ActionParameters::isValidParameter(type, paramId)) {
            throw ReadFailed(tr("Grouper out slot action: invalid parameter: %1").arg(paramId));
        }

        ActionParameters::ParameterType pType = ActionParameters::getType(paramId);
        QVariant var;
        bool ok = false;
        bool b = false;
        int num = 0;
        switch (pType) {
            case ActionParameters::INTEGER:
                num = param.toInt(&ok);
                if (!ok) {
                    throw ReadFailed(tr("Grouper out slot action: bad int '%1' at parameter %2")
                        .arg(param).arg(paramId));
                }
                var = num;
                break;
            case ActionParameters::BOOLEAN:
                if ("true" == param) {
                    b = true;
                } else if ("false" == param) {
                    b = false;
                } else {
                    throw ReadFailed(tr("Grouper out slot action: bad bool '%1' at parameter %2")
                        .arg(param).arg(paramId));
                }
                var = b;
                break;
            case ActionParameters::STRING:
                var = param;
                break;
        }

        result.setParameterValue(paramId, var);
    }

    return result;
}

void HRSchemaSerializer::parseGrouperOutSlots(Actor *proc, const QStringList &outSlotDefs, const QString &attrId) {
    GrouperOutSlotAttribute *attr = dynamic_cast<GrouperOutSlotAttribute*>(proc->getParameter(attrId));
    Tokenizer tokenizer;

    QStringList names;

    foreach (const QString &slotDef, outSlotDefs) {
        tokenizer.tokenizeSchema(slotDef);
        QString name;
        QString inSlot;
        std::auto_ptr<GrouperSlotAction> action(NULL);

        while (tokenizer.notEmpty()) {
            QString tok = tokenizer.take();
            QString next = tokenizer.take();
            if (EQUALS_SIGN == next) {
                if (NAME_ATTR == tok) {
                    name = tokenizer.take();
                    if (names.contains(name)) {
                        throw ReadFailed(tr("Grouper out slot: duplicated slot name: %1").arg(name));
                    }
                    names << name;
                } else if (IN_SLOT == tok) {
                    inSlot = tokenizer.take();
                } else {
                    throw ReadFailed(tr("Grouper out slot: unknown attribute: %1").arg(tok));
                }
            } else if (BLOCK_START == next) {
                if (ACTION != tok) {
                    throw ReadFailed(tr("Grouper out slot: unknown block definition: '%1'. %2 expected").arg(tok).arg(ACTION));
                }
                action.reset(new GrouperSlotAction(parseAction(tokenizer)));
            } else {
                throw ReadFailed(tr("Grouper out slot: unknown token: '%1'. %2 or %3 expected").arg(next).arg(BLOCK_START).arg(EQUALS_SIGN));
            }
        }

        if (name.isEmpty()) {
            throw ReadFailed(tr("Grouper out slot: empty slot name"));
        }
        if (inSlot.isEmpty()) {
            throw ReadFailed(tr("Grouper out slot: empty in-slot"));
        }

        GrouperOutSlot slot(name, inSlot);
        if (NULL != action.get()) {
            slot.setAction(*action.get());
        }
        attr->addOutSlot(slot);

        Port *outPort = proc->getOutputPorts().at(0);
        assert(outPort->getOutputType()->isMap());
        QMap<Descriptor, DataTypePtr> outTypeMap = outPort->getOutputType()->getDatatypesMap();
        Descriptor newTmpSlot = Descriptor(name, name, name);
        outTypeMap[newTmpSlot] = ActionTypes::getDataTypeByAction(action.get() ? action->getType() : "");
        DataTypePtr newType(new MapDataType(dynamic_cast<Descriptor&>(*(outPort->getType())), outTypeMap));
        outPort->setNewType(newType);
    }
}

void HRSchemaSerializer::finalizeGrouperSlots(const QMap<QString, Actor*> &actorMap) {
    foreach (Actor *p, actorMap.values()) {
        if (p->getId() != "grouper") { // TODO: fix it
            continue;
        }

        // check incoming slots
    }
}

QString HRSchemaSerializer::parseAt(const QString & dottedStr, int ind) {
    QStringList list = dottedStr.split(DOT);
    return list.size() > ind ? list.at(ind) : "";
}

static QString parseAfter(const QString & dottedStr, int ind) {
    QStringList list = dottedStr.split(HRSchemaSerializer::DOT);
    QString res;
    for(int i = ind + 1; i < list.size(); ++i) {
        res += list.at(i) + HRSchemaSerializer::DOT;
    }
    return res.mid(0, res.size() - HRSchemaSerializer::DOT.size());
}

void HRSchemaSerializer::parseFunctionDefinition(Tokenizer &tokenizer, QMap<QString, Actor*> &actorMap) {
    QString name = tokenizer.take();
    QString actorName = parseAt(name, 0);
    QString functionName = parseAfter(name, 0);
    if (!actorMap.contains(actorName)) {
        throw ReadFailed(tr("Unknown actor name \"%1\" at a function definition").arg(actorName));
    }
    tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);

    ParsedPairs pairs(tokenizer);
    QString functionType = pairs.equalPairs.take(TYPE_ATTR);
    if(functionType.isEmpty()) {
        throw ReadFailed(tr("Type attribute is not set for %1 function").arg(functionName));
    }
    if (functionType == HRSchemaSerializer::MARKER) {
        HRSchemaSerializer::parseMarkerDefinition(actorMap[actorName], functionName, pairs);
    }
}

void HRSchemaSerializer::parseMarkerDefinition(Actor *proc, const QString &markerId, ParsedPairs &pairs) {
    MarkerAttribute *markerAttr = dynamic_cast<MarkerAttribute*>(proc->getParameter("markers"));
    if (NULL == markerAttr) {
        throw ReadFailed(tr("%1 actor has not markers attribute").arg(proc->getId()));
    }
    QString markerType = pairs.equalPairs.take(HRSchemaSerializer::MARKER_TYPE);
    QString markerName = pairs.equalPairs.take(HRSchemaSerializer::MARKER_NAME);
    QMap<QString, Marker*> &markersMap = markerAttr->getMarkers();
    if(markerType.isEmpty()) {
        throw ReadFailed(tr("Type attribute is not set for %1 marker").arg(markerId));
    }
    if(markerName.isEmpty()) {
        throw ReadFailed(tr("Name attribute is not set for %1 marker").arg(markerId));
    }
    if (!markersMap.contains(markerId)) {
        throw ReadFailed(tr("Unknown %1 marker at %2 actor").arg(markerId).arg(proc->getId()));
    } else if (NULL != markersMap.value(markerId)) {
        throw ReadFailed(tr("Redefinition of %1 marker at %2 actor").arg(markerId).arg(proc->getId()));
    }

    Marker *marker = NULL;

    if (markerType == MarkerTypes::QUAL_INT_VALUE_MARKER_ID
     || markerType == MarkerTypes::QUAL_TEXT_VALUE_MARKER_ID
     || markerType == MarkerTypes::QUAL_FLOAT_VALUE_MARKER_ID) {
        QString qualName = pairs.equalPairs.take(HRSchemaSerializer::QUAL_NAME);
        if(qualName.isEmpty()) {
            throw ReadFailed(tr("Qualifier name attribute is not set for %1 marker").arg(markerId));
        }
        marker = new QualifierMarker(markerType, markerName, qualName);
    } else if (MarkerTypes::ANNOTATION_LENGTH_MARKER_ID == markerType
     || MarkerTypes::ANNOTATION_COUNT_MARKER_ID == markerType) {
        QString annName = pairs.equalPairs.take(HRSchemaSerializer::ANN_NAME);
        marker = new AnnotationMarker(markerType, markerName, annName);
    } else if (MarkerTypes::TEXT_MARKER_ID == markerType) {
        marker = new TextMarker(markerType, markerName);
    } else {
        marker = new SequenceMarker(markerType, markerName);
    }
    foreach(const QString & key, pairs.equalPairs.keys()) {
        marker->addValue(key, pairs.equalPairs.value(key));
    }

    markersMap.insert(markerId, marker);
    // TODO: make common way to get marked object output port
    assert(1 == proc->getOutputPorts().size());
    Port *outPort = proc->getOutputPorts().at(0);
    assert(outPort->getOutputType()->isMap());
    QMap<Descriptor, DataTypePtr> outTypeMap = outPort->getOutputType()->getDatatypesMap();
    Descriptor newSlot = MarkerSlots::getSlotByMarkerType(markerType, markerName);
    outTypeMap[newSlot] = BaseTypes::STRING_TYPE();
    DataTypePtr newType(new MapDataType(dynamic_cast<Descriptor&>(*(outPort->getType())), outTypeMap));
    outPort->setNewType(newType);
}

Iteration HRSchemaSerializer::parseIteration(Tokenizer & tokenizer, const QString & iterationName, 
                                             const QMap<QString, Actor*> & actorMap, bool pasteMode) {
    QPair<QString, QString> idPair = ParsedPairs::parseOneEqual(tokenizer);
    if( idPair.first != ITERATION_ID) {
        throw ReadFailed(tr("%1 definition expected at .iterations block").arg(ITERATION_ID));
    }
    
    Iteration iteration(iterationName);
    bool ok = false;
    iteration.id = idPair.second.toInt(&ok);
    if(!ok) {
        throw ReadFailed(tr("Cannot parse integer from '%1': iteration id").arg(idPair.second));
    }
    
    while(tokenizer.look() != BLOCK_END) {
        QString actorName = tokenizer.take();
        if( !actorMap.contains(actorName)) {
            if(!pasteMode) {
                throw ReadFailed(tr("Element id '%1' undefined in .iteration block").arg(actorName));
            } else {
                tokenizer.assertToken(BLOCK_START);
                skipBlock(tokenizer);
                continue;
            }
        }
        
        tokenizer.assertToken(BLOCK_START);
        ParsedPairs pairs(tokenizer, true /*bigBlocks*/);
        tokenizer.assertToken(BLOCK_END);

        QString actorId = actorMap[actorName]->getId();
        foreach (Attribute *attr, actorMap[actorName]->getParameters()) {
            QString attrId = attr->getId();
            if (pairs.equalPairs.contains(attrId)) {
                iteration.cfg[actorId][attrId] =
                    getAttrValue(actorMap[actorName], attrId, pairs.equalPairs[attrId]);
            }
            if (NULL == dynamic_cast<URLAttribute*>(attr)) {
                continue;
            }
            QList<Dataset> sets = parseUrlAttribute(attrId, pairs.blockPairsList);
            if (!sets.isEmpty()) {
                iteration.cfg[actorId][attrId] = qVariantFromValue(sets);
            }
        }
    }
    return iteration;
}

static void tryToConnect(Schema * schema, Port * input, Port * output) {
    if(!input || !output || !input->canBind(output)) {
        throw HRSchemaSerializer::ReadFailed(HRSchemaSerializer::tr("Cannot bind %1:%2 to %3:%4").
            arg(input->owner()->getId()).arg(input->getId()).arg(output->owner()->getId()).arg(output->getId()));
    }
    schema->addFlow(new Link(input, output));
}

void HRSchemaSerializer::parseActorBindings(Tokenizer &tokenizer, WorkflowSchemaReaderData &data) {
    const ActorBindingsGraph & graph = data.schema->getActorBindingsGraph();

    if (!graph.isEmpty()) {
        throw HRSchemaSerializer::ReadFailed(HRSchemaSerializer::tr("Links list is not empty. Maybe .meta is defined earlier than actor-bindings"));
    }

    while (tokenizer.look() != HRSchemaSerializer::BLOCK_END) {
        QString from = tokenizer.take();
        QString srcActorName = HRSchemaSerializer::parseAt(from, 0);
        Actor * srcActor = data.actorMap.value(srcActorName);
        if(srcActor == NULL) {
            throw HRSchemaSerializer::ReadFailed(HRSchemaSerializer::tr("Undefined element id: '%1'").arg(srcActorName));
        }
        QString srcPortId = HRSchemaSerializer::parseAt(from, 1);
        Port * srcPort = srcActor->getPort(srcPortId);
        if(srcPort == NULL) {
            throw HRSchemaSerializer::ReadFailed(HRSchemaSerializer::tr("Cannot find '%1' port at '%2'").arg(srcPortId).arg(srcActorName));
        }

        tokenizer.assertToken(HRSchemaSerializer::DATAFLOW_SIGN);
        QString to = tokenizer.take();
        QString dstActorName = HRSchemaSerializer::parseAt(to, 0);
        Actor * dstActor = data.actorMap.value(dstActorName);
        if(dstActor == NULL) {
            throw HRSchemaSerializer::ReadFailed(HRSchemaSerializer::tr("Undefined element id: '%1'").arg(dstActorName));
        }
        QString dstPortId = HRSchemaSerializer::parseAt(to, 1);
        Port * dstPort = dstActor->getPort(dstPortId);
        if(dstPort == NULL) {
            throw HRSchemaSerializer::ReadFailed(HRSchemaSerializer::tr("Cannot find '%1' port at '%2'").arg(dstPortId).arg(dstActorName));
        }

        tryToConnect(data.schema, srcPort, dstPort);
    }

    data.defineGraph();

    QString message;
    if (!graph.validateGraph(message)) {
        throw ReadFailed(tr("Validating actor bindings graph failed: '%1'").arg(message));
    }
}

void parseAndCheckParameterAlias(const QString &paramString, const QMap<QString, Actor*> &actorMap,
                                 QString &actorName, QString &paramId) {
    actorName = HRSchemaSerializer::parseAt(paramString, 0);
    Actor *actor = actorMap.value(actorName);
    if(actor == NULL) {
        throw HRSchemaSerializer::ReadFailed(
            HRSchemaSerializer::tr("%1 element is undefined: at \"%2\" in aliases block").arg(actorName).arg(paramString));
    }

    paramId = HRSchemaSerializer::parseAt(paramString, 1);
    Attribute *attr = actor->getParameter(paramId);
    if(NULL == attr) {
        throw HRSchemaSerializer::ReadFailed(
            HRSchemaSerializer::tr("%1 parameter is undefined: at \"%2\" in aliases block").arg(paramId).arg(paramString));
    }
}

void HRSchemaSerializer::parseParameterAliases(Tokenizer & tokenizer, const QMap<QString, Actor*> & actorMap) {
    QList<QString> newParamNames; // keeps all unique parameters aliases
    QList<QString> paramStrings; // keeps all unique aliased parameters

    while (tokenizer.look() != BLOCK_END) {
        QString paramString = tokenizer.take();
        tokenizer.assertToken(BLOCK_START);
        if (paramStrings.contains(paramString)) {
            throw ReadFailed(tr("Duplicate parameter alias \"%1\"").arg(paramString));
        }
        paramStrings.append(paramString);

        QString actorName;
        QString paramId;
        parseAndCheckParameterAlias(paramString, actorMap, actorName, paramId);

        ParsedPairs pairs(tokenizer);
        if(!pairs.blockPairs.isEmpty()) {
            throw ReadFailed(tr("Empty parameter alias block: \"%1\"").arg(paramString));
        }

        QString alias = pairs.equalPairs.take(ALIAS);
        if (alias.isEmpty()) {
            alias = paramString;
            alias.replace(DOT, "_at_");
        }
        if (newParamNames.contains(alias)) {
            throw ReadFailed(tr("Duplicate parameter alias name \"%1\" at \"%2\"").arg(alias).arg(paramString));
        }
        newParamNames.append(alias);

        QString descr = pairs.equalPairs.take(DESCRIPTION);

        Actor *actor = actorMap[actorName];
        actor->getParamAliases()[paramId] = alias;
        actor->getAliasHelp()[alias] = descr;

        tokenizer.assertToken(BLOCK_END);
    }
}

// -------------- backward compatibility --------------
    void HRSchemaSerializer::parseOldAliases(Tokenizer & tokenizer, const QMap<QString, Actor*> & actorMap) {
        ParsedPairs pairs(tokenizer);
        if(!pairs.blockPairs.isEmpty()) {
            throw ReadFailed(tr("No other blocks allowed in alias block"));
        }
        foreach(const QString & key, pairs.equalPairs.keys()) {
            QString actorName = parseAt(key, 0);
            Actor * actor = actorMap.value(actorName);
            if( actor == NULL ) {
                throw ReadFailed(tr("%1 element undefined in aliases block").arg(actorName));
            }
            QString attributeId = parseAt(key, 1);
            if( !actor->hasParameter(attributeId) ) {
                throw ReadFailed(tr("%1 has no parameter %2: in aliases block").arg(actorName).arg(attributeId));
            }
            actor->getParamAliases()[attributeId] = pairs.equalPairs.value(key);
        }
    }

    void HRSchemaSerializer::parseAliasesHelp(Tokenizer & tokenizer, const QList<Actor*> & procs) {
        ParsedPairs pairs(tokenizer);
        if(!pairs.blockPairs.isEmpty()) {
            throw ReadFailed(tr("No other blocks allowed in help block"));
        }
        
        foreach(const QString & key, pairs.equalPairs.keys()) {
            QString paramName;
            Actor * actor = WorkflowUtils::findActorByParamAlias(procs, key, paramName, false);
            if(actor == NULL) {
                throw ReadFailed(tr("Undefined parameter alias used in help block: '%1'").arg(key));
            }
            QString help = pairs.equalPairs.value(key);
            assert(!help.isEmpty());
            actor->getAliasHelp()[key] = help;
        }
    }
// ----------------------------------------------------

void parseAndCheckPortAlias(const QString &portString, const QMap<QString, Actor*> &actorMap,
                    QString &actorName, QString &portId) {
    actorName = HRSchemaSerializer::parseAt(portString, 0);
    Actor *actor = actorMap.value(actorName);
    if(actor == NULL) {
        throw HRSchemaSerializer::ReadFailed(
            HRSchemaSerializer::tr("%1 element is undefined: at \"%2\" in aliases block").arg(actorName).arg(portString));
    }

    portId = HRSchemaSerializer::parseAt(portString, 1);
    Port *port = actor->getPort(portId);
    if(NULL == port) {
        throw HRSchemaSerializer::ReadFailed(
            HRSchemaSerializer::tr("%1 port is undefined: at \"%2\" in aliases block").arg(portId).arg(portString));
    }
}

void parseSlotAlias(const QString &slotString, const QMap<QString, Actor*> &actorMap,
                    QString &actorName, QString &portId, QString &slotId) {
    parseAndCheckPortAlias(slotString, actorMap, actorName, portId);
    
    slotId = HRSchemaSerializer::parseAt(slotString, 2);
    DataTypePtr dt = actorMap.value(actorName)->getPort(portId)->Port::getType();
    QList<Descriptor> descs = dt->getAllDescriptors();
    if(!descs.contains(slotId)) {
        throw HRSchemaSerializer::ReadFailed(
            HRSchemaSerializer::tr("%1 slot is undefined: at \"%2\" in aliases block'").arg(slotId).arg(slotString));
    }
}

void HRSchemaSerializer::parsePortAliases(Tokenizer & tokenizer, const QMap<QString, Actor*> & actorMap, QList<PortAlias> &portAliases) {
    QList<QString> newPortNames; // keeps all unique ports aliases
    QList<QString> portStrings; // keeps all unique aliased ports

    while(tokenizer.look() != BLOCK_END) {
        QString portString = tokenizer.take();
        if (portStrings.contains(portString)) {
            throw ReadFailed(tr("Duplicate port alias \"%1\"").arg(portString));
        }
        portStrings.append(portString);

        QString sourceActorName;
        QString sourcePortId;
        parseAndCheckPortAlias(portString, actorMap, sourceActorName, sourcePortId);
        tokenizer.assertToken(BLOCK_START);

        ParsedPairs pairs(tokenizer);
        if( !pairs.blockPairs.isEmpty() ) {
            throw ReadFailed(tr("Empty port aliases are not allowed: %1").arg(portString));
        }

        QString alias = pairs.equalPairs.take(ALIAS);
        if (alias.isEmpty()) {
            alias = portString;
            alias.replace(DOT, "_at_");
        }
        if (newPortNames.contains(alias)) {
            throw ReadFailed(tr("Duplicate port alias name \"%1\" at \"%2\"").arg(alias).arg(portString));
        }
        newPortNames.append(alias);

        QString descr = pairs.equalPairs.take(DESCRIPTION);
        if (descr.isEmpty()) {
            descr = alias;
        }

        PortAlias newPortAlias(actorMap[sourceActorName]->getPort(sourcePortId), alias, descr);
        
        foreach(const QString &slotString, pairs.equalPairs.keys()) {
            QString actorName;
            QString portId;
            QString slotId;
            parseSlotAlias(slotString, actorMap, actorName, portId, slotId);
            Port *port = actorMap[actorName]->getPort(portId);

            QString newSlotId = pairs.equalPairs.value(slotString);
            if (!newPortAlias.addSlot(port, slotId, newSlotId)) {
                throw ReadFailed(tr("Duplicate slot alias \"%1\" at port alias\"%2\"").arg(slotString).arg(portString));
            }
        }
        portAliases.append(newPortAlias);
        tokenizer.assertToken(BLOCK_END);
    }
}

QPair<Port*, Port*> HRSchemaSerializer::parseDataflow(Tokenizer & tokenizer, const QString & srcTok, const QMap<QString, Actor*> & actorMap) {
    QString srcActorName = parseAt(srcTok, 0);
    QString srcSlotId = parseAfter(srcTok, 0);
    if(!actorMap.contains(srcActorName)) {
        throw ReadFailed(tr("Undefined element id '%1' at '%2'").arg(srcActorName).arg(srcTok));
    }
    bool slotFound = false;
    Port * srcPort = NULL;
    foreach(Port * port, actorMap.value(srcActorName)->getOutputPorts()) {
        DataTypePtr dt = port->Port::getType();
        QList<Descriptor> descs = dt->getAllDescriptors(); descs << *dt;
        slotFound = slotFound || descs.contains(srcSlotId);
        if(slotFound) {
            srcPort = port;
            break;
        }
    }
    if(!slotFound) {
        throw ReadFailed(tr("Undefined slot id '%1' at '%2'").arg(srcSlotId).arg(srcTok));
    }
    
    tokenizer.assertToken(DATAFLOW_SIGN); // "->"
    QString destTok = tokenizer.take();
    QString destActorName = parseAt(destTok, 0);
    QString destPortId = parseAt(destTok, 1);
    QString destSlotId = parseAfter(destTok, 1);
    if( !actorMap.contains(destActorName) ) {
        throw ReadFailed(tr("Undefined element id '%1' at '%2'").arg(destActorName).arg(destTok));
    }
    
    Port * destPort = actorMap.value(destActorName)->getPort(destPortId);
    if(destPort == NULL) {
        throw ReadFailed(tr("Undefined port id '%1' at '%2'").arg(destPortId).arg(destTok));
    }
    if( !destPort->isInput() ) {
        throw ReadFailed(tr("Destination port should be input: %1").arg(destPortId));
    }
    
    DataTypePtr dt = destPort->Port::getType();
    QList<Descriptor> descs = dt->getAllDescriptors(); descs << *dt;
    if(!descs.contains(destSlotId)) {
        throw ReadFailed(tr("Undefined slot id '%1' at '%2'").arg(destSlotId).arg(destTok));
    }

    IntegralBusPort *bus = qobject_cast<IntegralBusPort*>(destPort);
    IntegralBusSlot slot(srcSlotId, "", actorMap.value(srcActorName)->getId());
    bus->setBusMapValue(destSlotId, slot.toString());

    QString token = tokenizer.look();
    if (BLOCK_START == token) {
        tokenizer.assertToken(BLOCK_START);
        ParsedPairs pairs(tokenizer);
        tokenizer.assertToken(BLOCK_END);

        if (pairs.equalPairs.contains(PATH_THROUGH)) {
            QStringList path;
            QString value = pairs.equalPairs.take(PATH_THROUGH);
            foreach(QString p, value.split(",")) {
                p = p.trimmed();
                if (!actorMap.contains(p)) {
                    throw ReadFailed(tr("Undefined actor id '%1' at '%2'").arg(p).arg(value));
                }
                path.append(p);
            }

            bus->addPathBySlotsPair(destSlotId, slot.toString(), path);
        }
    }
    return QPair<Port*, Port*>(srcPort, destPort);
}

static void parseMeta(WorkflowSchemaReaderData & data) {
    QString tok = data.tokenizer.look();
    if (HRSchemaSerializer::BLOCK_START != tok) {
        data.schema->setTypeName(tok);
        data.tokenizer.take();
    }

    data.tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
    while(data.tokenizer.look() != HRSchemaSerializer::BLOCK_END) {
        QString tok = data.tokenizer.take();
        if(HRSchemaSerializer::PARAM_ALIASES_START == tok) {
            data.tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
            HRSchemaSerializer::parseParameterAliases(data.tokenizer, data.actorMap);
            data.tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else if (HRSchemaSerializer::PORT_ALIASES_START == tok) {
            data.tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
            HRSchemaSerializer::parsePortAliases(data.tokenizer, data.actorMap, data.portAliases);
            data.tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else if(HRSchemaSerializer::VISUAL_START == tok) {
            data.tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
            if (NULL == data.meta) {
                skipBlock(data.tokenizer);
            } else {
                HRVisualParser vp(data);
                U2OpStatus2Log os;
                vp.parse(os);
                if (os.hasError()) {
                    data.meta->resetVisual();
                }
                data.tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
            }
        } else if (HRSchemaSerializer::OLD_ALIASES_START == tok) {
            data.tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
            HRSchemaSerializer::parseOldAliases(data.tokenizer, data.actorMap);
            data.tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else if(HRSchemaSerializer::ALIASES_HELP_START == tok) {
            data.tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
            HRSchemaSerializer::parseAliasesHelp(data.tokenizer, data.actorMap.values());
            data.tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else if (HRWizardParser::WIZARD == tok) {
            data.tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
            HRWizardParser ws(data.tokenizer, data.actorMap);
            U2OpStatusImpl os;
            Wizard *w = ws.parseWizard(os);
            CHECK_OP_EXT(os, throw HRSchemaSerializer::ReadFailed(os.getError()), );
            data.wizards << w;
            data.tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else {
            throw HRSchemaSerializer::ReadFailed(HRSchemaSerializer::UNDEFINED_META_BLOCK.arg(tok));
        }
    }
}

static void parseBody(WorkflowSchemaReaderData & data) {
    HRSchemaSerializer::Tokenizer & tokenizer = data.tokenizer;
    while(tokenizer.notEmpty() && tokenizer.look() != HRSchemaSerializer::BLOCK_END) {
        QString tok = tokenizer.take();
        QString next = tokenizer.look();
        if(tok == HRSchemaSerializer::META_START) {
            parseMeta(data);
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else if(tok == HRSchemaSerializer::DOT_ITERATION_START) {
            QString itName = tokenizer.look() == HRSchemaSerializer::BLOCK_START ? "" : tokenizer.take();
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
            data.schema->addIteration(HRSchemaSerializer::parseIteration(tokenizer, itName, data.actorMap));
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else if (tok == HRSchemaSerializer::ACTOR_BINDINGS) {
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
            HRSchemaSerializer::parseActorBindings(tokenizer, data);
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else if (tok == HRSchemaSerializer::FUNCTION_START) {
            HRSchemaSerializer::parseFunctionDefinition(tokenizer, data.actorMap);
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else if(next == HRSchemaSerializer::DATAFLOW_SIGN) {
            data.dataflowLinks << HRSchemaSerializer::parseDataflow(tokenizer, tok, data.actorMap);
        } else if(next == HRSchemaSerializer::BLOCK_START) {
            tokenizer.take();
            Actor * proc = HRSchemaSerializer::parseElementsDefinition(tokenizer, tok, data.actorMap, data.idMap);
            data.schema->addProcess(proc);
            proc->updateDelegateTags();
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else {
            throw HRSchemaSerializer::ReadFailed(HRSchemaSerializer::UNDEFINED_CONSTRUCT.arg(tok).arg(next));
        }
    }

    foreach (Actor *proc, data.actorMap.values()) {
        ActorPrototype *proto = proc->getProto();
        if (NULL != proto->getEditor()) {
            ActorConfigurationEditor *actorEd = dynamic_cast<ActorConfigurationEditor*>(proto->getEditor());
            if (NULL != actorEd) {
                ActorConfigurationEditor *editor = dynamic_cast<ActorConfigurationEditor*>(proto->getEditor()->clone());
                editor->setConfiguration(proc);
                proc->setEditor(editor);
            }
        }
    }
}

static void setFlows(WorkflowSchemaReaderData & data) {
    if (data.isGraphDefined()) {
        return;
    }
    if (!data.links.isEmpty()) {
        QList<QPair<Port*, Port*> >::iterator i = data.links.begin();
        for (; i!=data.links.end(); i++) {
            tryToConnect(data.schema, i->first, i->second);
        }
    } else {
        HRSchemaSerializer::FlowGraph graph(data.dataflowLinks);
        graph.minimize();
        foreach(Port * input, graph.graph.keys()) {
            foreach(Port * output, graph.graph.value(input)) {
                tryToConnect(data.schema, input, output);
            }
        }
    }
}

void HRSchemaSerializer::addEmptyValsToBindings(const QList<Actor*> & procs) {
    foreach(Actor * actor, procs) {
        foreach(Port * p, actor->getInputPorts()) {
            IntegralBusPort * port = qobject_cast<IntegralBusPort*>(p);
            QStrStrMap busMap = port->getParameter(IntegralBusPort::BUS_MAP_ATTR_ID)->getAttributeValueWithoutScript<QStrStrMap>();
            DataTypePtr t = port->Port::getType();
            assert(t->isMap());
            QMap<Descriptor, DataTypePtr> typeMap = t->getDatatypesMap();
            foreach(const Descriptor & d, typeMap.keys()) {
                if(!busMap.contains(d.getId())) {
                    port->setBusMapValue(d.getId(), "");
                }
            }
        }
    }
}

QString HRSchemaSerializer::string2Schema(const QString & bytes, Schema * schema, Metadata * meta, QMap<ActorId, ActorId>* idMap, QList<QString> includedUrls) {
    try {
        WorkflowSchemaReaderData data(bytes, schema, meta, idMap);
        parseHeader(data.tokenizer, data.meta);
        data.tokenizer.removeCommentTokens();

        QString tok = data.tokenizer.look();
        while (HRSchemaSerializer::INCLUDE == tok) {
            parseIncludes(data.tokenizer, includedUrls);
            tok = data.tokenizer.look();
        }
        parseBodyHeader(data.tokenizer, data.meta);

        if( schema != NULL ) {
            data.tokenizer.assertToken(BLOCK_START);
            parseBody(data);
            data.tokenizer.assertToken(BLOCK_END);
            setFlows(data);
            addEmptyValsToBindings(data.actorMap.values());
            data.schema->setPortAliases(data.portAliases);
            data.schema->setWizards(data.wizards);
        }
    } catch( const HRSchemaSerializer::ReadFailed & ex ) {
        return ex.what;
    } catch (...) {
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

void HRSchemaSerializer::parsePorts(Tokenizer & tokenizer, QList<DataConfig>& ports) {
    while(tokenizer.look() != BLOCK_END) {
        DataConfig cfg;
        cfg.attrName = tokenizer.take();
        tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
        ParsedPairs pairs(tokenizer);
        cfg.type = pairs.equalPairs.take(TYPE_PORT);
        cfg.format = pairs.equalPairs.take(FORMAT_PORT);
        cfg.description = pairs.equalPairs.take(DESCRIPTION);
        tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        ports << cfg;
    }
}

void HRSchemaSerializer::parseAttributes(Tokenizer & tokenizer, QList<AttributeConfig>& attrs) {
    while(tokenizer.look() != BLOCK_END) {
        AttributeConfig cfg;
        cfg.attrName = tokenizer.take();
        tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
        ParsedPairs pairs(tokenizer);
        cfg.type = pairs.equalPairs.take(TYPE_PORT);
        cfg.description = pairs.equalPairs.take(DESCRIPTION);
        tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        attrs << cfg;
    }
}

ExternalProcessConfig*  HRSchemaSerializer::parseActorBody(Tokenizer & tokenizer) {
    ExternalProcessConfig *cfg = new ExternalProcessConfig();
    cfg->name = tokenizer.take();
    while(tokenizer.notEmpty() && tokenizer.look() != HRSchemaSerializer::BLOCK_END) {
        QString tok = tokenizer.take();
        QString next = tokenizer.look();
        if(tok == HRSchemaSerializer::INPUT_START) {
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
            HRSchemaSerializer::parsePorts(tokenizer, cfg->inputs);
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else if(tok == HRSchemaSerializer::OUTPUT_START) {
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
            HRSchemaSerializer::parsePorts(tokenizer, cfg->outputs);
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else if(tok == HRSchemaSerializer::ATTRIBUTES_START) {
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
            HRSchemaSerializer::parseAttributes(tokenizer, cfg->attrs);
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else if(tok == HRSchemaSerializer::BLOCK_START) {
            //tokenizer.take();
            /*Actor * proc = HRSchemaSerializer::parseElementsDefinition(tokenizer, tok, data.actorMap, data.idMap);
            data.schema->addProcess(proc);
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);*/
        } else if(tok == HRSchemaSerializer::CMDLINE) {
            tokenizer.assertToken(COLON);
            cfg->cmdLine = tokenizer.take();
        } else if(tok == HRSchemaSerializer::DESCRIPTION)  {
            tokenizer.assertToken(COLON);
            cfg->description = tokenizer.take();
        }else if(tok == HRSchemaSerializer::PROMPTER) {
            tokenizer.assertToken(COLON);
            cfg->templateDescription = tokenizer.take();
        } else {
            throw HRSchemaSerializer::ReadFailed(HRSchemaSerializer::UNDEFINED_CONSTRUCT.arg(tok).arg(next));
        }
    }
    return cfg;
}

ExternalProcessConfig* HRSchemaSerializer::string2Actor(const QString & bytes) {
    ExternalProcessConfig *cfg = NULL;
    try {
        WorkflowSchemaReaderData data(bytes, NULL, NULL, NULL);
        parseHeader(data.tokenizer, data.meta);
        cfg = parseActorBody(data.tokenizer);
    } catch (...) {
        return NULL;
    }
    return cfg;
}


void HRSchemaSerializer::addPart( QString & to, const QString & w) {
    QString what = w;
    if( !what.endsWith(NEW_LINE) ) {
        what.append(NEW_LINE);
    }
    to += what + NEW_LINE;
}

QString HRSchemaSerializer::header2String(const Metadata * meta) {
    QString res = HRSchemaSerializer::HEADER_LINE + "\n";
    if(meta != NULL) {
        QStringList descLines = meta->comment.split(HRSchemaSerializer::NEW_LINE, QString::KeepEmptyParts);
        for (int lineIdx=0; lineIdx<descLines.size(); lineIdx++) {
            const QString &line = descLines.at(lineIdx);
            bool lastLine = (lineIdx == descLines.size()-1);
            if (lastLine && line.isEmpty()) {
                continue;
            }
            res += HRSchemaSerializer::SERVICE_SYM + line + HRSchemaSerializer::NEW_LINE;
        }
    }
    return res;
}

QString HRSchemaSerializer::makeBlock(const QString & title, const QString & name, 
                                      const QString & blockItself, int tabsNum, bool nl, bool sc) {
    QString indent = makeIndent(tabsNum);
    QString blockStart = BLOCK_START + NEW_LINE;
    if(nl) {
        blockStart += NEW_LINE;
    }
    QString blockEnd = BLOCK_END;
    if(sc) {
        blockEnd += SEMICOLON;
    }
    blockEnd += NEW_LINE;
    return indent + title + " " + valueString(name) + blockStart + blockItself + indent + blockEnd;
}

QString HRSchemaSerializer::makeEqualsPair(const QString & key, const QString & value, int tabsNum) {
    return makeIndent(tabsNum) + key + EQUALS_SIGN + valueString(value) + SEMICOLON + NEW_LINE;
}

QString HRSchemaSerializer::makeArrowPair( const QString & left, const QString & right, int tabsNum ) {
    return makeIndent(tabsNum) + left + DATAFLOW_SIGN + right;
}

QString HRSchemaSerializer::scriptBlock(const QString & scriptText, int tabsNum) {
    QString indent = makeIndent(tabsNum);
    QString res;
    QStringList scriptLines = scriptText.split(NEW_LINE, QString::SkipEmptyParts);
    foreach(const QString & line, scriptLines) {
        res += indent + line + NEW_LINE;
    }
    return res;
}

QString HRSchemaSerializer::grouperOutSlotsDefinition(Attribute *attribute) {
    GrouperOutSlotAttribute *a = dynamic_cast<GrouperOutSlotAttribute*>(attribute);
    QString result;

    foreach (const GrouperOutSlot &slot, a->getOutSlots()) {
        QString mRes;
        mRes += makeEqualsPair(NAME_ATTR, slot.getOutSlotId(), 3);
        mRes += makeEqualsPair(IN_SLOT, slot.getInSlotStr(), 3);

        GrouperSlotAction *const action = slot.getAction();
        if (NULL != action) {
            QString actionBlock;
            actionBlock += makeEqualsPair(TYPE_ATTR, action->getType(), 4);
            foreach (const QString &paramId, action->getParameters().keys()) {
                QVariant value = action->getParameterValue(paramId);
                actionBlock += makeEqualsPair(paramId, value.toString(), 4);
            }
            mRes += makeBlock(ACTION, NO_NAME, actionBlock, 3);
        }

        result += makeBlock(OUT_SLOT_ATTR, NO_NAME, mRes, 2);
    }

    return result;
}

class HRUrlSerializer : public URLContainerVisitor {
public:
    HRUrlSerializer(int _tabCount) : tabCount(_tabCount) {}

    virtual void visit(FileUrlContainer *url) {
        result = HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::FILE_URL, url->getUrl(), tabCount);
    }

    virtual void visit(DirUrlContainer *url) {
        if (url->getIncludeFilter().isEmpty() && url->getExcludeFilter().isEmpty() && !url->isRecursive()) {
            result = HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::DIRECTORY_URL, url->getUrl(), tabCount);
            return;
        }

        QString res;
        res += HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::PATH, url->getUrl(), tabCount+1);
        if (!url->getIncludeFilter().isEmpty()) {
            res += HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::INC_FILTER, url->getIncludeFilter(), tabCount+1);
        }
        if (!url->getExcludeFilter().isEmpty()) {
            res += HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::EXC_FILTER, url->getExcludeFilter(), tabCount+1);
        }
        if (url->isRecursive()) {
            QString recStr("false");
            if (url->isRecursive()) {
                recStr = "true";
            }
            res += HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::RECURSIVE, recStr, tabCount+1);
        }
        result = HRSchemaSerializer::makeBlock(HRSchemaSerializer::DIRECTORY_URL, HRSchemaSerializer::NO_NAME, res, tabCount);
    }

    const QString & getResult() {
        return result;
    }

private:
    int tabCount;
    QString result;
};

static QString inUrlDefinitionBlocks(const QString &attrId, const QList<Dataset> &sets, int depth) {
    QString res;
    foreach (const Dataset &dSet, sets) {
        QString setDef;
        setDef += HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::DATASET_NAME, dSet.getName(), depth + 1);
        foreach (URLContainer *url, dSet.getUrls()) {
            HRUrlSerializer us(depth + 1);
            url->accept(&us);
            setDef += us.getResult();
        }
        res += HRSchemaSerializer::makeBlock(attrId,
            HRSchemaSerializer::NO_NAME, setDef, depth);
    }
    return res;
}

static QString elementsDefinitionBlock(Actor * actor, bool copyMode) {
    assert(actor != NULL);
    QString res;
    // save global attributes
    res += HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::TYPE_ATTR, actor->getProto()->getId());
    res += HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::NAME_ATTR, actor->getLabel());
    if(copyMode) {
        res += HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::ELEM_ID_ATTR, actor->getId());
    }
    AttributeScript * actorScript = actor->getScript();
    if(actorScript != NULL && !actorScript->getScriptText().trimmed().isEmpty()) {
        res += HRSchemaSerializer::makeBlock(HRSchemaSerializer::SCRIPT_ATTR, HRSchemaSerializer::NO_NAME, 
            actorScript->getScriptText() + HRSchemaSerializer::NEW_LINE, 2, false, true);
    }

    // save local attributes
    foreach(Attribute * attribute, actor->getParameters().values()) {
        assert(attribute != NULL);
        if (attribute->getGroup() == GROUPER_SLOT_GROUP) {
            res += HRSchemaSerializer::grouperOutSlotsDefinition(attribute);
        } else {
            if (attribute->getId() == BaseAttributes::URL_IN_ATTRIBUTE().getId()) {
                QVariant v = attribute->getAttributePureValue();
                if (v.canConvert< QList<Dataset> >()) {
                    QList<Dataset> sets = v.value< QList<Dataset> >();
                    res += inUrlDefinitionBlocks(BaseAttributes::URL_IN_ATTRIBUTE().getId(), sets, 2);
                    continue;
                }
            }
            QString attributeId = attribute->getId();
            assert(!attributeId.contains(QRegExp("\\s")));
            
            const AttributeScript & attrScript = attribute->getAttributeScript();
            if(!attrScript.isEmpty()) {
                res += HRSchemaSerializer::makeBlock(attributeId, HRSchemaSerializer::NO_NAME, 
                                                     attrScript.getScriptText() + HRSchemaSerializer::NEW_LINE, 2, false, true);
                continue;
            }
            
            if (attribute->isDefaultValue()) {
                continue;
            }
            QVariant value = attribute->getAttributePureValue();
            assert(value.isNull() || value.canConvert<QString>());
            QString valueStr = value.toString();
            if(!valueStr.isEmpty()) {
                res += HRSchemaSerializer::makeEqualsPair(attributeId, valueStr);
            }
        }
    }

    return res;
}

static QString tryGetRelativePath(const QString &path) {
    QString dir;

    if (path.startsWith(WorkflowSettings::getExternalToolDirectory())) {
        dir = WorkflowSettings::getExternalToolDirectory();
    } else if (path.startsWith(WorkflowSettings::getUserDirectory())) {
        dir = WorkflowSettings::getUserDirectory();
    } else if (path.startsWith(WorkflowSettings::getIncludedElementsDirectory())) {
        dir = WorkflowSettings::getIncludedElementsDirectory();
    }

    if (dir.isEmpty()) {
        return path;
    } else {
        return path.mid(dir.length());
    }
}

QString HRSchemaSerializer::includesDefinition(const QList<Actor*> & procs) {
    QString res;
    foreach (Actor *proc, procs) {
        ActorPrototype *proto = proc->getProto();
        if (!proto->isStandardFlagSet()) {
            res += HRSchemaSerializer::INCLUDE + " \"" + tryGetRelativePath(proto->getFilePath()) + "\" ";
            res += HRSchemaSerializer::INCLUDE_AS + " \"" + proto->getId() + "\"" + NEW_LINE;
        }
    }

    return res;
}

QString HRSchemaSerializer::elementsDefinition(const QList<Actor*> & procs, const NamesMap & nmap, bool copyMode) {
    QString res;
    foreach( Actor * actor, procs) {
        QString idStr = nmap[actor->getId()];
        SAFE_POINT(!idStr.contains(QRegExp("\\s")), tr("Error: element name in the scheme file contains spaces"), QString());
        res += makeBlock(idStr, NO_NAME, elementsDefinitionBlock(actor, copyMode));
    }
    return res + NEW_LINE;
}

static QString markerDefinitionBlock(Marker *marker, bool ) {
    assert(marker != NULL);
    QString res;
    res += HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::TYPE_ATTR, HRSchemaSerializer::MARKER);
    res += HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::MARKER_TYPE, marker->getType());
    res += HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::MARKER_NAME, marker->getName());

    if (QUALIFIER == marker->getGroup()) {
        const QString &qualName = dynamic_cast<QualifierMarker*>(marker)->getQualifierName();
        if (!qualName.isEmpty()) {
            res += HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::QUAL_NAME, qualName);
        }
    } else if (ANNOTATION == marker->getGroup()) {
        const QString &annName = dynamic_cast<AnnotationMarker*>(marker)->getAnnotationName();
        if (!annName.isEmpty()) {
            res += HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::ANN_NAME, annName);
        }
    }

    foreach(QString key, marker->getValues().keys()) {
        QString val = marker->getValues().value(key);
        res += HRSchemaSerializer::makeEqualsPair("\"" + key + "\"", val);
    }
    return res;
}

QString HRSchemaSerializer::markersDefinition(const QList<Actor*> & procs, const NamesMap & nmap, bool copyMode) {
    QString res;
    foreach( Actor * actor, procs) {
        foreach (Attribute *attr, actor->getAttributes()) {
            if (MARKER_GROUP == attr->getGroup()) {
                MarkerAttribute *mAttr = dynamic_cast<MarkerAttribute*>(attr);
                foreach (QString markerId, mAttr->getMarkers().keys()) {
                    QString blockName = FUNCTION_START + nmap[actor->getId()] + DOT;
                    blockName += markerId;
                    res += makeBlock(blockName, NO_NAME, markerDefinitionBlock(mAttr->getMarkers().value(markerId), copyMode));
                }
            }
        }
    }
    return res + NEW_LINE;
}

static QString actorBindingsBlock(const ActorBindingsGraph & graph, const HRSchemaSerializer::NamesMap &nmap, bool ) {
    QString res;
    
    foreach (Port *srcPort, graph.getBindings().keys()) {
        QString srcActorId = nmap[srcPort->owner()->getId()];
        QString srcPortId = srcPort->getId();
        foreach (Port *dstPort, graph.getBindings().value(srcPort)) {
            QString dstActorId = nmap[dstPort->owner()->getId()];
            QString dstPortId = dstPort->getId();

            res += HRSchemaSerializer::makeArrowPair(srcActorId+HRSchemaSerializer::DOT+srcPortId,
                dstActorId+HRSchemaSerializer::DOT+dstPortId, 2) + HRSchemaSerializer::NEW_LINE;
        }
    }
    return res;
}

QString HRSchemaSerializer::actorBindings(const ActorBindingsGraph & graph, const NamesMap &nmap, bool copyMode) {
    QString res;
    res += makeBlock(HRSchemaSerializer::ACTOR_BINDINGS, NO_NAME, actorBindingsBlock(graph, nmap, copyMode));
    return res + NEW_LINE;
}

static bool containsProcWithId(const QList<Actor*> & procs, const ActorId & id) {
    foreach(Actor * a, procs) {
        if(a->getId() == id) {
            return true;
        }
    }
    return false;
}

QString HRSchemaSerializer::dataflowDefinition(const QList<Actor*> & procs, const NamesMap & nmap) {
    QString res;
    foreach(Actor * actor, procs) {
        foreach(Port * inputPort, actor->getInputPorts()) {
            QStrStrMap busMap = inputPort->getParameter(IntegralBusPort::BUS_MAP_ATTR_ID)->getAttributeValueWithoutScript<QStrStrMap>();
            IntegralBusPort *busPort = qobject_cast<IntegralBusPort*>(inputPort);

            foreach( const QString & key, busMap.keys() ) {
                QStringList srcList = busMap.value(key).split(";", QString::SkipEmptyParts);
                QStringList uniqList;
                foreach(QString src, srcList) {
                    if (!uniqList.contains(src)) {
                        uniqList << src;
                    }
                }

                foreach(QString src, uniqList) {
                    if(src.isEmpty()) { continue; }
                    QList<QStringList> paths = busPort->getPathsBySlotsPair(key, src);
                    src = src.replace(COLON, DOT);
                    ActorId srcActorId = parseAt(src, 0);

                    if(containsProcWithId(procs, srcActorId)) {
                        QString arrowPair = makeArrowPair(src.replace(srcActorId, nmap[srcActorId]),
                            nmap[actor->getId()] + DOT + inputPort->getId() + DOT + key, 0);

                        if (paths.isEmpty()) {
                            res += makeIndent(1) + arrowPair + NEW_LINE;
                        } else {
                            foreach (const QStringList &path, paths) {
                                QString pathString = path.join(", ");
                                QString pair = makeEqualsPair(PATH_THROUGH, pathString, 2);
                                res += makeBlock(arrowPair, NO_NAME, pair);
                            }
                        }
                    }
                }
            }
        }
    }
    return res + NEW_LINE;
}

static QString elementsIterationData(const QVariantMap & data) {
    QString res;
    foreach( const QString & attributeId, data.uniqueKeys() ) {
        assert(!attributeId.contains(QRegExp("\\s")));
        QVariant value = data.value(attributeId);
        assert(!value.isNull());
        if (value.canConvert<QString>()) {
            QString valueStr = value.toString();
            if(!valueStr.isEmpty()) {
                res += HRSchemaSerializer::makeEqualsPair(attributeId, valueStr, 3);
            }
        } else if (value.canConvert< QList<Dataset> >()) {
            res += inUrlDefinitionBlocks(attributeId, value.value< QList<Dataset> >(), 3);
        }

    }
    return res;
}

static QString iterationData(const Iteration & iteration, const HRSchemaSerializer::NamesMap& nmap) {
    QString res = HRSchemaSerializer::makeEqualsPair(HRSchemaSerializer::ITERATION_ID, 
                                                     QString::number(iteration.id)) + HRSchemaSerializer::NEW_LINE;
    foreach(const ActorId & aid, iteration.cfg.uniqueKeys() ) {
        QString blockName = nmap[aid];
        if(blockName.isEmpty()) {
            blockName = aid;
        }
        if (!iteration.cfg.value(aid).isEmpty()) {
            res += HRSchemaSerializer::makeBlock(blockName, HRSchemaSerializer::NO_ERROR, elementsIterationData(iteration.cfg.value(aid)), 2);
        }
    }
    return res;
}

QString HRSchemaSerializer::iterationsDefinition(const QList<Iteration> & iterations, const NamesMap& nmap, bool checkDummyIteration) {
    if(checkDummyIteration) {
        if( iterations.size() == 1 && iterations.at(0).isEmpty()) {
            return QString();
        }
    }
    
    QString res;
    foreach( const Iteration & iteration, iterations ) {
        res += HRSchemaSerializer::makeBlock(HRSchemaSerializer::DOT_ITERATION_START, iteration.name, iterationData(iteration, nmap));
    }
    return res;
}

static QString visualData(const Schema & schema, const HRSchemaSerializer::NamesMap& nmap) {
    QString res;
    foreach(Link* link, schema.getFlows()) {
        Port * src = link->source();
        Port * dst = link->destination();
        res += HRSchemaSerializer::makeArrowPair(nmap[src->owner()->getId()] + HRSchemaSerializer::DOT + src->getId(),
                    nmap[dst->owner()->getId()] + HRSchemaSerializer::DOT + dst->getId(), 0) + HRSchemaSerializer::NEW_LINE;
    }
    return res;
}

static QString itemsMetaData(const QList<Actor*> &actors, const Metadata *meta, const HRSchemaSerializer::NamesMap &nmap) {
    QString res;
    bool hasParameterAliases = false;
    foreach (Actor *a, actors) {
        if (a->hasParamAliases()) {
            hasParameterAliases = true;
            break;
        }
    }
    if(hasParameterAliases) {
        res += HRSchemaSerializer::makeBlock(HRSchemaSerializer::PARAM_ALIASES_START, HRSchemaSerializer::NO_NAME,
            HRSchemaSerializer::schemaParameterAliases(actors, nmap), 2);
    }

    if (NULL != meta) {
        HRVisualSerializer vs(*meta, nmap);
        res += vs.serialize(2);
    }
    return res;
}

static QString metaData(const Schema & schema, const Metadata * meta, const HRSchemaSerializer::NamesMap& nmap) {
    QString res;

    res += itemsMetaData(schema.getProcesses(), meta, nmap);

    if(schema.hasPortAliases()) {
        res += HRSchemaSerializer::makeBlock(HRSchemaSerializer::PORT_ALIASES_START, HRSchemaSerializer::NO_NAME, 
            HRSchemaSerializer::schemaPortAliases(nmap, schema.getPortAliases()), 2);
    }

    if (NULL == meta) {
        res += HRSchemaSerializer::makeBlock(HRSchemaSerializer::VISUAL_START, HRSchemaSerializer::NO_NAME, visualData(schema, nmap), 2);
    }

    foreach (Wizard *w, schema.getWizards()) {
        HRWizardSerializer ws;
        res += ws.serialize(w, 2);
    }
    return res;
}

//QString HRSchemaSerializer::schemaParameterAliases(const QList<Actor*> & procs, const NamesMap& nmap) {
//    QString res;
//    foreach(Actor * actor, procs) {
//        const QMap<QString, QString> & aliases = actor->getParamAliases();
//        foreach(const QString & attrId, aliases.uniqueKeys()) {
//            res += HRSchemaSerializer::makeEqualsPair(nmap[actor->getId()] + HRSchemaSerializer::DOT + attrId, aliases.value(attrId), 3);
//        }
//    }
//    return res;
//}

QString HRSchemaSerializer::schemaParameterAliases(const QList<Actor*> & procs, const NamesMap& nmap) {
    QString res;
    foreach(Actor * actor, procs) {
        const QMap<QString, QString> & aliases = actor->getParamAliases();
        foreach(const QString & attrId, aliases.uniqueKeys()) {
            QString pairs;
            QString alias = aliases.value(attrId);
            QString descr = actor->getAliasHelp()[alias];
            pairs += HRSchemaSerializer::makeEqualsPair(ALIAS, alias, 4);
            if (!descr.isEmpty()) {
                pairs += HRSchemaSerializer::makeEqualsPair(DESCRIPTION, descr, 4);
            }
            QString paramString = nmap[actor->getId()] + HRSchemaSerializer::DOT + attrId;
            res += makeBlock(paramString, NO_NAME, pairs, 3);
        }
    }
    return res;
}

QString HRSchemaSerializer::schemaPortAliases(const NamesMap &nmap, const QList<PortAlias> &portAliases) {
    QString res;

    foreach (const PortAlias &portAlias, portAliases) {
        QString pairs;
        pairs += makeEqualsPair(ALIAS, portAlias.getAlias(), 4);
        if (!portAlias.getDescription().isEmpty()) {
            pairs += makeEqualsPair(DESCRIPTION, portAlias.getDescription(), 4);
        }
        foreach (const SlotAlias &slotAlias, portAlias.getSlotAliases()) {
            QString actorName = nmap[slotAlias.getSourcePort()->owner()->getId()];
            QString portId = slotAlias.getSourcePort()->getId();
            QString slotString = actorName + DOT + portId + DOT + slotAlias.getSourceSlotId();
            pairs += makeEqualsPair(slotString, slotAlias.getAlias(), 4);
        }

        const Port *sourcePort = portAlias.getSourcePort();
        QString sourceActorName = nmap[sourcePort->owner()->getId()];
        QString sourcePortId = sourcePort->getId();
        QString portString = sourceActorName + DOT + sourcePortId;
        res += makeBlock(portString, NO_NAME, pairs, 3);
    }

    return res;
}

HRSchemaSerializer::NamesMap HRSchemaSerializer::generateElementNames(const QList<Actor*>& procs) {
    QMap<ActorId, QString> nmap;
    foreach(Actor * proc, procs) {
        QString id = aid2str(proc->getId());
        QString name = id.replace(QRegExp("\\s"), "-");
        nmap[proc->getId()] = name;//generateElementName(proc, nmap.values());
    }
    return nmap;
}

static QString bodyItself(const Schema & schema, const Metadata * meta, bool copyMode) {
    HRSchemaSerializer::NamesMap nmap = HRSchemaSerializer::generateElementNames(schema.getProcesses());
    QString res;
    res += HRSchemaSerializer::elementsDefinition(schema.getProcesses(), nmap, copyMode);
    res += HRSchemaSerializer::markersDefinition(schema.getProcesses(), nmap, copyMode);
    res += HRSchemaSerializer::actorBindings(schema.getActorBindingsGraph(), nmap, copyMode);
    res += HRSchemaSerializer::dataflowDefinition(schema.getProcesses(), nmap);
    res += HRSchemaSerializer::iterationsDefinition(schema.getIterations(), nmap, true);
    res += HRSchemaSerializer::makeBlock(HRSchemaSerializer::META_START, schema.getTypeName(), metaData(schema, meta, nmap));
    return res;
}

QString HRSchemaSerializer::schema2String(const Schema & schema, const Metadata * meta, bool copyMode) {
    QString res;
    addPart(res, header2String(meta));
    addPart(res, includesDefinition(schema.getProcesses()));
    addPart(res, makeBlock(BODY_START, meta ? meta->name : "", bodyItself(schema, meta, copyMode), 0, true));
    return res;
}

QString HRSchemaSerializer::items2String(const QList<Actor*> &actors, const QList<Iteration> &iterations, const Metadata *meta) {
    assert(!actors.isEmpty());
    QString res;
    HRSchemaSerializer::addPart(res, HRSchemaSerializer::header2String(meta));

    QString iData;
    HRSchemaSerializer::NamesMap nmap = HRSchemaSerializer::generateElementNames(actors);
    iData += HRSchemaSerializer::elementsDefinition(actors, nmap);
    iData += HRSchemaSerializer::markersDefinition(actors, nmap);
    iData += HRSchemaSerializer::dataflowDefinition(actors, nmap);
    iData += HRSchemaSerializer::iterationsDefinition(iterations, nmap, false);
    iData += HRSchemaSerializer::makeBlock(HRSchemaSerializer::META_START, HRSchemaSerializer::NO_NAME, itemsMetaData(actors, meta, nmap));

    HRSchemaSerializer::addPart(res, HRSchemaSerializer::makeBlock(HRSchemaSerializer::BODY_START, HRSchemaSerializer::NO_NAME,
        iData, 0, true));
    return res;
}

QMap<ActorId, ActorId> HRSchemaSerializer::deepCopy(const Schema& from, Schema* to, U2OpStatus &os) {
    assert(to != NULL);
    QString data = schema2String(from, NULL, true);
    QMap<ActorId, ActorId> idMap;
    QString err = string2Schema(data, to, NULL, &idMap);
    if(!err.isEmpty()) {
        os.setError(err);
        coreLog.details(err);
        to->reset();
        return QMap<ActorId, ActorId>();
    }
    to->setDeepCopyFlag(true);
    return idMap;
}

static QString inputsDefenition(const QList<DataConfig> &inputs) {
    QString res = HRSchemaSerializer::TAB + HRSchemaSerializer::INPUT_START + " {\n";
    foreach(const DataConfig& cfg, inputs) {
        res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + cfg.attrName + " {\n";
        res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + "type:" + cfg.type + ";\n";
        res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + "format:" + cfg.format + ";\n";
        if(!cfg.description.isEmpty()) {
            res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + "description:\"" + cfg.description + "\";\n";
        }
        res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + "}\n";
    }
    res += HRSchemaSerializer::TAB + "}\n";
    return res;
}

static QString outputsDefenition(const QList<DataConfig> &inputs) {
    QString res = HRSchemaSerializer::TAB + HRSchemaSerializer::OUTPUT_START + " {\n";
    foreach(const DataConfig& cfg, inputs) {
        res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + cfg.attrName + " {\n";
        res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + "type:" + cfg.type + ";\n";
        res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + "format:" + cfg.format + ";\n";
        if(!cfg.description.isEmpty()) {
            res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + "description:\"" + cfg.description + "\";\n";
        }
        res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + "}\n";
    }
    res += HRSchemaSerializer::TAB + "}\n";
    return res;
}

static QString attributesDefinition(const QList<AttributeConfig> &attrs) {
    QString res = HRSchemaSerializer::TAB + HRSchemaSerializer::ATTRIBUTES_START + " {\n";
    foreach(const AttributeConfig &cfg, attrs) {
        res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + cfg.attrName + " {\n";
        res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + "type:" + cfg.type + ";\n";
        if(!cfg.description.isEmpty()) {
            res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + "description:\"" + cfg.description + "\";\n";
        }
        res += HRSchemaSerializer::TAB + HRSchemaSerializer::TAB + "}\n";
    }
    res += HRSchemaSerializer::TAB + "}\n";
    return res;
}

QString HRSchemaSerializer::actor2String(ExternalProcessConfig *cfg ) {
    QString res = HRSchemaSerializer::HEADER_LINE + "\n";
    res += "\"" + cfg->name + "\" {\n";
    res += inputsDefenition(cfg->inputs);
    res += outputsDefenition(cfg->outputs);
    res += attributesDefinition(cfg->attrs);
    res += HRSchemaSerializer::TAB + HRSchemaSerializer::CMDLINE + ":\"" + cfg->cmdLine + "\";\n";
    if(!cfg->description.isEmpty()) {
        res += HRSchemaSerializer::TAB + HRSchemaSerializer::DESCRIPTION + ":\"" + cfg->description + "\";\n";
    }
    if(!cfg->templateDescription.isEmpty()) {
        res += HRSchemaSerializer::TAB + HRSchemaSerializer::PROMPTER + ":\"" + cfg->templateDescription + "\";\n";
    }
    res += "}";
    return res;
}

Actor* HRSchemaSerializer::deprecatedActorsReplacer(const QString &id, const QString &protoId, ParsedPairs &pairs ){
    Actor *a = NULL;
    ActorPrototype *apt = NULL;
    if(protoId == CoreLibConstants::WRITE_CLUSTAL_PROTO_ID){
        apt = WorkflowEnv::getProtoRegistry()->getProto(SchemaSerializer::getElemType(CoreLibConstants::WRITE_MSA_PROTO_ID));
        a = apt->createInstance(id);
        a->setParameter(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId(), BaseDocumentFormats::CLUSTAL_ALN);
        pairs.blockPairs.remove("accumulate");
        pairs.equalPairs.remove("accumulate");
    }
    if(protoId == CoreLibConstants::WRITE_STOCKHOLM_PROTO_ID){
        apt = WorkflowEnv::getProtoRegistry()->getProto(SchemaSerializer::getElemType(CoreLibConstants::WRITE_MSA_PROTO_ID));
        a = apt->createInstance(id);
        a->setParameter(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId(), BaseDocumentFormats::STOCKHOLM);
        pairs.blockPairs.remove("accumulate");
        pairs.equalPairs.remove("accumulate");
    }
    if(protoId == CoreLibConstants::WRITE_FASTQ_PROTO_ID){
        apt = WorkflowEnv::getProtoRegistry()->getProto(SchemaSerializer::getElemType(CoreLibConstants::WRITE_SEQ_PROTO_ID));
        a = apt->createInstance(id);
        a->setParameter(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId(), BaseDocumentFormats::FASTQ);
    }
    if(protoId == CoreLibConstants::WRITE_GENBANK_PROTO_ID){
        apt = WorkflowEnv::getProtoRegistry()->getProto(SchemaSerializer::getElemType(CoreLibConstants::WRITE_SEQ_PROTO_ID));
        a = apt->createInstance(id);
        a->setParameter(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId(), BaseDocumentFormats::PLAIN_GENBANK);
    }
    return a;
}


} // U2
