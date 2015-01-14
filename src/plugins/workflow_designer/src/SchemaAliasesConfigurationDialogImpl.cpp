/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <cassert>

#include <U2Lang/ActorModel.h>
#include "SchemaAliasesConfigurationDialogImpl.h"
#include <U2Gui/HelpButton.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPushButton>
#else
#include <QtWidgets/QPushButton>
#endif


namespace U2 {
namespace Workflow {

SchemaAliasesConfigurationDialogImpl::SchemaAliasesConfigurationDialogImpl( const Schema & schema, QWidget * p )
: QDialog(p), procNameMaxSz(0) {
    setupUi(this);
    new HelpButton(this, buttonBox, "2097213");

    QPushButton* cancelPushButton = buttonBox->button(QDialogButtonBox::Cancel);
    QPushButton* okPushButton = buttonBox->button(QDialogButtonBox::Ok);

    connect( cancelPushButton, SIGNAL(clicked()), SLOT(reject()) );
    connect( okPushButton, SIGNAL(clicked()), SLOT(accept()));

    okPushButton->setDefault(true);
    paramAliasesTableWidget->verticalHeader()->hide();

#if (QT_VERSION < 0x050000) //Qt 5
    paramAliasesTableWidget->horizontalHeader()->setClickable(false);
#else
    paramAliasesTableWidget->horizontalHeader()->setSectionsClickable(false);
#endif
    paramAliasesTableWidget->horizontalHeader()->setStretchLastSection( true );

    foreach( Actor * actor, schema.getProcesses() ) {
        assert( actor != NULL );
        int pos = procsListWidget->count();
        QListWidgetItem * item = new QListWidgetItem( actor->getLabel() );
        procsListWidget->insertItem( pos, item );
        procListMap.insert( pos, actor->getId() );
        int pointSz = item->font().pointSize();
        procNameMaxSz = qMax(pointSz * actor->getLabel().size(), procNameMaxSz);
    }

    connect( procsListWidget, SIGNAL(currentRowChanged( int )), SLOT(sl_procSelected( int )) );
    connect( paramAliasesTableWidget, SIGNAL( cellChanged(int, int) ), SLOT(sl_onDataChange(int, int)) );

    initializeModel(schema);

}

void SchemaAliasesConfigurationDialogImpl::initializeModel( const Schema & schema ) {
    foreach( Actor * actor, schema.getProcesses() ) {
        assert( actor != NULL );
        QMap<Descriptor, QString> aliasMap;
        QMap<Descriptor, QString> helpMap;
        foreach( Attribute * attr, actor->getParameters().values() ) {
            assert( attr != NULL );
            QString alias = actor->getParamAliases().value(attr->getId());
            QString help = actor->getAliasHelp().value(alias);
            aliasMap.insert(*attr, alias);
            helpMap.insert(*attr, help);
        }
        ActorId id = actor->getId();
        model.aliases.insert(id, aliasMap);
        model.help.insert(id, helpMap);
    }
}

SchemaAliasesCfgDlgModel SchemaAliasesConfigurationDialogImpl::getModel() const {
    SchemaAliasesCfgDlgModel ret;
    foreach(const ActorId & id, model.aliases.keys()) {
        QMap<Descriptor, QString> aliases;
        foreach(const Descriptor & d, model.aliases.value(id).keys()) {
            QString aliasStr = model.aliases.value(id).value(d);
            if(!aliasStr.isEmpty()) {
                aliases.insert(d, aliasStr);
            }
        }
        ret.aliases.insert(id, aliases);
        QMap<Descriptor, QString> help;
        foreach(const Descriptor & d, model.help.value(id).keys()) {
            QString helpStr = model.help.value(id).value(d);
            if(!helpStr.isEmpty()) {
                help.insert(d, helpStr);
            }
        }
        ret.help.insert(id, help);
    }
    return ret;
}

bool SchemaAliasesConfigurationDialogImpl::validateModel() const {
    SchemaAliasesCfgDlgModel m = getModel();
    QStringList allAliases;
    foreach(const ActorId & id, m.aliases.keys()) {
        foreach(const Descriptor & d, m.aliases.value(id).keys()) {
            allAliases << m.aliases.value(id).value(d);
        }
    }
    return allAliases.removeDuplicates() == 0;
}

void SchemaAliasesConfigurationDialogImpl::clearAliasTable() {
    paramAliasesTableWidget->clearContents();
    paramAliasesTableWidget->setRowCount(0);
}

void SchemaAliasesConfigurationDialogImpl::sl_procSelected( int row ) {
    if( row == -1 ) {
        return;
    }
    clearAliasTable();

    assert( row >= 0 && row < procsListWidget->count() );
    ActorId currentActor = procListMap.value( row );
    assert( !currentActor.isEmpty() );

    int rowInd = 0;
    QMap<Descriptor, QString> aliasMap = model.aliases.value( currentActor );
    QMap<Descriptor, QString>::const_iterator it = aliasMap.constBegin();
    while( it != aliasMap.constEnd() ) {
        paramAliasesTableWidget->insertRow(rowInd);

        QTableWidgetItem * paramNameItem = new QTableWidgetItem(it.key().getDisplayName()) ;
        paramAliasesTableWidget->setItem( rowInd, 0, paramNameItem );
        paramNameItem->setData( Qt::UserRole, QVariant::fromValue( it.key() ) );
        paramNameItem->setFlags( paramNameItem->flags() ^ Qt::ItemIsSelectable ^ Qt::ItemIsEditable );

        QTableWidgetItem * aliasItem = new QTableWidgetItem( it.value() );
        paramAliasesTableWidget->setItem( rowInd, 1, aliasItem );

        QTableWidgetItem * helpItem = new QTableWidgetItem(model.help.value(currentActor).value(it.key()));
        paramAliasesTableWidget->setItem(rowInd, 2, helpItem);

        rowInd++;
        ++it;
    }
    paramAliasesTableWidget->resizeColumnToContents(0);
    if(procNameMaxSz > 0 && procNameMaxSz < splitter->width()) {
        QList<int> szs;
        szs << procNameMaxSz;
        szs << splitter->width() - procNameMaxSz;
        splitter->setSizes(szs);
    }
}

void SchemaAliasesConfigurationDialogImpl::sl_onDataChange( int row, int col ) {
    assert( row >= 0 && row < paramAliasesTableWidget->rowCount() );
    if( col != 1 && col != 2) {
        return;
    }

    ActorId id = procListMap.value( procsListWidget->currentRow() );
    assert( !id.isEmpty() );

    Descriptor desc = paramAliasesTableWidget->item( row, 0 )->data( Qt::UserRole ).value<Descriptor>();
    assert(model.aliases.value(id).contains(desc));
    if(col == 1) {
        model.aliases[id][desc] = paramAliasesTableWidget->item( row, 1 )->text();
    } else {
        assert(col == 2);
        model.help[id][desc] = paramAliasesTableWidget->item( row, 2 )->text();
    }
}

} // Workflow
} // U2
