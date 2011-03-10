#ifndef _U2_FIND_TANDEMS_DIALOG_H_
#define _U2_FIND_TANDEMS_DIALOG_H_

#include <ui/ui_FindTandemsDialog.h>
#include "RF_SArray_TandemFinder.h"

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/U2Region.h>

#include <QtGui/QDialog>
#include <QtGui/QCheckBox>
#include <QtGui/QToolButton>

namespace U2 {
class CreateAnnotationWidgetController;
class ADVSequenceObjectContext;

class FindTandemsDialog : public QDialog, public Ui_FindTandemsDialog {
    Q_OBJECT
public:
    FindTandemsDialog(ADVSequenceObjectContext* seq);

    static FindTandemsTaskSettings defaultSettings();
    
    virtual void accept();

protected slots:
    //void sl_setPredefinedAnnotationName();
    void sl_startRangeChanged(int i);
    void sl_endRangeChanged(int i);
    void minPeriodChanged(int min);
    void maxPeriodChanged(int max);
    void presetSelected(int preset);
    void customization();
    //void sl_hundredPercent();
    //void sl_repeatParamsChanged(int);
    //void sl_minMaxToggle(bool);

private:
    U2Region getActiveRange() const;
    void saveState();
    QStringList getAvailableAnnotationNames() const;
    bool getRegions(QCheckBox* cb, QLineEdit* le, QVector<U2Region>& res);
    void prepareAMenu(QToolButton* tb, QLineEdit* le, const QStringList& names);
    void updateStatus();

    //methods for results count estimation
    quint64 areaSize() const ;
    int estimateResultsCount() const;

    ADVSequenceObjectContext*           sc;
    CreateAnnotationWidgetController*   ac;
};

} //namespace

#endif
