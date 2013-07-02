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

#include "TextConversionUtils.h"

#include "globals.h"

extern "C" {

U2SCRIPT_EXPORT const char *getErrorString( U2ErrorType type ) {
    switch ( type ) {
    case U2_OK :
        return "success";
    case U2_INVALID_CALL :
        return "the action cannot be performed";
    case U2_INVALID_NAME :
        return "the element does not have the given identifier";
    case U2_INVALID_PATH :
        return "the path does not exist";
    case U2_INVALID_SCHEME :
        return "the scheme object contains inconsistent data";
    case U2_INVALID_STRING :
        return "the supplied string cannot be used";
    case U2_ELEMENT_NOT_FOUND :
        return "the element does not present in the scheme";
    case U2_UNKNOWN_ELEMENT :
        return "the element does not exist";
    case U2_TOO_SMALL_BUFFER :
        return "the result string does not fit the buffer";
    case U2_FAILED_TO_CREATE_FILE :
        return "unable to create file";
    case U2_FAILED_TO_CREATE_DIR :
        return "unable to create directory";
    case U2_ELEMENT_NOT_SUIT_SAS :
        return "the requested algorithm cannot be used in SAS";
    case U2_FAILED_TO_READ_FILE :
        return "unable to read file";
    case U2_FAILED_TO_REMOVE_TMP_FILE :
        return "unable to remove temporary file";
    case U2_NOT_ENOUGH_MEMORY :
        return "the operation ran out of memory";
    case U2_NUM_ARG_OUT_OF_RANGE :
        return "the given numeric value is illegal";
    default :
        return "unknown error code";
    }
}

U2SCRIPT_EXPORT U2ErrorType getLastFailedStringValue( int maxExpectedLength, char *destination,
    int *requiredSize )
{
    return U2::TextConversionUtils::repeatLastConversion( maxExpectedLength, destination,
        requiredSize );
}

};