// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// $Id: reducedSurface.h,v 1.47.18.1 2007-03-25 21:25:29 oliver Exp $
//

#ifndef BALL_STRUCTURE_REDUCEDSURFACE_H
#define BALL_STRUCTURE_REDUCEDSURFACE_H

#ifndef BALL_MATHC_COMMON_H
#	include <BALL/MATHS/common.h>
#endif

#ifndef BALL_MATHS_SIMPLEBOX3_H
#	include <BALL/MATHS/simpleBox3.h>
#endif

#ifndef BALL_MATHS_CIRCLE3_H
#	include <BALL/MATHS/circle3.h>
#endif

#ifndef BALL_MATHS_SPHERE_H
#	include <BALL/MATHS/sphere3.h>
#endif

#ifndef BALL_MATHS_VECTOR3_H
#	include <BALL/MATHS/vector3.h>
#endif

#ifndef BALL_DATATYPE_HASHMAP_H
#	include <BALL/DATATYPE/hashMap.h>
#endif

#ifndef BALL_DATATYPE_HASHSET_H
#	include <BALL/DATATYPE/hashSet.h>
#endif

#ifndef BALL_COMMON_EXCEPTION_H
#	include <BALL/COMMON/exception.h>
#endif

#ifndef BALL_STRUCTURE_RSEDGE_H
#	include <BALL/STRUCTURE/RSEdge.h>
#endif

#ifndef BALL_STRUCTURE_RSFACE_H
#	include <BALL/STRUCTURE/RSFace.h>
#endif

#ifndef BALL_STRUCTURE_RSVERTEX_H
#	include <BALL/STRUCTURE/RSVertex.h>
#endif

#include <list>
#include <vector>


namespace BALL
{
	class RSComputer;
	class SolventExcludedSurface;
	class SESComputer;
	class SESSingularityCleaner;
	class TriangulatedSES;
	class SolventAccessibleSurface;
	class TriangulatedSAS;
	class SESTriangulator;

	/** Generic ReducedSurface Class.
	\ingroup Surface
	*/
	class BALL_EXPORT ReducedSurface
	{
		public:

		/** @name Class friends

					- class RSComputer
					- class SolventExcludedSurface
					- class TriangulatedSES
					- class SESComputer
					- class SESSingularityCleaner
					- class SolventAccessibleSurface
					- class TriangulatedSAS
					- class SESTriangulator
				
		*/
		friend class RSComputer;
		friend class SolventExcludedSurface;
		friend class SESComputer;
		friend class SESSingularityCleaner;
		friend class SolventAccessibleSurface;
		friend class TriangulatedSES;
		friend class TriangulatedSAS;
		friend class SESTriangulator;

		BALL_CREATE(ReducedSurface)

		/**	@name	Constructors and Destructors
		*/
		//@{

		/**	Default constructor.
				All components are initialized to <tt>(T)0</tt> or <tt>NULL</tt>,
				respectivly.
		*/
		ReducedSurface()
			throw();

		/**	Copy constructor.
				@param reduced_surface the ReducedSurface object to be copied
				@param bool ignored - just for interface consistency
		*/
		ReducedSurface(const ReducedSurface& reduced_surface, bool = true)
			throw();

		/**	Detailed constructor.
				Create a new ReducedSurface object from a list of spheres.
		*/
		ReducedSurface
				(const std::vector< TSphere3<double> >& spheres,
				 const double& probe_radius)
			throw();

		/**	Destructor.
		*/
		virtual ~ReducedSurface()
			throw();

		//@}
		/** @name Assignment
		*/
		//@{

		/**	Assign from another ReducedSurface.
				@param reduced_surface the ReducedSurface object to assign from
		*/
		void operator=(const ReducedSurface& reduced_surface)
			throw();

		/**	Assign from another ReducedSurface.
				@param reduced_surface the ReducedSurface object to assign from
		*/
		void set(const ReducedSurface& reduced_surface)
			throw();

		/** Delete all vertices, edges and faces.
		*/
		void clear()
			throw();

