
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
