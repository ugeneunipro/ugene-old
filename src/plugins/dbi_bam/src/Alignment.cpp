#include "Alignment.h"

namespace U2 {
namespace BAM {

// Alignment::CigarOperation
Alignment::CigarOperation::CigarOperation(int length, Operation operation):
    length(length),
    operation(operation)
{
}

int Alignment::CigarOperation::getLength()const {
    return length;
}

Alignment::CigarOperation::Operation Alignment::CigarOperation::getOperation()const {
    return operation;
}

void Alignment::CigarOperation::setLength(int length) {
    this->length = length;
}

void Alignment::CigarOperation::setOperation(Operation operation) {
    this->operation = operation;
}

// Alignment

Alignment::Alignment():
    referenceId(-1),
    position(-1),
    bin(0),
    mapQuality(0xff),
    flags(0),
    nextReferenceId(-1),
    nextPosition(-1),
    templateLength(0)
{
}

int Alignment::getReferenceId()const {
    return referenceId;
}

int Alignment::getPosition()const {
    return position;
}

int Alignment::getBin()const {
    return bin;
}

int Alignment::getMapQuality()const {
    return mapQuality;
}

Alignment::Flags Alignment::getFlags()const {
    return flags;
}

int Alignment::getNextReferenceId()const {
    return nextReferenceId;
}

int Alignment::getNextPosition()const {
    return nextPosition;
}

int Alignment::getTemplateLength()const {
    return templateLength;
}

const QByteArray &Alignment::getName()const {
    return name;
}

const QList<Alignment::CigarOperation> &Alignment::getCigar()const {
    return cigar;
}

const QByteArray &Alignment::getSequence()const {
    return sequence;
}

const QByteArray &Alignment::getQuality()const {
    return quality;
}

const QMap<QByteArray, QVariant> &Alignment::getOptionalFields()const {
    return optionalFields;
}

void Alignment::setReferenceId(int referenceId) {
    this->referenceId = referenceId;
}

void Alignment::setPosition(int position) {
    this->position = position;
}

void Alignment::setBin(int bin) {
    this->bin = bin;
}

void Alignment::setMapQuality(int mapQuality) {
    this->mapQuality = mapQuality;
}

void Alignment::setFlags(Flags flags) {
    this->flags = flags;
}

void Alignment::setNextReferenceId(int nextReferenceId) {
    this->nextReferenceId = nextReferenceId;
}

void Alignment::setNextPosition(int nextPosition) {
    this->nextPosition = nextPosition;
}

void Alignment::setTemplateLength(int templateLength) {
    this->templateLength = templateLength;
}

void Alignment::setName(const QByteArray &name) {
    this->name = name;
}

void Alignment::setCigar(const QList<CigarOperation> &cigar) {
    this->cigar = cigar;
}

void Alignment::setSequence(const QByteArray &sequence) {
    this->sequence = sequence;
}

void Alignment::setQuality(const QByteArray &quality) {
    this->quality = quality;
}

void Alignment::setOptionalFields(const QMap<QByteArray, QVariant> &optionalFields) {
    this->optionalFields = optionalFields;
}

int Alignment::computeLength(const Alignment &alignment) {
    int length = 0;
    foreach(const Alignment::CigarOperation &operation, alignment.getCigar()) {
        if((operation.getOperation() != Alignment::CigarOperation::Insertion)) {
            length += operation.getLength();
        }
    }
    return length;
}

} // namespace BAM
} // namespace U2
