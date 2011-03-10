#ifndef _U2_ANNOTATION_H_
#define _U2_ANNOTATION_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Region.h>

#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtCore/QSharedData>

namespace U2 {

class U2Strand {
public:
    enum Direction {
        Direct = 1,
        Complementary = -1
    };

    U2Strand() : value(Direct)  {}
    U2Strand(Direction val) {value = val == Complementary ? Complementary : Direct;}
    
    bool isDirect() const { return value == Direct;}
    
    bool isCompementary() const { return value == Complementary;}
    
    bool operator==(const U2Strand& s) const {return value == s.value;}
        
    bool operator!=(const U2Strand& s) const {return value != s.value;}

 private:
    int value;
};


/**
    Annotation qualifier - a textual property/remark for annotation
*/
class U2CORE_EXPORT U2Qualifier {
public:
    /** Name of qualifier */
    QString name;

    /** Value of qualifier */
    QString value;



    /** Constructs new empty (and invalid) qualifier */
    U2Qualifier(){}
    
    /** Constructs new qualifier instance with name and value set */
    U2Qualifier(const QString& _name, const QString& _value) : name(_name), value(_value){}

    /** U2Qualifier is valid if  its name is not empty */
    bool isValid() const {return !name.isEmpty();}

    /** Any two qualifiers are equal if their names & values are equal */
    bool operator== ( const U2Qualifier & q ) const { return q.name == name && q.value == value; }
    
    /** Any two qualifiers are not equal if either their names or values are  not equal */
    bool operator!= ( const U2Qualifier & q ) const { return !(*this == q); }

};

/** 
    For annotation with multiple regions this flag indicates how to process joins
*/
enum U2LocationOperator {
    /** Location elements must be joined */
    U2LocationOperator_Join = 1,
    /** No need to join location elements */
    U2LocationOperator_Order = 2
};

/** 
    Type of location region
*/
enum U2LocationRegionType {
    
    /** default */
    U2LocationRegionType_Default = 0,

    /** a site between indicated adjoining bases */
    U2LocationRegionType_Site = 1, 

    /** a single base chosen from within a specified range of bases (not allowed for new entries) */
    U2LocationRegionType_SingleBase = 2
};


/** Annotation location */
class U2CORE_EXPORT U2LocationData : public QSharedData {
public:

    /** Location operator */
    U2LocationOperator      op;

    /**  Strand of the location. */
    U2Strand                strand;
    
    /** 
        Descriptor of the region: no special info, site, ...
        Note that all non-default values are affective only for 
        single region locations
    */
    U2LocationRegionType    regionType;


    /** Annotation region extends further to the left */
    bool                    truncateLeft;

    /** Annotation region extends further to the right  */
    bool                    truncateRight;

    /** Annotated regions coordinates */
    QVector<U2Region>         regions;



    /** Constructs empty location */
    U2LocationData() { reset(); }

    bool isEmpty() const {return regions.isEmpty();}

    /** Resets location to initial empty state */
    void reset();


    bool isOrder() const {return op == U2LocationOperator_Order;}

    bool isJoin() const {return op == U2LocationOperator_Join;}

    bool isMultiRegion() const {return regions.size() > 1;}

    bool isSingleRegion() const {return regions.size() == 1;}

    bool operator==(const U2LocationData& l) const;
    
    bool operator!=(const U2LocationData& l) const {return !(*this == l);}
};

inline bool U2LocationData::operator==(const U2LocationData& l) const {
    bool res = regions == l.regions && strand == l.strand && op == l.op && regionType == l.regionType 
        && truncateLeft == l.truncateLeft && truncateRight == l.truncateRight;
    return res;
}

/** 
    U2Location is a shared pointer to U2LocationData 
    The difference in behavior with normal shared pointer is that 
    it creates default U2LocationData for non-arg constructor 
    and compares LocationData by value for == and != operators
*/
class U2CORE_EXPORT U2Location {
public:
    U2Location() : d(new U2LocationData()){}
    U2Location( U2LocationData *l) : d(l){}
    
    U2LocationData&	operator*() {return *d;}
    const U2LocationData&	operator*() const {return *d;}

    U2LocationData*	operator->() {return d;}
    const U2LocationData*	operator->() const {return d;}
    
    U2LocationData *data() {return d.data(); }
    const U2LocationData *data() const {return d.constData();}

    inline operator U2LocationData *() { return d; }
    inline operator const U2LocationData *() const { return d; }

    U2Location&	operator=(const U2Location& other) {d = other.d; return *this;}

private: 
    QSharedDataPointer<U2LocationData> d;
};

inline bool	operator!= ( const U2Location & ptr1, const U2Location & ptr2 ) {return *ptr1  != *ptr2;}
inline bool	operator!= ( const U2Location & ptr1, const U2LocationData * ptr2 ) {return *ptr1  != *ptr2;}
inline bool	operator!= ( const U2LocationData * ptr1, const U2Location & ptr2 ) {return *ptr1  != *ptr2;}
inline bool	operator== ( const U2Location & ptr1, const U2Location & ptr2 ) {return *ptr1  == *ptr2;}
inline bool	operator== ( const U2Location & ptr1, const U2LocationData * ptr2 ) {return *ptr1  == *ptr2;}
inline bool	operator== ( const U2LocationData * ptr1, const U2Location & ptr2 ) {return *ptr1 == *ptr2;}

/** 
    Sequence annotation. Used to annotate set of sequence regions
*/
class U2CORE_EXPORT U2Annotation : public U2Entity {
public:
    /** Sequence this annotation is related to */
    U2DataId                sequenceId;
    
    /** Key (name) of the annotation */
    QString                 key;
    
    /** Annotation location */
    U2Location              location;

    /** Annotation related qualifiers (textual properties) */
    QVector<U2Qualifier>    qualifiers;

    /** 
        Groups (folders) this annotation must is shown in. 
        Annotation must belong to at least one group, otherwise it is automatically removed.
    */
    QStringList             groups;

    /** Version of the annotation. Same as modification count */
    qint64                  version;
};


//////////////////////////////////////////////////////////////////////////
// functions impl

inline void U2LocationData::reset() {
    regions.clear(); 
    strand = U2Strand::Direct; 
    op = U2LocationOperator_Join; 
    regionType = U2LocationRegionType_Default;
    truncateLeft = false;
    truncateRight = false;
}

} //namespace

#endif
