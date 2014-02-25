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
const QString BioStruct3DSerializer::ID = "3d_1.14";

#define CHECK_SIZE(size, result) \
    if (offset + size > length) { \
        os.setError("The data is too short"); \
        return result; \
    }

namespace {
    template<class T>
    inline QByteArray packNum(const T &num) {
        T leNum = qToLittleEndian<T>(num);
        return QByteArray((char*)&leNum, sizeof(T));
    }
    template<>
    inline QByteArray packNum(const double &num) {
        QByteArray numStr = QByteArray::number(num);
        return packNum<int>(numStr.size()) + numStr;
    }
    template<>
    inline QByteArray packNum(const float &num) {
        QByteArray numStr = QByteArray::number(num);
        return packNum<int>(numStr.size()) + numStr;
    }
    template<class T>
    inline T unpackNum(const uchar *data, int length, int &offset, U2OpStatus &os) {
        CHECK_SIZE(int(sizeof(T)), T());
        T result = qFromLittleEndian<T>(data + offset);
        offset += sizeof(T);
        return result;
    }
    inline QByteArray unpackReal(const uchar *data, int length, int &offset, U2OpStatus &os) {
        int size = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, "");
        CHECK_SIZE(size, "");
        QByteArray result((const char*)data + offset, size);
        offset += size;
        return result;
    }
    template<>
    inline double unpackNum(const uchar *data, int length, int &offset, U2OpStatus &os) {
        QByteArray numStr = unpackReal(data, length, offset, os);
        CHECK_OP(os, 0.0);
        return numStr.toDouble();
    }
    template<>
    inline float unpackNum(const uchar *data, int length, int &offset, U2OpStatus &os) {
        QByteArray numStr = unpackReal(data, length, offset, os);
        CHECK_OP(os, 0.0f);
        return numStr.toFloat();
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

    inline QByteArray packCharVector(const QVector<char> &vector) {
        QByteArray result;
        result += packNum<int>(vector.size());
        foreach (const char &c, vector) {
            result += c;
        }
        return result;
    }
    inline QVector<char> unpackCharVector(const uchar *data, int length, int &offset, U2OpStatus &os) {
        QVector<char> result;
        int size = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, result);
        for (int i=0; i<size; i++) {
            CHECK_SIZE(1, result);
            result << data[offset];
            offset++;
        }
        return result;
    }

    inline QByteArray packBool(bool value) {
        char c = (value) ? 1 : 0;
        return QByteArray(1, c);
    }

    inline bool unpackBool(const uchar *data, int length, int &offset, U2OpStatus &os) {
        CHECK_SIZE(1, false);
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

/************************************************************************/
/* BioStruct3DSerializer */
/************************************************************************/
namespace {
    class PackContext {
    public:
        QList<SharedAtom> atoms;
    };

    template<class T>
    T unpack(const uchar *data, int length, int &offset, U2OpStatus &os);
    template<class T>
    T unpack(const uchar *data, int length, int &offset, U2OpStatus &os, PackContext &ctx);

    template<>
    inline char unpack(const uchar *data, int length, int &offset, U2OpStatus &os) {
        CHECK_SIZE(1, 0);
        char result = data[offset];
        offset++;
        return result;
    }

    inline QByteArray pack(const QByteArray &data) {
        return packNum<int>(data.size()) + data;
    }
    template<>
    inline QByteArray unpack(const uchar *data, int length, int &offset, U2OpStatus &os) {
        int size = unpackNum<int>(data, length, offset, os);
        CHECK_SIZE(size, "");
        QByteArray result((const char*)data + offset, size);
        offset += size;
        return result;
    }

    inline QByteArray pack(const QString &data) {
        return pack(data.toUtf8());
    }
    template<>
    inline QString unpack(const uchar *data, int length, int &offset, U2OpStatus &os) {
        return QString::fromUtf8(unpack<QByteArray>(data, length, offset, os));
    }

    inline QByteArray pack(const ResidueIndex &data) {
        QByteArray result;
        result += packNum<int>(data.toInt());
        result += packNum<int>(data.getOrder());
        result += data.getInsCode();
        return result;
    }
    template<>
    inline ResidueIndex unpack(const uchar *data, int length, int &offset, U2OpStatus &os) {
        int resId = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, ResidueIndex());
        int order = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, ResidueIndex());
        char insCode = unpack<char>(data, length, offset, os);
        CHECK_OP(os, ResidueIndex());

        ResidueIndex result(resId, insCode);
        result.setOrder(order);
        return result;
    }
    template<>
    inline QByteArray packNum(const ResidueIndex &num) {
        return pack(num);
    }
    template<>
    inline ResidueIndex unpackNum(const uchar *data, int length, int &offset, U2OpStatus &os) {
        return unpack<ResidueIndex>(data, length, offset, os);
    }

    inline QByteArray pack(const ResidueData &data) {
        QByteArray result;
        result += char(data.type);
        result += pack(data.name);
        result += data.acronym;
        result += packNum<int>(data.chainIndex);
        return result;
    }
    inline QByteArray pack(const SharedResidue &data, PackContext & /*ctx*/) {
        return pack(*data.data());
    }
    template<>
    inline SharedResidue unpack(const uchar *data, int length, int &offset, U2OpStatus &os, PackContext & /*ctx*/) {
        SharedResidue result(new ResidueData());
        result->type = ResidueData::Type(unpack<char>(data, length, offset, os));
        CHECK_OP(os, result);

        result->name = unpack<QByteArray>(data, length, offset, os);
        CHECK_OP(os, result);

        result->acronym = unpack<char>(data, length, offset, os);
        CHECK_OP(os, result);

        result->chainIndex = unpackNum<int>(data, length, offset, os);
        return result;
    }

    inline QByteArray pack(const Vector3D &data) {
        QByteArray result;
        result += packNum<double>(data.x);
        result += packNum<double>(data.y);
        result += packNum<double>(data.z);
        return result;
    }
    template<>
    inline Vector3D unpack(const uchar *data, int length, int &offset, U2OpStatus &os) {
        Vector3D result;
        result.x = unpackNum<double>(data, length, offset, os);
        CHECK_OP(os, result);
        result.y = unpackNum<double>(data, length, offset, os);
        CHECK_OP(os, result);
        result.z = unpackNum<double>(data, length, offset, os);
        return result;
    }

    inline QByteArray pack(const Matrix44 &data) {
        QByteArray result;
        for (int i=0; i<16; i++) {
            result += packNum<float>(data[i]);
        }
        return result;
    }
    template<>
    inline Matrix44 unpack(const uchar *data, int length, int &offset, U2OpStatus &os) {
        Matrix44 result;
        for (int i=0; i<16; i++) {
            result[i] = unpackNum<float>(data, length, offset, os);
            CHECK_OP(os, result);
        }
        return result;
    }

    inline QByteArray pack(const AtomData &data) {
        QByteArray result;
        result += packNum<int>(data.atomicNumber);
        result += packNum<int>(data.chainIndex);
        result += pack(data.residueIndex);
        result += pack(data.name);
        result += pack(data.coord3d);
        result += packNum<float>(data.occupancy);
        result += packNum<float>(data.temperature);
        return result;
    }
    inline QByteArray pack(const SharedAtom &data, PackContext &ctx) {
        if (ctx.atoms.contains(data)) {
            int num = ctx.atoms.indexOf(data);
            return packNum<int>(num);
        }
        QByteArray result;
        result += packNum<int>(ctx.atoms.size());
        result += pack(*data.data());
        ctx.atoms << data;
        return result;
    }
    template<>
    inline AtomData unpack(const uchar *data, int length, int &offset, U2OpStatus &os) {
        AtomData result;
        result.atomicNumber = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, result);
        result.chainIndex = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, result);
        result.residueIndex = unpack<ResidueIndex>(data, length, offset, os);
        CHECK_OP(os, result);
        result.name = unpack<QByteArray>(data, length, offset, os);
        CHECK_OP(os, result);
        result.coord3d = unpack<Vector3D>(data, length, offset, os);
        CHECK_OP(os, result);
        result.occupancy = unpackNum<float>(data, length, offset, os);
        CHECK_OP(os, result);
        result.temperature = unpackNum<float>(data, length, offset, os);
        return result;
    }
    template<>
    inline SharedAtom unpack(const uchar *data, int length, int &offset, U2OpStatus &os, PackContext &ctx) {
        int num = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, SharedAtom());
        if (num < ctx.atoms.size()) {
            return ctx.atoms[num];
        }
        SAFE_POINT_EXT(num == ctx.atoms.size(), os.setError("Unexpected atom number"), SharedAtom());
        AtomData atom = unpack<AtomData>(data, length, offset, os);
        CHECK_OP(os, SharedAtom());
        SharedAtom result(new AtomData(atom));
        ctx.atoms << result;
        return result;
    }

    inline QByteArray pack(const Bond &data, PackContext &ctx) {
        QByteArray result;
        result += pack(data.getAtom1(), ctx);
        result += pack(data.getAtom2(), ctx);
        return result;
    }
    template<>
    inline Bond unpack(const uchar *data, int length, int &offset, U2OpStatus &os, PackContext &ctx) {
        SharedAtom atom1 = unpack<SharedAtom>(data, length, offset, os, ctx);
        CHECK_OP(os, Bond(SharedAtom(), SharedAtom()));
        SharedAtom atom2 = unpack<SharedAtom>(data, length, offset, os, ctx);
        return Bond(atom1, atom2);
    }

    inline QByteArray pack(const SecondaryStructure &data) {
        QByteArray result;
        result += char(data.type);
        result += data.chainIdentifier;
        result += packNum<int>(data.chainIndex);
        result += packNum<int>(data.startSequenceNumber);
        result += packNum<int>(data.endSequenceNumber);
        return result;
    }
    inline QByteArray pack(const SharedSecondaryStructure &data, PackContext & /*ctx*/) {
        return pack(*data.data());
    }
    template<>
    inline SharedSecondaryStructure unpack(const uchar *data, int length, int &offset, U2OpStatus &os, PackContext & /*ctx*/) {
        SharedSecondaryStructure result(new SecondaryStructure());
        result->type = SecondaryStructure::Type(unpack<char>(data, length, offset, os));
        CHECK_OP(os, result);

        result->chainIdentifier = unpack<char>(data, length, offset, os);
        CHECK_OP(os, result);

        result->chainIndex = unpackNum<int>(data, length, offset, os);
        result->startSequenceNumber = unpackNum<int>(data, length, offset, os);
        result->endSequenceNumber = unpackNum<int>(data, length, offset, os);
        return result;
    }

    template<class T>
    inline QByteArray packList(const QList<T> &data, PackContext &ctx) {
        QByteArray result;
        result += packNum<int>(data.size());
        foreach (const T &d, data) {
            result += pack(d, ctx);
        }
        return result;
    }
    template<class T>
    inline QList<T> unpackList(const uchar *data, int length, int &offset, U2OpStatus &os, PackContext &ctx) {
        QList<T> result;
        int size = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, result);
        for (int i=0; i<size; i++) {
            result << unpack<T>(data, length, offset, os, ctx);
            CHECK_OP(os, result);
        }
        return result;
    }

    inline QByteArray pack(const Molecule3DModel &data, PackContext &ctx) {
        QByteArray result;
        result += packList<SharedAtom>(data.atoms, ctx);
        result += packList<Bond>(data.bonds, ctx);
        return result;
    }
    template<>
    inline Molecule3DModel unpack(const uchar *data, int length, int &offset, U2OpStatus &os, PackContext &ctx) {
        Molecule3DModel result;
        result.atoms = unpackList<SharedAtom>(data, length, offset, os, ctx);
        CHECK_OP(os, result);
        result.bonds = unpackList<Bond>(data, length, offset, os, ctx);
        CHECK_OP(os, result);
        return result;
    }

    template<class KeyT, class ValueT>
    inline QByteArray packMap(const QMap<KeyT, ValueT> &data, PackContext &ctx) {
        QByteArray result;
        result += packNum<int>(data.size());
        foreach (const KeyT &idx, data.keys()) {
            result += packNum<KeyT>(idx);
            result += pack(data[idx], ctx);
        }
        return result;
    }
    template<class KeyT, class ValueT>
    inline QMap<KeyT, ValueT> unpackMap(const uchar *data, int length, int &offset, U2OpStatus &os, PackContext &ctx) {
        QMap<KeyT, ValueT> result;
        int size = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, result);
        for (int i=0; i<size; i++) {
            KeyT key = unpackNum<KeyT>(data, length, offset, os);
            CHECK_OP(os, result);
            ValueT value = unpack<ValueT>(data, length, offset, os, ctx);
            CHECK_OP(os, result);
            result[key] = value;
        }
        return result;
    }

    inline QByteArray pack(const MoleculeData &data, PackContext &ctx) {
        QByteArray result;
        result += packMap<ResidueIndex, SharedResidue>(data.residueMap, ctx);
        result += packList<Molecule3DModel>(data.models, ctx);
        result += pack(data.name);
        result += packBool(data.engineered);
        return result;
    }
    inline QByteArray pack(const SharedMolecule &data, PackContext &ctx) {
        return pack(*data.data(), ctx);
    }
    template<>
    inline SharedMolecule unpack(const uchar *data, int length, int &offset, U2OpStatus &os, PackContext &ctx) {
        SharedMolecule result(new MoleculeData());
        result->residueMap = unpackMap<ResidueIndex, SharedResidue>(data, length, offset, os, ctx);
        CHECK_OP(os, result);
        result->models = unpackList<Molecule3DModel>(data, length, offset, os, ctx);
        CHECK_OP(os, result);
        result->name = unpack<QString>(data, length, offset, os);
        CHECK_OP(os, result);
        result->engineered = unpackBool(data, length, offset, os);
        return result;
    }

    inline QByteArray pack(const AtomCoordSet &data, PackContext &ctx) {
        QByteArray result;
        result += packNum<int>(data.size());
        foreach (int idx, data.keys()) {
            result += packNum<int>(idx);
            result += pack(data[idx], ctx);
        }
        return result;
    }
    template<>
    inline AtomCoordSet unpack(const uchar *data, int length, int &offset, U2OpStatus &os, PackContext &ctx) {
        AtomCoordSet result;
        int size = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, result);
        for (int i=0; i<size; i++) {
            int key = unpackNum<int>(data, length, offset, os);
            CHECK_OP(os, result);
            SharedAtom value = unpack<SharedAtom>(data, length, offset, os, ctx);
            CHECK_OP(os, result);
            result[key] = value;
        }
        return result;
    }
}

