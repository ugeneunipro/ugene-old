#ifndef _U2_ACTOR_CFG_H_
#define _U2_ACTOR_CFG_H_

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QAbstractTableModel>

#include <U2Lang/ActorModel.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/Schema.h>
#include <U2Designer/DelegateEditors.h>

namespace U2 {

using namespace Workflow;

class ActorCfgModel : public QAbstractTableModel {
    Q_OBJECT
public:
    ActorCfgModel(QObject *parent, QList<Iteration>& lst);
    ~ActorCfgModel();
    
    void setActor(Actor * cfg);
    
    void selectIteration(int i);
    
    void setIterations(QList<Iteration>& lst);

    int columnCount(const QModelIndex &) const;
    
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    
    Qt::ItemFlags flags( const QModelIndex & index ) const;
    
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    
    /*Used to supply item data to views and delegates. 
    Generally, models only need to supply data for Qt::DisplayRole and any application-specific user roles, 
    but it is also good practice to provide data for Qt::ToolTipRole, Qt::AccessibleTextRole, and Qt::AccessibleDescriptionRole.*/
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    
    /*Used to modify the item of data associated with a specified model index. 
    To be able to accept user input, provided by user interface elements, this function must handle data associated with Qt::EditRole. 
    The implementation may also accept data associated with many different kinds of roles specified by Qt::ItemDataRole. 
    After changing the item of data, models must emit the dataChanged() signal to inform other components of the change.*/
    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

    void changeScriptMode(bool _mode);

private:
    bool setAttributeValue( const Attribute * attr, QVariant & attrValue ) const;
    void setupAttributesScripts();
    
private:
    Actor*              subject;
    QList<Attribute*>   attrs;
    AttributeScriptDelegate * scriptDelegate;
    
    QList<Iteration>&   iterations;
    int                 iterationIdx;
    QVariantMap         listValues;
    bool scriptMode;
        
}; // ActorCfgModel

}//namespace U2

#endif // _U2_ACTOR_CFG_H_
