// $Id: superpose.cpp 644 2008-12-22 15:05:30Z asaladin $
/****************************************************************************
 *   Copyright (C) 2006-2008   Adrien Saladin                               *
 *   adrien.saladin@gmail.com                                               *
 *   Copyright (C) 2008   Pierre Poulain                                    *
 *   Copyright (C) 2008   Sebastien Fiorucci                                *
 *   Copyright (C) 2008   Chantal Prevost                                   *
 *   Copyright (C) 2008   Martin Zacharias                                  *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation, either version 3 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                          *
 ***************************************************************************/



#include <iostream>
#include <iomanip>
#include <cassert>
#include <time.h>
#include <stdlib.h> //drand

#include "superpose.h"
#include "geometry.h" // for ScalProd 
#include "rigidbody.h"
#include "rmsd.h"

#include <vector>
using std::vector;

#define EPSILON 1e-5

namespace PTools
{

// unused by UGENE
/*static void PrintMat(Mat33 mat)
{
    for (uint i=0; i<3;i++)
    {
        for (uint j=0; j<3;j++)
            std::cout << "| " << mat[i][j] << " " ;
        std::cout << " | "<<std::endl;
    }


}*/



static void Mat33xMat33(Mat33 left, Mat33 right, Mat33 out) //matrix multiplication. Works with out==left or out==right
{
    Mat33 tmp;
    for (uint k=0; k<3; k++)
        for (uint l=0; l<3; l++)
        {
            tmp[k][l]=0;
            for (uint i=0; i<3;i++)
                tmp[k][l] += left[k][i]*right[i][l];
        }

    //recopie de la matrice temporaire:
    for(uint i=0; i<3;i++)
        for(uint j=0;j<3;j++)
            out[i][j]=tmp[i][j];

}


static void XRotMatrix(double theta, Mat33 out)
{
    out[0][0]=1;
    out[0][1]=0;
    out[0][2]=0;

    out[1][0]=0;
    out[1][1]=cos(theta);
    out[1][2]=-sin(theta);

    out[2][0]=0;
    out[2][1]=sin(theta);
    out[2][2]=cos(theta);

}


static void YRotMatrix(double theta, Mat33 out)
{
    out[0][0]=cos(theta);
    out[0][1]=0;
    out[0][2]=sin(theta);

    out[1][0]=0;
    out[1][1]=1;
    out[1][2]=0;

    out[2][0]=-sin(theta);
    out[2][1]=0;
    out[2][2]=cos(theta);

}



static void ZRotMatrix(double theta, Mat33 out)
{
    out[0][0]=cos(theta);
    out[0][1]=-sin(theta);
    out[0][2]=0;

    out[1][0]=sin(theta);
    out[1][1]=cos(theta);
    out[1][2]=0;

    out[2][0]=0;
    out[2][1]=0;
    out[2][2]=1;
}



void Rotate(Rigidbody& rigid, Mat33 mat)
{
    double x,y,z;
    double X,Y,Z;
    for (uint i=0; i<rigid.Size(); i++)
    {
        Coord3D c;
        c=rigid.GetCoords(i);
        x=c.x;
        y=c.y;
        z=c.z;

        X = x*mat[0][0] + y*mat[0][1] + z*mat[0][2] ;
        Y = x*mat[1][0] + y*mat[1][1] + z*mat[1][2] ;
        Z = x*mat[2][0] + y*mat[2][1] + z*mat[2][2] ;

        c.x=X; 
        c.y=Y;
        c.z=Z;

        rigid.SetCoords(i,c) ;
    }


}

static void Mat33xcoord3D(Mat33 mat, Coord3D& in, Coord3D& out)
{
    Coord3D temp;
    temp.x = in.x*mat[0][0] + in.y*mat[0][1] + in.z*mat[0][2] ;
    temp.y = in.x*mat[1][0] + in.y*mat[1][1] + in.z*mat[1][2] ;
    temp.z = in.x*mat[2][0] + in.y*mat[2][1] + in.z*mat[2][2] ;

    out = temp ;

}


//static void rigidToMatrix(const Rigidbody & rig, double output[][3])
// VLA compartibility for non GCC compilers, by UGENE
static void rigidToMatrix(const Rigidbody & rig, vector<vector<double> > &output)
{
    for (uint atom=0; atom<rig.Size();atom++)
    {
        Coord3D c = rig.GetCoords(atom);
        output[atom][0]=c.x;
        output[atom][1]=c.y;
        output[atom][2]=c.z;
    }

}

/**
Calculates the tensor, as described by Sippl.
 */
static void MakeTensor( Rigidbody & ref, Rigidbody & mob, Mat33 out)
{

    if (ref.Size()!=mob.Size())
    {
        std::string msg = "in superpose.cpp, in MakeTensor: Error, the two rigidbodies must have the same size !!!\n";
        std::runtime_error fail(msg);
        throw fail;
    }

    //get coordinates into an array
    //double cref[ref.Size()][3];
    //double cmob[mob.Size()][3];

    // VLA compartibility for non GCC compilers, by UGENE
    vector<vector<double> > cref(ref.Size(), vector<double>(3));
    vector<vector<double> > cmob(ref.Size(), vector<double>(3));

    rigidToMatrix(ref, cref);
    rigidToMatrix(mob, cmob);

    for (uint c=0; c<3; c++)
        for(uint l=0;l<3; l++)
        {
            out[l][c]=0;
            for (uint k=0;k<ref.Size();k++)
                out[l][c]+=cref[k][c]*cmob[k][l];
        }
}



// unused by UGENE
/**
transpose a 3x3 matrix
also works when (out==in)
*/
/*static void transpose(Mat33 in, Mat33 out)
{
    Mat33 temp;

    for (uint i=0; i<3; i++)
        for (uint j=0; j<3; j++)
            temp[i][j]=in[j][i];

    for (uint i=0; i<3; i++)
        for (uint j=0; j<3; j++)
            out[i][j]=temp[i][j];

}*/

/**
Transforms a Mat44 matrix into a classical rotation matrix followed by a translation
*/
void Mat44toMat33trans(Mat44 mat44, Mat33 mat33, Coord3D& trans)
{

for (uint i=0; i<3; i++)
        for (uint j=0; j<3; j++)
                mat33[i][j]=mat44[i][j];

trans.x = mat44[3][0];
trans.y = mat44[3][1];
trans.z = mat44[3][2];
}




/*! \brief Returns uniques screw parameters that corresponds to the rotation/translation parameters
*
* Ref: Hexanions: 6D Space for Twists. 
* Alexis Angelidis. 
* Departement of Computer Science, University of Otago. (2004). Technical Report
*/
// Vissage MatTrans2screw(Mat33 rotmatrix, Coord3D t0, Coord3D t1)
Screw MatTrans2screw(const Matrix& mat)
{

    Coord3D trans;
    Mat33 rotmatrix;

    trans.x = mat(0,3);
    trans.y = mat(1,3);
    trans.z = mat(2,3);


//     Coord3D trans = t0-t1;


    for(int i=0; i<3; i++)
      for(int j=0; j<3; j++)
          rotmatrix[i][j]=mat(i,j);

//     std::cout << trans.toString();

    Screw screw;
    Coord3D eigenvect;

    Coord3D x,y,z;
    x.x = rotmatrix[0][0] ; x.y = rotmatrix[1][0]; x.z=rotmatrix[2][0];
    y.x = rotmatrix[0][1] ; y.y = rotmatrix[1][1]; y.z=rotmatrix[2][1];
    z.x = rotmatrix[0][2] ; z.y = rotmatrix[1][2]; z.z=rotmatrix[2][2];

    Coord3D pt ; //un point de l'axe de rotation


    double a = rotmatrix[0][0] ; // Xx
    double b = rotmatrix[1][1] ; // Yy
    double c = rotmatrix[2][2] ; // Zz

    if (fabs(1+a-b-c) > EPSILON)
    {
        eigenvect.x = x.x + 1 - b - c ;
        eigenvect.y = x.y + y.x ;
        eigenvect.z = x.z + z.x ;
        screw.unitVector = eigenvect / Norm(eigenvect);
        screw.normtranslation = ScalProd(screw.unitVector,trans);

        Coord3D s = trans - screw.normtranslation*screw.unitVector ;
        screw.point.x = 0 ;
        screw.point.y = s.z*z.y + s.y*(1-z.z) ;
        screw.point.z = s.y*y.z+s.z*(1-y.y);
        screw.point = screw.point/(1+x.x-y.y-z.z) ;


    }
    else if (fabs(1-a+b-c)> EPSILON)
    {
        eigenvect.x = y.x + x.y;
        eigenvect.y = y.y + 1 - x.x - z.z ;
        eigenvect.z = y.z + z.y ;

        screw.unitVector = eigenvect / Norm(eigenvect);
        screw.normtranslation = ScalProd(screw.unitVector,trans);

        Coord3D s = trans - screw.normtranslation*screw.unitVector ;
        screw.point.x =  s.z*z.x + s.x*(1-z.z);
        screw.point.y =  0 ;
        screw.point.z =  s.x*x.z + s.z*(1-x.x);
        screw.point = screw.point/(1-x.x+y.y-z.z) ;
    }
    else if (fabs(1-a-b+c)>EPSILON)
    {
        eigenvect.x = z.x + x.z ;
        eigenvect.y = z.y + y.z ;
        eigenvect.z = z.z + 1 - x.x - y.y ;

        screw.unitVector = eigenvect / Norm(eigenvect);
        screw.normtranslation = ScalProd(screw.unitVector,trans);

        Coord3D s = trans - screw.normtranslation*screw.unitVector ;
        screw.point.x = s.y*y.x + s.x*(1-y.y) ;
        screw.point.y = s.x*x.y + s.y*(1-x.x) ;
        screw.point.z =  0 ;
        screw.point = screw.point/(1-x.x-y.y+z.z) ;

    }
    else     // angle=0
      { 
            screw.point = Coord3D(0,0,0);
            if(Norm(trans)!=0)
            {
            screw.unitVector=trans /Norm(trans);
            }
            else {  screw.unitVector =  Coord3D(0,0,1); /*axe arbitraire*/ }
            screw.normtranslation=Norm(trans);
            screw.angle=0;
            return screw;


    }

    //creation d'un vecteur non aligne avec screw.unitVector:
    Coord3D v (1,0,0);
    if (fabs(Angle(screw.unitVector,v)) < 0.1) v= Coord3D(0,0,1); //v et axe colin�aires: on change v


    Coord3D u = v - ScalProd(v,screw.unitVector)*screw.unitVector ;
    u = u/Norm(u);

    Coord3D uprime;
    Mat33xcoord3D(rotmatrix,u,uprime);

    double cost = ScalProd(u,uprime);

    Coord3D usec;
    VectProd(screw.unitVector,u,usec);
    double sint = ScalProd(usec,uprime);

    
    if (cost < -1 ) cost=-1;
    if (cost >1 ) cost= 1 ; 
    screw.angle = acos(cost);
    if (sint < 0) screw.angle = -screw.angle ;

    screw.angle *= -1;
    return screw ;
}






/** \brief Superpose mob on ref
Calculates a 4x4 transformation Matrix that should be applied on 'mob' in order to minimize
RMSD between mob and ref.
Algorithm taken from Sippl et Stegbuchner. Computers Chem. Vol 15, No. 1, p 73-78, 1991.
*/
Superpose_t superpose(const Rigidbody& ref, const Rigidbody& mob, int /*verbosity*/)
{

    Rigidbody reference(ref); //copie de ref pour pouvoir centrer
    Rigidbody mobile(mob); // copie de mobile

    assert((ref.Size() == mob.Size()) && "The two AtomSelection objects must have the same size!");
    /*
    if (ref.Size()!=mob.Size()) {std::cout << "Error in superpose.cpp: \
                                       the two AtomSelection objects must have\
                                       the same size !" << std::endl;  abort();};
    */


    Mat33 rot;
    Mat33 ident;
    for (uint i=0; i<3; i++)
        for(uint j=0; j<3; j++)
        if (i!=j) {ident[i][j]=0;} else {ident[i][j]=1;} ;


    //find the translational component 
    Coord3D t0 = ref.FindCenter();
    Coord3D t1 = mob.FindCenter();


    //centre les deux objets:
    reference.CenterToOrigin();
    mobile.CenterToOrigin();


    Mat33 U; //mixed tensor
    MakeTensor(reference, mobile, U);

    for(uint i=0; i<35; i++)
    {

        double arg1,arg2;

        arg1 = U[2][1] - U[1][2] ;
        arg2 = U[1][1] + U[2][2];
        double alpha = atan2(arg1 , arg2 );

        XRotMatrix(-alpha, rot);
        Mat33xMat33(rot,ident,ident);

        //------------------------------------


        Mat33xMat33(rot,U,U);

        arg1 = U[2][0]-U[0][2];
        arg2 = U[0][0]+U[2][2];

        double beta = atan2(arg1,arg2);
        YRotMatrix(beta,rot);
        Mat33xMat33(rot,ident,ident);

        //--------------------------------------


        Mat33xMat33(rot,U,U);

        arg1 = U[1][0] - U[0][1];
        arg2 = U[0][0] + U[1][1];
        double gamma = atan2(arg1,arg2);

        ZRotMatrix(-gamma,rot);
        Mat33xMat33(rot,ident,ident);


        Mat33xMat33(rot,U,U);

    }

    //creates a 4x4 matrix that tracks transformations for mobile
    Mat44 tracker;
    MakeTranslationMat44(Coord3D()-t1,tracker);

    //copy the 3x3 matrix into a 4x4 matrix
    Mat44 rotation;
    MakeTranslationMat44(Coord3D(),rotation); //identity matrix;
    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
         rotation[i][j]=ident[i][j];



    mat44xmat44(rotation, tracker, tracker);

    MakeTranslationMat44(t0, rotation);
    mat44xmat44(rotation, tracker, tracker);

    Matrix output(4,4);

    for (int i=0; i<4; i++)
     for (int j=0; j<4;j++)
       output(i,j)=tracker[i][j];

    Superpose_t sup;
    sup.matrix = output;

    Rigidbody probe(mob);
    probe.ApplyMatrix(output);

    sup.rmsd = Rmsd(ref,probe);
    return sup;



//     screw = MatTrans2screw(ident, t0, t1);
//     screw.point = screw.point + t1 ;




/*
    if (verbosity==1)
    {


        Rigidbody newmob(mob);
        Rigidbody newref(ref);
        selref.setRigid(newref);
        selmob.setRigid(newmob);
        newmob.transform(screw);

        std::cout << "verif screw, rmsdca = " << rmsd(selmob && CA(newmob),selref && CA(newref)) << std::endl ;

    }
*/

//     return screw;
}


}

