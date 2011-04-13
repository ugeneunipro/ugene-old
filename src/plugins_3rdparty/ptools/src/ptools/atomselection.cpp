// $Id: atomselection.cpp 648 2008-12-23 11:05:27Z asaladin $
//
// C++ Implementation: atomselection
//
// Description:
//
//

#include "atomselection.h"
#include <algorithm> //required for set_union|intersection


namespace PTools {

AtomSelection::AtomSelection(const Rigidbody& rigid)
{
    m_rigid=&rigid;
    for (uint i=0; i < rigid.Size(); i++)
    {
        this->AddAtomIndex(i);
    }
}


AtomSelection::~AtomSelection()
{
}

AtomSelection::AtomSelection(const AtomSelection& oldsel)
{
    this->m_rigid = oldsel.m_rigid;
    this->m_list = oldsel.m_list;
}



Rigidbody AtomSelection::CreateRigid()
{
    Rigidbody newrigid;
    for (uint i=0; i<this->Size(); i++)
    {
        Atom at = m_rigid->CopyAtom(m_list[i]);
        newrigid.AddAtom(at);
    }

    return newrigid;
}

AtomSelection AtomSelection::non(const AtomSelection& atsel)
{
return !atsel;
}

AtomSelection operator& (const AtomSelection& atsel1,const  AtomSelection& atsel2)
{
    AtomSelection selout;
    if (atsel1.m_rigid != atsel2.m_rigid)
    {
        selout.m_rigid=0;
        // suppressed stdout/stderr output by UGENE
        //std::cout << "Warning: tring to find the intersection of two different rigidbody" << std::endl;
        return selout;
    }
    //else:

    selout.m_rigid = atsel1.m_rigid;
    set_intersection(atsel1.m_list.begin(), atsel1.m_list.end(),
                     atsel2.m_list.begin(), atsel2.m_list.end(), back_inserter(selout.m_list));

    return selout;
}



AtomSelection operator| (const AtomSelection& atsel1,const AtomSelection& atsel2)
{
    AtomSelection selout;
    AtomSelection cpatsel1(atsel1); //makes a copy of atsel1
    AtomSelection cpatsel2(atsel2); // makes a copy of atsel2

    if (atsel1.m_rigid != atsel2.m_rigid)
    {
        selout.m_rigid=0;
        // suppressed stdout/stderr output by UGENE
        //std::cout<<"Warning: for now you should not make union of two different rigidbody this way!" << std::endl;
        return selout;
    }
    //else:
    selout.m_rigid = atsel1.m_rigid;
    sort(cpatsel1.m_list.begin(), cpatsel1.m_list.end());
    sort(cpatsel2.m_list.begin(), cpatsel2.m_list.end());
    set_union(cpatsel1.m_list.begin(), cpatsel1.m_list.end(),
              cpatsel2.m_list.begin(), cpatsel2.m_list.end(), back_inserter(selout.m_list));

    std::vector<uint> list2;
    unique_copy(selout.m_list.begin(),selout.m_list.end(),back_inserter(list2));
    swap(selout.m_list, list2);

    return selout;
}


AtomSelection operator! (const AtomSelection& seltoinverse)
{
      //TODO: tests!
      AtomSelection selout;
      selout.SetRigid(*seltoinverse.m_rigid);
      AtomSelection all = seltoinverse.m_rigid->SelectAllAtoms();
      set_difference(all.m_list.begin(), all.m_list.end(),
                     seltoinverse.m_list.begin(), seltoinverse.m_list.end(),
                     back_inserter(selout.m_list));

      return selout;
}

}
