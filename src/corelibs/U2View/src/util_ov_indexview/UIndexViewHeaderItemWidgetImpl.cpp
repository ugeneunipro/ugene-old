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


#include <QtGui/QAction>

#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>

#include <cassert>

#include "UIndexViewHeaderItemWidgetImpl.h"

namespace U2 {

QMap< UIndexKeySimpleRuleType, QString >        UIndexViewHeaderItemWidgetImpl::ruleTypeMap;
QMap< UIndexKeySimpleRuleType, ValueDataType >  UIndexViewHeaderItemWidgetImpl::ruleValueMap;
QAtomicInt                                      UIndexViewHeaderItemWidgetImpl::rulesMapInitialized;

void UIndexViewHeaderItemWidgetImpl::initializeRulesMap() {
    ruleTypeMap[ALL_VALUES]     = tr("[All values] filter");
    ruleTypeMap[STARTS_WITH]    = tr("[Starts with] filter ");
    ruleTypeMap[ENDS_WITH]      = tr("[Ends with] filter");
    ruleTypeMap[CONTAINS]       = tr("[Contains] filter");
    ruleTypeMap[EQUAL]          = tr("[Equal] filter");
    ruleTypeMap[NOT_EQUAL]      = tr("[Not equal] filter");
    ruleTypeMap[MORE_THAN]      = tr("[Greater than] filter");
    ruleTypeMap[LESS_THAN]      = tr("[Lower than] filter");
    ruleTypeMap[IN_RANGE]       = tr("[In range] filter");
    
    ruleValueMap[ALL_VALUES]    = NO_VALUE_TYPE;
    ruleValueMap[STARTS_WITH]   = STR_TYPE;
    ruleValueMap[ENDS_WITH]     = STR_TYPE;
    ruleValueMap[CONTAINS]      = STR_TYPE;
    ruleValueMap[EQUAL]         = STR_TYPE;
    ruleValueMap[NOT_EQUAL]     = STR_TYPE;
    ruleValueMap[MORE_THAN]     = NUM_TYPE;
    ruleValueMap[LESS_THAN]     = NUM_TYPE;
    ruleValueMap[IN_RANGE]      = NUM_NUM_TYPE;
}

UIndexViewHeaderItemWidgetImpl::UIndexViewHeaderItemWidgetImpl(QWidget* p, const QStringList& nsl,
                                                               const QList< UIndexKeyType >& ktl, UIndexKey* k )
: QWidget( p ), key( k ), keyNamesList(nsl), keyTypesList(ktl) 
{
    assert( !keyNamesList.isEmpty() );
    setupUi( this );

    SELECTED_KEY_LABEL_DEFAULT = tr( "Select new key" );

    INPUT_VALUES_TITLE     = tr( "Enter a filter value" );
    GREETING_STR_TYPE      = tr( "Enter a string value" );
    GREETING_NUM_TYPE      = tr( "Enter a number value" );
    GREETING1_NUM_MUM_TYPE = tr( "Enter a lower bound" );
    GREETING2_NUM_MUM_TYPE = tr( "Enter an upper bound" );


    if( rulesMapInitialized.testAndSetAcquire( 0, 1 ) ) {
        UIndexViewHeaderItemWidgetImpl::initializeRulesMap();
    }
    
    if( NULL == key ) {
        selectedKeyLabel->setText( SELECTED_KEY_LABEL_DEFAULT );
    } else {
        assert( keyNamesList.contains( key->keyName ) );
        selectedKeyLabel->setText( key->keyName );
    }
    buildMenu();
    connect( selectKeyButton, SIGNAL( clicked() ), SLOT( sl_selectKeyButtonClicked() ) );
}

void UIndexViewHeaderItemWidgetImpl::buildMenu() {
    selectKeyButtonMenu = new QMenu( this );
    
    QAction* noneAction = selectKeyButtonMenu->addAction( keyNamesList.first() );
    connect( noneAction, SIGNAL( triggered() ), SLOT( sl_noneKeySelected() ) );
    selectKeyButtonMenu->addSeparator();
    
    int sz = keyNamesList.size();
    for( int i = 1; i < sz; ++i ) {
        QMenu* sub = selectKeyButtonMenu->addMenu( keyNamesList.at( i ) );
        buildSubMenu( sub, keyTypesList.at( i ) );
    }
}

void UIndexViewHeaderItemWidgetImpl::buildSubMenu( QMenu* sub, UIndexKeyType t ) {
    assert( NULL != sub );
    QMap< UIndexKeySimpleRuleType, QString >::const_iterator it = ruleTypeMap.constBegin();
    while( ruleTypeMap.constEnd() != it ) {
        if( STR_TYPE == ruleValueMap[it.key()] || TYPE_NUM == t ) {
            QAction* curFilterAction = sub->addAction( it.value() );
            connect( curFilterAction, SIGNAL( triggered() ), SLOT( sl_filterSelected() ) );
        }
        ++it;
    }
}

void UIndexViewHeaderItemWidgetImpl::sl_noneKeySelected() {
    selectedKeyLabel->setText( SELECTED_KEY_LABEL_DEFAULT );
    emit si_stateChanged( NULL );
}

void UIndexViewHeaderItemWidgetImpl::sl_selectKeyButtonClicked() {
    selectKeyButtonMenu->exec( QCursor::pos() );
}

void UIndexViewHeaderItemWidgetImpl::sl_filterSelected() {
    QAction* filterAction = qobject_cast< QAction* >( sender() );
    
    QList< UIndexKeySimpleRuleType > typesList = ruleTypeMap.keys( filterAction->text() );
    assert( !typesList.isEmpty() );
    UIndexKeySimpleRuleType ruleType = typesList.first();
    QString data = getData( ruleType );
    if( data.isEmpty() ) {
        return;
    }
    
    QString keyName = getKeyName( filterAction );
    assert( keyNamesList.contains( keyName ) );
    selectedKeyLabel->setText( keyName );
    
    UIndexKeyRule* rule = new UIndexKeyRule( new UIndexKeyRuleItem( ruleType, data, BAD_OP ) );
    UIndexKeyType keyType = ( NULL == key )? TYPE_STR: key->type;
    UIndexKey* newKey = new UIndexKey( keyName, keyType, rule );
    emit si_stateChanged( newKey );
}

QString UIndexViewHeaderItemWidgetImpl::getKeyName( QAction* action ) {
    assert( NULL != action );
    QMenu* parentMenu = qobject_cast< QMenu* >( action->parentWidget() );
    assert( NULL != parentMenu );
    return parentMenu->title();
}

QString UIndexViewHeaderItemWidgetImpl::getData( const UIndexKeySimpleRuleType& keyType ) {
    QString ret;
    ValueDataType valueType = ruleValueMap[keyType];
    
    switch( valueType ) {
    case NO_VALUE_TYPE:
        ret = " "; // not empty is important
        break;
    case STR_TYPE:
        ret = getString();
        break;
    case NUM_TYPE:
        ret = getNum();
        break;
    case NUM_NUM_TYPE:
        ret = getNumNum();
        break;
    default:
        assert( 0 );
    }
    return ret;
}

QString UIndexViewHeaderItemWidgetImpl::getString() {
    bool ok = false;
    QString ret;

    do {
        ret = QInputDialog::getText( this, INPUT_VALUES_TITLE, GREETING_STR_TYPE, QLineEdit::Normal, "", &ok );
        if( ok && ret.isEmpty() ) {
            QMessageBox::critical( this, tr( "Error" ), tr( "The value can't be empty" ) );
            continue;
        }
        break;
    }while( 1 );

    if( !ok ) {
        ret.clear();
    }
    return ret;
}

QString UIndexViewHeaderItemWidgetImpl::getNum() {
    bool ok = false;
    QString ret;
    int val = 0;

    val = QInputDialog::getInteger( this, INPUT_VALUES_TITLE, GREETING_NUM_TYPE, 0, -INT_MAX, INT_MAX, 1, &ok );
    if( ok ) {
        ret = QString::number( val );
    }
    return ret;
}

QString UIndexViewHeaderItemWidgetImpl::getNumNum() {
    bool ok = false;
    QString ret;
    int val1 = 0;
    int val2 = 0;

    do {
        ret.clear();
        val1 = QInputDialog::getInteger( this, INPUT_VALUES_TITLE, GREETING1_NUM_MUM_TYPE, 0, -INT_MAX, INT_MAX, 1, &ok );
        if( ok ) {
            ret = QString::number( val1 ) + " ";
            val2 = QInputDialog::getInteger( this, INPUT_VALUES_TITLE, GREETING2_NUM_MUM_TYPE, 0, -INT_MAX, INT_MAX, 1, &ok );
            if( ok ) {
                if( val2 < val1 ) {
                    QMessageBox::critical( this, tr( "Error!" ), tr( "Upper bound must be greater than lower" ) );
                    continue;
                }
                ret += QString::number( val2 );
            } else {
                ret.clear();
                break;
            }
        }
        break;
    }while( 1 );
    return ret;
}

} // U2
