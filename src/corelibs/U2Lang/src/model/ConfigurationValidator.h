#ifndef _U2_WORKFLOW_CONFIGURATION_VALIDATOR_H_
#define _U2_WORKFLOW_CONFIGURATION_VALIDATOR_H_

#include <U2Lang/Descriptor.h>

namespace U2 {

class Configuration;

/**
 * base class for configuration validators
 * standard Configuration::validate does only simple validations: checks if required attribute is not empty
 * here you can set non-trivial validations
 */
class U2LANG_EXPORT ConfigurationValidator {
public:
    virtual ~ConfigurationValidator() { }
    virtual bool validate(const Configuration*, QStringList& output) const = 0;
    
}; // ConfigurationValidator

} // U2

#endif // _U2_WORKFLOW_CONFIGURATION_VALIDATOR_H_
