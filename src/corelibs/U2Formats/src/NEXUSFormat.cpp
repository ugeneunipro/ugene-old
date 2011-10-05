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

#include "NEXUSFormat.h"
#include "NEXUSParser.h"

#include <U2Core/U2OpStatus.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/PhyTreeObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2DbiUtils.h>

#include <U2Formats/DocumentFormatUtils.h>

namespace U2 
{

NEXUSFormat::NEXUSFormat(QObject *p) :
        DocumentFormat(p, DocumentFormatFlag_SupportWriting, QStringList()<<"nex"<<"nxs")      // disable streaming for now
{
    formatName = tr("NEXUS");
	formatDescription = tr("Nexus is a multiple alignment and phylogenetic trees file format");
    supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;
    supportedObjectTypes += GObjectTypes::PHYLOGENETIC_TREE;
}

const int Tokenizer::BUFF_SIZE = 1024;

QString Tokenizer::look() {
    if (next.isNull()) {
        get();
    }
    return next;
}

QString Tokenizer::get() {
    QString token = "";

    QChar c;
    QChar quote;

    int nest = 0;

    enum State {NONE, QUOTED, WORD, NUMBER, SPACE, COMMENT, SYMBOL} state = NONE;

    while (true) {
        if (buffStream.atEnd()) {
            QByteArray tmp(BUFF_SIZE, '\0');
            int len = io->readBlock(tmp.data(), BUFF_SIZE);

            if (len == 0) {
                break;
            }

            buff = tmp.left(len);
            buffStream.setString(&buff);
        }

        if (state == NONE) {
            // look at first symbol
            buffStream >> c;
            buffStream.seek(buffStream.pos() - 1);

            if (c == '"' || c == '\'') {
                state = QUOTED;
            } else if (c.isLetter() || c == '_') {
                state = WORD;
            } else if (c.isDigit() || c == '.') {
                state = NUMBER;
            } else if (c.isSpace()) {
                state = SPACE;
            } else if (c == '[') {
                state = COMMENT;
            } else {
                state = SYMBOL;
            }
        }

        if (state == QUOTED) {
            // read quoted string
            buffStream >> quote;
            buffStream >> c;

            while (c != quote && !c.isNull()) {
                token += c;
                buffStream >> c;
            }

            if (c == quote) {
                // next quoted string may appear 'asd''fgh' == 'asdfgh'
                buffStream >> c;
                buffStream.seek(buffStream.pos() - 1);

                if (c == '"' || c == '\'') {
                    state = QUOTED;
                    continue;
                } else {
                    state = NONE;
                    break;
                }
            } else {
                continue;
            }
        } else if (state == WORD) {
            // read whole word
            buffStream >> c;
            while ((c.isLetter() || c.isDigit() || c == '_' || c == '.') && !c.isNull()) {
                token += c;
                buffStream >> c;
            }

            if (!c.isNull()) {
                // put last symbol back
                buffStream.seek(buffStream.pos() - 1);
                state = NONE;
                break;
            } else {
                continue;
            }
        } else if (state == NUMBER) {
            // read whole number
            buffStream >> c;
            while ((c.isDigit() || c == '.' || c == 'e' ||c == 'E' || c == '+' ||c == '-') && !c.isNull()) {
                token += c;
                buffStream >> c;
            }

            if (!c.isNull()) {
                // put last symbol back
                buffStream.seek(buffStream.pos() - 1);
                state = NONE;
                break;
            } else {
                continue;
            }
        } else if (state == SPACE) {
            // skip whites
            buffStream.skipWhiteSpace();

            state = NONE;
            if (token.isEmpty()) {
                continue;
            } else {
                break;
            }
        } else if (state == COMMENT) {
            // skip comment
            buffStream >> c;
            while (!c.isNull()) {
                if (c == ']') {
                    --nest;
                } else if (c == '[') {
                    ++nest;
                }

                if (nest == 0) {
                    state = NONE;
                    break;
                }

                buffStream >> c;
            }

            continue;
        } else if (state == SYMBOL) {
            // return single symbol
            buffStream >> c;
            token = c;
            state = NONE;
            break;
        } else {
            assert(0 && "Unknown status, see enum");
        }
    }

    QString tmp = next;
    next = token;
    return tmp;
}

void Tokenizer::skipUntil(QString what, Qt::CaseSensitivity cs) {
    while (look().compare(what, cs) != 0) {
        get();
    }
}

QStringList Tokenizer::getUntil(QString what, Qt::CaseSensitivity cs) {
    QStringList words;
    while (look().compare(what, cs) != 0) {
        words.append(get());
    }

    return words;
}

QString Tokenizer::readUntil(QRegExp rwhat)
{
    // push 'next' token back
    QString str = next + buffStream.readAll();
    QString result = "";

    while (true) {
        int pos = str.indexOf(rwhat);
        if (pos < 0) {
            result += str;

            QByteArray tmp(BUFF_SIZE, '\0');
            int len = io->readBlock(tmp.data(), BUFF_SIZE);

            if (len == 0) {
                break;
            }

            str = tmp.left(len);
        } else {
            result += str.left(pos);
            buff = str.mid(pos);
            buffStream.setString(&buff);
            break;
        }
    }

    get();
    return result;
}

// lower case
const QString NEXUSParser::BEGIN           = "begin";
const QString NEXUSParser::END             = "end";
const QString NEXUSParser::BLK_TAXA        = "taxa";
const QString NEXUSParser::BLK_DATA        = "data";
const QString NEXUSParser::BLK_CHARACTERS  = "characters";
const QString NEXUSParser::CMD_DIMENSIONS  = "dimensions";
const QString NEXUSParser::CMD_FORMAT      = "format";
const QString NEXUSParser::CMD_MATRIX      = "matrix";
const QString NEXUSParser::BLK_TREES       = "trees";
const QString NEXUSParser::CMD_TREE        = "tree";
const QString NEXUSParser::CMD_UTREE       = "utree";

QList<GObject*> NEXUSParser::loadObjects()
{
    while (tz.look() != "") {
        if (!readBlock(global)) {
            break;
        }
        reportProgress();
    }
    return objects;
}

bool NEXUSParser::skipCommand() {
    tz.skipUntil(";");
    if (tz.get() != ";"){ 
        errors.append("\';\' expected"); 
        return false; 
    }
    return true;
}

bool NEXUSParser::readSimpleCommand(Context &ctx)
{
    tz.skip();  // command name
    QStringList wl = tz.getUntil(";");

    while (!wl.isEmpty()) {
        QString name = wl.takeFirst();
        QString value = "";

        if (wl.size() > 1 && wl.first() == "=") {
            wl.removeFirst();
            value = wl.takeFirst();
        }

        ctx.insert(name, value);
    }

    if (tz.get() != ";"){ 
        errors.append("\';\' expected"); 
        return false; 
    }

    return true;
}

bool NEXUSParser::readBlock(Context &ctx) {
    if (tz.get().toLower() != BEGIN){ 
        errors.append(QString("\'%1\' expected").arg(BEGIN)); 
        return false; 
    }

    QString blockName = tz.get().toLower();

    if (tz.get().toLower() != ";"){ 
        errors.append(QString("\'%1\' expected").arg(";")); 
        return false; 
    }

    if (blockName == BLK_TAXA) {
        if (!readTaxaContents(ctx)) {
            return false;
        }
    } else if (blockName == BLK_DATA || blockName == BLK_CHARACTERS) {
        if (!readDataContents(ctx)) {
            return false;
        }
    } else if (blockName == BLK_TREES) {
        if (!readTreesContents(ctx)) {
            return false;
        }
    } else {
        if (!skipBlockContents()) {
            return false;
        }
    }

    if (tz.get().toLower() != END) { 
        errors.append(QString("\'%1\' expected").arg(END)); 
        return false; 
    }

    if ((tz.get().toLower() != ";")){ 
        errors.append(QString("\'%1\' expected").arg(";")); 
        return false; 
    }

    return true;
}

bool NEXUSParser::skipBlockContents()
{
    tz.skipUntil(END);
    return true;
}

bool NEXUSParser::readTaxaContents(Context&) {
    while (true) {
        QString cmd = tz.look().toLower();
        if (cmd == END) {
            break;
        } else {
            if (!skipCommand()) {
                return false;
            }
        }
    }

    return true;
}

bool NEXUSParser::readDataContents(Context &ctx) {
    while (true) {
        QString cmd = tz.look().toLower(); // cmd name
        if (cmd == CMD_FORMAT || cmd == CMD_DIMENSIONS) {
            if (!readSimpleCommand(ctx)) {
                return false;
            }
        } else if (cmd == CMD_MATRIX) {
            tz.get(); // cmd name == CMD_MATRIX

            QMap<QString, MAlignmentRow> rows;

            // Read matrix
            while (true) {
                QString name = tz.get();

                if (name == "") {
                    errors.append("Unexpected EOF");
                    return false;
                }

                if (name == ";") {
                    break;
                }

                name.replace('_', ' ');

                // Read value
                QString value = "";
                value = tz.readUntil(QRegExp("\n|;"));

                // Remove spaces
                value.remove(QRegExp("\\s"));

                // Replace gaps
                if (ctx.contains("gap")) {
                    value.replace(ctx["gap"], QChar(MAlignment_GapChar));
                }

                if (rows.contains(name)) {
                    MAlignmentRow &row = rows[name];
                    row.append( MAlignmentRow(name, value.toAscii()), row.getCoreEnd() );
                } else {
                    rows.insert(name, MAlignmentRow(name, value.toAscii()));
                }

                reportProgress();
            }

            // Build MAlignment object
            MAlignment ma(tz.getIO()->getURL().baseFileName(), 0, rows.values());

            // Determine alphabet & replace missing chars
            if (ctx.contains("missing")) {
                char missing = ctx["missing"].toAscii()[0];
                U2AlphabetUtils::assignAlphabet(ma, missing);
                CHECK_EXT(ma.getAlphabet() != NULL, errors.append("Unknown alphabet"), false);
                
                char ourMissing = ma.getAlphabet()->getDefaultSymbol();

                // replace missing
                for (int i = 0; i < ma.getNumRows(); ++i) {
                    ma.replaceChars(i, missing, ourMissing);
                }
            } else {
                U2AlphabetUtils::assignAlphabet(ma);
                CHECK_EXT(ma.getAlphabet() != NULL, errors.append("Unknown alphabet"), false);
            }

            if (ma.getAlphabet() == 0) {
                errors.append("Unknown alphabet");
                return false;
            }

            addObject(new MAlignmentObject(ma));
        } else if (cmd == END) {
            break;
        } else {
            if (!skipCommand()) {
                return false;
            }
        }
    }

    return true;
}

bool NEXUSParser::readTreesContents(Context&) {
    while (true) {
        QString cmd = tz.look().toLower(); // cmd name
        bool weightDefault = false, weightValue = false;

        if (cmd == CMD_TREE || cmd == CMD_UTREE) {
            tz.skip(); // cmd name == CMD_TREE

            QString treeName = "Tree";
            // "tree_name =" or "="
            if (tz.look() == ";") {
                warnings.append("Empty tree");
                continue;
            } else if (tz.look() == "=") {
                tz.skip(); // "="
            } else {
                treeName = tz.get();
                if (tz.get() != "="){ 
                    errors.append(QString("\'%1\' expected").arg("=")); 
                    return false; 
                }
            }

            QStack<PhyNode*> nodeStack;
            nodeStack.push(new PhyNode());

            QStack<PhyBranch*> branchStack;

            enum State {NAME, WEIGHT} state = NAME;
            QString acc, name;

            QSet<QString> ops = QSet<QString>()<<"("<<","<<")"<<":"<<";";
            while (true) {
                QString tok = tz.get();
                if (tok == "")
                    { errors.append("Unexpected EOF"); break; }

                if (!ops.contains(tok)) {
                    acc += tok;
                    continue;
                }

                if (tok == "(") {
                    if (!acc.isEmpty()) { 
                        errors.append(QString("\'%1\' expected").arg(",")); 
                        break; 
                    }

                    PhyNode *top = new PhyNode();
                    PhyBranch *br = nodeStack.top()->addBranch(nodeStack.top(), top, 1);
                    nodeStack.push(top);
                    branchStack.push(br);
                } else if (tok == "," || tok == ")") {
                    if (nodeStack.size() < 2){ 
                        errors.append(QString("Unexpected \'%1\'").arg(")")); 
                        break; 
                    }

                    double weight = 1;  // default value

                    if (state == WEIGHT) {
                        if (!acc.isEmpty()) {
                            // convert weight
                            bool ok;
                            weight = acc.toDouble(&ok);
                            if (!ok){ 
                                errors.append(QString("Invalid weight value \'%1\'").arg(acc)); 
                                break; 
                            }

                            weightValue = true;
                        } else  {
                            // was ':' but was no value
                            errors.append("Weight value expected"); 
                            break; 
                        }
                    } else {
                        assert(state == NAME);

                        weightDefault = true;
                        name = acc;
                        acc.clear();
                    }

                    name.replace('_', ' ');
                    nodeStack.top()->name = name;
                    branchStack.top()->distance = weight;

                    nodeStack.pop();
                    branchStack.pop();

                    if (tok == ",") {
                        PhyNode *top = new PhyNode();
                        PhyBranch *br = nodeStack.top()->addBranch(nodeStack.top(), top, 1);
                        nodeStack.push(top);
                        branchStack.push(br);
                    }

                    acc.clear();
                    state = NAME;
                } else if (tok == ":") {
                    if (state == WEIGHT)
                        { errors.append(QString("Unexpected \'%1\'").arg(":")); break; }
                    name = acc;
                    acc.clear();
                    state = WEIGHT;
                } else if (tok == ";") {
                    if (state == NAME) {
                        name = acc;
                    }

                    name.replace('_', ' ');
                    nodeStack.top()->name = name;
                    break;
                } else {
                    assert(0 && "invalid state: all non ops symbols must go to acc");
                }
            }

            if (weightDefault && weightValue) {
                errors.append("Weight value expected");
            } else if (weightDefault) {
                warnings.append(QString("Tree '%1' weights set by default (1.0)").arg(treeName));
            }

            if (hasError()) {
                // clean up
                PhyNode *n = nodeStack.pop();
                delete n;  // PhyNode::~PhyNode() is recursive

                warnings.push_back(QString("Tree '%1' ignored due following errors: %2").arg(treeName, errors.takeLast()));
                continue;
            } else {
                assert(nodeStack.size() == 1 && "Node stack must contain only root");

                // build tree object
                PhyTree tree(new PhyTreeData());
                tree->rootNode = nodeStack.pop();
                addObject(new PhyTreeObject(tree, treeName));
            }
        } else if (cmd == END) {
            break;
        } else {
            if (!skipCommand()) {
                return false;
            }
        }
    }

    return true;
}

void NEXUSParser::addObject(GObject *obj) {
    QString name = TextUtils::variate(obj->getGObjectName(), "_", objectNames, false, 1);
    objectNames.insert(name);
    obj->setGObjectName(name);
    objects.append(obj);
}

QList<GObject*> NEXUSFormat::loadObjects(IOAdapter *io, U2OpStatus &ti) {
    assert(io && "io must exist");

    const int HEADER_LEN = 6;
    QByteArray header(HEADER_LEN, 0);
    int rd = io->readLine(header.data(), HEADER_LEN);
    header.truncate(rd);
    if (header != "#NEXUS") {
        ti.setError(tr("#NEXUS header missing"));
        return QList<GObject*>();
    }

    NEXUSParser parser(io, ti);
    QList<GObject*> objects = parser.loadObjects();
    
    if (parser.hasError()) {
        QByteArray msg = "NEXUSParser: ";
        msg += parser.getErrors().first();
        ti.setError(tr(msg.data()));
    }
    
    return objects;
}

Document* NEXUSFormat::loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& fs, U2OpStatus& os){
    assert(io && "IO must exist");

    QList<GObject*> objects = loadObjects(io, os);
    CHECK_OP_EXT(os, qDeleteAll(objects), NULL);
    
    Document *d = new Document(this, io->getFactory(), io->getURL(), dbiRef, dbiRef.isValid(), objects, fs);
    return d;
}

