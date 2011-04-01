// $Id: atomselection.h 648 2008-12-23 11:05:27Z asaladin $
//
// C++ Interface: atomselection
//
// Description:
//
//

#ifndef ATOMSELECTION_H
#define ATOMSELECTION_H

#include "rigidbody.h"

namespace PTools {


class AtomSelection{

private:
    //private data
    const Rigidbody* m_rigid;
    std::vector<uint> m_list;

    //private methods



public:
    AtomSelection(){};
    AtomSelection(const AtomSelection& oldsel);
    AtomSelection(const Rigidbody& rigid); ///< translate a Rigidbody object into an AtomSelection
    ~AtomSelection();

    uint Size() const {return m_list.size();}; ///< returns the size of the selection
    void SetRigid(const Rigidbody& rig) {m_rigid=&rig;};

    /// return the i-th atom of the list
    Atom operator[] (uint i) const {
          if (i>=this->Size()) throw std::range_error("AtomSelection: array out of bounds");
          return m_rigid->CopyAtom(m_list[i]);}; 

    Atom CopyAtom(uint i) const {return m_rigid->CopyAtom(m_list[i]);}
    void AddAtomIndex(uint i) {m_list.push_back(i);}; ///< adds an Atom index
    Rigidbody CreateRigid(); ///< makes a new rigidcopy (independant copy) from an AtomsSlection object.

    AtomSelection non(const AtomSelection& atsel);


    //friends:
    friend AtomSelection operator& (const AtomSelection& atsel1,const  AtomSelection& atsel2);
    friend AtomSelection operator| (const AtomSelection& atsel1,const AtomSelection& atsel2);
    friend AtomSelection operator! (const AtomSelection& seltoinverse);
};


AtomSelection operator& (const AtomSelection& atsel1,const  AtomSelection& atsel2);
AtomSelection operator| (const AtomSelection& atsel1,const AtomSelection& atsel2);
AtomSelection operator! (const AtomSelection& seltoinverse);

}

#endif // ATOMSELECTION_H
