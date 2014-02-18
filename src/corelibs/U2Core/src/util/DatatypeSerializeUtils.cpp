/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <QtCore/QBitArray>
#include <QtCore/QStack>
#include <QtCore/QtEndian>

#include <U2Core/StringAdapter.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2SafePoints.h>

#include "DatatypeSerializeUtils.h"

namespace U2 {

const QString DNAChromatogramSerializer::ID = "chroma_1.14";
const QString NewickPhyTreeSerializer::ID = "newick_1.14";

namespace {
    template<class T>
    inline QByteArray packNum(const T &num) {
        T leNum = qToLittleEndian<T>(num);
        return QByteArray((char*)&leNum, sizeof(T));
    }

    template<class T>
    inline QByteArray packNumVector(const QVector<T> &vector) {
        QByteArray result;
        result += packNum<int>(vector.size());
        foreach (const T &num, vector) {
            result += packNum<T>(num);
        }
        return result;
    }

    inline QByteArray packCharVector(const QVector<char> &vector) {
        QByteArray result;
        result += packNum<int>(vector.size());
        foreach (const char &c, vector) {
            result += c;
        }
        return result;
    }

    inline QByteArray packBool(bool value) {
        char c = (value) ? 1 : 0;
        return QByteArray(1, c);
    }

    template<class T>
    inline T unpackNum(const uchar *data, int length, int &offset, U2OpStatus &os) {
        if (offset + int(sizeof(T)) > length) {
            os.setError("The data is too short");
            return T();
        }
        T result = qFromLittleEndian<T>(data + offset);
        offset += sizeof(T);
        return result;
    }

    template<class T>
    inline QVector<T> unpackNumVector(const uchar *data, int length, int &offset, U2OpStatus &os) {
        QVector<T> result;
        int size = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, result);
        for (int i=0; i<size; i++) {
            T num = unpackNum<T>(data, length, offset, os);
            CHECK_OP(os, result);
            result << num;
        }
        return result;
    }

    inline QVector<char> unpackCharVector(const uchar *data, int length, int &offset, U2OpStatus &os) {
        QVector<char> result;
        int size = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, result);
        for (int i=0; i<size; i++) {
            if (offset + 1 > length) {
                os.setError("The data is too short");
                return result;
            }
            result << data[offset];
            offset++;
        }
        return result;
    }

    inline bool unpackBool(const uchar *data, int length, int &offset, U2OpStatus &os) {
        if (offset + 1 > length) {
            os.setError("The data is too short");
            return false;
        }
        uchar c = data[offset];
        offset++;
        return (0 == c) ? false : true;
    }
}

/************************************************************************/
/* DNAChromatogramSerializer */
/************************************************************************/
QByteArray DNAChromatogramSerializer::serialize(const DNAChromatogram &chroma) {
    QByteArray result;
    result += packNum<int>(chroma.traceLength);
    result += packNum<int>(chroma.seqLength);
    result += packNumVector<ushort>(chroma.baseCalls);
    result += packNumVector<ushort>(chroma.A);
    result += packNumVector<ushort>(chroma.C);
    result += packNumVector<ushort>(chroma.G);
    result += packNumVector<ushort>(chroma.T);
    result += packCharVector(chroma.prob_A);
    result += packCharVector(chroma.prob_C);
    result += packCharVector(chroma.prob_G);
    result += packCharVector(chroma.prob_T);
    result += packBool(chroma.hasQV);
    return result;
}

DNAChromatogram DNAChromatogramSerializer::deserialize(const QByteArray &binary, U2OpStatus &os) {
    DNAChromatogram result;
    const uchar *data = (const uchar*)(binary.data());
    int offset = 0;
    int length = binary.length();

    result.traceLength = unpackNum<int>(data, length, offset, os);
    CHECK_OP(os, result);
    result.seqLength = unpackNum<int>(data, length, offset, os);
    CHECK_OP(os, result);
    result.baseCalls = unpackNumVector<ushort>(data, length, offset, os);
    CHECK_OP(os, result);
    result.A = unpackNumVector<ushort>(data, length, offset, os);
    CHECK_OP(os, result);
    result.C = unpackNumVector<ushort>(data, length, offset, os);
    CHECK_OP(os, result);
    result.G = unpackNumVector<ushort>(data, length, offset, os);
    CHECK_OP(os, result);
    result.T = unpackNumVector<ushort>(data, length, offset, os);
    CHECK_OP(os, result);
    result.prob_A = unpackCharVector(data, length, offset, os);
    CHECK_OP(os, result);
    result.prob_C = unpackCharVector(data, length, offset, os);
    CHECK_OP(os, result);
    result.prob_G = unpackCharVector(data, length, offset, os);
    CHECK_OP(os, result);
    result.prob_T = unpackCharVector(data, length, offset, os);
    CHECK_OP(os, result);
    result.hasQV = unpackBool(data, length, offset, os);
    return result;
}