		/** Remove all NULL-pointers
		*/
		void clean()
			throw();

		//@}
		/**	@name	Accessors
		*/
		//@{

		/** Return the number of atoms.
				@return Size the number of atoms
		*/
		Size numberOfAtoms() const
			throw();

		/** Return the number of rsvertices.
				@return Size the number of rsvertices
		*/
		Size numberOfVertices() const
			throw();

		/** Return the number of rsedges.
				@return Size the number of rsedges
		*/
		Size numberOfEdges() const
			throw();

		/** Return the number of rsfaces.
				@return Size the number of rsfaces
		*/
		Size numberOfFaces() const
			throw();

		/** Return the probe radius.
				@return double the radius of the used probe sphere
		*/
		double getProbeRadius() const
			throw();

		/** Return the i'th sphere.
				@param	i	the index of the sphere that should be given back
				@return TSphere3<double>, the i'th sphere
		*/
    TSphere3<double> getSphere(Position i) const
			throw(Exception::IndexOverflow);

		/** Return the i'th rsvertex.
				@param	i	the index of the rsvertex that should be given back
				@return RSVertex, the i'th rsvertex
		*/
		RSVertex* getVertex(Position i) const
			throw(Exception::IndexOverflow);

		/** Return the i'th rsedge.
				@param	i	the index of the rsedge that should be given back
				@return RSEdge, the i'th rsedge
		*/
		RSEdge* getEdge(Position i) const
			throw(Exception::IndexOverflow);

		/** Return the i'th rsface.
				@param	i	the index of the rsface that should be given back
				@return RSFace, the i'th rsface
		*/
		RSFace* getFace(Position i) const
			throw(Exception::IndexOverflow);

		/** Insert a new RSVertex.
				@param	rsvertex	a pointer to the RSVertex to insert
		*/
		void insert(RSVertex* rsvertex)
			throw();

		/** Insert a new RSEdge.
				@param	rsedge	a pointer to the RSEdge to insert
		*/
		void insert(RSEdge* rsedge)
			throw();

		/** Insert a new RSFace.
				@param	rsface	a pointer to the RSFace to insert
		*/
		void insert(RSFace* rsface)
			throw();

		/** Return the maximal radius of te atoms of te molecule
				@return double	the maximal radius
		*/
		double getMaximalRadius() const
			throw();

		/** Return the bounding box of the centers of te atoms of te molecule
				@return TSimpleBox3<double>	the bounding box
		*/
		TSimpleBox3<double> getBoundingBox() const
			throw();

		/** Delete a pair of similar faces.
				@param	face1	a pointer to the first face
				@param	face2	a pointer to the second face
		*/
		void deleteSimilarFaces(RSFace* face1, RSFace* face2)
			throw();

		/** Get the angle between two RSFaces.
				@param	face1		a pointer to the first face
				@param	face2		a pointer to the second face
				@param	vertex1	a pointer to the first vertex to roll over
				@param	vertex2	a pointer to the second vertex to roll over
				@param	angle		the computed angle
				@param	check		you kan ask to check whether the angle can be computed
		*/
		bool getAngle
				(RSFace* face1,
				 RSFace* face2,
				 RSVertex* vertex1,
				 RSVertex* vertex2,
				 TAngle<double>& angle,
				 bool check = false)			const
			throw();

		/** Compute the reduced surface.
		*/
		void compute()
			throw(Exception::GeneralException,
						Exception::DivisionByZero,
						Exception::IndexOverflow);

		//@}

		private:

		/*_ Test whether a ReducedSurface object can be copied.
		*/
		bool canBeCopied(const ReducedSurface& reduced_surface)
			throw();

		/*_ Copy a ReducedSurface object.
		*/
		void copy(const ReducedSurface& reduced_surface)
			throw();

		/*_
		*/
		void correctEdges
				(RSFace* face1,
				 RSFace* face2,
				 RSEdge* edge1,
			 	 RSEdge* edge2)
			throw();