void writeHeader(IOAdapter *io, U2OpStatus&) {
    QByteArray line;
    QTextStream(&line) << "#NEXUS\n\n";
    io->writeBlock(line);
}

void writeMAligment(const MAlignment &ma, IOAdapter *io, U2OpStatus&) {
    QByteArray line;
    QByteArray tabs, tab(4, ' ');

    QTextStream(&line) << tabs << "begin data;" << "\n";
    io->writeBlock(line);
    line.clear();

    tabs.append(tab);

    int ntax, nchar;
    ntax = ma.getNumRows();
    nchar = ma.getLength();

    QTextStream(&line) << tabs << "dimensions ntax=" << ntax << " nchar=" << nchar << ";\n";
    io->writeBlock(line);
    line.clear();

    //datatype for MrBayes files
    QString dataType;
    const QString& alphabetId = ma.getAlphabet()->getId();
    if(alphabetId == BaseDNAAlphabetIds::NUCL_DNA_DEFAULT() || alphabetId == BaseDNAAlphabetIds::NUCL_DNA_EXTENDED()){
        dataType = "dna";
    }else if(alphabetId == BaseDNAAlphabetIds::NUCL_RNA_DEFAULT() || alphabetId == BaseDNAAlphabetIds::NUCL_RNA_EXTENDED()){
       dataType = "rna"; 
    }else if (alphabetId == BaseDNAAlphabetIds::AMINO_DEFAULT()){
        dataType = "protein";
    }else {
        dataType = "standard";
    }

    QTextStream(&line) << tabs << "format datatype="<<dataType<<" gap=" << MAlignment_GapChar << ";\n";
    io->writeBlock(line);
    line.clear();

    QTextStream(&line) << tabs << "matrix" << "\n";
    io->writeBlock(line);
    line.clear();

    tabs.append(tab);

    const QList<MAlignmentRow> &rows = ma.getRows();

    int nameMaxLen = 0;
    foreach (MAlignmentRow row, rows) {
        if (row.getName().size() > nameMaxLen) {
            nameMaxLen = row.getName().size();
        }
    }
    nameMaxLen += 2;    // quotes may appear

    foreach (const MAlignmentRow& row, rows)
    {
        QString name = row.getName();
        
        if (name.contains(QRegExp("\\s")))
        {
            //name = "'" + name + "'";
            name.replace(' ','_');
        }

        name = name.leftJustified(nameMaxLen);

        QTextStream(&line) << tabs << name << " " << row.toByteArray(nchar) << "\n";
        io->writeBlock(line);
        line.clear();
    }

    tabs.chop(tab.size());

    QTextStream(&line) << tabs << ";" << "\n";
    io->writeBlock(line);
    line.clear();

    tabs.chop(tab.size());

    QTextStream(&line) << tabs << "end;" << "\n";
    io->writeBlock(line);
    line.clear();
}

