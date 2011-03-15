/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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


#ifndef _U2_UINDEX_VIEW_WIDGET_FIELD_KEY_H_
#define _U2_UINDEX_VIEW_WIDGET_FIELD_KEY_H_

#include <QString>
#include <QList>

namespace U2 {

enum UIndexKeySimpleRuleType {
    BAD_TYPE = -1,
    ALL_VALUES,
    STARTS_WITH,
    ENDS_WITH,
    CONTAINS,
    EQUAL,
    NOT_EQUAL,
    // REGEXP,
    MORE_THAN,
    LESS_THAN,
    IN_RANGE
}; // UIndexKeyRuleType

enum UIndexKeyRuleOperation {
    BAD_OP = -1,
    AND,
    OR,
    NOT
}; // UIndexKeyRuleOperation

struct UIndexKeyRuleItem {
    UIndexKeySimpleRuleType       type;
    QString                       data;
    UIndexKeyRuleOperation        op;
    QList< UIndexKeyRuleItem* >   children;
    
    UIndexKeyRuleItem( const UIndexKeySimpleRuleType& t = ALL_VALUES, const QString& data = QString::null,
                       const UIndexKeyRuleOperation& op = BAD_OP );
    
    bool isOperationItem() const;
    
}; // UIndexKeyRuleItem

/*
 * A rule is represented as a tree of boolean operations ( AND, OR, NOT )
 * and simple rules as a lists of that tree
 */
class UIndexKeyRule {
public:
    UIndexKeyRule( UIndexKeyRuleItem* root = NULL );
    ~UIndexKeyRule();
    void delItem( UIndexKeyRuleItem* it );
    
    bool isSimple() const { return NULL == root || root->children.isEmpty(); }
    
    bool exec( const QString& val );
    
private:
    UIndexKeyRule( const UIndexKeyRule& );
    UIndexKeyRule& operator =( const UIndexKeyRule& );
    
    bool execItem( UIndexKeyRuleItem* item, const QString& val );
    bool execSimpleItem( UIndexKeyRuleItem* item, const QString& val );
    
public:
    UIndexKeyRuleItem* root;
    
}; // UIndexKeyRule

enum UIndexKeyType {
    TYPE_STR,
    TYPE_NUM
}; // UIndexKeyType

struct UIndexKey {
    QString         keyName;
    UIndexKeyType   type;
    UIndexKeyRule*   rule;
    
    UIndexKey( const QString& name = QString::null, const UIndexKeyType& t = TYPE_STR, UIndexKeyRule* r = NULL );
    ~UIndexKey();
    
    bool isEmpty() const;
    
}; // UIndexKey

} // U2

#endif // _U2_UINDEX_VIEW_WIDGET_FIELD_KEY_H_
