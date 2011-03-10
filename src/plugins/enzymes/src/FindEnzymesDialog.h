#ifndef _U2_ENZYMES_DIALOG_H_
#define _U2_ENZYMES_DIALOG_H_

#include <U2Gui/MainWindow.h>

#include <ui/ui_EnzymesSelectorWidget.h>
#include <ui/ui_FindEnzymesDialog.h>

#include "EnzymeModel.h"

#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

namespace U2 {

class ADVSequenceObjectContext;
class CreateAnnotationWidgetController;
class EnzymeGroupTreeItem;

class EnzymesSelectorWidget : public QWidget, public Ui_EnzymesSelectorWidget {
    Q_OBJECT
public:
    EnzymesSelectorWidget();
    ~EnzymesSelectorWidget();

    static void initSettings();
    static void saveSettings();
    static QList<SEnzymeData> getLoadedEnzymes();
    QList<SEnzymeData> getSelectedEnzymes();
    int getNumSelected();
    int getTotalNumber() { return totalEnzymes; }
       
    
signals:
    void si_selectionModified(int,int);
private slots:
    void sl_selectFile();
    void sl_selectAll();
    void sl_selectNone();
    void sl_selectByLength();
    void sl_inverseSelection();
    void sl_saveSelectionToFile();
    void sl_openDBPage();
    void sl_itemChanged(QTreeWidgetItem* item, int col);
private:
    void loadFile(const QString& url);
    void saveFile(const QString& url);
    void setEnzymesList(const QList<SEnzymeData>& enzymes);
    void updateStatus();
   

    EnzymeGroupTreeItem* findGroupItem(const QString& s, bool create);

    static QList<SEnzymeData>   loadedEnzymes;
    //saves selection between calls to getSelectedEnzymes()
    static QSet<QString>        lastSelection;

    int                         totalEnzymes;
    bool                        ignoreItemChecks;
    int                         minLength;
};

class FindEnzymesDialog : public QDialog, public Ui_FindEnzymesDialog {
    Q_OBJECT
public:
    FindEnzymesDialog(ADVSequenceObjectContext* ctx);
    static void initDefaultSettings();
    virtual void accept();
private slots:
    void sl_onSelectionModified(int total, int nChecked);
    void sl_onFillRangeButtonClicked();
private:
    void initSettings();
    void saveSettings();
    ADVSequenceObjectContext*           seqCtx;
    EnzymesSelectorWidget*              enzSel;
    
};

class EnzymeTreeItem;
class EnzymeGroupTreeItem : public QTreeWidgetItem {
public:
    EnzymeGroupTreeItem(const QString& s);
    void updateVisual();
    QString s;
    QSet<EnzymeTreeItem*> checkedEnzymes;
    bool operator<(const QTreeWidgetItem & other) const;
};

class EnzymeTreeItem : public QTreeWidgetItem {
public:
    EnzymeTreeItem(const SEnzymeData& ed);
    SEnzymeData enzyme;
    bool operator<(const QTreeWidgetItem & other) const;
};

#define ANY_VALUE   -1


} //namespace

#endif
