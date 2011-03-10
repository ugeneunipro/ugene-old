#ifndef _U2_WINDOW_STEP_SELECTOR_WIDGET_
#define _U2_WINDOW_STEP_SELECTOR_WIDGET_

#include <QtGui/QDialog>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGroupBox>
#include <U2Core/U2Region.h>

namespace U2 {

class U2VIEW_EXPORT WindowStepSelectorWidget :  public QWidget {
    Q_OBJECT
public:
    WindowStepSelectorWidget(QWidget* p, const U2Region& winRange, int win, int step);
    QString validate() const;

    int getWindow() const;
    int getStep() const;

private:
    QSpinBox*   windowEdit;
    QSpinBox*   stepsPerWindowEdit;
};

class U2VIEW_EXPORT MinMaxSelectorWidget : public QWidget {
    Q_OBJECT
public:
    MinMaxSelectorWidget(QWidget* p, double min, double max, bool enabled);
    QString validate() const;

    double getMin() const;
    double getMax() const;
    bool getState() const;

private:
    QGroupBox* minmaxGroup;
    QDoubleSpinBox* minBox;
    QDoubleSpinBox* maxBox;
};

class U2VIEW_EXPORT WindowStepSelectorDialog : public QDialog {
    Q_OBJECT
public: 
    WindowStepSelectorDialog(QWidget* p, const U2Region& winRange, int win, int step, double min, double max, bool e);
    WindowStepSelectorWidget* getWindowStepSelector() const {return wss;}
    MinMaxSelectorWidget* getMinMaxSelector() const {return mms;}

private slots:
    void sl_onCancelClicked(bool);
    void sl_onOkClicked(bool);
private:
    WindowStepSelectorWidget* wss;
    MinMaxSelectorWidget* mms;
};

} // namespace

#endif
