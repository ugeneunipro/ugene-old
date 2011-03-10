#include "BaseAttributes.h"

static const QString URL_LOCATION_ATTR_ID("url_location");
static const QString URL_OUT_ATTR_ID("url-out");
static const QString URL_IN_ATTR_ID("url-in");
static const QString DOCUMENT_FORMAT_ATTR_ID("document-format");
static const QString APPEND_ATTR_ID("accumulate");
static const QString READ_BY_LINES_ATTR_ID("read-by-lines");
static const QString FILE_MODE_ATTR_ID("write-mode");

static const QString STRAND_ATTR_ID("strand");

namespace U2 {
namespace Workflow {

const Descriptor BaseAttributes::URL_IN_ATTRIBUTE() {
    return Descriptor(URL_IN_ATTR_ID, tr("Input files"), tr("Semicolon-separated list of paths to the input files."));
}

const Descriptor BaseAttributes::URL_OUT_ATTRIBUTE() {
    return Descriptor(URL_OUT_ATTR_ID, tr("Output file"), tr("Location of output data file. If this attribute is set," 
        " slot \"Location\" in port will not be used"));
}

const Descriptor BaseAttributes::URL_LOCATION_ATTRIBUTE() {
    return Descriptor(URL_LOCATION_ATTR_ID, tr("Located on"), tr("Machine file(s) are located on")); 
}

const Descriptor BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE() {
    return Descriptor(DOCUMENT_FORMAT_ATTR_ID, tr("Document format"), tr("Document format of output file"));
}

const Descriptor BaseAttributes::ACCUMULATE_OBJS_ATTRIBUTE() {
    return Descriptor(APPEND_ATTR_ID, tr("Accumulate objects"), 
        tr("Accumulate all incoming data in one file or create separate files for each input."
        "In the latter case, an incremental numerical suffix is added to the file name."));
}

const Descriptor BaseAttributes::READ_BY_LINES_ATTRIBUTE() {
    return Descriptor(READ_BY_LINES_ATTR_ID, tr("Read by lines"), tr("Read input file by lines"));
}

const Descriptor BaseAttributes::FILE_MODE_ATTRIBUTE() {
    return Descriptor(FILE_MODE_ATTR_ID, tr("Existing file"), tr("If a target file already exists, you can specify" 
        "how it should be handled: either overwritten, renamed" 
        " or appended (if supported by file format)."));
}

const Descriptor BaseAttributes::STRAND_ATTRIBUTE() {
    return Descriptor(STRAND_ATTR_ID, tr("Search in"), tr("Which strands should be searched: direct, complement or both."));
}

const QString BaseAttributes::STRAND_BOTH() {
    return "both";
}

const QString BaseAttributes::STRAND_DIRECT() {
    return "direct";
}

const QString BaseAttributes::STRAND_COMPLEMENTARY() {
    return "complementary";
}

const QVariantMap BaseAttributes::STRAND_ATTRIBUTE_VALUES_MAP() {
    QVariantMap strandMap; 
    strandMap[tr("both strands")] = STRAND_BOTH();
    strandMap[tr("direct strand")] = STRAND_DIRECT();
    strandMap[tr("complement strand")] = STRAND_COMPLEMENTARY();
    return strandMap;
}

} // Workflow
} // U2
