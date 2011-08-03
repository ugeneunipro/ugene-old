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

#include "NewickFormat.h"

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/PhyTreeObject.h>
#include <U2Core/TextUtils.h>

namespace U2 {

/* TRANSLATOR U2::IOAdapter */
/* TRANSLATOR U2::NewickFormat */

NewickFormat::NewickFormat(QObject* p) : DocumentFormat(p, DocumentFormatFlags_W1) 
{
    fileExtensions << "nwk" << "newick" << "nh" << "ph";
    formatName = tr("Newick Standard");
    formatDescription = tr("Newick is a simple format used to write out trees in a text file");
    supportedObjectTypes += GObjectTypes::PHYLOGENETIC_TREE;
}


#define BUFF_SIZE 1024

static QList<GObject*> parseTrees(IOAdapter* io, TaskStateInfo& si);

Document* NewickFormat::loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode) {
    QList<GObject*> objects = parseTrees(io, ti);
    if (ti.hasError()) {
        qDeleteAll(objects);
        return NULL;
    }

    Document* d = new Document(this, io->getFactory(), io->getURL(), objects, fs);
    return d;
}

static void writeNode(IOAdapter* io, PhyNode* node) {
    int branches = node->branches.size();
    if (branches == 1 && (node->name == "" || node->name == "ROOT")) {
        assert(node != node->branches[0]->node2);
        writeNode(io, node->branches[0]->node2);
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
                writeNode(io, node->branches[i]->node2);
                io->writeBlock(":", 1);
                io->writeBlock(QByteArray::number(node->branches[i]->distance));
            }
        }
        io->writeBlock(")", 1);
    } else {
        io->writeBlock(node->name.replace(' ', '_').toAscii());
    }
}

void NewickFormat::storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io) {
    Q_UNUSED(ts);
    assert(d->getDocumentFormat() == this);

    foreach(GObject* obj, d->getObjects()) {
        PhyTreeObject* phyObj = qobject_cast<PhyTreeObject*>(obj);
        if (phyObj != NULL) {
            writeNode(io, phyObj->getTree()->rootNode);
            io->writeBlock(";\n", 2);
        }
    }
}

FormatCheckResult NewickFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    const char* data = rawData.constData();
    int size = rawData.size();
    bool containsBinary = TextUtils::contains(TextUtils::BINARY, data, size);
    if (containsBinary) {
        return FormatDetection_NotMatched;
    }
    int brackets = 0;
    typedef enum { letter, letter_than_whites, any } Cases;
    Cases last = any;
    for (int i = 0; i < size; ++i) {
        switch (data[i]) {
            case '(':
                ++brackets;
                break;
            case ')':
                if (brackets == 0) {
                    return FormatDetection_NotMatched;
                }
                --brackets;
                break;
            case ';':
                if (brackets != 0) {
                    return FormatDetection_NotMatched;
                }
                break;
            default:
                if(data[i] < 0) { // for ex. if file contains utf-8 symbols
                    return FormatDetection_NotMatched;
                }
                if (TextUtils::ALPHA_NUMS[data[i]] || data[i] == '-' || data[i] == '_') {
                    if (last == letter_than_whites) {
                        return FormatDetection_NotMatched;
                    }
                    last = letter;
                    continue;
                }
                if (TextUtils::WHITES[data[i]]) {
                    if (last == letter || last == letter_than_whites) {
                        last = letter_than_whites;
                        continue;
                    }
                }
        }
        last = any;
    }
    if (QRegExp("[a-zA-Z\r\n]*").exactMatch(rawData)) {
        return FormatDetection_LowSimilarity;
    }
    int braces = (rawData.contains('(') ? 1 : 0) + (rawData.contains(')') ? 1 : 0) ;
    if (braces == 0 && rawData.length() > 50)  {
        return FormatDetection_LowSimilarity;
    }
    if (braces == 1) {
        return FormatDetection_NotMatched;
    }
    return FormatDetection_HighSimilarity;
}


