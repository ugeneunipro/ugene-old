#ifndef _U2_EXPERT_DIS_TREEWIDGETS_H_
#define _U2_EXPERT_DIS_TREEWIDGETS_H_

#include "DDisc/Signal.h"
#include "DDisc/MetaInfo.h"
//#include "ExpertDiscoveryCSUtil.h"
#include "ExpertDiscoveryData.h"

#include <QAction>
#include <QMenu>
#include <QTreeWidget>

namespace U2 {

using namespace DDisc;



inline QString toString(double v)
{
    QString str = "%1";
    return str.arg(v);
}

inline QString toString(int v)
{
    QString str = "%1";
    return str.arg(v);
}
inline QString toString(QString v)
{
    return v;
}

inline QString toString(std::string v)
{
    return QString::fromStdString(v);
}

inline QString toString(size_t v)
{
    return toString((int) v);
}

class ICallback {
public:
    virtual ~ICallback() {}
    virtual QString call() = 0;
    virtual ICallback* clone() = 0;
};

template<class T, class Result>
class Callback : public ICallback{
public:
    typedef Result (T::*PFUNK)() const;
    typedef PFUNK PCALLBACK;
    Callback(T* object, PCALLBACK callback)
    {
        m_callback = callback;
        m_object = object;
    }
    virtual ~Callback() {}
    virtual QString call()
    {
        return toString((m_object->*m_callback)());
    }
    virtual ICallback* clone() 
    {
        return new Callback<T, Result>(m_object, m_callback);
    }

private:
    PCALLBACK m_callback;
    T* m_object;
};

enum EItemType {
    PIT_NONE,
    PIT_SEQUENCEROOT,
    PIT_POSSEQUENCEBASE,
    PIT_NEGSEQUENCEBASE,
    PIT_CONTROLSEQUENCEBASE,
    PIT_SEQUENCE,
    PIT_CONTROLSEQUENCE,
    PIT_CS_ROOT,
    PIT_CS_FOLDER,
    PIT_CS,
    PIT_CSN_UNDEFINED,
    PIT_CSN_DISTANCE,
    PIT_CSN_REPETITION,
    PIT_CSN_INTERVAL,
    PIT_CSN_WORD,
    PIT_CSN_MRK_ITEM,
    PIT_MRK_ROOT,
    PIT_MRK_FAMILY,
    PIT_MRK_ITEM,
    PIT_WORK_ROOT,
    PIT_WORK_RUNNING,
    PIT_WORK_PAUSED
};

class EDPIPropertyType : public QObject 
{
    Q_OBJECT
public:
    EDPIPropertyType();
    virtual ~EDPIPropertyType();

    virtual bool	isNumber() const = 0;
    virtual bool	hasEdit() const = 0;
    virtual bool	hasPredefinedValues() const = 0;
    virtual int		getValueNumber() const = 0;
    virtual QString	getValue(int i) const = 0;
    virtual bool	isValidValue(QString strValue) const = 0;
};

class EDPIProperty : public QObject 
{
    Q_OBJECT
public:
    EDPIProperty(const EDPIProperty& rProperty);
    EDPIProperty(QString strName = "");
    ~EDPIProperty();

    const EDPIProperty& operator =(const EDPIProperty&);
    const EDPIPropertyType* getType() const;
    void setType(EDPIPropertyType*);

    QString getName() const;
    void	setName(QString strName);
    QString	getValue() const;
    void setCallback(ICallback* pCallback);

private:
    QString m_strName;
    EDPIPropertyType* m_pType;
    ICallback* m_pCallback;
};


class EDPIPropertyGroup : public QObject
{
    Q_OBJECT
public:
    EDPIPropertyGroup(QString strName ="");
    EDPIPropertyGroup(const EDPIPropertyGroup&);
    ~EDPIPropertyGroup();
    const EDPIPropertyGroup& operator=(const EDPIPropertyGroup&);

    QString getName() const;
    void	setName(QString strName);

