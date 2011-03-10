#include "AnnotationData.h"

namespace U2 {

QDataStream& operator>>(QDataStream& dataStream, TriState& state) {
    int st;
    dataStream >> st;
    switch (st) {
        case 0: state = TriState_Yes; break;
        case 1: state = TriState_No; break;
        default: state = TriState_Unknown;
    }
    return dataStream;
}

QDataStream& operator<<(QDataStream& dataStream, const TriState& state) {
    switch (state) {
        case TriState_Yes: return dataStream << 0;
        case TriState_No: return dataStream << 1;
        default: return dataStream << 2;
    }
}

QDataStream& operator>>(QDataStream& dataStream, U2Qualifier& q) {
    return dataStream >> q.name >> q.value;
}

QDataStream& operator<<(QDataStream& dataStream, const U2Qualifier& q) {
    return dataStream << q.name << q.value;
}

QDataStream& operator>>(QDataStream& dataStream, AnnotationData& data) {
    assert(0);
    return dataStream; 
//    return dataStream >> data.name >> data.location >> data.qualifiers;
}

QDataStream& operator<<(QDataStream& dataStream, const AnnotationData& data) {
    assert(0);
    return dataStream; 
//    return dataStream << data.name << data.location << data.qualifiers;
}

} //namespace
