
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>

#include <QtCore/QList>
#include <U2Core/CreateFileIndexTask.h>

#include "UIndexViewWidgetKey.h"

#include "UIndexViewWidgetImpl.h"

namespace U2 {

const float UIndexViewWidgetImpl::SHOW_KEY_PERCENT_BORDER       = 30.0f;

UIndexViewWidgetImpl::UIndexViewWidgetImpl( QWidget* p, const UIndex& i ) : QWidget( p ), ind( i ) {
    VALUE_NOT_SELECTED          = tr( "No filter selected" );
    KEY_FORMAT                  = tr( "Document Format" );
    KEY_URL                     = tr( "URL" );
    KEY_NONE                    = tr( "None" );
    COLUMN_MENU_ACTION_HIDE     = tr( "Hide column" );
    COLUMN_MENU_ACTION_DELETE   = tr( "Remove column" );
    NO_HIDDEN_COLUMNS           = tr( "No hidden columns" );
    SHOW_HIDDEN_COLS_STR        = tr( "Show hidden columns:" );
    SHOW_HIDDEN_COLS_BEGIN      = tr( "column " );
    ADD_LAST_EMPTY_COL          = tr( "Add column" );
    
    setupUi( this );
    setContextMenuPolicy( Qt::CustomContextMenu );
    uindexTable->horizontalHeader()->setHighlightSections( false );
    uindexTable->horizontalHeader()->setClickable( true );
    uindexTable->setEditTriggers( QAbstractItemView::NoEditTriggers );
    uindexTable->setSelectionBehavior( QAbstractItemView::SelectRows );
    
    connect( uindexTable->horizontalHeader(), SIGNAL( sectionClicked( int ) ), SLOT( sl_horHeaderSectionClicked( int ) ) );
//    connect( manageColumsButton, SIGNAL( clicked() ), SLOT( sl_manageColumnsButtonClicked() ) );
    manageColumsButton->setVisible(false);
    
    initKeyNamesList();
    detectTypes();
    uindexTable->setRowCount( ind.items.size() + 1 );
    initTable();
    execRules();
    uindexTable->resizeColumnsToContents();
}

UIndexViewWidgetImpl::~UIndexViewWidgetImpl() {
    foreach( UIndexViewHeaderItemWidgetImpl* headerIt, headerItems ) {
        assert( NULL != headerIt );
        UIndexKey* key = headerIt->getKey();
        delete key;
    }
}

void UIndexViewWidgetImpl::initKeyNamesList() {
    keyNamesList << KEY_NONE;
    keyNamesList << KEY_FORMAT;
    keyNamesList << KEY_URL;
    
    foreach( const UIndex::ItemSection& item, ind.items ) {
        QHash< QString, QString >::const_iterator it = item.keys.constBegin();
        while( item.keys.constEnd() != it ) {
            if( !keyNamesList.contains( it.key() ) ) {
                keyNamesList << it.key();
            }
            ++it;
        }
    }
    sortKeyNamesList();
}

void UIndexViewWidgetImpl::sortKeyNamesList() {
    int i = 0;
    int sz = keyNamesList.size();
    
    for( i = 0; i < sz; ++i  ) {
        const QString& keyName = keyNamesList.at( i );
        if( CreateFileIndexTask::KEY_ACCESSION == keyName || CreateFileIndexTask::KEY_NAME == keyName || 
            CreateFileIndexTask::KEY_DESCRIPTION == keyName ) {
            keyNamesList.move( i, 1 );
        }
    }
}

void UIndexViewWidgetImpl::detectTypes() {
    int i = 0;
    int sz = keyNamesList.size();
    for( i = 0; i < sz; ++i ) {
        const QString& keyName = keyNamesList.at( i );
        if( KEY_NONE == keyName || KEY_FORMAT == keyName || KEY_URL == keyName ) {
            keyTypesList << TYPE_STR;
            continue;
        }
        bool isNumber = true;
        foreach( const UIndex::ItemSection& item, ind.items ) {
            if( item.keys.contains( keyName ) ) {
                bool ok = false;
                const QString& val = item.keys.value( keyName );
                val.toInt( &ok );
                isNumber = isNumber && ok;
                if( !isNumber ) {
                    break;
                }
            }
        }
        keyTypesList << ( ( isNumber )? TYPE_NUM: TYPE_STR );
    }
}

void UIndexViewWidgetImpl::initTable() {
    int curCol = 0;
    int i = 0;
    int sz = keyNamesList.size();
    
    for( i = 0; i < sz; ++i ) {
        const QString& keyName = keyNamesList.at( i );
        if( KEY_NONE == keyName || KEY_FORMAT == keyName || KEY_URL == keyName ) {
            continue;
        }
        if( isSignificantKey( keyName ) ) {
            UIndexKeyRule* rule = new UIndexKeyRule( new UIndexKeyRuleItem() );
            UIndexKey* key = new UIndexKey( keyName, TYPE_STR, rule );
            UIndexViewHeaderItemWidgetImpl* header = new UIndexViewHeaderItemWidgetImpl( this, keyNamesList, keyTypesList, key );
            insertHeaderItem( header );
            addColumn( header );
            setColumnVals( curCol );
            curCol++;
        }
    }
    sl_addLastEmptyCol();
}

bool UIndexViewWidgetImpl::isSignificantKey( const QString& keyName ) {
    float oneKeyPercent = ( 1.0f / ind.items.size() ) * 100.0f;
    float curPercentCount = 0.0f;
    bool res = false;
    foreach( const UIndex::ItemSection& it, ind.items ) {
        if( it.keys.contains( keyName ) ) {
            curPercentCount += oneKeyPercent;
        }
        if( SHOW_KEY_PERCENT_BORDER <= curPercentCount ) {
            res = true;
            break;
        }
    }
    return res;
}

void UIndexViewWidgetImpl::insertHeaderItem( UIndexViewHeaderItemWidgetImpl* headerIt ) {
    assert( NULL != headerIt );
    headerItems << headerIt;
    connect( headerIt, SIGNAL( si_stateChanged( UIndexKey* ) ), SLOT( sl_headerWidgetStateChanged( UIndexKey* ) ) );
}

void UIndexViewWidgetImpl::addColumn( UIndexViewHeaderItemWidgetImpl* headerIt ) {
    assert( NULL != headerIt );
    int ind = uindexTable->columnCount();
    
    uindexTable->insertColumn( ind );
    QTableWidgetItem* colItem = new QTableWidgetItem();
    uindexTable->setHorizontalHeaderItem( ind, colItem );
    uindexTable->setCellWidget( HEADER_ITEMS_ROW, ind, headerIt );
    setHorizontalHeaderLabel( ind );
}

QString UIndexViewWidgetImpl::getRuleStrRepresent( const UIndexKeyRule* r ) const {
    assert( NULL != r );
    QString ret;
    
    if( r->isSimple() ) {
        assert( NULL != r->root );
        ret = UIndexViewHeaderItemWidgetImpl::ruleTypeMap[r->root->type] + ": " + r->root->data;
    } else {
        ret = tr( "Complex rule" );
    }
    return ret;
}

void UIndexViewWidgetImpl::setHorizontalHeaderLabel( int col ) {
    assert( 0 <= col && uindexTable->columnCount() > col );
    UIndexKey* key = headerItems.at( col )->getKey();
    
    if( NULL == key || NULL == key->rule ) {
        uindexTable->horizontalHeaderItem( col )->setText( VALUE_NOT_SELECTED );
    } else {
        QString name = getRuleStrRepresent( key->rule );
        assert( !name.isEmpty() );
        uindexTable->horizontalHeaderItem( col )->setText( name );
    }
}

void UIndexViewWidgetImpl::setColumnVals( int col ) {
    assert( 0 <= col && uindexTable->columnCount() > col );
    UIndexKey* key = headerItems.at( col )->getKey();
    QList< UIndex::ItemSection > items = ind.items;
    int sz = items.size();
    int i = 0;
    
    if( NULL == key ) { // init with empty items
        for( i = 0; i < sz; ++i ) {
            QTableWidgetItem* it = new QTableWidgetItem( "" );
            uindexTable->setItem( i + 1, col, it );
        }
    } else {
        QString keyName = key->keyName;
        QString val;
        for( i = 0; i < sz; ++i ) {
            UIndex::ItemSection curIt = items[i];
            
            if( KEY_FORMAT == keyName ) {
                val = curIt.docFormat;
            } else if( KEY_URL == keyName ) {
                UIndex::IOSection curIO = ind.getIOSection( curIt.ioSectionId );
                assert( !curIO.sectionId.isEmpty() && !curIO.url.isEmpty() );
                val = curIO.url;
            } else {
                val = curIt.keys.value( keyName );
            }
            QTableWidgetItem* it = new QTableWidgetItem( val );
            uindexTable->setItem( i + 1, col, it );
        }
    }
}

void UIndexViewWidgetImpl::sl_headerWidgetStateChanged( UIndexKey* newKey ) {
    UIndexViewHeaderItemWidgetImpl* headerItem = qobject_cast< UIndexViewHeaderItemWidgetImpl* >( sender() );
    assert( NULL != headerItem );
    
    int ind = headerItems.indexOf( headerItem );
    UIndexKey* oldKey = headerItem->getKey();
    delete oldKey;

    headerItem->setKey( newKey );
    setHorizontalHeaderLabel( ind );
    
    if( uindexTable->columnCount() - 1 == ind && NULL != newKey ) {
        UIndexViewHeaderItemWidgetImpl* newHeaderIt = new UIndexViewHeaderItemWidgetImpl( this, keyNamesList, keyTypesList, NULL );
        insertHeaderItem( newHeaderIt );
        addColumn( newHeaderIt );
    }
    setColumnVals( ind );
    execRules();
    uindexTable->resizeColumnsToContents();
}

bool UIndexViewWidgetImpl::execOneRule( int col, const QString& val ) {
    assert( 0 <= col && uindexTable->columnCount() > col );
    
    if( val.isEmpty() ) {
        return false;
    }
    UIndexKey* key = headerItems.at( col )->getKey();
    assert( NULL != key );
    UIndexKeyRule* rule = key->rule;
    assert( NULL != rule );
    return rule->exec( val );
}

bool UIndexViewWidgetImpl::rulesPassed( int row ) {
    assert( 0 <= row && uindexTable->rowCount() > row );
    int colSz = uindexTable->columnCount();
    int i = 0;
    bool res = true;
    
    for( i = 0; i < colSz; ++i ) {
        if( isEmptyCol( i ) ) {
            continue;
        }
        QString val = uindexTable->item( row, i )->text();
        res = res && execOneRule( i, val );
        if( false == res ) {
            break;
        }
    }
    return res;
}

void UIndexViewWidgetImpl::execRules() {
    int rowCount = uindexTable->rowCount();
    int i = 0;
    bool hasNotEmptyColumn = hasNotEmptyCol();
    
    for( i = 1; i < rowCount; ++i ) {
        bool showThis = hasNotEmptyColumn && rulesPassed( i );
        if( showThis ) {
            uindexTable->showRow( i );
        } else {
            uindexTable->hideRow( i );
        }
    }
}

bool UIndexViewWidgetImpl::isEmptyCol( int col ) const{
    assert( 0 <= col && uindexTable->columnCount() > col );
    UIndexViewHeaderItemWidgetImpl* headerIt = headerItems.at( col );
    assert( NULL != headerIt );
    return NULL == headerIt->getKey();
}

bool UIndexViewWidgetImpl::hasNotEmptyCol() const {
    int colCount = uindexTable->columnCount();
    int i = 0;
    bool res = false;
    
    for( i = 0; i < colCount; ++i ) {
        res = res || !isEmptyCol( i );
        if( res ) {
            break;
        }
    }
    return res;
}

void UIndexViewWidgetImpl::sl_horHeaderSectionClicked( int col ) {
    QMenu columnMenu(this);
    QAction* delAction = NULL;
    QAction* hideAction = NULL;
    if (col!=0 && col + 1 != uindexTable->columnCount()) {
        delAction = columnMenu.addAction(tr("Remove column"));
        delAction->setParent(&columnMenu);
    }
    if (columnMenu.isEmpty()) {
        return;
    }
    QAction* selected = columnMenu.exec( QCursor::pos() );
    if (selected == NULL) {
        return;
    }
    if( selected == hideAction) {
        uindexTable->hideColumn( col );
        assert( !hiddenColumns.contains( col ) );
        hiddenColumns << col;
    } else if( selected == delAction) {
        removeColumn( col );
        execRules();
    } 
}

void UIndexViewWidgetImpl::removeColumn( int col ) {
    assert( 0 <= col && uindexTable->columnCount() > col );
    uindexTable->removeColumn( col );
    UIndexKey* key = headerItems.at( col )->getKey();
    delete key;

    headerItems.removeAt( col );
}

void UIndexViewWidgetImpl::sl_manageColumnsButtonClicked() {
    QMenu manageColsMenu;
    
    if( hiddenColumns.isEmpty() ) {
        manageColsMenu.addAction( NO_HIDDEN_COLUMNS )->setParent(&manageColsMenu);
    } else {
        manageColsMenu.addAction( SHOW_HIDDEN_COLS_STR )->setParent(&manageColsMenu);
        manageColsMenu.addSeparator();
        foreach( int colNum, hiddenColumns ) {
            assert( 0 <= colNum && uindexTable->columnCount() > colNum );
            UIndexViewHeaderItemWidgetImpl* head = headerItems.at( colNum );
            UIndexKey* key = head->getKey();
            QString keyName = ( NULL == key )? keyNamesList.first(): key->keyName;
            QString showColStr = SHOW_HIDDEN_COLS_BEGIN + QString::number( colNum ) + ":" + keyName;
            QAction* showColAction = manageColsMenu.addAction( showColStr );
            showColAction->setParent(&manageColsMenu);
            connect( showColAction, SIGNAL( triggered() ), SLOT( sl_showHiddenColumn() ) );
        }
    }
    int colCount = uindexTable->columnCount();
    if( !colCount || !isEmptyCol( colCount - 1 ) ) {
        manageColsMenu.addSeparator();
        QAction* addLastCol = manageColsMenu.addAction( ADD_LAST_EMPTY_COL );
        addLastCol->setParent(&manageColsMenu);
        connect( addLastCol, SIGNAL( triggered() ), SLOT( sl_addLastEmptyCol() ) );
    }
    manageColsMenu.exec( QCursor::pos() );
}

void UIndexViewWidgetImpl::sl_addLastEmptyCol() {
    UIndexViewHeaderItemWidgetImpl* header = new UIndexViewHeaderItemWidgetImpl( this, keyNamesList, keyTypesList, NULL );
    insertHeaderItem( header );
    addColumn( header );
    setColumnVals( uindexTable->columnCount() - 1 );
}

void UIndexViewWidgetImpl::sl_showHiddenColumn() {
    QAction* showColAction = qobject_cast< QAction* >( sender() );
    assert( NULL != showColAction );
    QString txt = showColAction->text();
    assert( !txt.isEmpty() );
    
    int numStrEnd = txt.indexOf( ":" );
    int numStrBegin = SHOW_HIDDEN_COLS_BEGIN.size() - 1;
    assert( numStrBegin < numStrEnd );
    QString numStr = txt.mid( numStrBegin, numStrEnd - numStrBegin );
    bool ok = false;
    int colNum = numStr.toInt( &ok );
    assert( ok && 0 <= colNum && uindexTable->columnCount() > colNum );
    assert( hiddenColumns.contains( colNum ) );
    
    uindexTable->showColumn( colNum );
    hiddenColumns.removeAll( colNum );
}

UIndex UIndexViewWidgetImpl::getIndex() const {
    return ind;
}

QList< int > UIndexViewWidgetImpl::getSelectedDocNums() const {
    QList< QTableWidgetItem* > itemz = uindexTable->selectedItems();
    QList< int > ret;
    foreach( QTableWidgetItem* it, itemz ) {
        int docNum = it->row() - 1;
        if( !ret.contains( docNum ) ) {
            ret << docNum;
        }
    }
    return ret;
}

} // U2
