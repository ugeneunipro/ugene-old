#pragma once

#include "DDisc/Signal.h"

#include <ui/ui_ExpertDiscoverySigExtrWiz.h>

#include "ExpertDiscoveryCSUtil.h"

#include <QStackedLayout>
#include <QSpacerItem>

namespace U2 {

struct State{
    double  dProbability;
    double  dCoverage;
    double  dFisher;
    double  dIntProbability;
    double  dIntFisher;
    int     nMinComplexity;
    int     nMaxComplexity;
    double  dMinPosCorrelation;
    double  dMaxPosCorrelation;
    double  dMinNegCorrelation;
    double  dMaxNegCorrelation;
    bool    bCorrelationImportant;
    bool    bCheckFisherMinimization;
    bool    bStoreOnlyDifferent;
    bool    bUmEnabled;
    int     nUmSamplesBound;
    double  dUmBound;

    void setDefaultState(){
        dProbability    = 25;
        dCoverage    = 25;
        dFisher = 0.05;
        dIntProbability = 0;
        dIntFisher = 0.2;
        nMinComplexity = 1;
        nMaxComplexity = 5;
        dMinPosCorrelation    = 0.5;
        dMaxPosCorrelation    = 1;
        dMinNegCorrelation    = -1;
        dMaxNegCorrelation    = 1;
        bCorrelationImportant = false;
        bCheckFisherMinimization = true;
        bStoreOnlyDifferent = true;
        bUmEnabled = false;
        dUmBound = 0.05;
        nUmSamplesBound = 50;
    }
};

using namespace DDisc;

class OperationSet : public QWidget{
    Q_OBJECT
public:
    OperationSet(QWidget* parent):QWidget(parent){};
    virtual void loadData(void *pData){};
    virtual void saveData(void *pData){};
    virtual bool isReadyToClose(){return true;}

};

class ExpertDiscoveryExtSigWiz : public QWizard, public Ui_SignalsExtrWiz{
    Q_OBJECT
public:
    ExpertDiscoveryExtSigWiz(QWidget *parent, CSFolder* f, int positiveSize);
    virtual ~ExpertDiscoveryExtSigWiz();

    double  getProbability() {return state.dProbability/100;}
    double  getCoverage() {return state.dCoverage/100;}
    double  getFisher() {return state.dFisher;}
    double  getIntProbability() {return state.dIntProbability/100;}
    double  getIntFisher() {return state.dIntFisher;}
    int     getMinComplexity() {return state.nMinComplexity;}
    int     getMaxComplexity() {return state.nMaxComplexity;}
    double  getMinPosCorrelation() {return state.dMinPosCorrelation;}
    double  getMaxPosCorrelation() {return state.dMaxPosCorrelation;}
    double  getMinNegCorrelation() {return state.dMinNegCorrelation;}
    double  getMaxNegCorrelation() {return state.dMaxNegCorrelation;}
    bool    getCorrelationImportant() {return state.bCorrelationImportant;}
    bool    getCheckFisherMinimization() {return state.bCheckFisherMinimization;}
    bool    getStoreOnlyDifferent() {return state.bStoreOnlyDifferent;}
    bool    getUmEnabled() {return state.bUmEnabled;}
    int     getUmSamplesBound() {return state.nUmSamplesBound;}
    double  getUmBound() {return state.dUmBound;}
    std::vector<Operation*>& getPredicates() {return predicates;}

    void setFolder(CSFolder* f){folder = f;}
    CSFolder* getFolder(){return folder;}

    virtual void accept();

protected slots:
    void sl_advButton();

    void sl_distButton();
    void sl_repetButton();
    void sl_intervButton();
    void sl_deleteButton();

    void sl_idChanged(int id);
    void sl_createSubfolder();

    void sl_selectionChanged (QTreeWidgetItem * current, QTreeWidgetItem * previous);
signals:
    void si_newFolder();

private:
    int posSize;
    State state;
    std::vector<Operation*> predicates;
    CSFolder* folder;
    QTreeWidgetItem *distItem, *intervItem, *repetItem;

    OperationSet* sigSet[4];
    QStackedLayout* sigSetLayout;

    void initSet();

    void updateTree(const CSFolder* pFolder = NULL, QTreeWidgetItem* treeItem = NULL);

    bool checkD(const QLineEdit* lineE) const;

    void hideParameters();

};

class DistanceSet : public OperationSet{
    Q_OBJECT

private:
    int to;
    int from;
    bool isOrderImportant;
    bool isMaxUNL;
    QLineEdit* fromEditD;
    QLineEdit* toEditD;
    QCheckBox* unlimCheckBoxD;
    QCheckBox* orderCheckBoxD;

public:
    DistanceSet (QWidget* parent);

    virtual void loadData(void *pData);
    virtual void saveData(void *pData);
    virtual bool isReadyToClose();
    void updateData(bool side=true);

protected slots:
    void sl_unlim();
    void sl_order();
};

class IntervalSet : public OperationSet{
    Q_OBJECT
private:
    int from;
    int to;
    bool isMaxUNL;
    QLineEdit* fromIEdit;
    QLineEdit* toIEdit;
    QCheckBox* unlimCheckBoxI;

public:
    IntervalSet (QWidget* parent);

    virtual void loadData(void *pData);
    virtual void saveData(void *pData);
    virtual bool isReadyToClose();
    void updateData(bool side=true);

protected slots:
    void sl_unlim();
};

class RepetitionSet : public OperationSet{
    Q_OBJECT
private:
    int nmin;
    int nmax;
    int min;
    int max;
    bool isMaxUNL;
    QLineEdit* fromDEdit;
    QLineEdit* toDEditR;
    QCheckBox* unlimCheckBoxR;
    QLineEdit* fromCEditR;
    QLineEdit* toCEditR;

public:
    RepetitionSet (QWidget* parent);

    virtual void loadData(void *pData);
    virtual void saveData(void *pData);
    virtual bool isReadyToClose();
    void updateData(bool side=true);

protected slots:
    void sl_unlim();
};

enum {
    T_UNDEFINED,
    T_DISTANCE,
    T_REITERATION,
    T_INTERVAL
};



}//namespace