		/*_
		*/
		void joinVertices
				(RSFace* face1,
				 RSFace* face2,
				 RSVertex* vertex1,
			 	 RSVertex* vertex2)
			throw();

		/*_
		*/
		void findSimilarVertices
			 (RSFace* face1,
				RSFace* face2,
				std::vector<RSVertex*>& rsvertex1,
				std::vector<RSVertex*>& rsvertex2)
			throw();

		/*_
		*/
		void findSimilarEdges
			 (RSFace* face1,
				RSFace* face2,
				std::vector<RSEdge*>& rsedge1,
				std::vector<RSEdge*>& rsedge2)
			throw();

		protected:

    /*_ the number of atoms of the reduced surface
    */
    Size number_of_atoms_;
    /*_ the atoms of the molecule
    */
    std::vector< TSphere3<double> > atom_;
    /*_	probe radius
    */
    double probe_radius_;
    /*_ the number of vertices of the reduced surface
    */
    Size number_of_vertices_;
    /*_ the vertices of the reduced surface
    */
    std::vector< RSVertex* > vertices_;
    /*_ the number of edges of the reduced surface
    */
    Size number_of_edges_;
    /*_ the edges of the reduced surface
    */
    std::vector< RSEdge* > edges_;
    /*_ the number of faces of the reduced surface
    */
    Size number_of_faces_;
    /*_ the faces of the reduced surface
    */
    std::vector< RSFace* > faces_;
    /*_ maximal radius of all atoms
    */
    double r_max_;
    /*_ bounding SimpleBox of the atom centers of the molecule
    */
		TSimpleBox3<double> bounding_box_;

	};


	/**	@name	Storers
	*/
	//@{

	/**	Output- Operator.
			Prints out a ReducedSurface
	*/
	BALL_EXPORT std::ostream& operator << (std::ostream& s, const ReducedSurface& rs);

	//@}




	/** Generic RSComputer Class.
			\ingroup Surface
	*/
	class BALL_EXPORT RSComputer
	{
		public:

		BALL_CREATE(RSComputer)

		/**	@name	Enums
		*/
		//@{

		/** status of the probe positions of three atoms:
				<tt>0</tt> ok
				<tt>1</tt> not ok
				<tt>2</tt> not tested
		*/
		enum ProbeStatus
		{
			STATUS_OK  = 0,
			STATUS_NOT_OK = 1,
			STATUS_NOT_TESTED = 2
		};

		/** status of an atom
				<tt>0</tt> on surface
				<tt>1</tt> inside
				<tt>2</tt> outside
		*/
		enum AtomStatus
		{
			STATUS_ON_SURFACE  = 0,
			STATUS_INSIDE = 1,
			STATUS_UNKNOWN = 2
		};
		//@}

		struct ProbePosition
		{
			ProbeStatus status[2];
			TVector3<double> point[2];
		};

		/**	@name	Constructors and Destructors
		*/
		//@{

		/**	Default constructor.
				All components are initialized to <tt>(T)0</tt> or <tt>NULL</tt>,
				respectivly.
		*/
		RSComputer()
			throw();

		/**	Detiled constructor.
		*/
		RSComputer(ReducedSurface* rs)
			throw();

		/**	Destructor.
		*/
		virtual ~RSComputer()
			throw();

		//@}
		/**	@name	Accessors
		*/
		//@{

		/** Compute the reduced surface
		*/
		void run()
			throw(Exception::GeneralException,
						Exception::DivisionByZero,
						Exception::IndexOverflow);

		//@}


		private:

		/*_ @name Computing reduced surface
		*/
		//@{

		/*_
		*/
		void preProcessing()
			throw();

		/*_ Compute a RSComponent.
		*/
		void getRSComponent()
			throw(Exception::GeneralException,
						Exception::DivisionByZero,
						Exception::IndexOverflow);

		/*_ Treat all edges of a face.
			@param	face	the RSFace to be treated
		*/
		bool treatFace(RSFace* face)
			throw(Exception::GeneralException,
						Exception::DivisionByZero,
						Exception::IndexOverflow);