static void writeNode(PhyNode* node, IOAdapter* io) {
    int branches = node->branches.size();

    if (branches == 1 && (node->name=="" || node->name=="ROOT")) {
        assert(node != node->branches[0]->node2);
        writeNode(node->branches[0]->node2, io);
        return;
    }

    if (branches > 1) {
        io->writeBlock("(", 1);
        bool first = true;
        for (int i = 0; i < branches; ++i) {
            if (node->branches[i]->node2 != node) {
                if (first) {
                    first = false;
                } else {
                    io->writeBlock(",", 1);
                }
                writeNode(node->branches[i]->node2, io);
                io->writeBlock(":", 1);
                io->writeBlock(QByteArray::number(node->branches[i]->distance));
            }
        }
        io->writeBlock(")", 1);
    } else {
        bool quotes = node->name.contains(QRegExp("\\s"));

        if (quotes) {
            io->writeBlock("'", 1);
        }
        io->writeBlock(node->name.toAscii());
        if (quotes) {
            io->writeBlock("'", 1);
        }
    }
}

// spike: PhyTreeData don't have own name
void writePhyTree(const PhyTree &pt, QString name, IOAdapter *io, U2OpStatus&)
{
    QByteArray line;
    QByteArray tabs, tab(4, ' ');

    QTextStream(&line) << tabs << "begin trees;" << "\n";
    io->writeBlock(line);
    line.clear();

    tabs.append(tab);

    QTextStream(&line) << tabs << "tree " << name << " = ";
    io->writeBlock(line);
    line.clear();

    writeNode(pt->rootNode, io);
    io->writeBlock(";\n", 2);

    tabs.chop(tab.size());

    QTextStream(&line) << tabs << "end;" << "\n";
    io->writeBlock(line);
    line.clear();
}

