/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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
