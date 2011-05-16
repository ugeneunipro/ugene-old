#ifndef _U2_EXPERT_DIS_PERSISTENT_H_
#define _U2_EXPERT_DIS_PERSISTENT_H_

#include "DDisc/Sequence.h"
#include "DDisc/MetaInfo.h"
#include "DDisc/Signal.h"
#include "ExpertDiscoveryCSUtil.h"

#include <QDataStream>

namespace U2 {

using namespace DDisc;

class EDPMSequence
{
public:
    static void save(QDataStream& ds, Sequence& rSequence);
    static void load(QDataStream& ds, Sequence& rSequence);
private:
    EDPMSequence(void);
    ~EDPMSequence(void);
    const EDPMSequence& operator =(const EDPMSequence&);
};

class EDPMSeqBase
{
public:
    static void save(QDataStream& ar, SequenceBase& rSeqBase);
    static void load(QDataStream& ar, SequenceBase& rSeqBase);
private:
    EDPMSeqBase(void);
    ~EDPMSeqBase(void);
    const EDPMSeqBase& operator =(const EDPMSeqBase&);
};


class EDPMMrkSignal
{
public:
    static void save(QDataStream& ar, Marking::IntervalSet& rMrk);
    static void load(QDataStream& ar, Marking::IntervalSet& rMrk);
private:
    EDPMMrkSignal(void);
    ~EDPMMrkSignal(void);
    const EDPMMrkSignal& operator =(const EDPMMrkSignal&);
};

class EDPMMrkFamily
{
public:
    static void save(QDataStream& ar, Marking::FamilyMarking& rMrk);
    static void load(QDataStream& ar, Marking::FamilyMarking& rMrk);
private:
    EDPMMrkFamily(void);
    ~EDPMMrkFamily(void);
    const EDPMMrkFamily& operator =(const EDPMMrkFamily&);
};

class EDPMMrk
{
public:
    static void save(QDataStream& ar, Marking& rMrk);
    static void load(QDataStream& ar, Marking& rMrk);
private:
    EDPMMrk(void);
    ~EDPMMrk(void);
    const EDPMMrk& operator =(const EDPMMrk&);
};

class EDPMMrkBase
{
public:
    static void save(QDataStream& ar, MarkingBase& rMrkBase, int nSeqCount);
    static void load(QDataStream& ar, MarkingBase& rMrkBase, int nSeqCount);
private:
    EDPMMrkBase(void);
    ~EDPMMrkBase(void);
    const EDPMMrkBase& operator =(const EDPMMrkBase&);
};


class EDPMDescInfo
{
public:
    static void save(QDataStream& ar, MetaInfo& rInfo);
    static void load(QDataStream& ar, MetaInfo& rInfo);
private:
    EDPMDescInfo(void);
    ~EDPMDescInfo(void);
    const EDPMDescInfo& operator =(const EDPMDescInfo&);
};

class EDPMDescFamily
{
public:
    static void save(QDataStream& ar, Family& rFamily);
    static void load(QDataStream& ar, Family& rFamily);
private:
    EDPMDescFamily(void);
    ~EDPMDescFamily(void);
    const EDPMDescFamily& operator =(const EDPMDescFamily&);
};


class EDPMDescription
{
public:
    static void save(QDataStream& ar, MetaInfoBase& rDesc);
    static void load(QDataStream& ar, MetaInfoBase& rDesc);
private:
    EDPMDescription(void);
    ~EDPMDescription(void);
    const EDPMDescription& operator =(const EDPMDescription&);
};

class EDPMCS
{
public:
    static void save(QDataStream& ar, Signal* pSignal);
    static void load(QDataStream& ar, Signal* pSignal);
private:
    EDPMCS(void);
    ~EDPMCS(void);
    const EDPMCS& operator =(const EDPMCS&);
};

class EDPMCSFolder
{
public:
    static void save(QDataStream& ar, CSFolder* pFolder);
    static void load(QDataStream& ar, CSFolder* pFolder);
private:
    EDPMCSFolder(void);
    ~EDPMCSFolder(void);
    const EDPMCSFolder& operator =(const EDPMCSFolder&);
};

class EDPMOperation
{
public:
    static void save(QDataStream& ar, Operation*& pOp);

    static void saveDistance(QDataStream& ar, OpDistance* pOp);
    static void saveReiteration(QDataStream& ar, OpReiteration* pOp);
    static void saveInterval(QDataStream& ar, OpInterval* pOp);
    static void saveTS(QDataStream& ar, TS* pOp);

    static void load(QDataStream& ar, Operation*& pOp);

    static void loadDistance(QDataStream& ar, OpDistance* pOp);
    static void loadReiteration(QDataStream& ar, OpReiteration* pOp);
    static void loadInterval(QDataStream& ar, OpInterval* pOp);
    static void loadTS(QDataStream& ar, TS* pOp);

private:
    EDPMOperation(void);
    ~EDPMOperation(void);
    const EDPMOperation& operator =(const EDPMOperation&);
};



   
} //namespace

#endif
