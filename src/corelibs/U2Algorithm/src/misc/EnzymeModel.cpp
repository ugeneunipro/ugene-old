#include "EnzymeModel.h"

namespace U2 {

const QString EnzymeSettings::DATA_DIR_KEY("enzymes");
const QString EnzymeSettings::DATA_FILE_KEY("plugin_enzymes/lastFile");
const QString EnzymeSettings::LAST_SELECTION("plugin_enzymes/selection");
const QString EnzymeSettings::ENABLE_HIT_COUNT("plugin_enzymes/enable_hit_count");
const QString EnzymeSettings::MAX_HIT_VALUE("plugin_enzymes/max_hit_value");
const QString EnzymeSettings::MIN_HIT_VALUE("plugin_enzymes/min_hit_value");
const QString EnzymeSettings::NON_CUT_REGION("plugin_enzymes/non_cut_region");
const QString EnzymeSettings::MAX_RESULTS("plugin_enzymes/max_results");
const QString EnzymeSettings::COMMON_ENZYMES( "ClaI,BamHI,BglII,DraI,EcoRI,EcoRV,HindIII,PstI,SalI,SmaI,XmaI");

EnzymeData::EnzymeData() {
    cutDirect = ENZYME_CUT_UNKNOWN;
    cutComplement = ENZYME_CUT_UNKNOWN;
    alphabet = NULL;
}

} // namespace
