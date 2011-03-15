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


#ifndef _U2_UINDEX_VIEW_HEADER_ITEM_WIDGET_IMPL_H_
#define _U2_UINDEX_VIEW_HEADER_ITEM_WIDGET_IMPL_H_

#include <QAtomicInt>
#include <QtCore/QMap>

#include <QtGui/QMenu>

#include <ui/ui_UIndexViewHeaderItemWidget.h>
#include "UIndexViewWidgetKey.h"

namespace U2 {

enum ValueDataType {
    NO_VALUE_TYPE,
    STR_TYPE,
    NUM_TYPE,
    NUM_NUM_TYPE
}; // ValueDataType

class UIndexViewHeaderItemWidgetImpl : public QWidget, public Ui::UIndexViewHeaderItemWidget {
    Q_OBJECT
public:
    static QMap< UIndexKeySimpleRuleType, QString > ruleTypeMap;
    static QMap< UIndexKeySimpleRuleType, ValueDataType > ruleValueMap;
    static QAtomicInt                               rulesMapInitialized;
    static void initializeRulesMap();
    
public:
    UIndexViewHeaderItemWidgetImpl( QWidget* p, const QStringList& keyNamesList,
                                    const QList< UIndexKeyType >& keyTypesList,UIndexKey* key = NULL );
    
    UIndexKey* getKey() { return key; }
    void setKey( UIndexKey* newKey ) { key = newKey; }
    
private:
    void buildMenu();
    void buildSubMenu( QMenu* sub, UIndexKeyType t );

    QString getKeyName( QAction* action );
    QString getData( const UIndexKeySimpleRuleType& keyType );
    QString getString();
    QString getNum();
    QString getNumNum();
    
signals:
    void si_stateChanged( UIndexKey* newKey );
    
private slots:
    void sl_selectKeyButtonClicked();
    void sl_noneKeySelected();
    void sl_filterSelected();
    
private:
    UIndexKey*              key;
    QStringList             keyNamesList;
    QList< UIndexKeyType >  keyTypesList;
    QMenu*                  selectKeyButtonMenu;

    QString SELECTED_KEY_LABEL_DEFAULT;

    QString INPUT_VALUES_TITLE;
    QString GREETING_STR_TYPE;
    QString GREETING_NUM_TYPE;
    QString GREETING1_NUM_MUM_TYPE;
    QString GREETING2_NUM_MUM_TYPE;
}; // UIndexViewHeaderItemWidgetImpl

} // U2

#endif // _U2_UINDEX_VIEW_HEADER_ITEM_WIDGET_IMPL_H_