		/*_ Roll over an edge that belongs to only one face and find the other one.
				@param	edge	the RSEdge to be treated
		*/
		bool treatEdge(RSEdge* edge)
			throw(Exception::GeneralException,
						Exception::DivisionByZero,
						Exception::IndexOverflow);

		/*_ Treat an ambiguous situation.
				All vertices on an ambiguous atom are deleted with all its edges and
				faces. The radius of the atom is decreased by 10 EPSILON.
				@param	atom	the index of the atom
		*/
		void correct(Index atom)
	  	throw();

		/*_ Check all new created vertices for extensions
		*/
		void extendComponent()
			throw(Exception::GeneralException,
						Exception::DivisionByZero,
						Exception::IndexOverflow);


		/*_ Find a third atom rolling over two vertices starting on a face.
				From all atoms which can be touced by the probe sphere when it
				touches the given two vertices this one is chosen, for which is
				the rotation angle the smalest.
				If the rotation angle equals zero, the probe sphere can touch four
				atoms an an exception is thrown.
				If no atom can be found an exception is thrown.
			@param	vertex1	the first vertex
			@param	vertex2	the second vertex
			@param	face		the starting face
			@param	probe		the new probe sphere
			@param	phi			the rotation angle
			@return	Index		index of the found atom
		*/
		Index thirdAtom
			 (RSVertex*	vertex1,
				RSVertex* vertex2,
				RSFace*		face,
				TSphere3<double>&	probe,
				TAngle<double>&		phi)
			throw(Exception::GeneralException,
						Exception::DivisionByZero,
						Exception::IndexOverflow);

		//@}
		/*_ @name Finding a start position
		*/
		//@{

		/*_ Find a start position
			@param	vertex		a pointer to the found vertex, if only a vertex
												can be found
			@param	edge			a pointer to the found edge, if only an edge can be
												found
			@param	face			a pointer to the found face, if a face can be found
			@return Position	0, if no start position is found,
												1, if a single vertex is found,
												2, if an edge is found,
												3, if a face is found
		*/
		Position getStartPosition()
			throw(Exception::DivisionByZero);

		//@}
		/*_ @name Finding a first face
		*/
		//@{

		/*_ Try to find a starting face
			@return	RSFace*	a pointer to the found face, if a face can be found,
													NULL otherwise
		*/
		RSFace* findFirstFace()
			throw(Exception::DivisionByZero);

		/*_ Try to find a starting face in a given direction
			@param	direction		search in x-direction, if direction is 0,
													search in y-direction, if direction is 1,
													search in z-direction, if direction is 2
			@param	extrem			search in min direction, if extrem is 0,
													search in max direction, if extrem is 1
			@return	RSFace*	a pointer to the found face, if a face can be found,
													NULL otherwise
		*/
		RSFace* findFace(Position direction, Position extrem)
			throw(Exception::DivisionByZero);

		//@}
		/*_ @name Finding a first edge
		*/
		//@{

		/*_ Try to find a starting edge
			@return	RSEdge*	a pointer to the found edge, if a face can be found,
													NULL otherwise
		*/
		RSEdge* findFirstEdge()
			throw();

		/*_ Try to find a starting edge in a given direction
			@param	direction		search in x-direction, if direction is 0,
													search in y-direction, if direction is 1,
													search in z-direction, if direction is 2
			@param	extrem			search in min direction, if extrem is 0,
													search in max direction, if extrem is 1
			@return	RSEdge*	a pointer to the found edge, if a face can be found,
													NULL otherwise
		*/
		RSEdge* findEdge(Position direction, Position extrem)
			throw();

		//@}
		/*_ @name Finding a first vertex
		*/
		//@{

		/*_ Try to find a single atom
			@return	RSVertex*	a pointer to the found vertex, if a vertex can be
														found, NULL otherwise
		*/
		RSVertex* findFirstVertex()
			throw();

