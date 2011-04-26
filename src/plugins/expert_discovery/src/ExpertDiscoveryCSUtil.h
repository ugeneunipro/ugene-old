#ifndef _U2_EXPERT_DIS_CSUTIL_H_
#define _U2_EXPERT_DIS_CSUTIL_H_

#include "DDisc/Signal.h"
#include "DDisc/Sequence.h"
#include "ExpertDiscoverySet.h"
#include <vector>
#include <QVector>
#include <QMap>
#include <set>

#include <QObject>

namespace U2 {

using namespace DDisc;

class CSFolder: public QObject{
    Q_OBJECT
public:
    CSFolder(CSFolder* parentFolder = NULL);
    ~CSFolder();

    QString				getName() const;
    void				setName(QString strName);

    CSFolder*           getParentFolder() const;
    void				setParentFolder(CSFolder* pParentFolder);

    void				clear();

    int					getSignalNumber() const;
    Signal*				getSignal(int id);
    const Signal*		getSignal(int id) const;
    int					addSignal(Signal *pSignal, bool bReplace = false);
    int					getSignalIndexByName(QString strName) const;
    void				deleteSignal(int id);
    QString				makeUniqueSignalName() const;
    CSFolder*			clone() const;

    int					getFolderNumber() const;
    CSFolder*			getSubfolder(int id);
    const CSFolder*	    getSubfolder(int id) const;
    int					addFolder(CSFolder *pFolder, bool bMerge = false);
    int					getFolderIndexByName(QString strName) const;
    void				deleteFolder(int id);
    QString				makeUniqueFolderName() const;

    QString				getPathToSignal(const Signal* pSignal) const;
    const Signal*		getSignalByPath(QString strPath) const;

    

protected:
    bool				doConstructPath(QString& strPath, const Signal* pSignal) const;

private:
    std::vector<Signal*>	signalsVect;
    QVector<CSFolder*>	    folders;

    QString					strName;
};

#define UNDEFINED_VALUE (double) 0xFFFFFFFF

class EDProcessedSignal
{
protected:
    virtual void process(Operation *pOp, const SequenceBase *pYesBase, const SequenceBase *pNoBase) {}

public:
    //void Serialize(CArchive& ar);
    virtual ~EDProcessedSignal() {}
    static EDProcessedSignal* processSignal(Operation *pOp, const SequenceBase *pYesBase, const SequenceBase *pNoBase);
    void makeStandardProcessing(Operation *pOp, const SequenceBase *pYesBase, const SequenceBase *pNoBase);
    const Set&	getYesRealizations(int iSequence) const { return m_arYesRealizations[iSequence];}
    const Set&	getNoRealizations(int iSequence)  const { return m_arNoRealizations[iSequence];}
    QString	getTextDescription() const { return m_strDescription; }

    int			getPropertyNumber() const { return (int) m_arNames.size(); }
    QString		getPropertyName (int iProperty) const { return m_arNames[iProperty];  }
    QString		getPropertyValue(int iProperty) const { return m_arValues[iProperty]; }
    QString     getPropertyValue(QString name) const;
    int getYesSequenceNumber() const {
        return (int)m_arYesRealizations.size();
    }
    int getNoSequenceNumber() const {
        return (int)m_arNoRealizations.size();
    }

    double getProbability() const {
        return m_dProbability;
    }

    double getFisher() const {
        return m_dFisher;
    }

    double getUl() const {
        return m_dUl;
    }

    double getPosCoverage() const {
        return m_dPosCoverage;
    }

    double getNegCoverage() const {
        return m_dNegCoverage;
    }

protected:
    void addProperty(QString strName, QString strValue) {
        m_arNames.push_back(strName);
        m_arValues.push_back(strValue);
    }
    EDProcessedSignal();
private:
    const EDProcessedSignal& operator=(const EDProcessedSignal&);
    EDProcessedSignal(const EDProcessedSignal&);
    void setYesRealizations(int iSequence, const Set& set) { 
        m_arYesRealizations[iSequence] = set;
    }
    void setNoRealizations(int iSequence, const Set& set) { 
        m_arNoRealizations[iSequence] = set;
    }
    void setYesSequenceNumber(int nSeqNum) {
        m_arYesRealizations.resize(nSeqNum);
    }
    void setNoSequenceNumber(int nSeqNum) {
        m_arNoRealizations.resize(nSeqNum);
    }
    void setTextDescription(QString strDesc) { 
        m_strDescription = strDesc; 
    }

private:
    QVector<QString> m_arNames;
    QVector<QString> m_arValues;
    QString			m_strDescription;
    std::vector<Set>		m_arYesRealizations;
    std::vector<Set>		m_arNoRealizations;
    double			m_dProbability;
    double			m_dFisher;
    double			m_dUl;
    double			m_dPosCoverage;
    double			m_dNegCoverage;
};


class EDProcessedInterval : public EDProcessedSignal {
protected:
    virtual void process(Operation *pOp, const SequenceBase *pYesBase, const SequenceBase *pNoBase);
};

class EDProcessedDistance : public EDProcessedSignal {
protected:
    virtual void process(Operation *pOp, const SequenceBase *pYesBase, const SequenceBase *pNoBase);
};

class EDProcessedReiteration : public EDProcessedSignal {
protected:
    virtual void process(Operation *pOp, const SequenceBase *pYesBase, const SequenceBase *pNoBase);
};


class EDProcessedTS : public EDProcessedSignal {
protected:
    virtual void process(Operation *pOp, const SequenceBase *pYesBase, const SequenceBase *pNoBase);
};

using std::set;
typedef set<const Signal*> SignalList;

class SelectedSignalsContainer
{
public:
    SelectedSignalsContainer(void);
    ~SelectedSignalsContainer(void);
    void AddSignal(const Signal* pSignal);
    void RemoveSignal(const Signal* pSignal);
    const SignalList& GetSelectedSignals() const;
    bool IsSelected(const Signal *pSignal) const;
    //void Serialize(CArchive& ar);
    void Clear() { m_SelectedSignals.clear(); }
private:
    SignalList m_SelectedSignals;	
};

typedef std::vector<double> RecognizationData;

class RecognizationDataStorage
{
public:
    ~RecognizationDataStorage();
    void clear();
    void addSequence(Sequence* seq);
    bool getRecognizationData(RecognizationData& d, const Sequence* seq, const SelectedSignalsContainer& rSe);
    
private:
    RecognizationData* getRecData(const Sequence* seq);
    QMap<const Sequence*, RecognizationData*> recMap;
};

    
} //namespace

#endif