QByteArray BioStruct3DSerializer::serialize(const BioStruct3D &bioStruct) {
    PackContext ctx;
    QByteArray result;
    result += packMap<int, SharedMolecule>(bioStruct.moleculeMap, ctx);
    result += packMap<int, AtomCoordSet>(bioStruct.modelMap, ctx);
    result += packList<SharedSecondaryStructure>(bioStruct.secondaryStructures, ctx);
    result += packList<Bond>(bioStruct.interMolecularBonds, ctx);
    result += pack(bioStruct.descr);
    result += pack(bioStruct.pdbId);
    result += packNum<double>(bioStruct.getRadius());
    result += pack(bioStruct.getCenter());
    result += pack(bioStruct.getTransform());
    return result;
}

BioStruct3D BioStruct3DSerializer::deserialize(const QByteArray &binary, U2OpStatus &os) {
    const uchar *data = (const uchar*)(binary.data());
    int offset = 0;
    int length = binary.length();

    PackContext ctx;
    BioStruct3D result;
    result.moleculeMap = unpackMap<int, SharedMolecule>(data, length, offset, os, ctx);
    CHECK_OP(os, result);
    result.modelMap = unpackMap<int, AtomCoordSet>(data, length, offset, os, ctx);
    CHECK_OP(os, result);
    result.secondaryStructures = unpackList<SharedSecondaryStructure>(data, length, offset, os, ctx);
    CHECK_OP(os, result);
    result.interMolecularBonds = unpackList<Bond>(data, length, offset, os, ctx);
    CHECK_OP(os, result);
    result.descr = unpack<QString>(data, length, offset, os);
    CHECK_OP(os, result);
    result.pdbId = unpack<QByteArray>(data, length, offset, os);
    CHECK_OP(os, result);
    result.setRadius(unpackNum<double>(data, length, offset, os));
    CHECK_OP(os, result);
    result.setCenter(unpack<Vector3D>(data, length, offset, os));
    CHECK_OP(os, result);
    result.setTransform(unpack<Matrix44>(data, length, offset, os));
    return result;
}

} // U2
