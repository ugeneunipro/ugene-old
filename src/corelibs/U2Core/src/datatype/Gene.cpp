#include "Gene.h"

namespace U2 {

Gene::Gene(const U2DataId& _featureId, QString& _name, const QString& _accession, const QString& _altname, const QString& _note, const U2Region& _region, const U2Region& _cdsRegion, QVector<U2Region>& _exons, bool complemented)
:featureId(_featureId)
,name(_name)
,accession(_accession)
,note(_note)
,region(_region)
,cdsRegion(_cdsRegion)
,exons(_exons)
,complement(complemented)
,altname(_altname)
,disease("")
{
}

const QString &Gene::getName() const {
    return name;
}

const QString &Gene::getAltName() const {
    return altname;
}

const QString &Gene::getAccession() const {
    return accession;
}

const QString &Gene::getNote() const {
    return note;
}

const U2Region &Gene::getRegion() const {
    return region;
}

const QString & Gene::getDisease() const{
    return disease;
}

const QString & Gene::getType() const{
    return type;
}

const QVector<U2Region> &Gene::getExons() const {
    return exons;
}

const U2Region & Gene::getCDSRegion() const{
    return cdsRegion;
}

const U2DataId & Gene::getFeatureId() const{
    return featureId;
}

bool Gene::isCoding() const{
//     //old annotations DB format
//     if(cdsRegion.startPos == cdsRegion.endPos() && cdsRegion.startPos == region.startPos ){
//             return false;
//     }
// 
//     //new annotations DB format
//     if(cdsRegion == region){
//         return false;
//     }

//all genes are considered coding
    return true;
}
bool Gene::isComplemented() const {
    return complement;
}

void Gene::setFeatureId( const U2DataId& _featureId ){
    featureId = _featureId;
}

void Gene::setName( const QString& _name ){
    name = _name;
}

void Gene::setAccession( const QString& _accession ){
    accession = _accession;
}

void Gene::setAltName( const QString& _altName ){
    altname = _altName;
}

void Gene::setNote( const QString& _note ){
    note = _note;
}

void Gene::setRegion( const U2Region& _region ){
    region = _region;
}

void Gene::setCDSRegion( const U2Region& _cdsRegion ){
    cdsRegion = _cdsRegion;
}

void Gene::setExons( const QVector<U2Region>& _exons ){
    exons = _exons;
}

void Gene::setComplement( bool _complemented ){
    complement = _complemented;
}

void Gene::setDisease( const QString& _disease ){
    disease = _disease;
}

void Gene::setType( const QString& _type ){
    type = _type;
}

} //namespace
