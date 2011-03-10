#ifndef _U2_CORE_ATTRIBUTES_H_
#define _U2_CORE_ATTRIBUTES_H_

#include <U2Core/global.h>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
// Reserved attribute names

class U2CORE_EXPORT U2BaseAttributeName {

    /** 
    Reserved to represent length: sequence or alignment 
    This attribute can be artificial and derived directly from structure values 
    If structure length is updated, system is responsible to update the attribute too
    */
    static const QString length;//          = "length";

    /** 
    Reserved to represent alphabet: sequence or alignment 
    This attribute can be artificial and derived directly from structure values 
    If structure alphabet is updated, system is responsible to update the attribute too
    */
    static const QString alphabet;//        = "alphabet";

    /** 
    Reserved to represent GC-content: sequence or alignment 
    This attribute can be artificial and derived directly from structure values 
    If structure gc-content is updated, system is responsible to update the attribute too
    */
    static const QString gc_content;//      = "GC-content";

    /** Structure modification date */
    static const QString update_time;//     = "update-time";

    /** Structure creation date */
    static const QString create_date; //    = "creation-time";

};

#endif