void writePhyTree(const PhyTree &pt, IOAdapter *io, U2OpStatus &ti) {
    writePhyTree(pt, "Tree", io, ti);
}

void NEXUSFormat::storeObjects(QList<GObject*> objects, IOAdapter *io, U2OpStatus &ti) {
    assert(io && "IO must exist");
    writeHeader(io, ti);

    foreach (GObject *object, objects) {
        MAlignmentObject *mao = qobject_cast<MAlignmentObject*> (object);
        PhyTreeObject *pto = qobject_cast<PhyTreeObject*> (object);
        if (mao) {
            writeMAligment(mao->getMAlignment(), io, ti);
            io->writeBlock(QByteArray("\n"));
        } else if (pto) {
            writePhyTree(pto->getTree(), pto->getGObjectName(), io, ti);
            io->writeBlock(QByteArray("\n"));
        } else {
            ti.setError("No data to write");
            return;
        }
    }
}

void NEXUSFormat::storeDocument(Document *d, U2OpStatus &ts, IOAdapter *io) {
    QList<GObject*> objects = d->getObjects();
    storeObjects(objects, io, ts);
}

FormatCheckResult NEXUSFormat::checkRawData(const QByteArray &rawData, const GUrl&) const {
    if (rawData.startsWith("#NEXUS")) {
        return FormatDetection_VeryHighSimilarity;
    }
    return FormatDetection_NotMatched;
}

}    // namespace U2

