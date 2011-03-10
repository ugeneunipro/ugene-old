
#include <cassert>

#include <QtCore/QStringList>

#include "UIndexViewWidgetKey.h"

namespace {

using namespace U2;

bool booleanResStart( UIndexKeyRuleOperation op ) {
    if( AND == op ) {
        return true;
    } else if( OR == op ) {
        return false;
    } else {
        assert( 0 );
    }
    return false;
}

bool booleanResContinue( bool& oldRes, bool newRes, UIndexKeyRuleOperation op ) {
    if( AND == op ) {
        oldRes = oldRes && newRes;
        if( true == oldRes ) {
            return true;
        }
        return false;
    } else if( OR == op ) {
        oldRes = oldRes || newRes;
        if( false == oldRes ) {
            return true;
        }
        return false;
    } else {
        assert( 0 );
    }
    return false;
}

} // anonymous namespace

namespace U2 {

UIndexKeyRuleItem::UIndexKeyRuleItem( const UIndexKeySimpleRuleType& t, const QString& d,
                                      const UIndexKeyRuleOperation& o )
: type( t ), data( d ), op( o ) {
}

bool UIndexKeyRuleItem::isOperationItem() const {
    if( BAD_OP != op ) {
        assert( BAD_TYPE == type );
        return true;
    }
    return false;
}

UIndexKeyRule::UIndexKeyRule( UIndexKeyRuleItem* r ) : root( r ) {
}

UIndexKeyRule::~UIndexKeyRule() {
    delItem( root );
}

void UIndexKeyRule::delItem( UIndexKeyRuleItem* it ) {
    if( NULL == it ) {
        return;
    }
    foreach( UIndexKeyRuleItem* chld, it->children ) {
        delItem( chld );
    }
    delete it;
}

bool UIndexKeyRule::exec( const QString& val ) {
    assert( !val.isEmpty() );
    assert( NULL != root );
    return execItem( root, val );
}

bool UIndexKeyRule::execItem( UIndexKeyRuleItem* item, const QString& val ) {
    assert( NULL != item && !val.isEmpty() );
    bool res = true;
    int chldSz = item->children.size();
    
    if( item->isOperationItem() ) {
        switch( item->op ) {
        case AND:
        case OR:
            {
                assert( 0 != chldSz );
                res = booleanResStart( item->op );
                for( int i = 0; i < chldSz; ++i ) {
                    bool chldRes = execItem( item->children.at( i ), val );
                    if( booleanResContinue( res, chldRes, item->op ) ) {
                        continue;
                    }
                    break;
                }
            }
            break;
        case NOT:
            assert( 1 != chldSz );
            res = !execItem( item->children.first(), val );
            break;
        default:
            assert( 0 );
        }
    } else { // simple items
        res = execSimpleItem( item, val );
    }
    
    return res;
}

bool UIndexKeyRule::execSimpleItem( UIndexKeyRuleItem* item, const QString& val ) {
    assert( NULL != item && !val.isEmpty() );
    UIndexKeySimpleRuleType t = item->type;
    assert( BAD_TYPE != t );
    bool res = false;
    
    switch( t ) {
        case ALL_VALUES:
            res = true;
            break;
        case STARTS_WITH:
            res = val.startsWith( item->data, Qt::CaseInsensitive );
            break;
        case ENDS_WITH:
            res = val.endsWith( item->data,  Qt::CaseInsensitive );
            break;
        case CONTAINS:
            res = val.contains( item->data,  Qt::CaseInsensitive );
            break;
        case EQUAL:
            res = val == item->data;
            break;
        case NOT_EQUAL:
            res = val != item->data;
            break;
        case MORE_THAN:
            {
                bool ok1 = false;
                bool ok2 = false;
                int intData = val.toInt( &ok1 );
                int intVal = item->data.toInt( &ok2 );
                res = ok1 && ok2 && ( intData >= intVal );
            }
            break;
        case LESS_THAN:
            {
                bool ok1 = false;
                bool ok2 = false;
                int intData = val.toInt( &ok1 );
                int intVal = item->data.toInt( &ok2 );
                res = ok1 && ok2 && ( intData <= intVal );
            }
            break;
        case IN_RANGE:
            {
                bool ok1 = false;
                bool ok2 = false;
                bool ok3 = false;
                QStringList numList = item->data.split( " ", QString::SkipEmptyParts );
                if( 2 != numList.size() ) {
                    res = false;
                    break;
                }
                int intData = val.toInt( &ok1 );
                int intValMin = numList.at( 0 ).toInt( &ok2 );
                int intValMax = numList.at( 1 ).toInt( &ok3 );
                res = ok1 && ok2 && ok3 && ( intValMin <= intData && intData <= intValMax );
            }
            break;
        default:
            assert( 0 );
    }
    return res;
}

UIndexKey::UIndexKey(const QString &name, const UIndexKeyType &t, UIndexKeyRule* r )
: keyName( name ), type( t ), rule( r ) {
}

UIndexKey::~UIndexKey() {
    delete rule;
}

bool UIndexKey::isEmpty() const {
    return keyName.isEmpty();
}

} // U2
