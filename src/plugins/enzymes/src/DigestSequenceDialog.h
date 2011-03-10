#ifndef _U2_DIGEST_SEQUENCE_DIALOG_H_
#define _U2_DIGEST_SEQUENCE_DIALOG_H_

#include <U2Core/U2Region.h>
#include <U2Gui/MainWindow.h>

#include <ui/ui_DigestSequenceDialog.h>

#include "EnzymeModel.h"
#include "EnzymesQuery.h"

#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

namespace U2 {

class ADVSequenceObjectContext;
class DNASequenceObject;
class CreateAnnotationWidgetController;
class AnnotationTableObject;

class DigestSequenceDialog : public QDialog, public Ui_DigestSequenceDialog {
    Q_OBJECT
public:
    DigestSequenceDialog(ADVSequenceObjectContext* ctx, QWidget* parent);
    virtual void accept();

private slots:
    void sl_addPushButtonClicked();
    void sl_addAllPushButtonClicked();
    void sl_removePushButtonClicked();
    void sl_clearPushButtonClicked();
    void sl_searchSettingsPushButtonClicked();
    void sl_useAnnotatedRegionsSelected(bool toggle);
    
private:
    void addAnnotationWidget();
    void searchForAnnotatedEnzymes(ADVSequenceObjectContext* ctx);
    void updateAvailableEnzymeWidget();
    void updateSelectedEnzymeWidget();
    bool loadEnzymesFile();
    QList<SEnzymeData> findEnzymeDataById(const QString& id);
    
    ADVSequenceObjectContext*           seqCtx;
    DNASequenceObject*                  dnaObj;
    AnnotationTableObject*              sourceObj;
    EnzymesSelectorDialogHandler        enzymesSelectorHandler;
    CreateAnnotationWidgetController*   ac;
    QSet<QString>                       availableEnzymes;
    QSet<QString>                       selectedEnzymes;
    QMap<QString,U2Region>               annotatedEnzymes;
    QList<SEnzymeData>                  enzymesBase;
    
};


} //namespace

#endif
