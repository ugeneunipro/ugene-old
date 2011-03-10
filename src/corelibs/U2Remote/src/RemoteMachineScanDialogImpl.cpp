
#include <cassert>

#include <QtCore/QtAlgorithms>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>

#include <U2Core/AppContext.h>
#include <U2Remote/ProtocolInfo.h>
#include <U2Remote/RemoteMachineScanner.h>

#include "RemoteMachineScanDialogImpl.h"

namespace U2 {

RemoteMachineScanDialogImpl::RemoteMachineScanDialogImpl() {
    setupUi( this );
    ProtocolInfoRegistry * pir = AppContext::getProtocolInfoRegistry();
    assert( NULL != pir );
    
    QList< ProtocolInfo * > protocolInfos = pir->getProtocolInfos();
    foreach( ProtocolInfo * pi, protocolInfos ) {
        assert( NULL != pi );
//         if( NULL != pi->getRemoteMachineScanner() ) {
//             protocolComboBox->addItem( pi->getId() );
//         }
     }
    
    connect( cancelPushButton, SIGNAL( clicked() ), SLOT( sl_cancelPushButtonClicked() ) );
    connect( okPushButton, SIGNAL( clicked() ), SLOT( sl_okPushButtonClicked() ) );
    
    if( 0 == protocolComboBox->count() ) {
        okPushButton->setEnabled( false );
        QLabel * errorLable = new QLabel( tr( "No protocols that supports scanning found!" ), this );
        QVBoxLayout * topLayout = qobject_cast< QVBoxLayout* >( layout() );
        assert( NULL != topLayout );
        topLayout->insertWidget( 1, errorLable );
        return;
    }
    
    connect( protocolComboBox, SIGNAL( activated( const QString & ) ), SLOT( sl_startScan( const QString & ) ) );
    connect( &updateTimer, SIGNAL( timeout() ), SLOT( sl_updatePushButtonClicked() ) );
    updateTimer.start( SCAN_UPDATE_TIME );
    
    machinesTableWidget->horizontalHeader()->setHighlightSections( false );
    machinesTableWidget->horizontalHeader()->setClickable( false );
    machinesTableWidget->verticalHeader()->setClickable( false );
    machinesTableWidget->setSelectionMode( QAbstractItemView::NoSelection );
    machinesTableWidget->setEditTriggers( QAbstractItemView::NoEditTriggers );
    resizeTable();
    
    sl_startScan( protocolComboBox->currentText() );
}

RemoteMachineScanDialogImpl::~RemoteMachineScanDialogImpl() {
}

void RemoteMachineScanDialogImpl::cleanup() {
    qDeleteAll( model );
    model.clear();
}

void RemoteMachineScanDialogImpl::sl_cancelPushButtonClicked() {
    cleanup();
    reject();
}

void RemoteMachineScanDialogImpl::sl_okPushButtonClicked() {
    int sz = machinesTableWidget->rowCount();
    QList< int > delIndexes;
    for( int i = 0; i < sz; ++i ) {
        QCheckBox * checkBox = qobject_cast<QCheckBox*>( machinesTableWidget->cellWidget( i, 0 ) );
        assert( NULL != checkBox );
        if( !checkBox->isChecked() ) {
            delIndexes << i;
        }
    }
    
    qSort( delIndexes.begin(), delIndexes.end(), qGreater<int>() );
    sz = delIndexes.size();
    for( int i = 0; i < sz; ++i ) {
        delete model.takeAt( delIndexes.at( i ) );
    }
    
    accept();
}

void RemoteMachineScanDialogImpl::sl_startScan(  const QString & protoId ) {
//     RemoteMachineScanner * scanner = AppContext::getProtocolInfoRegistry()->getProtocolInfo( protoId )->getRemoteMachineScanner();
//     assert( NULL != scanner );
//     scanner->startScan();
//     if( !runningScanners.contains( scanner ) ) {
//         runningScanners << scanner;
//     }
}

void RemoteMachineScanDialogImpl::addMachines( const QList< RemoteMachineSettings* > newMachines ) {
    foreach( RemoteMachineSettings * machine, newMachines ) {
        addMachine( machine );
    }
}

void RemoteMachineScanDialogImpl::addMachine( RemoteMachineSettings * machine ) {
    assert( NULL != machine );
    if( hasSameMachineInTheList( machine ) ) {
        delete machine;
    } else {
        model.append( machine );
        addNextMachineToTable( machine );
    }
    resizeTable();
}

void RemoteMachineScanDialogImpl::addNextMachineToTable( RemoteMachineSettings * settings ) {
    assert( NULL != settings );
    int sz = machinesTableWidget->rowCount();
    machinesTableWidget->insertRow( sz );
    
    QCheckBox * checkBox = new QCheckBox();
    checkBox->setCheckState( Qt::Checked );
    machinesTableWidget->setCellWidget( sz, 0, checkBox );
    machinesTableWidget->setItem( sz, 1, new QTableWidgetItem( settings->getName() ) );
    machinesTableWidget->setItem( sz, 2, new QTableWidgetItem( settings->getProtocolId() ) );
    machinesTableWidget->resizeColumnToContents( 2 );
}

void RemoteMachineScanDialogImpl::sl_updatePushButtonClicked() {
    foreach( RemoteMachineScanner * scanner, runningScanners ) {
        assert( NULL != scanner );
        QList< RemoteMachineSettings* > newScanned = scanner->takeScanned();
        addMachines( newScanned );
    }
}

bool RemoteMachineScanDialogImpl::hasSameMachineInTheList( RemoteMachineSettings* suspect ) const {
    assert( NULL != suspect );
    foreach( RemoteMachineSettings * machine, model ) {
        assert( NULL != machine );
        if( *machine == *suspect ) {
            return true;
        }
    }
    return false;
}

RemoteMachineScanDialogModel RemoteMachineScanDialogImpl::getModel() const {
    return model;
}

void RemoteMachineScanDialogImpl::resizeTable() {
    machinesTableWidget->horizontalHeader()->setResizeMode( 1, QHeaderView::Stretch );
    machinesTableWidget->horizontalHeader()->resizeSections( QHeaderView::ResizeToContents );    
}

} // U2