/************************************************************************/
/* NewickPhyTreeSerializer */
/************************************************************************/
namespace {
    void packTreeNode(QByteArray &binary, const PhyNode *node) {
        int branches = node->branchCount();
        if (branches == 1 && (node->getName() == "" || node->getName() == "ROOT")) {
            assert(node != node->getSecondNodeOfBranch(0));
            packTreeNode(binary, node->getSecondNodeOfBranch(0));
            return;
        }
        if (branches > 1) {
            binary.append("(");
            bool first = true;
            for (int i = 0; i < branches; ++i) {
                if (node->getSecondNodeOfBranch(i)!= node) {
                    if (first) {
                        first = false;
                    } else {
                        binary.append(",");
                    }
                    packTreeNode(binary, node->getSecondNodeOfBranch(i));
                    binary.append(":");
                    binary.append(QByteArray::number(node->getBranchesDistance(i)));
                }
            }
            binary.append(")");
        } else {
            binary.append(QString(node->getName()).replace(' ', '_').toLatin1());
        }
    }
}

#define BUFF_SIZE 1024
/* TODO:
 Unquoted labels may not contain blanks, parentheses, square brackets, single_quotes, colons, semicolons, or commas.
 Single quote characters in a quoted label are represented by two single quotes.
 Blanks or tabs may appear anywhere except within unquoted labels or branch_lengths.
 Newlines may appear anywhere except within labels or branch_lengths.
 Comments are enclosed in square brackets and may appear anywhere newlines are permitted. 
*/
QList<PhyTree> NewickPhyTreeSerializer::parseTrees(IOAdapter *io, U2OpStatus& si) {
    QList<PhyTree> result;
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
                nodeStack.top()->setName(lastStr);
            } else {
                assert(state == RS_WEIGHT);
                if (!branchStack.isEmpty()) { //ignore root node weight if present
                    if (nodeStack.size() < 2) {
                        si.setError(QObject::tr("Unexpected weight: %1").arg(lastStr));
                    }
                    bool ok = false;
                    branchStack.top()->distance = lastStr.toDouble(&ok);
                    if (!ok) {
                        si.setError(QObject::tr("Error parsing weight: %1").arg(lastStr));
                        break;
                    }           
                }
            }

            // advance in state
            if (c == '(') { //new child
                assert(!nodeStack.isEmpty());
                PhyNode* pn = new PhyNode();
                PhyBranch* bd = PhyTreeData::addBranch(nodeStack.top(),pn, 0);
                nodeStack.push(pn);
                branchStack.push(bd);
                state = RS_NAME;
            } else if (c == ':') { //weight start
                if (state == RS_WEIGHT) {
                    si.setError(QObject::tr("Unexpected weight start token: %1").arg(lastStr));
                    break;
                }
                state = RS_WEIGHT;
            } else if ( c == ',') { //new sibling
                assert(!nodeStack.isEmpty());
                assert(!branchStack.isEmpty());
                if (nodeStack.isEmpty() || branchStack.isEmpty()) {
                    si.setError(QObject::tr("Unexpected new sibling %1").arg(lastStr));
                    break;
                }
                nodeStack.pop();
                branchStack.pop();
                PhyNode* pn = new PhyNode();
                PhyBranch* bd = PhyTreeData::addBranch(nodeStack.top(), pn, 0);
                nodeStack.push(pn);
                branchStack.push(bd);
                state = RS_NAME;
            } else if ( c == ')' ) { //end of the branch, go up
                nodeStack.pop();
                if (nodeStack.isEmpty()) {
                    si.setError(QObject::tr("Unexpected closing bracket :%1").arg(lastStr));
                    break;
                }
                assert(!branchStack.isEmpty());
                branchStack.pop();
                state = RS_NAME;
            } else if (c == ';') {
                if (!branchStack.isEmpty() || nodeStack.size()!=1) {
                    si.setError(QObject::tr("Unexpected end of file"));
                    break;
                }
                PhyTree tree(new PhyTreeData());
                tree->setRootNode(nodeStack.pop());
                result << tree;
                nodeStack.push(rd = new PhyNode());
                done = true;
            } 
            lastStr.clear();
        }
        if (si.isCoR()) {
            delete rd;
            rd = NULL;
            break;
        }
        si.setProgress(io->getProgress());
    }
    if (!si.isCoR()) {
        if (!branchStack.isEmpty() || nodeStack.size()!=1) {
            delete rd;
            si.setError(QObject::tr("Unexpected end of file"));
            return result;
        }
        if (!done) {
            PhyNode *node = nodeStack.pop();
            PhyTree tree(new PhyTreeData());
            tree->setRootNode(node);
            result << tree;
        } else {
            delete rd;
            if (result.empty()) {
                si.setError(QObject::tr("Empty file"));
            }
        }
    }
    return result;
}

QByteArray NewickPhyTreeSerializer::serialize(const PhyTree &tree) {
    QByteArray result;
    packTreeNode(result, tree->getRootNode());
    result.append(";\n");
    return result;
}

PhyTree NewickPhyTreeSerializer::deserialize(const QByteArray &binary, U2OpStatus &os) {
    StringAdapter io(binary);
    QList<PhyTree> trees = parseTrees(&io, os);
    CHECK_OP(os, PhyTree());
    if (1 != trees.size()) {
        os.setError("Unexpected count of trees");
        return PhyTree();
    }
    return trees.first();
}

} // U2
