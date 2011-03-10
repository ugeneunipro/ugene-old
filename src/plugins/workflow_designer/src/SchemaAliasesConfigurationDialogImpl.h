#ifndef _U2_WCFG_SCHEMA_ALIASES_DIALOG_CONTROLLER_H_
#define _U2_WCFG_SCHEMA_ALIASES_DIALOG_CONTROLLER_H_

#include <U2Lang/Schema.h>
#include <ui/ui_SchemaAliasesConfigurationDialog.h>

namespace U2 {
namespace Workflow {

//typedef QMap<ActorId, QMap<Descriptor, QString> > SchemaAliasesCfgDlgModel;
struct SchemaAliasesCfgDlgModel {
    QMap<ActorId, QMap<Descriptor, QString> > aliases;
    QMap<ActorId, QMap<Descriptor, QString> > help;
};

class SchemaAliasesConfigurationDialogImpl : public QDialog, public Ui_SchemaAliasesConfigurationDialog {
    Q_OBJECT
public:
    SchemaAliasesConfigurationDialogImpl( const Schema & sc, QWidget * p = NULL );
    // remove empty aliases from model
    SchemaAliasesCfgDlgModel getModel() const;
    // aliases for schema should be different
    bool validateModel() const;
    
private:
    void initializeModel( const Schema & schema );
    void clearAliasTable();
    
private slots:
    void sl_procSelected( int row );
    void sl_onDataChange( int row, int col );
    
private:
    SchemaAliasesCfgDlgModel    model;
    QMap<int, ActorId>          procListMap; // pairs (row, actorId)
    int                         procNameMaxSz;
    
}; // SchemaAliasesConfigurationDialogImpl

} // Workflow
} // U2

#endif // _U2_WCFG_SCHEMA_ALIASES_DIALOG_CONTROLLER_H_