/* TODO:
 Unquoted labels may not contain blanks, parentheses, square brackets, single_quotes, colons, semicolons, or commas.
 Single quote characters in a quoted label are represented by two single quotes.
 Blanks or tabs may appear anywhere except within unquoted labels or branch_lengths.
 Newlines may appear anywhere except within labels or branch_lengths.
 Comments are enclosed in square brackets and may appear anywhere newlines are permitted. 
*/
static QList<GObject*> parseTrees(IOAdapter *io, TaskStateInfo& si) {
    QList<GObject*> objects;
    QByteArray block(BUFF_SIZE, '\0');
    int blockLen;
    bool done = true;
    
    QBitArray ops(256);
    ops['('] = ops[')'] = ops[':']  = ops[','] = ops[';'] = true;
    enum ReadState {RS_NAME, RS_WEIGHT};
    ReadState state = RS_NAME;
    QString lastStr;
    PhyNode *rd = new PhyNode();

    QStack<PhyNode*> nodeStack;
    QStack<PhyBranch*>  branchStack;
    nodeStack.push(rd);
    while ((blockLen = io->readBlock(block.data(), BUFF_SIZE)) > 0) {
        for (int i = 0; i < blockLen; ++i) {
            unsigned char c = block[i];
            if (TextUtils::WHITES[(uchar)c]) {
                continue;
            }
            done = false;
            if (!ops[(uchar)c]) { //not ops -> cache
                lastStr.append(c);
                continue;
            }
            // use cached value
            if (state == RS_NAME) {
                nodeStack.top()->name = lastStr.replace('_', ' ');
            } else {
                assert(state == RS_WEIGHT);
                if (!branchStack.isEmpty()) { //ignore root node weight if present
                    if (nodeStack.size() < 2) {
                        si.setError(NewickFormat::tr("Unexpected weight: %1").arg(lastStr));
                    }
                    bool ok = false;
                    branchStack.top()->distance = lastStr.toDouble(&ok);
                    if (!ok) {
                        si.setError(NewickFormat::tr("Error parsing weight: %1").arg(lastStr));
                        break;
                    }           
                }
            }
            
            // advance in state
            if (c == '(') { //new child
                assert(!nodeStack.isEmpty());
                PhyNode* pn = new PhyNode();
                PhyBranch* bd = PhyNode::addBranch(nodeStack.top(),pn, 0);
                nodeStack.push(pn);
                branchStack.push(bd);
                state = RS_NAME;
            } else if (c == ':') { //weight start
                if (state == RS_WEIGHT) {
                    si.setError(NewickFormat::tr("Unexpected weight start token: %1").arg(lastStr));
                    break;
                }
                state = RS_WEIGHT;
            } else if ( c == ',') { //new sibling
                assert(!nodeStack.isEmpty());
                assert(!branchStack.isEmpty());
                if (nodeStack.isEmpty() || branchStack.isEmpty()) {
                    si.setError(NewickFormat::tr("Unexpected new sibling %1").arg(lastStr));
                    break;
                }
                nodeStack.pop();
                branchStack.pop();
                PhyNode* pn = new PhyNode();
                PhyBranch* bd = PhyNode::addBranch(nodeStack.top(), pn, 0);
                nodeStack.push(pn);
                branchStack.push(bd);
                state = RS_NAME;
            } else if ( c == ')' ) { //end of the branch, go up
                nodeStack.pop();
                if (nodeStack.isEmpty()) {
                    si.setError(NewickFormat::tr("Unexpected closing bracket :%1").arg(lastStr));
                    break;
                }
                assert(!branchStack.isEmpty());
                branchStack.pop();
                state = RS_NAME;
            } else if (c == ';') {
                if (!branchStack.isEmpty() || nodeStack.size()!=1) {
                    si.setError(NewickFormat::tr("Unexpected end of file"));
                    break;
                }
                PhyTree tree(new PhyTreeData());
                tree->rootNode = nodeStack.pop();
                QString objName = (objects.size() == 0) ? QString("Tree") : QString("Tree%1").arg(objects.size() + 1);
                objects.append(new PhyTreeObject(tree, objName));
                nodeStack.push(rd = new PhyNode());
                done = true;
            } 
            lastStr.clear();
        }
        if (si.hasError() || si.cancelFlag) {
            delete rd;
            rd = NULL;
            break;
        }
        si.progress = io->getProgress();
    }
    if (!si.hasError() && !si.cancelFlag) {
        if (!branchStack.isEmpty() || nodeStack.size()!=1) {
            delete rd;
            si.setError(NewickFormat::tr("Unexpected end of file"));
            return objects;
        }
        if (!done) {
            PhyNode *node = nodeStack.pop();
            PhyTree tree(new PhyTreeData());
            tree->rootNode = node;
            QString objName = (objects.size() == 0) ? QString("Tree") : QString("Tree%1").arg(objects.size() + 1);
            objects.append(new PhyTreeObject(tree, objName));
        } else {
            delete rd;
            if (objects.empty()) {
                si.setError(NewickFormat::tr("Empty file"));
            }
        }
    }
    return objects;
}

}//namespace
