
#ifndef _U2_UINDEX_VIEW_WIDGET_IMPL_H_
#define _U2_UINDEX_VIEW_WIDGET_IMPL_H_

#include <QtGui/QMenu>

#include "UIndexViewHeaderItemWidgetImpl.h"

#include "UIndexViewWidgetKey.h"

#include <ui/ui_UIndexViewWidget.h>
#include <U2Core/UIndex.h>

namespace U2 {

class UIndexViewWidgetImpl : public QWidget, public Ui::UIndexViewWidget {
    Q_OBJECT
public:
    UIndexViewWidgetImpl( QWidget* p, const UIndex& ind );
    ~UIndexViewWidgetImpl();
    
    QList< int > getSelectedDocNums() const;
    UIndex getIndex() const;
    
private slots:
    void sl_headerWidgetStateChanged( UIndexKey* newKey );
    void sl_horHeaderSectionClicked( int col );
    void sl_manageColumnsButtonClicked();
    void sl_showHiddenColumn();
    void sl_addLastEmptyCol();
    
private:
    void initTable();
    bool isSignificantKey( const QString& keyName );
    
    void initKeyNamesList();
    void sortKeyNamesList();
    void insertHeaderItem( UIndexViewHeaderItemWidgetImpl* headerIt );
    
    void setColumnVals( int col );
    void addColumn( UIndexViewHeaderItemWidgetImpl* headerIt );
    void removeColumn( int col );
    void setHorizontalHeaderLabel( int col );
    QString getRuleStrRepresent( const UIndexKeyRule* r ) const;
    
    void execRules();
    bool rulesPassed( int row );
    bool execOneRule( int col, const QString& val );
    bool isEmptyCol( int col )const;
    bool hasNotEmptyCol() const;
    
    void detectTypes();
    
private:

    static const int HEADER_ITEMS_ROW   = 0;
    static const int COL_DOC_FORMAT     = 0;
    static const int COL_URL            = 1;

    static const float SHOW_KEY_PERCENT_BORDER;

    QString COLUMN_MENU_ACTION_HIDE;
    QString COLUMN_MENU_ACTION_DELETE;
    QString NO_HIDDEN_COLUMNS;


    QString SHOW_HIDDEN_COLS_STR;
    QString SHOW_HIDDEN_COLS_BEGIN;
    QString ADD_LAST_EMPTY_COL;

    QString VALUE_NOT_SELECTED;

    QString KEY_FORMAT;
    QString KEY_URL;
    QString KEY_NONE;


    UIndex                  ind;
    QStringList             keyNamesList;
    QList< UIndexKeyType >  keyTypesList;
    QList< int >            hiddenColumns;
    
    QList< UIndexViewHeaderItemWidgetImpl* > headerItems;
    
}; // UindexViewWidgetImpl

} // U2

#endif // _U2_UINDEX_VIEW_WIDGET_IMPL_H_