		/*_ Find a single atom in a given direction
			@param	direction	search in x-direction, if direction is 0,
												search in y-direction, if direction is 1,
												search in z-direction, if direction is 2
			@param	extrem		search in min direction, if extrem is 0,
												search in max direction, if extrem is 1
			@return	Index			the index of the found atom
		*/
		Index findFirstAtom(Position direction, Position extrem)
			throw();

		/*_ Find a second atom close enougth to the first atom in a given direction
			@param	atom1			the index of the first atom
			@param	direction	search in x-direction, if direction is 0,
												search in y-direction, if direction is 1,
												search in z-direction, if direction is 2
			@param	extrem		search in min direction, if extrem is 0,
												search in max direction, if extrem is 1
			@return	Index			the index of the found atom
		*/
		Index findSecondAtom(Index atom, Position direction, Position extrem)
			throw();

		/*_ Find a second atom close enougth to the first two atoms
			@param	atom1			the index of the first atom
			@param	atom2			the index of the second atom
			@param	atom_list	a HashSet of the indices of all candidate atoms
			@return	::std::list< ::std::pair< Index,TSphere3<double> > >
												a list of all candidates with their probe spheres
		*/
		void findThirdAtom
			 (Index																		 atom1,
				Index																		 atom2,
				const std::list<Index>&									 third,
				std::list< std::pair< Index,TSphere3<double> > >& atoms)
			throw();

		//@}
		/*_ @name Some utilities
		*/
		//@{

		/*_ Find all atoms close enougth to two given atoms.
			The indices of all atoms which can be touched by the probe sphere when
			it touches the given atoms are computed.
			@param	atom1				the index of the first given atom
			@param	atom2				the index of the second given atom
			@param	output_list	list of all atoms close enougth to the given atoms
		*/
		void neighboursOfTwoAtoms(Index atom1, Index atom2)
			throw();

		/*_ Find all atoms close enougth to three given atoms.
			The indices of all atoms which can be touched by the probe sphere when
			it touches the given atoms are computed.
			@param	atom1				the index of the first given atom
			@param	atom2				the index of the second given atom
			@param	atom3				the index of the third given atom
			@param	output_list	list of all atoms close enougth to the given atoms
		*/
		void neighboursOfThreeAtoms
			 (Index							atom1,
				Index							atom2,
				Index							atom3,
				std::list<Index>& output_list)
			throw();


    /*_ Get the extrem coordinate of a circle in a given direction
    	@param	circle		the circle
			@param	direction	search in x-direction, if direction is 0,
												search in y-direction, if direction is 1,
												search in z-direction, if direction is 2
			@param	extrem		search in min direction, if extrem is 0,
												search in max direction, if extrem is 1
			@return	double					the extrem coordinate
    */
		double getCircleExtremum
			 (const TCircle3<double>& circle,
				Position			 direction,
				Position			 extrem)
			throw();

		//@}
		/*_ @name Creating / updating edges / faces
		*/
		//@{

		/*_ Create a free edge from two vertices if it is a free edge
			@param	vertex1			a pointer to the first vertex
			@param	vertex2			a pointer to the second vertex
			@return	RSEdge* a pointer to the created free edge, if there is one,
													NULL otherwise
		*/
		RSEdge* createFreeEdge(RSVertex* vertex1, RSVertex* vertex2)
			throw();

		/*_ Get the circle described by the center of the probe sphere and the two
				contact circles with the atoms when the probe sphere rolls over two
				atoms
				@param	atom1		the index of the first atom
				@param	atom2		the index of the second atom
				@param	circle1	the circle described by the center of the probe sphere
				@param	circle2	the contact circle with atom1
				@param	circle3	the contact circle with atom2
				@return	bool		true, if the probe sphere can touch both atoms,
												false, otherwise
		*/
		bool getCircles
			 (Index atom1,
				Index atom2,
				TCircle3<double>& circle1,
				TCircle3<double>& circle2,
				TCircle3<double>& circle3)
			throw();