    int					getPropertiesNumber() const;
    EDPIProperty&		getProperty(int nProp);
    const EDPIProperty&	getProperty(int nProp) const;
    void				addProperty(EDPIProperty &rProperty);

private:
    QString m_strName;
    QVector<EDPIProperty> m_arProperties;
};
///////////////////////////////////////////////////////////////////////
#define DEFINE_GETINSTANCE(class_name) \
public: inline static class_name* getInstance() { return &s_##class_name##Instance; } \
    virtual void	Release() {}; \
    virtual EDPIPropertyType* Clone() { return this;}; \
private: static class_name s_##class_name##Instance;

#define IMPLEMENT_GETINSTANCE(class_name) \
    class_name class_name::s_##class_name##Instance;

class EDPIPropertyTypeStaticString : public EDPIPropertyType 
{
    DEFINE_GETINSTANCE(EDPIPropertyTypeStaticString)
public:
    EDPIPropertyTypeStaticString();
    virtual ~EDPIPropertyTypeStaticString();

    virtual bool	isNumber() const;
    virtual bool	hasEdit() const;
    virtual bool	hasPredefinedValues() const;
    virtual int		getValueNumber() const;
    virtual QString	getValue(int i) const;
    virtual bool	isValidValue(QString strValue) const;
};

class EDPIPropertyTypeString : public EDPIPropertyTypeStaticString
{
    DEFINE_GETINSTANCE(EDPIPropertyTypeString)
public:
    EDPIPropertyTypeString();
    virtual ~EDPIPropertyTypeString();
    virtual bool	hasEdit() const;
};


class EDPIPropertyTypeUnsignedInt : public EDPIPropertyType 
{
    DEFINE_GETINSTANCE(EDPIPropertyTypeUnsignedInt)
public:
    EDPIPropertyTypeUnsignedInt();
    virtual ~EDPIPropertyTypeUnsignedInt();
    virtual bool	isNumber() const;
    virtual bool	hasEdit() const;
    virtual bool	hasPredefinedValues() const;
    virtual int		getValueNumber() const;
    virtual QString	getValue(int i) const;
    virtual bool	isValidValue(QString strValue) const;
};

class EDPIPropertyTypeList : public EDPIPropertyType 
{
public:
    EDPIPropertyTypeList();
    EDPIPropertyTypeList(const EDPIPropertyTypeList& rList) { m_arValues.clear(); m_arValues = rList.m_arValues; }
    virtual ~EDPIPropertyTypeList();
    virtual bool	isNumber() const;
    virtual bool	hasEdit() const;
    virtual bool	hasPredefinedValues() const;
    virtual int		getValueNumber() const;
    virtual QString	getValue(int i) const;
    virtual bool	isValidValue(QString strValue) const;
    int		getValueId(QString strValue) const;
protected:
    void	addValue(QString strValue);

private:
    QStringList m_arValues;
};

class EDPIPropertyTypeDynamicList : public EDPIPropertyTypeList
{
public:
    EDPIPropertyTypeDynamicList() {};
    EDPIPropertyTypeDynamicList(const EDPIPropertyTypeDynamicList& rList) : EDPIPropertyTypeList(rList) {};
    virtual ~EDPIPropertyTypeDynamicList() {};
    void	addValue(QString strValue) {EDPIPropertyTypeList::addValue(strValue);}; 

    inline static EDPIPropertyTypeDynamicList* getInstance() { return new EDPIPropertyTypeDynamicList(); }
    virtual void Release() { delete this;}
    virtual EDPIPropertyType* Clone() { return new EDPIPropertyTypeDynamicList(*this);}
};

class EDPIPropertyTypeListEdit : public EDPIPropertyTypeList
{
public:
    EDPIPropertyTypeListEdit() : EDPIPropertyTypeList() {}
    virtual ~EDPIPropertyTypeListEdit() {}
    virtual bool	isNumber() const {return false;}
    virtual bool	hasEdit() const {return true;}
    virtual bool	isValidValue(QString strValue) const {return true;}
};


class EDPIPropertyTypeUnsignedIntWithUnl : public EDPIPropertyTypeListEdit
{
    DEFINE_GETINSTANCE(EDPIPropertyTypeUnsignedIntWithUnl)
public:
    EDPIPropertyTypeUnsignedIntWithUnl();
    virtual ~EDPIPropertyTypeUnsignedIntWithUnl();
    virtual bool	isNumber() const;
    virtual bool	isValidValue(QString strValue) const;
};


class EDPIPropertyTypeListCSNodeTypes : public EDPIPropertyTypeList
{
    DEFINE_GETINSTANCE(EDPIPropertyTypeListCSNodeTypes);
public:
    enum {
        DISTANCE,
        REPETITION,
        INTERVAL,
        WORD,
        MRK_ITEM
    };
    EDPIPropertyTypeListCSNodeTypes();
};

class EDPIPropertyTypeBool : public EDPIPropertyTypeList
{
    DEFINE_GETINSTANCE(EDPIPropertyTypeBool);
public:
    enum {
        False,
        True
    };
    EDPIPropertyTypeBool();
};

class EDPIPropertyTypeDistType : public EDPIPropertyTypeList
{
    DEFINE_GETINSTANCE(EDPIPropertyTypeDistType);
public:
    enum {
        FINISH_TO_START,
        START_TO_START,
        MIDDLE_TO_START
    };
    EDPIPropertyTypeDistType();
};
////////////////////////////////////////////////////

class EDProjectItem : public QObject, public QTreeWidgetItem
{
Q_OBJECT
public:
    //CProjectItem(QTreeWidget * parent);
    
    EDProjectItem();
    virtual ~EDProjectItem();

    virtual QString getName() const {return name;}
    virtual void	setName(const QString& strName){ name = strName;}

    int						getGroupNumber() const;
    EDPIPropertyGroup&		getGroup(int nGroup);
    const EDPIPropertyGroup&	getGroup(int nGroup) const;
    int						addGroup(const EDPIPropertyGroup& rGroup);

    
    virtual EItemType	getType() const;
    virtual	void		update(bool bUpdateChildren) = 0;
    virtual bool		isConnectedTo(void *pData) const = 0;
    const EDProjectItem*	findItemConnectedTo(void *pData) const;

    void setMetainfoBase(const MetaInfoBase* base){mInf = base;}
    const MetaInfoBase* getMinfo(){return mInf;}

private:
    QString name;
protected:
    void clearGroups();

private:
    //mutable int					m_nArgNo;
    //mutable HTREEITEM			m_hTreeItem;
    QVector<EDPIPropertyGroup>	m_arGroups;
    //CArray<CProjectItem*>		m_arSubitems;
    const MetaInfoBase* mInf;

signals:
    void si_getMetaInfoBase();
};

class EDPICSDirectory : public EDProjectItem
{
public:
    EDPICSDirectory(const CSFolder* pFolder) : folder (pFolder)  {}
    virtual ~EDPICSDirectory() {}
    const CSFolder* getFolder() const;
    void setFolder(const CSFolder* pFolder);
    virtual	void update(bool bUpdateChildren);
    virtual bool isConnectedTo(void *pData) const;
private:
    const CSFolder* folder;

};

class EDPICSFolder : public EDPICSDirectory
{
public:
    EDPICSFolder(const CSFolder *pFolder);
    virtual ~EDPICSFolder();
    EItemType getType() const;
    virtual	void update(bool bUpdateChildren);
    //virtual CExtPopupMenuWnd*	CreatePopupMenu(HWND hWndCmdRecieve) const;
    virtual QString getName() const;
};
class ExpertDiscoveryData;
class EDPICSNode : public EDProjectItem
{
public:
    EDPICSNode(Operation *pOp);
    virtual ~EDPICSNode();
    Operation* getOperation();
    const Operation* getOperation() const;
    virtual	void update(bool bUpdateChildren);

    static EDPICSNode* createCSN(Operation *pOp);
    virtual bool isConnectedTo(void *pData) const;
    const EDProcessedSignal* getProcessedSignal(ExpertDiscoveryData& edData);

    QString getProbability() const;
    QString getFisher() const;
    QString getUl() const;
    QString getPosCoverage() const;
    QString getNegCoverage() const;
    QString getTypeAsString() const;
protected:
    Operation* pOp;
    EDProcessedSignal* m_pPS;
};

class EDPICSRoot : public EDPICSDirectory
{
public:
    EDPICSRoot(CSFolder &root);
    virtual ~EDPICSRoot();
    EItemType getType() const;
    virtual	void update(bool bUpdateChildren);
};

class EDPICS : public EDPICSNode
{
public:
    EDPICS(const Signal *pSignal);
    virtual ~EDPICS();
    EItemType getType() const;
    const Signal* getSignal() const;
    virtual	void update(bool bUpdateChildren);
    virtual bool isConnectedTo(void *pData) const;
    //virtual CExtPopupMenuWnd*	CreatePopupMenu(HWND hWndCmdRecieve) const;
    //virtual bool IsSelected() const;

    QString getPriorProbability() const;
    QString getPriorFisher() const;
    QString getPriorPosCoverage() const;
    QString getPriorNegCoverage() const;
private:
    const Signal *m_pSignal;
};

class EDPICSNUndefined : public EDPICSNode
{
public:
    EDPICSNUndefined();
    virtual ~EDPICSNUndefined();
    EItemType getType() const;
    virtual	void update(bool bUpdateChildren);
};

class EDPICSNDistance : public EDPICSNode
{
public:
    EDPICSNDistance(OpDistance *pOp);
    virtual ~EDPICSNDistance();
    EItemType getType() const;
    virtual	void update(bool bUpdateChildren);

    QString getPropDistType() const;
    QString getPropFrom() const;
    QString getPropTo() const;
    QString getPropOrder() const;
};


class EDPICSNRepetition : public EDPICSNode
{
public:
    EDPICSNRepetition(OpReiteration *pOp);
    virtual ~EDPICSNRepetition();
    EItemType getType() const;
    virtual	void update(bool bUpdateChildren);
    QString getPropDistType() const;
    QString getPropFrom() const;
    QString getPropTo() const;
    QString getCountFrom() const;
    QString getCountTo() const;
};

class EDPICSNInterval : public EDPICSNode
{
public:
    EDPICSNInterval(OpInterval *pOp);
    virtual ~EDPICSNInterval();
    EItemType getType() const;
    virtual	void update(bool bUpdateChildren);
    QString getPropFrom() const;
    QString getPropTo() const;
};

class EDPICSNTSWord : public EDPICSNode
{
public:
    EDPICSNTSWord(TS *pTS);
    virtual ~EDPICSNTSWord();
    EItemType getType() const;
    virtual	void update(bool bUpdateChildren);
    QString getPropWord() const;
};

class EDPICSNTSMrkItem : public EDPICSNode
{
public:
    EDPICSNTSMrkItem(TS *pTS);
    virtual ~EDPICSNTSMrkItem();
    EItemType getType() const;
    virtual	void update(bool bUpdateChildren);
    QString getPropFamily() const;
    QString getPropSignal() const;
};


class EDPIMrkRoot : public EDProjectItem 
{
public:
    EDPIMrkRoot();
    virtual ~EDPIMrkRoot();
    EItemType getType() const;
    virtual void update(bool bUpdateChildren);
    void updMarkup(const ExpertDiscoveryData& d);
    virtual bool isConnectedTo(void *pData) const;
    //virtual CExtPopupMenuWnd*	CreatePopupMenu(HWND hWndCmdRecieve) const;
};

class EDPIMrkFamily : public EDProjectItem 
{
public:
    EDPIMrkFamily(const DDisc::Family& rFamily);
    virtual ~EDPIMrkFamily();
    EItemType getType() const;
    virtual	void update(bool bUpdateChildren);
    virtual bool isConnectedTo(void *pData) const;

private:
    const DDisc::Family& m_rFamily;
};

class EDPIMrkItem : public EDPICSNode 
{
public:
    EDPIMrkItem(QString strFamilyName, const MetaInfo& rMetaInfo);
    virtual ~EDPIMrkItem();
    EItemType getType() const;
    virtual	void update(bool bUpdateChildren);
    virtual bool isConnectedTo(void *pData) const;

private:
    const MetaInfo& m_rMetaInfo;
};



} //namespace

#endif