		/*_ Get the normal vector of the face described by three atoms and a probe
			@param	atom1				the index of the first atom
			@param	atom2				the index of the second atom
			@param	atom3				the index of the third atom
			@param	probe				the probe sphere lying on atom1, atom2, atom3
			@return	TVector3<double>	the normal vector
		*/
		TVector3<double> getFaceNormal
			 (const TSphere3<double>& atom1,
				const TSphere3<double>& atom2,
				const TSphere3<double>& atom3,
				const TSphere3<double>& probe)
			throw();

		/*_ Update a face and it's edges
			@param	v1		the first vertex of the face
			@param	v2		the second vertex of the face
			@param	v3		the third vertex of the face
			@param	e1		the first edge
			@param	e2		the second edge
			@param	e3		the third edge
			@param	f			the face
			@param	probe	the probe sphere of the face
		*/
		void updateFaceAndEdges
			 (RSVertex* v1,
				RSVertex* v2,
				RSVertex* v3,
				RSEdge* e1,
				RSEdge* e2,
				RSEdge* e3,
				RSFace* f,
				const TSphere3<double>& probe)
			throw();

		/*_ Test, weather a face exists or not
			@param	face				a pointer to the face to be tested
			@return	RSFace*	a pointer to the face, if it exists, otherwise NULL
		*/
		RSFace* faceExists
			 (RSFace* face,
				const std::list< RSVertex* >& vertices)
			throw();

		//@}
		/*_ @name Finding a probe sphere
		*/
		//@{

		/*_ Get the centers of the probe sphere when it lies on three atoms
			@param	a1		the first atom
			@param	a2		the second atom
			@param	a3		the third atom
			@param	c1		the first center
			@param	c2		the second center
			@return	bool	true, if the probe sphere can touch the three atoms,
										false, otherwise
		*/
		bool centerOfProbe
			 (Index		 a1,
				Index		 a2,
				Index		 a3,
				TVector3<double>& c1,
				TVector3<double>& c2)
			throw();

		/*_ Check,weather a probe sphere is inside an atom
			@param	probe	the probe sphere to be tested
			@return	bool	true, if the probe sphere is not intersecting any atom
										false, otherwise
		*/
		bool checkProbe
			 (const TSphere3<double>& probe,
				Index atom1,
				Index atom2,
				Index atom3)
			throw();

		/*_
		*/
		void correctProbePosition(Position atom)
			throw();

		/*_
		*/
		void sort
			 (Index		u1, Index		u2, Index		u3,
				Index&	s1,	Index&	s2, Index&	s3)
			throw();

		/*_
		*/
		void correctProbePosition(Position a1, Position a2, Position a3)
			throw();

		/*_
		*/
		void insert(RSVertex* vertex)
			throw();

		/*_
		*/
		void insert(RSEdge* edge)
			throw();

		/*_
		*/
		void insert(RSFace* face)
			throw();

		//@}

		protected:

		/*_ a pointer to the reduced surface to compute
		*/
		ReducedSurface* rs_;
		/*_ for each atom a list of its neighbours
		*/
		std::vector< std::list<Index> > neighbours_;
		/*_ for each atom a status
		*/
		std::vector< AtomStatus > atom_status_;
		/*_ for each pair of atoms a list of its neighbours
		*/
		HashMap< Position,
						 HashMap< Position,
						 					std::list<Index>
						 				>
					 > neighbours_of_two_;
		/*_ for each triple of atoms its probe positions
		*/
		HashMap< Position,
						 HashMap< Position,
											HashMap< Position,
															 ProbePosition*
														 >
										>
					 > probe_positions_;
		/*_ all new created vertices which are not yet checked for extensions
		*/
		HashSet<RSVertex*> new_vertices_;
		/*_ all new created faces which are not completely treated yet
		*/
		HashSet<RSFace*> new_faces_;
		/*_ for each atom a list of the rsvertices of the atom
		*/
		std::vector< std::list<RSVertex*> > vertices_;
	};


   
} // namespace BALL

#endif  // BALL_STRUCTURE_REDUCEDSURFACE_H
