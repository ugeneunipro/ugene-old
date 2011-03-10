// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// $Id: reducedSurface.C,v 1.9.28.1 2007-03-22 11:48:19 oliver Exp $
//

#include <BALL/STRUCTURE/reducedSurface.h>

#include <BALL/MATHS/analyticalGeometry.h>
#include <BALL/DATATYPE/hashGrid.h>

namespace BALL
{

	ReducedSurface::ReducedSurface()
		throw()
		:	number_of_atoms_(0),
			atom_(),
			probe_radius_(0.0),
			number_of_vertices_(0),
			vertices_(),
			number_of_edges_(0),
			edges_(),
			number_of_faces_(0),
			faces_(),
	    r_max_(0),
			bounding_box_()
	{
	}


	ReducedSurface::ReducedSurface
			(const ReducedSurface& reduced_surface, bool)
		throw()
		:	number_of_atoms_(0),
			atom_(),
			probe_radius_(0.0),
			number_of_vertices_(0),
			vertices_(),
			number_of_edges_(0),
			edges_(),
			number_of_faces_(0),
			faces_(),
	    r_max_(0),
			bounding_box_()
	{
		copy(reduced_surface);
	}


	ReducedSurface::ReducedSurface
		 (const std::vector< TSphere3<double> >& spheres,
			const double& probe_radius)
		throw()
		:	number_of_atoms_(spheres.size()),
			atom_(spheres),
			probe_radius_(probe_radius),
			number_of_vertices_(0),
			vertices_(),
			number_of_edges_(0),
			edges_(),
			number_of_faces_(0),
			faces_(),
	    r_max_(0),
			bounding_box_()
	{
	}


	ReducedSurface::~ReducedSurface()
		throw()
	{
		clear();
	}


	void ReducedSurface::set(const ReducedSurface& reduced_surface)
		throw()
	{
		copy(reduced_surface);
	}


	void ReducedSurface::operator=(const ReducedSurface& reduced_surface)
		throw()
	{
		copy(reduced_surface);
	}


	void ReducedSurface::clear()
		throw()
	{
		for (Position i = 0; i < number_of_vertices_; i++)
		{
			delete vertices_[i];
		}
		for (Position i = 0; i < number_of_edges_; i++)
		{
			delete edges_[i];
		}
		for (Position i = 0; i < number_of_faces_; i++)
		{
			delete faces_[i];
		}
		vertices_.clear();
		edges_.clear();
		faces_.clear();
		number_of_vertices_ = 0;
		number_of_edges_ = 0;
		number_of_faces_ = 0;
	}


	void ReducedSurface::clean()
		throw()
	{
		while ((number_of_vertices_ > 0) &&
					 (vertices_[number_of_vertices_-1] == NULL))
		{
			vertices_.pop_back();
			number_of_vertices_--;
		}
		for (Position i = 0; i < number_of_vertices_; i++)
		{
			if (vertices_[i] == NULL)
			{
				vertices_[i] = vertices_[number_of_vertices_-1];
				vertices_[i]->index_ = i;
				vertices_.pop_back();
				number_of_vertices_--;
				while (vertices_[number_of_vertices_-1] == NULL)
				{
					vertices_.pop_back();
					number_of_vertices_--;
				}
			}
		}
		while ((number_of_edges_ > 0) &&
					 (edges_[number_of_edges_-1] == NULL))
		{
			edges_.pop_back();
			number_of_edges_--;
		}
		for (Position i = 0; i < number_of_edges_; i++)
		{
			if (edges_[i] == NULL)
			{
				edges_[i] = edges_[number_of_edges_-1];
				edges_[i]->index_ = i;
				edges_.pop_back();
				number_of_edges_--;
				while (edges_[number_of_edges_-1] == NULL)
				{
					edges_.pop_back();
					number_of_edges_--;
				}
			}
		}
		while ((number_of_faces_ > 0) &&
					 (faces_[number_of_faces_-1] == NULL))
		{
			faces_.pop_back();
			number_of_faces_--;
		}
		for (Position i = 0; i < number_of_faces_; i++)
		{
			if (faces_[i] == NULL)
			{
				faces_[i] = faces_[number_of_faces_-1];
				faces_[i]->index_ = i;
				faces_.pop_back();
				number_of_faces_--;
				while (faces_[number_of_faces_-1] == NULL)
				{
					faces_.pop_back();
					number_of_faces_--;
				}
			}
		}
	}


	Size ReducedSurface::numberOfAtoms() const
		throw()
	{
		return number_of_atoms_;
	}


	Size ReducedSurface::numberOfVertices() const
		throw()
	{
		return number_of_vertices_;
	}


	Size ReducedSurface::numberOfEdges() const
		throw()
	{
		return number_of_edges_;
	}


	Size ReducedSurface::numberOfFaces() const
		throw()
	{
		return number_of_faces_;
	}


	double ReducedSurface::getProbeRadius() const
		throw()
	{
		return probe_radius_;
	}


	TSphere3<double> ReducedSurface::getSphere(Position i) const
		throw(Exception::IndexOverflow)
	{
		if (i < number_of_atoms_)
		{
			return atom_[i];
		}
		else
		{
			throw Exception::IndexOverflow(__FILE__, __LINE__,
																		 i,number_of_atoms_-1);
		}
	}


	RSVertex* ReducedSurface::getVertex(Position i) const
		throw(Exception::IndexOverflow)
	{
		if (i < number_of_vertices_)
		{
			return vertices_[i];
		}
		else
		{
			throw Exception::IndexOverflow(__FILE__, __LINE__,
																		 i,number_of_vertices_-1);
		}
	}


	RSEdge* ReducedSurface::getEdge(Position i) const
		throw(Exception::IndexOverflow)
	{
		if (i < number_of_edges_)
		{
			return edges_[i];
		}
		else
		{
			throw Exception::IndexOverflow(__FILE__, __LINE__,
																		 i,number_of_edges_-1);
		}
	}


	RSFace* ReducedSurface::getFace(Position i) const
		throw(Exception::IndexOverflow)
	{
		if (i < number_of_faces_)
		{
			return faces_[i];
		}
		else
		{
			throw Exception::IndexOverflow(__FILE__, __LINE__,
																		 i,number_of_faces_-1);
		}
	}


	void ReducedSurface::insert(RSVertex* rsvertex)
		throw()
	{
		rsvertex->index_ = number_of_vertices_;
		vertices_.push_back(rsvertex);
		number_of_vertices_++;
	}


	void ReducedSurface::insert(RSEdge* rsedge)
		throw()
	{
		rsedge->index_ = number_of_edges_;
		edges_.push_back(rsedge);
		number_of_edges_++;
	}


	void ReducedSurface::insert(RSFace* rsface)
		throw()
	{
		rsface->index_ = number_of_faces_;
		faces_.push_back(rsface);
		number_of_faces_++;
	}


	double ReducedSurface::getMaximalRadius() const
		throw()
	{
		return r_max_;
	}


	TSimpleBox3<double> ReducedSurface::getBoundingBox() const
		throw()
	{
		return bounding_box_;
	}


	void ReducedSurface::deleteSimilarFaces
		 (RSFace* face1,
			RSFace* face2)
		throw()
	{
		if ((*face1) *= (*face2))
		{
			// find the similar edges
			std::vector<RSEdge*> rsedge1(3);
			std::vector<RSEdge*> rsedge2(3);
			//RSEdge* rsedge1[3];
			//RSEdge* rsedge2[3];
			findSimilarEdges(face1,face2,rsedge1,rsedge2);
			// find the similar vertices
			std::vector<RSVertex*> rsvertex1(3);
			std::vector<RSVertex*> rsvertex2(3);
			//RSVertex* rsvertex1[3];
			//RSVertex* rsvertex2[3];
			findSimilarVertices(face1,face2,rsvertex1,rsvertex2);
			// join the similar vertices and delete the faces in their face lists
			for (Position i = 0; i < 3; i++)
			{
				joinVertices(face1,face2,rsvertex1[i],rsvertex2[i]);
			}
			// correct the edges
			for (Position i = 0; i < 3; i++)
			{
				correctEdges(face1,face2,rsedge1[i],rsedge2[i]);
			}
			faces_[face1->index_] = NULL;
			faces_[face2->index_] = NULL;
			delete face1;
			delete face2;
		}
	}


	void ReducedSurface::findSimilarEdges
		 (RSFace* face1,
			RSFace* face2,
			std::vector<RSEdge*>& rsedge1,
			std::vector<RSEdge*>& rsedge2)
			//RSEdge*& rsedge1[3],
			//RSEdge*& rsedge2[3])
		throw()
	{
		rsedge1[0] = face1->edge_[0];
		rsedge1[1] = face1->edge_[1];
		rsedge1[2] = face1->edge_[2];
		RSEdge* edge;
		for (Position j = 0; j < 3; j++)
		{
			for (Position i = 0; i < 3; i++)
			{
				edge = face2->getEdge(i);
				if (*edge *= *rsedge1[j])
				{
					rsedge2[j] = edge;
				}
			}
		}
	}


	void ReducedSurface::findSimilarVertices
		 (RSFace* face1,
			RSFace* face2,
			std::vector<RSVertex*>& rsvertex1,
			std::vector<RSVertex*>& rsvertex2)
			//RSVertex*& rsvertex1[3],
			//RSVertex*& rsvertex2[3])
		throw()
	{
		rsvertex1[0] = face1->vertex_[0];
		rsvertex1[1] = face1->vertex_[1];
		rsvertex1[2] = face1->vertex_[2];
		RSVertex* vertex;
		for (Position j = 0; j < 3; j++)
		{
			for (Position i = 0; i < 3; i++)
			{
				vertex = face2->getVertex(i);
				if (vertex->atom_ == rsvertex1[j]->atom_)
				{
					rsvertex2[j] = vertex;
				}
			}
		}
	}


	void ReducedSurface::joinVertices
			(RSFace* face1,
			 RSFace* face2,
			 RSVertex* vertex1,
		 	 RSVertex* vertex2)
		throw()
	{
		if (vertex1 != vertex2)
		{
			vertex1->join(*vertex2);
			vertex2->substitute(vertex1);
			vertices_[vertex2->index_] = NULL;
			delete vertex2;
		}
		vertex1->faces_.erase(face1);
		vertex1->faces_.erase(face2);
	}


	void ReducedSurface::correctEdges
			(RSFace* face1,
			 RSFace* face2,
			 RSEdge* edge1,
		 	 RSEdge* edge2)
		throw()
	{
		if (edge1 == edge2)
		{
			if (edge1->singular_)
			{
				edge1->vertex_[0]->edges_.erase(edge1);
				edge1->vertex_[1]->edges_.erase(edge1);
				edges_[edge1->index_] = NULL;
				delete edge1;
			}
			else
			{
				edge1->face_[0] = NULL;
				edge1->face_[1] = NULL;
				edge1->angle_.value = 2*Constants::PI;
			}
		}
		else
		{
			RSFace* neighbour2 = edge2->other(face2);
			if (edge1->face_[0] == face1)
			{
				edge1->face_[0] = neighbour2;
			}
			else
			{
				edge1->face_[1] = neighbour2;
			}
			for (Position j = 0; j < 3; j++)
			{
				if (neighbour2->getEdge(j) == edge2)
				{
					neighbour2->setEdge(j,edge1);
				}
			}
			edge2->vertex_[0]->edges_.erase(edge2);
			edge2->vertex_[1]->edges_.erase(edge2);
			edges_[edge2->index_] = NULL;
			delete edge2;
			// recomputation of rsedge1[i]->angle_
			RSFace* neighbour1 = edge1->face_[0];
			neighbour2 = edge1->face_[1];
			getAngle(neighbour1,neighbour2,edge1->vertex_[0],
							 edge1->vertex_[1],edge1->angle_,false);
		}
	}


	bool ReducedSurface::getAngle
			(RSFace* face1,
			 RSFace* face2,
			 RSVertex* vertex1,
			 RSVertex* vertex2,
			 TAngle<double>& angle,
			 bool check) const
		throw()
	{
		if (check)
		{
			if (!(face1->has(vertex1) && face1->has(vertex2) 
						&& face2->has(vertex1) && face2->has(vertex2)))
			{
				return false;
			}
		}

		RSVertex* vertex3(face1->third(vertex1,vertex2));
		TSphere3<double> atom1(atom_[vertex1->atom_]);
		TSphere3<double> atom2(atom_[vertex2->atom_]);
		TSphere3<double> atom3(atom_[vertex3->atom_]);
		TVector3<double> axis(atom1.p - atom2.p);
		TVector3<double> test(axis % face1->normal_);
		if (Maths::isLess(test*(atom1.p - atom3.p), 0.0))
		{
			axis.negate();
		}
		atom1.radius += probe_radius_;
		atom2.radius += probe_radius_;
		TCircle3<double> circle;
		GetIntersection(atom1, atom2, circle);
		TVector3<double> v1 = face1->center_ - circle.p;
		TVector3<double> v2 = face2->center_ - circle.p;
		angle = getOrientedAngle(v1, v2, axis);

		return true;
	}


	void ReducedSurface::copy(const ReducedSurface& reduced_surface)
		throw()
	{
		if (canBeCopied(reduced_surface))
		{
			number_of_atoms_ = reduced_surface.number_of_atoms_;
			atom_ = reduced_surface.atom_;
			probe_radius_ = reduced_surface.probe_radius_;
			number_of_vertices_ = reduced_surface.number_of_vertices_;
			number_of_edges_ = reduced_surface.number_of_edges_;
			number_of_faces_ = reduced_surface.number_of_faces_;
			RSVertex* vertex;
			RSEdge* edge;
			RSFace* face;
			for (Position i = 0; i < number_of_vertices_; i++)
			{
				vertex = new RSVertex(*reduced_surface.vertices_[i],false);
				vertices_.push_back(vertex);
			}
			for (Position i = 0; i < number_of_edges_; i++)
			{
				edge = new RSEdge(*reduced_surface.edges_[i],false);
				edges_.push_back(edge);
			}
			for (Position i = 0; i < number_of_faces_; i++)
			{
				face = new RSFace(*reduced_surface.faces_[i],false);
				faces_.push_back(face);
			}
			HashSet<RSEdge*>::ConstIterator e;
			HashSet<RSFace*>::ConstIterator f;
			for (Position i = 0; i < number_of_vertices_; i++)
			{
				for (e = reduced_surface.vertices_[i]->edges_.begin();
						 e != reduced_surface.vertices_[i]->edges_.end();
						 e++)
				{
					vertices_[i]->edges_.insert(edges_[(*e)->index_]);
				}
				for (f = reduced_surface.vertices_[i]->faces_.begin();
						 f != reduced_surface.vertices_[i]->faces_.end();
						 f++)
				{
					vertices_[i]->faces_.insert(faces_[(*f)->index_]);
				}
			}
			for (Position i = 0; i < number_of_edges_; i++)
			{
				edge = reduced_surface.edges_[i];
				edges_[i]->vertex_[0] = vertices_[edge->vertex_[0]->index_];
				edges_[i]->vertex_[1] = vertices_[edge->vertex_[1]->index_];
				edges_[i]->face_[0] = faces_[edge->face_[0]->index_];
				edges_[i]->face_[1] = faces_[edge->face_[1]->index_];
			}
			for (Position i = 0; i < number_of_faces_; i++)
			{
				face = reduced_surface.faces_[i];
				faces_[i]->vertex_[0] = vertices_[face->vertex_[0]->index_];
				faces_[i]->vertex_[1] = vertices_[face->vertex_[1]->index_];
				faces_[i]->vertex_[2] = vertices_[face->vertex_[2]->index_];
				faces_[i]->edge_[0] = edges_[face->edge_[0]->index_];
				faces_[i]->edge_[1] = edges_[face->edge_[1]->index_];
				faces_[i]->edge_[2] = edges_[face->edge_[2]->index_];
			}
		}
	}


	bool ReducedSurface::canBeCopied
			(const ReducedSurface& reduced_surface)
		throw()
	{
		for (Position i = 0; i < number_of_vertices_; i++)
		{
			if (reduced_surface.vertices_[i] == NULL)
			{
				return false;
			}
			if (reduced_surface.vertices_[i]->index_ < 0)
			{
				return false;
			}
		}
		for (Position i = 0; i < number_of_edges_; i++)
		{
			if (reduced_surface.edges_[i] == NULL)
			{
				return false;
			}
			if (reduced_surface.edges_[i]->index_ < 0)
			{
				return false;
			}
		}
		for (Position i = 0; i < number_of_faces_; i++)
		{
			if (reduced_surface.faces_[i] == NULL)
			{
				return false;
			}
			if (reduced_surface.faces_[i]->index_ < 0)
			{
				return false;
			}
		}
		return true;
	}


	void ReducedSurface::compute()
		throw(Exception::GeneralException,
					Exception::DivisionByZero,
					Exception::IndexOverflow)
	{
		RSComputer rsc(this);
		rsc.run();
	}


	std::ostream& operator << (std::ostream& s, const ReducedSurface& rs)
	{
		s << "Spheres:\n";
		for (Position i = 0; i < rs.numberOfAtoms(); i++)
		{
			s << "  " << rs.getSphere(i) << "\n";
	  }
	  s << "RSVertices:\n";
		for (Position i = 0; i < rs.numberOfVertices(); i++)
		{
			if (rs.getVertex(i) == NULL)
      {
				s << "  --\n";
      }
      else
      {
				s << "  " << rs.getVertex(i) << "  " << *(rs.getVertex(i)) << "\n";
      }
	  }
	  s << "RSEdges:\n";
		for (Position i = 0; i < rs.numberOfEdges(); i++)
		{
			if (rs.getEdge(i) == NULL)
      {
				s << "  --\n";
      }
      else
      {
				s << "  " << rs.getEdge(i) << "  " << *(rs.getEdge(i)) << "\n";
			}
	  }
	  s << "RSFaces:\n";
		for (Position i = 0; i < rs.numberOfFaces(); i++)
		{
			if (rs.getFace(i) == NULL)
      {
				s << "  --\n";
      }
      else
      {
				s << "  " << rs.getFace(i) << "  " << *(rs.getFace(i)) << "\n";
			}
	  }
	  return s;
	}

////////////////////////////////////

	RSComputer::RSComputer()
		throw()
		:	rs_(NULL),
			neighbours_(),
			atom_status_(),
			neighbours_of_two_(),
			probe_positions_(),
			new_vertices_(),
			new_faces_(),
			vertices_()
	{
	}


	RSComputer::RSComputer(ReducedSurface* rs)
		throw()
		:	rs_(rs),
			neighbours_(rs->number_of_atoms_),
			atom_status_(rs->number_of_atoms_,STATUS_UNKNOWN),
			neighbours_of_two_(),
			probe_positions_(),
			new_vertices_(),
			new_faces_(),
			vertices_(rs->number_of_atoms_)
	{
	}


	RSComputer::~RSComputer()
		throw()
	{
		// delete probe_positions
		HashMap< Position,
						 HashMap< Position,
						 					HashMap< Position,ProbePosition* > > >::Iterator pp1;
		HashMap< Position,HashMap< Position,ProbePosition* > >::Iterator pp2;
		HashMap< Position,ProbePosition* >::Iterator pp3;
		for (pp1 = probe_positions_.begin(); pp1 != probe_positions_.end(); pp1++)
		{
			for (pp2 = pp1->second.begin(); pp2 != pp1->second.end(); pp2++)
			{
				for (pp3 = pp2->second.begin(); pp3 != pp2->second.end(); pp3++)
				{
					delete pp3->second;
				}
			}
		}
	}


	void RSComputer::run()
		throw(Exception::GeneralException,
					Exception::DivisionByZero,
					Exception::IndexOverflow)
	{
		double epsilon = Constants::EPSILON;
		Constants::EPSILON = 1e-4;
		// find for each atom its neighbours
		preProcessing();
		// start the computation
		Position start = 1;
		while (start != 0)
		{
			start = getStartPosition();
			switch (start)
			{
				case 2 :	extendComponent();
									break;
				case 3 :	getRSComponent();
									break;
				default :	break;
			}
		}
		rs_->clean();
		Constants::EPSILON = epsilon;
	}


	void RSComputer::getRSComponent()
		throw(Exception::GeneralException,
					Exception::DivisionByZero,
					Exception::IndexOverflow)
	{
		Position i = 0;
		while (i < rs_->number_of_faces_)
		{
			if (rs_->faces_[i] != NULL)
			{
				if (treatFace(rs_->faces_[i]) == false)
				{
					i = 0;
				}
				else
				{
					i++;
				}
			}
			else
			{
				i++;
			}
		}
		extendComponent();
		//while (new_faces_.size() > 0)
		//{
		//	treatFace(*new_faces_.begin());
		//}
		//extendComponent();
	}


	bool RSComputer::treatFace(RSFace* face)
		throw(Exception::GeneralException,
					Exception::DivisionByZero,
					Exception::IndexOverflow)
	{
		if (face->edge_[0]->face_[1] == NULL)
		{
			if (!treatEdge(face->edge_[0]))
			{
				return false;
			}
		}
		if (face->edge_[1]->face_[1] == NULL)
		{
			if (!treatEdge(face->edge_[1]))
			{
				return false;
			}
		}
		if (face->edge_[2]->face_[1] == NULL)
		{
			if (!treatEdge(face->edge_[2]))
			{
				return false;
			}
		}
		new_faces_.erase(face);
		return true;
	}


	bool RSComputer::treatEdge(RSEdge* edge)
		throw(Exception::GeneralException,
					Exception::DivisionByZero,
					Exception::IndexOverflow)
	{
		// This function rolls the probe sphere over a RSEdge.
		// From all atoms that can be touced by the probe sphere when it touches
		// the two atoms of the edge is this one selected for which the rotation
		// angle is the smallest. A new face is found.
		// If this face already exists the edge exists twice, too. These two
		// edges and their vertices are joined.
		// If the face does not exist yet, it will be created. A new vertex and
		// two new edges will be created, too.
		// In both cases the treated edge will be updated. It has not to be
		// considerd again.

		// find third atom
		TAngle<double> phi;
		TSphere3<double> probe;
		RSFace* start_face(edge->face_[0]);			// the edge already knows the
		RSVertex* vertex1(edge->vertex_[0]);		// starting face and their
		RSVertex* vertex2(edge->vertex_[1]);		// two vertices
		RSVertex* vertex3(NULL);
		Index atom1(vertex1->atom_);
		Index atom2(vertex2->atom_);
		Index atom3;
		try
		{
			atom3 = thirdAtom(vertex1,vertex2,start_face,probe,phi);
		}
		catch (Exception::GeneralException e)
		{
			String message = e.getMessage();
			String test_message = "PROBE SPHERE TOUCHES FOUR ATOMS";
			if (message == test_message)
			{
				return false;
			}
			else
			{
				throw;
			}
		}
		TSphere3<double> sphere1(rs_->atom_[atom1]);
		TSphere3<double> sphere2(rs_->atom_[atom2]);
		TSphere3<double> sphere3(rs_->atom_[atom3]);
                // build a new face and two new edges
		vertex3 = new RSVertex(atom3);
		RSEdge* edge1;
		RSEdge* edge2;
		RSFace* new_face												// provisorial new face
				= new RSFace(vertex1,vertex2,vertex3,NULL,NULL,NULL,
												 probe.p,getFaceNormal(sphere1,sphere2,sphere3,probe),
												 false,-1);
		RSFace* test = faceExists(new_face,vertices_[vertex1->atom_]);
		if (test == NULL)
		{
			// built face doesn't exist yet
			// The new vertex has to be created since we don't know at this time
			// whether it is a new vertex or not.
			// Attention: one atom can build more than one vertex!
			insert(vertex3);
			edge1 = new RSEdge;
			edge1->vertex_[0] = vertex2;
			edge1->vertex_[1] = vertex3;
			edge1->face_[0] = new_face;
			edge2 = new RSEdge;
			edge2->vertex_[0] = vertex3;
			edge2->vertex_[1] = vertex1;
			edge2->face_[0] = new_face;
			new_face->edge_[0] = edge;
			new_face->edge_[1] = edge1;
			new_face->edge_[2] = edge2;
			TPlane3<double> plane(sphere1.p,sphere2.p,sphere3.p);
			new_face->singular_ = Maths::isLess(GetDistance(probe.p,plane),
																					rs_->probe_radius_);
			insert(new_face);
		}
		else
		{
			// built face exitsts already
			// the corresponding edge in the existing face has to be found
			RSEdge* test_edge;
			Index i = test->getSimilarEdge(edge,test_edge);
			// Now the corresponding vertices of the corresponding edges have to be
			// joined and one of them has to be deleted (if they are not equal). This
			// is neccessary since creating a new face always creates a new vertex.
			RSVertex* test_vertex1 = test_edge->vertex_[0];
			RSVertex* test_vertex2 = test_edge->vertex_[1];
			if (test_vertex1->atom_ == vertex2->atom_)
			{
				RSVertex* tmp = test_vertex1;
				test_vertex1 = test_vertex2;
				test_vertex2 = tmp;
			}
			// now we know which vertices are corresponding
			if (*vertex1 != *test_vertex1)
			{
				// the vertices only have to be joined if they are not equal
				vertex1->join(*test_vertex1);
				test_vertex1->substitute(vertex1);
				rs_->vertices_[test_vertex1->index_] = NULL;
				new_vertices_.erase(test_vertex1);
				vertices_[test_vertex1->atom_].remove(test_vertex1);
				delete test_vertex1;
			}
			if (*vertex2 != *test_vertex2)
			{
				// the vertices only have to be joined if they are not equal
				vertex2->join(*test_vertex2);
				test_vertex2->substitute(vertex2);
				rs_->vertices_[test_vertex2->index_] = NULL;
				new_vertices_.erase(test_vertex2);
				vertices_[test_vertex2->atom_].remove(test_vertex2);
				delete test_vertex2;
			}
			// The vertices should have only one of the two corresponding edges.
			// The other will be deleted later.
			vertex1->edges_.erase(test_edge);
			vertex2->edges_.erase(test_edge);
			// The face should have only one of the two corresponding edges, too.
			test->setEdge(i,edge);
			// Now can we delete the build face and vertex and the double edge.
			delete new_face;
			if (test_edge->index_ != -1)		// this can happens after a correct step
			{
				rs_->edges_[test_edge->index_] = NULL;
			}
			delete test_edge;
			delete vertex3;
			new_face = test;
		}			// face exitsts test
		// update edge
		TCircle3<double> circle1;
		TCircle3<double> circle2;
		TCircle3<double> circle3;
		getCircles(atom1,atom2,circle1,circle2,circle3);
		TVector3<double> ip1;		// intersection points between
		TVector3<double> ip2;		// the edge and the probe sphere
		TLine3<double> line(sphere1.p,sphere2.p,TLine3<double>::FORM__TWO_POINTS);
		bool singular(GetIntersection(probe,line,ip1,ip2));
		if (singular &&
				Maths::isLess(ip1.getSquareDistance(sphere2.p),
											ip2.getSquareDistance(sphere2.p)))
		{										// ip1 is the intersection point next to the first
			ip1.swap(ip2);		// vertex of the edge
		}
		edge->face_[1] = new_face;
		edge->center_of_torus_ = circle1.p;
		edge->radius_of_torus_ = circle1.radius;
		edge->angle_ = phi;
		edge->circle0_ = circle2;
		edge->circle1_ = circle3;
		edge->intersection_point0_ = ip1;
		edge->intersection_point1_ = ip2;
		edge->singular_ = singular;
		if (edge->index_ == -1)
		{
			rs_->insert(edge);
		}
		return true;
	}


	void RSComputer::correct(Index atom)
	  throw()
	{
		std::list<RSVertex*>::iterator v;
		RSVertex* vertex;
		HashSet<RSFace*> faces;
		HashSet<RSFace*> treat_faces;
		HashSet<RSFace*>::Iterator f;
		HashSet<RSVertex*> test_vertices;
		HashSet<RSEdge*> delete_edges;
		v = vertices_[atom].begin();
		while (v != vertices_[atom].end())
		{
			treat_faces.clear();
			test_vertices.clear();
			delete_edges.clear();
			vertex = *v;
			v++;
			faces = vertex->faces_;
			for (f = faces.begin(); f != faces.end(); f++)
			{
				(*f)->remove(delete_edges,test_vertices,treat_faces);
			}
			for (f = faces.begin(); f != faces.end(); f++)
			{
				treat_faces.erase(*f);
				new_faces_.erase(*f);
				rs_->faces_[(*f)->index_] = NULL;
				delete *f;
			}
			for (f = treat_faces.begin(); f != treat_faces.end(); f++)
			{
				rs_->faces_[(*f)->index_] = NULL;
				rs_->faces_.push_back(*f);
				(*f)->index_ = rs_->number_of_faces_;
				rs_->number_of_faces_++;
			}
			HashSet<RSEdge*>::Iterator edge;
			for (edge = delete_edges.begin(); edge != delete_edges.end(); edge++)
			{
				Index index = (*edge)->index_;
				if (index != -1)
				{
					rs_->edges_[index] = NULL;
				}
				delete *edge;
			}
			test_vertices.erase(vertex);
			HashSet<RSVertex*>::Iterator test;
			for (test = test_vertices.begin(); test != test_vertices.end(); test++)
			{
				if ((*test)->hasEdges() == false)
				{
					rs_->vertices_[(*test)->index_] = NULL;
					vertices_[(*test)->atom_].remove(*test);
					new_vertices_.erase(*test);
					delete *test;
				}
			}
			rs_->vertices_[vertex->index_] = NULL;
			vertices_[atom].remove(vertex);
			new_vertices_.erase(vertex);
			delete vertex;
		}
		rs_->atom_[atom].radius -= 10*Constants::EPSILON;
		atom_status_[atom] = STATUS_UNKNOWN;
		correctProbePosition(atom);
	}


	void RSComputer::extendComponent()
		throw(Exception::GeneralException,
					Exception::DivisionByZero,
					Exception::IndexOverflow)
	{
		while (new_vertices_.size() > 0)
		{
			RSFace* face = NULL;
			RSVertex* vertex1 = *new_vertices_.begin();
			new_vertices_.erase(new_vertices_.begin());
  		Index atom1(vertex1->atom_);
			std::list<Index>::const_iterator i;
			i = neighbours_[atom1].begin();
			bool stop = false;
			while (!stop && i != neighbours_[atom1].end())
			{
				if (atom_status_[*i] == STATUS_UNKNOWN)
				{
					Index atom2 = *i;
					neighboursOfTwoAtoms(atom1,atom2);
					std::list< std::pair< Index,TSphere3<double> > > candidates;
					findThirdAtom(atom1,atom2,neighbours_of_two_[atom1][atom2],
												candidates);
					if (candidates.size() == 0)
					{
						RSVertex* vertex2 = new RSVertex(atom2);
						RSEdge* edge = createFreeEdge(vertex1,vertex2);
						if (edge != NULL)
						{
							insert(edge);
							insert(vertex2);
							new_vertices_.insert(vertex1);
							// i = neighbours_[atom1].end()--; ???
							break;
						}
						else
						{
							delete vertex2;
						}
					}
					else
					{
						std::list< std::pair< Index,TSphere3<double> > >::iterator j = candidates.begin();
						while (j != candidates.end())
						{
							if (atom_status_[j->first] == STATUS_UNKNOWN)
							{
								Index atom3 = j->first;
								TSphere3<double> probe = j->second;
								if (checkProbe(probe,atom1,atom2,atom3) == true)
								{
									face = new RSFace;
									RSEdge* edge1 = new RSEdge;
									RSEdge* edge2 = new RSEdge;
									RSEdge* edge3 = new RSEdge;
									RSVertex* vertex2 = new RSVertex(atom2);
									RSVertex* vertex3 = new RSVertex(atom3);
									updateFaceAndEdges(vertex1,vertex2,vertex3,
																		 edge1,edge2,edge3,
																		 face,probe);
									insert(face);
									insert(vertex2);
									insert(vertex3);
									new_vertices_.insert(vertex1);
									// i = neighbours_[atom1].end()--;
									// j = candidates.end()--;
									// ????
									stop = true;
									break;
								}
							}
							j++;
						} // while j
					}
				}
				i++;
			} // while i
			if (face != NULL)
			{
				getRSComponent();
			}
		}
	}


	Index RSComputer::thirdAtom
		 (RSVertex*	vertex1,
			RSVertex*	vertex2,
			RSFace*		face,
			TSphere3<double>&	probe,
			TAngle<double>&		phi)
		throw(Exception::GeneralException,
					Exception::DivisionByZero,
					Exception::IndexOverflow)
	{
		// This function chooses from all atoms which can be touced by the probe
		// sphere when it touches the given two vertices this one, for which is
		// the rotation angle the smalest.
		// If the rotation angle equals zero, the probe sphere can touch four or
		// more atoms an an exception is thrown.
		// If no atom can be found an exception is thrown.
		Index atom1(vertex1->atom_);
		Index atom2(vertex2->atom_);
                neighboursOfTwoAtoms(atom1,atom2);
		std::list<std::pair<Index,TSphere3<double> > > candidates;
		findThirdAtom(atom1,atom2,neighbours_of_two_[atom1][atom2],candidates);
		std::list<std::pair<Index,TSphere3<double> > >::iterator k;
		TAngle<double> old_angle(3*Constants::PI,true);
		TAngle<double> new_angle;
		TAngle<double> two_pi(2*Constants::PI,true);
		TVector3<double> axis = rs_->atom_[atom1].p-rs_->atom_[atom2].p;
		TVector3<double> test_vector = face->normal_%axis;
		Index third_face_atom = face->third(vertex1,vertex2)->atom_;
		if (Maths::isLess(test_vector*rs_->atom_[third_face_atom].p,
											test_vector*rs_->atom_[atom1].p)					)
		{
			axis.negate();
		}
		TSphere3<double> sphere1(rs_->atom_[atom1]);
		TSphere3<double> sphere2(rs_->atom_[atom2]);
		sphere1.radius += rs_->probe_radius_;
		sphere2.radius += rs_->probe_radius_;
		TCircle3<double> circle;
		GetIntersection(sphere1,sphere2,circle);
		TVector3<double> start_probe = face->center_;
		TVector3<double> v1 = start_probe-circle.p;
		TVector3<double> face_normal = face->normal_;
		std::list<std::pair<Index,TSphere3<double> > > third;
		for (k = candidates.begin(); k != candidates.end(); k++)
		{
			if ((k->first != third_face_atom) || (k->second.p != start_probe))
					// not found the starting face
			{
				TVector3<double> v2(k->second.p-circle.p);
				new_angle = getOrientedAngle(v1,v2,axis);
				if (Maths::isZero(new_angle.value) || (new_angle == two_pi))
				{
					correct(k->first);
					throw Exception::GeneralException
							(__FILE__,__LINE__,"CAN'T COMPUTE RS",
																 "PROBE SPHERE TOUCHES FOUR ATOMS");
				}
				if (new_angle <= old_angle)
				{
					if (new_angle < old_angle)
					{
						old_angle = new_angle;
						std::list<std::pair<Index,TSphere3<double> > >::iterator t;
						for (t = third.begin(); t != third.end(); t++)
						{
							if (atom_status_[t->first] == STATUS_UNKNOWN)
							{
								atom_status_[t->first] = STATUS_INSIDE;
							}
						}
						third.clear();
					}
					third.push_back(*k);
				}
				else
				{
					if (atom_status_[k->first] == STATUS_UNKNOWN)
					{
						atom_status_[k->first] = STATUS_INSIDE;
					}
				}
			}
		}
		if (third.size() > 1)
		{
			k = third.begin();
			k++;
			while (k != third.end())
			{
				correct(k->first);
				k++;
			}
			throw Exception::GeneralException
					(__FILE__,__LINE__,"CAN'T COMPUTE RS",
														 "PROBE SPHERE TOUCHES FOUR ATOMS");
		}
		probe = third.front().second;
		phi.set(old_angle.value,true);
		atom_status_[third.front().first] = STATUS_ON_SURFACE;
		return third.front().first;
	}


	Position RSComputer::getStartPosition()
		throw(Exception::DivisionByZero)
	{
		if (findFirstFace() != NULL)
		{
			return 3;
		}
		if (findFirstEdge() != NULL)
		{
			return 2;
		}
		if (findFirstVertex() != NULL)
		{
			return 1;
		}
		return 0;
	}


	RSFace* RSComputer::findFirstFace()
		throw(Exception::DivisionByZero)
	{
		for (Position direction = 0; direction < 3; direction++)
		{
			for (Position extrem = 0; extrem < 1; extrem++)
			{
				RSFace* face = findFace(direction,extrem);
				if (face != NULL)
				{
					return face;
				}
			}
		}
		return NULL;
	}


	RSEdge* RSComputer::findFirstEdge()
		throw()
	{
		for (Position direction = 0; direction < 3; direction++)
		{
			for (Position extrem = 0; extrem < 1; extrem++)
			{
				RSEdge* edge = findEdge(direction,extrem);
				if (edge != NULL)
				{
					return edge;
				}
			}
		}
		return NULL;
	}


	RSVertex* RSComputer::findFirstVertex()
		throw()
	{
		for (Position i = 0; i < rs_->number_of_atoms_; i++)
		{
			if (atom_status_[i] == STATUS_UNKNOWN)
			{
				if (neighbours_[i].size() == 0)
				{
					RSVertex* vertex = new RSVertex(i);
					insert(vertex);
					return vertex;
				}
			}
		}
		return NULL;
	}


	RSFace* RSComputer::findFace(Position direction, Position extrem)
		throw(Exception::DivisionByZero)
	{
		Index a1 = findFirstAtom(direction,extrem);
		if (a1 == -1)
		{
			return NULL;
		}
		Index a2 = findSecondAtom(a1,direction,extrem);
		if (a2 == -1)
		{
			return NULL;
		}
		neighboursOfTwoAtoms(a1,a2);
		std::list<std::pair<Index,TSphere3<double> > > candidates;
		findThirdAtom(a1,a2,neighbours_of_two_[a1][a2],candidates);
		if (candidates.size() == 0)
		{
			return NULL;
		}
		std::list<std::pair<Index,TSphere3<double> > >::iterator i
				= candidates.begin();
		Index a3 = -1;
		TSphere3<double> probe;
		bool found = false;
		while ((found == false) && (i != candidates.end()))
		{
			a3 = i->first;
			probe = i->second;
			found = (atom_status_[a3] == STATUS_UNKNOWN) &&
							checkProbe(probe,a1,a2,a3);
			i++;
		}
		if (found)
		{
			RSVertex* vertex1 = new RSVertex(a1);
			RSVertex* vertex2 = new RSVertex(a2);
			RSVertex* vertex3 = new RSVertex(a3);
			RSEdge* e1 = new RSEdge;
			RSEdge* e2 = new RSEdge;
			RSEdge* e3 = new RSEdge;
			RSFace* face = new RSFace;
			updateFaceAndEdges(vertex1,vertex2,vertex3,e1,e2,e3,face,probe);
			insert(face);
			insert(vertex1);
			insert(vertex2);
			insert(vertex3);
			return face;
		}
		else
		{
			atom_status_[a1] = STATUS_INSIDE;
			atom_status_[a2] = STATUS_INSIDE;
			return NULL;
		}
	}


	RSEdge* RSComputer::findEdge(Position direction, Position extrem)
		throw()
	{
		Index a1 = findFirstAtom(direction,extrem);
		if (a1 == -1)
		{
			return NULL;
		}
		Index a2 = findSecondAtom(a1,direction,extrem);
		if (a2 == -1)
		{
			return NULL;
		}
                RSVertex* vertex1 = new RSVertex(a1);
		RSVertex* vertex2 = new RSVertex(a2);
		neighboursOfTwoAtoms(a1,a2);
		RSEdge* edge = createFreeEdge(vertex1,vertex2);
		if (edge != NULL)
		{
			insert(edge);
			insert(vertex1);
			insert(vertex2);
			return edge;
		}
		else
		{
			delete vertex1;
			delete vertex2;
			neighbours_[a1].remove(a2);
			neighbours_[a2].remove(a1);
			return NULL;
		}
	}


	Index RSComputer::findFirstAtom(Position direction, Position extrem)
		throw()
	{
		Index extrem_atom = -1;
		// find the first atom of unknown status
		Index i = 0;
		bool found = false;
		while ((found == false) && (i < (Index)rs_->number_of_atoms_))
		{
			if (atom_status_[i] == STATUS_UNKNOWN)
			{
				found = true;
			}
			else
			{
				i++;
			}
		}
		if (found)
		{
			extrem_atom = i;
			TSphere3<double>* next_atom = &(rs_->atom_[i]);
			double extrem_value
					= ((extrem == 0) ? next_atom->p[direction]-next_atom->radius
													 : next_atom->p[direction]+next_atom->radius);
			i++;
			// find the atom of unknown status lying on the extrem position
			while (i < (Index)rs_->number_of_atoms_)
			{
				if (atom_status_[i] == STATUS_UNKNOWN)
				{
					next_atom = &(rs_->atom_[i]);
					double extremum
							= ((extrem == 0) ? next_atom->p[direction]-next_atom->radius
															 : next_atom->p[direction]+next_atom->radius);
					if (((extrem == 0) && Maths::isLess(extremum,extrem_value)) ||
							((extrem != 0) && Maths::isGreater(extremum,extrem_value)))
					{
						extrem_value = extremum;
						extrem_atom = i;
					}
				}
				i++;
			}
		}
		return extrem_atom;
	}


	Index RSComputer::findSecondAtom
			(Index atom,
			 Position direction,
			 Position extrem)
		throw()
	{
		Index second_atom = -1;
		// find the first neighbour atom of unknown status
		std::list<Index>::const_iterator i = neighbours_[atom].begin();
		bool found = false;
		while ((found == false) && (i != neighbours_[atom].end()))
		{
			if (atom_status_[*i] == STATUS_UNKNOWN)
			{
				found = true;
			}
			else
			{
				i++;
			}
		}
		if (found)
		{
			second_atom = *i;
			TSphere3<double> first_atom(rs_->atom_[atom]);
			first_atom.radius += rs_->probe_radius_;
			double extrem_value
					= ((extrem == 0) ? first_atom.p[direction]+first_atom.radius
													 : first_atom.p[direction]-first_atom.radius);
			TSphere3<double> next_atom;
			TCircle3<double> intersection_circle;
			// find the neighbour atom of unknown status lying on the extrem position
			while (i != neighbours_[atom].end())
			{
				if (atom_status_[*i] == STATUS_UNKNOWN)
				{
					next_atom = rs_->atom_[*i];
					next_atom.radius += rs_->probe_radius_;
					if (GetIntersection(first_atom,next_atom,intersection_circle))
					{
						double next_extrem
								= getCircleExtremum(intersection_circle,direction,extrem);
						if (((extrem == 0) && Maths::isLess(next_extrem,extrem_value)) ||
								((extrem != 0) && Maths::isGreater(next_extrem,extrem_value)))
						{
							extrem_value = next_extrem;
							second_atom = *i;
						}
					}
				}
				i++;
			}
		}
		return second_atom;
	}


	void RSComputer::findThirdAtom
		 (Index																				 atom1,
			Index																				 atom2,
			const std::list<Index>&											 third,
			std::list<std::pair<Index,TSphere3<double> > >& atoms)
		throw()
	{
		// This function computes a list of all atoms (with its probe positions)
		// which can be touched by the probe sphere when it touches the two given
		// atoms
		std::pair<Index, TSphere3<double> > candidate;
		std::list<Index>::const_iterator i = third.begin();
		TVector3<double> center1, center2;
		TSphere3<double> probe;
		probe.radius = rs_->probe_radius_;
		while (i != third.end())
		{
			if (centerOfProbe(atom1,atom2,*i,center1,center2))
			{
				if ((Maths::isNan(center1.x) ||
						 Maths::isNan(center1.y) ||
						 Maths::isNan(center1.z)   ) == false)
				{
					probe.p.set(center1);
					candidate.first = *i;
					candidate.second = probe;
					atoms.push_back(candidate);
				}
				if ((Maths::isNan(center2.x) ||
						 Maths::isNan(center2.y) ||
						 Maths::isNan(center2.z)   ) == false)
				{
					probe.p.set(center2);
					candidate.first = *i;
					candidate.second = probe;
					atoms.push_back(candidate);
				}
			}
			i++;
		}
	}


	void RSComputer::neighboursOfTwoAtoms(Index atom1, Index atom2)
		throw()
	{
		bool found = false;
		HashMap<Position, HashMap<Position, std::list<Index> > >::Iterator n1 = neighbours_of_two_.find(atom1);
		if (n1 != neighbours_of_two_.end())
		{
			HashMap<Position, std::list<Index> >::Iterator n2 = n1->second.find(atom2);
			found = (n2 != n1->second.end());
		}
		if (found == false)
		{
			std::list<Index> empty;
			neighbours_of_two_[atom1][atom2] = empty;
			neighbours_of_two_[atom2][atom1] = empty;
			std::list<Index>::iterator i1 = neighbours_[atom1].begin();
			std::list<Index>::iterator i2 = neighbours_[atom2].begin();
			while ((i1 != neighbours_[atom1].end()) &&
						 (i2 != neighbours_[atom2].end()))
			{
				if (*i1 == *i2)
				{
					neighbours_of_two_[atom1][atom2].push_back(*i1);
					neighbours_of_two_[atom2][atom1].push_back(*i1);
					i1++;
					i2++;
				}
				else
				{
					if (*i1 < *i2)
					{
						i1++;
					}
					else
					{
						i2++;
					}
				}
			}
		}
	}


	void RSComputer::neighboursOfThreeAtoms
		 (Index								atom1,
			Index								atom2,
			Index								atom3,
			::std::list<Index>& output_list)
		throw()
	{
		neighboursOfTwoAtoms(atom1,atom2);
		neighboursOfTwoAtoms(atom1,atom3);
		HashMap<Position, HashMap<Position,std::list<Index> > >::Iterator n1
			= neighbours_of_two_.find(atom1);
		HashMap<Position, std::list<Index> >::Iterator n2 = n1->second.find(atom2);
		HashMap<Position, std::list<Index> >::Iterator n3 = n1->second.find(atom3);

		std::list<Index>::iterator i2 = n2->second.begin();
		// fixed by Andreas Moll, 19.7.06: i3 used to work on n2->second.begin()
		std::list<Index>::iterator i3 = n3->second.begin();
		while ((i2 != n2->second.end()) && (i3 != n3->second.end()))
		{
			if (*i2 == *i3)
			{
				output_list.push_back(*i2);
				i2++;
				i3++;
			}
			else
			{
				if (*i2 < *i3)
				{
					i2++;
				}
				else
				{
					i3++;
				}
			}
		}
	}


	double RSComputer::getCircleExtremum
		 (const TCircle3<double>& circle,
			Position					 direction,
			Position					 extrem)
		throw()
	{
		double min = 0;
		double max = 0;
		TVector3<double> norm2(circle.n.x * circle.n.x,
											circle.n.y * circle.n.y,
											circle.n.z * circle.n.z);
		switch (direction)
		{
			case 0 :
				if (Maths::isZero(circle.n.y) && Maths::isZero(circle.n.z))
				{
					min = max = circle.p.x;
				}
				else
				{
					double x_norm = norm2.y + norm2.z;
					x_norm /= norm2.x+x_norm;
					x_norm = circle.radius * sqrt(x_norm);
					min = (circle.p.x) - x_norm;
					max = (circle.p.x) + x_norm;
				}
				break;
			case 1 :
				if (Maths::isZero(circle.n.x) && Maths::isZero(circle.n.z))
				{
					min = max = circle.p.y;
				}
				else
				{
					double y_norm = norm2.x + norm2.z;
					y_norm /= norm2.y + y_norm;
					y_norm = circle.radius * sqrt(y_norm);
					min = (circle.p.y)-y_norm;
					max = (circle.p.y)+y_norm;
				}
				break;
			case 2 :
				if (Maths::isZero(circle.n.x) && Maths::isZero(circle.n.y))
				{
					min = max = circle.p.z;
				}
				else
				{
					double z_norm = norm2.x + norm2.y;
					z_norm /= norm2.z + z_norm;
					z_norm = circle.radius * sqrt(z_norm);
					min = circle.p.z - z_norm;
					max = circle.p.z + z_norm;
				}
				break;
		}
		if (extrem == 0)
		{
			return min;
		}
		else
		{
			return max;
		}
	}


	RSEdge* RSComputer::createFreeEdge
			(RSVertex* vertex1,
			 RSVertex* vertex2)
		throw()
	{
		Index atom1 = vertex1->atom_;
		Index atom2 = vertex2->atom_;
		TCircle3<double> circle1;
		TCircle3<double> circle2;
		TCircle3<double> circle3;
		if (getCircles(atom1, atom2, circle1, circle2, circle3) &&
				Maths::isGreater(circle1.radius,rs_->probe_radius_))
		{
			TPlane3<double> plane(circle1.p,circle1.n);
			::std::list<Index>::const_iterator i;
			TCircle3<double> test_circle;
			TSphere3<double> sphere;
			double dist;
			for (i = neighbours_of_two_[atom1][atom2].begin();
					 i != neighbours_of_two_[atom1][atom2].end();
					 i++)
			{
				sphere.set(rs_->atom_[*i].p,rs_->atom_[*i].radius+rs_->probe_radius_);
				if (GetIntersection(sphere,plane,test_circle))
				{
					dist = test_circle.radius-circle1.radius;
					if (Maths::isLessOrEqual(dist*dist,
																	 test_circle.p.getSquareDistance(circle1.p)))
					{
						return NULL;
					}
				}
			}
			TVector3<double> vector(0,0,0);
			RSEdge* edge = new RSEdge(vertex1,vertex2,NULL,NULL,
											 									circle1.p,circle1.radius,
																				TAngle<double>(2*Constants::PI,true),
																				circle2,circle3,
																				vector,vector,false,-1);
			return edge;
		}
		return NULL;
	}


	bool RSComputer::getCircles
		 (Index atom1,
			Index atom2,
			TCircle3<double>& circle1,
			TCircle3<double>& circle2,
			TCircle3<double>& circle3)
		throw()
	{
		TSphere3<double> sphere1(rs_->atom_[atom1]);
		TSphere3<double> sphere2(rs_->atom_[atom2]);
		sphere1.radius += rs_->probe_radius_;
		sphere2.radius += rs_->probe_radius_;
		if (GetIntersection(sphere1,sphere2,circle1))
		{
			double ratio = rs_->atom_[atom1].radius/sphere1.radius;
			circle2.radius = circle1.radius*ratio;
			circle2.p = sphere1.p+(circle1.p-sphere1.p)*ratio;
			ratio = rs_->atom_[atom2].radius/sphere2.radius;
			circle3.radius = circle1.radius*ratio;
			circle3.p = sphere2.p+(circle1.p-sphere2.p)*ratio;
			return true;
		}
		return false;
	}


	TVector3<double> RSComputer::getFaceNormal
		 (const TSphere3<double>& atom1,
			const TSphere3<double>& atom2,
			const TSphere3<double>& atom3,
			const TSphere3<double>& probe)
		throw()
	{
		TPlane3<double> plane(atom1.p,atom2.p,atom3.p);
		TVector3<double> norm(plane.n);
		if (Maths::isLess(norm*probe.p,norm*atom1.p))
		{
			norm.negate();
		}
		return norm;
	}


	void RSComputer::updateFaceAndEdges
		 (RSVertex* v1,
			RSVertex* v2,
			RSVertex* v3,
			RSEdge* e1,
			RSEdge* e2,
			RSEdge* e3,
			RSFace* f,
			const TSphere3<double>& probe)
		throw()
	{
		e1->vertex_[0] = v1;
		e1->vertex_[1] = v2;
		e1->face_[0] = f;
		e2->vertex_[0] = v2;
		e2->vertex_[1] = v3;
		e2->face_[0] = f;
		e3->vertex_[0] = v3;
		e3->vertex_[1] = v1;
		e3->face_[0] = f;
		f->vertex_[0] = v1;
		f->vertex_[1] = v2;
		f->vertex_[2] = v3;
		f->edge_[0] = e1;
		f->edge_[1] = e2;
		f->edge_[2] = e3;
		f->center_ = probe.p;
		TPlane3<double> plane(rs_->atom_[v1->atom_].p,
										 rs_->atom_[v2->atom_].p,
										 rs_->atom_[v3->atom_].p);
		f->normal_ = plane.n;
		if (Maths::isLess(f->normal_*probe.p,f->normal_*rs_->atom_[v1->atom_].p))
		{
			f->normal_.negate();
		}
		f->singular_ = Maths::isLess(GetDistance(probe.p,plane),probe.radius);
	}


	RSFace* RSComputer::faceExists
		 (RSFace* face,
			const std::list<RSVertex*>& vertices)
		throw()
	{
		std::list<RSVertex*>::const_iterator v;
		RSFace* f;
		for (v = vertices.begin(); v != vertices.end(); v++)
		{
			f = (*v)->has(face);
			if (f != NULL)
			{
				return f;
			}
		}
		return NULL;
	}


	bool RSComputer::centerOfProbe
		 (Index				 a1,
			Index				 a2,
			Index				 a3,
			TVector3<double>& c1,
			TVector3<double>& c2)
		throw()
	{
		sort(a1,a2,a3,a1,a2,a3);
		HashMap<Position,
						HashMap<Position,	HashMap<Position,ProbePosition* > > >::Iterator pp1;
		HashMap<Position, HashMap<Position,ProbePosition* > >::Iterator pp2;
		HashMap<Position, ProbePosition* >::Iterator pp3;
		bool back = false;
		bool found = false;
		pp1 = probe_positions_.find(a1);
		if (pp1 != probe_positions_.end())
		{
			pp2 = pp1->second.find(a2);
			if (pp2 != pp1->second.end())
			{
				pp3 = pp2->second.find(a3);
				if (pp3 != pp2->second.end())
				{
					found = true;
					if (pp3->second == NULL)
					{
						back = false;
					}
					else
					{
						c1 = pp3->second->point[0];
						c2 = pp3->second->point[1];
						back = true;
					}
				}
			}
		}
		if (found == false)
		{
			TSphere3<double> s1(rs_->atom_[a1]);
			TSphere3<double> s2(rs_->atom_[a2]);
			TSphere3<double> s3(rs_->atom_[a3]);
			s1.radius += rs_->probe_radius_;
			s2.radius += rs_->probe_radius_;
			s3.radius += rs_->probe_radius_;
			if (GetIntersection(s1,s2,s3,c1,c2,false))
			{
				ProbePosition* position = new ProbePosition;
				position->status[0] = STATUS_NOT_TESTED;
				position->status[1] = STATUS_NOT_TESTED;
				position->point[0] = c1;
				position->point[1] = c2;
				probe_positions_[a1][a2][a3] = position;
				back = true;
			}
			else
			{
				probe_positions_[a1][a2][a3] = NULL;
				back = false;
			}
		}
		return back;
	}


	bool RSComputer::checkProbe
			(const TSphere3<double>& probe,
			 Index atom1,
			 Index atom2,
			 Index atom3)
		throw()
	{
		sort(atom1,atom2,atom3,atom1,atom2,atom3);
		Position index;
		ProbePosition* position = probe_positions_[atom1][atom2][atom3];
		if (probe.p == position->point[0])
		{
			index = 0;
		}
		else
		{
			index = 1;
		}
		if (position->status[index] == STATUS_NOT_TESTED)
		{
			bool ok = true;
			std::list<Index> atom_list;
			neighboursOfThreeAtoms(atom1,atom2,atom3,atom_list);
			double dist;
			std::list<Index>::iterator i = atom_list.begin();
			while (ok && (i != atom_list.end()))
			{
				dist = probe.radius+rs_->atom_[*i].radius;
				if (Maths::isLess(probe.p.getSquareDistance(rs_->atom_[*i].p),
													dist*dist))
				{
					position->status[index] = STATUS_NOT_OK;
					ok = false;
				}
				i++;
			}
			if (ok)
			{
				position->status[index] = STATUS_OK;
			}
		}
		return (position->status[index] == STATUS_OK);
	}


	void RSComputer::correctProbePosition(Position atom)
		throw()
	{
		HashMap<Position,
						HashMap<Position,
						 					HashMap<Position,ProbePosition* > > >::Iterator pp1;
		HashMap<Position,HashMap<Position,ProbePosition* > >::Iterator pp2;
		HashMap<Position,ProbePosition* >::Iterator pp3;
		for (pp1 = probe_positions_.begin(); pp1 != probe_positions_.end(); pp1++)
		{
			if (pp1->first < atom)
			{
				for (pp2 = pp1->second.begin(); pp2 != pp1->second.end(); pp2++)
				{
					if (pp2->first < atom)
					{
						if (pp2->second.has(atom))
						{
							correctProbePosition(pp1->first,pp2->first,atom);
						}
					}
					else
					{
						if (pp2->first == atom)
						{
							for (pp3 = pp2->second.begin(); pp3 != pp2->second.end(); pp3++)
							{
								correctProbePosition(pp1->first,atom,pp3->first);
							}
						}
					}
				}
			}
			else
			{
				if (pp1->first == atom)
				{
					for (pp2 = pp1->second.begin(); pp2 != pp1->second.end(); pp2++)
					{
						for (pp3 = pp2->second.begin(); pp3 != pp2->second.end(); pp3++)
						{
							correctProbePosition(atom,pp2->first,pp3->first);
						}
					}
				}
			}
		}
	}


	void RSComputer::correctProbePosition
			(Position a1, Position a2, Position a3)
		throw()
	{
		Index ai1 = (Index)a1;
		Index ai2 = (Index)a2;
		Index ai3 = (Index)a3;

		sort(ai1, ai2, ai3, ai1, ai2, ai3);
		TSphere3<double> s1(rs_->atom_[ai1]);
		TSphere3<double> s2(rs_->atom_[ai2]);
		TSphere3<double> s3(rs_->atom_[ai3]);
		s1.radius += rs_->probe_radius_;
		s2.radius += rs_->probe_radius_;
		s3.radius += rs_->probe_radius_;
		TVector3<double> c1;
		TVector3<double> c2;
		if (GetIntersection(s1,s2,s3,c1,c2))
		{
			ProbePosition* position = probe_positions_[ai1][ai2][ai3];
			if (position == NULL)
			{
				probe_positions_[ai1][ai2][ai3] = new ProbePosition;
				position = probe_positions_[ai1][ai2][ai3];
			}
			position->status[0] = STATUS_NOT_TESTED;
			position->status[1] = STATUS_NOT_TESTED;
			position->point[0] = c1;
			position->point[1] = c2;
		}
		else
		{
			delete probe_positions_[ai1][ai2][ai3];
			probe_positions_[ai1][ai2][ai3] = NULL;
		}
	}


	void RSComputer::preProcessing()
		throw()
	{
		rs_->r_max_ = rs_->atom_[0].radius;
		double x_min = rs_->atom_[0].p.x;
		double y_min = rs_->atom_[0].p.y;
		double z_min = rs_->atom_[0].p.z;
		double x_max = x_min;
		double y_max = y_min;
		double z_max = z_min;
		for (Position i = 1; i < rs_->number_of_atoms_; i++)
		{
			rs_->r_max_ = ((rs_->r_max_ < rs_->atom_[i].radius)
																	? rs_->atom_[i].radius
																	: rs_->r_max_);
			x_min = ((x_min > rs_->atom_[i].p.x)
											? rs_->atom_[i].p.x
											: x_min);
			y_min = ((y_min > rs_->atom_[i].p.y)
											? rs_->atom_[i].p.y
											: y_min);
			z_min = ((z_min > rs_->atom_[i].p.z)
											? rs_->atom_[i].p.z
											: z_min);
			x_max = ((x_max < rs_->atom_[i].p.x)
											? rs_->atom_[i].p.x
											: x_max);
			y_max = ((y_max < rs_->atom_[i].p.y)
											? rs_->atom_[i].p.y
											: y_max);
			z_max = ((z_max < rs_->atom_[i].p.z)
											? rs_->atom_[i].p.z
											: z_max);
		}
		rs_->bounding_box_.set(x_min,y_min,z_min,x_max,y_max,z_max);
		double dist = 2*(rs_->r_max_+rs_->probe_radius_);
		Position nx = (Position)((x_max-x_min)/dist+5);
		Position ny = (Position)((y_max-y_min)/dist+5);
		Position nz = (Position)((z_max-z_min)/dist+5);
		Vector3 origin(x_min-2*dist,y_min-2*dist,z_min-2*dist);
		HashGrid3<Position> grid(origin,nx,ny,nz,dist);
		Vector3 pos;
		for (Position i = 0; i < rs_->number_of_atoms_; i++)
		{
			pos.set(rs_->atom_[i].p.x,rs_->atom_[i].p.y,rs_->atom_[i].p.z);
			grid.insert(pos,i);
		}
		TSphere3<double> next_atom;
		double offset;
		HashGridBox3<Position>* box;
		HashGridBox3<Position>::ConstBoxIterator b;
		HashGridBox3<Position>::ConstDataIterator d;
		for (Position i = 0; i < rs_->number_of_atoms_-1; i++)
		{
			offset = rs_->atom_[i].radius+2*rs_->probe_radius_;
			pos.set(rs_->atom_[i].p.x,rs_->atom_[i].p.y,rs_->atom_[i].p.z);
			box = grid.getBox(pos);
			for (b = box->beginBox(); b != box->endBox(); b++)
			{
				for (d = b->beginData(); d != b->endData(); d++)
				{
					if (*d > i)
					{
						next_atom = rs_->atom_[*d];
						double dist = next_atom.p.getSquareDistance(rs_->atom_[i].p);
						double max_dist = next_atom.radius+offset;
						max_dist *= max_dist;
						if (Maths::isLess(dist,max_dist))
						{
							neighbours_[i].push_back(*d);
							neighbours_[*d].push_back(i);
						}
					}
				}
			}
			neighbours_[i].sort();
		}
	}


	void RSComputer::insert(RSVertex* vertex)
		throw()
	{
		rs_->insert(vertex);
		new_vertices_.insert(vertex);
		vertices_[vertex->atom_].push_back(vertex);
		atom_status_[vertex->atom_] = STATUS_ON_SURFACE;
	}


	void RSComputer::insert(RSEdge* edge)
		throw()
	{
		rs_->insert(edge);
		edge->vertex_[0]->edges_.insert(edge);
		edge->vertex_[1]->edges_.insert(edge);
	}


	void RSComputer::insert(RSFace* face)
		throw()
	{
		rs_->insert(face);
		new_faces_.insert(face);
		face->vertex_[0]->faces_.insert(face);
		face->vertex_[1]->faces_.insert(face);
		face->vertex_[2]->faces_.insert(face);
		RSEdge* edge = face->edge_[0];
		edge->vertex_[0]->edges_.insert(edge);
		edge->vertex_[1]->edges_.insert(edge);
		edge = face->edge_[1];
		edge->vertex_[0]->edges_.insert(edge);
		edge->vertex_[1]->edges_.insert(edge);
		edge = face->edge_[2];
		edge->vertex_[0]->edges_.insert(edge);
		edge->vertex_[1]->edges_.insert(edge);
	}


	void RSComputer::sort
		 (Index		u1, Index		u2, Index		u3,
			Index&	s1, Index&	s2, Index&	s3)
		throw()
	{
		s1 = u1;
		s2 = u2;
		s3 = u3;
		Index tmp;
		if (s1 > s2)
		{
			tmp = s1;
			s1 = s2;
			s2 = tmp;
		}
		if (s1 > s3)
		{
			tmp = s1;
			s1 = s3;
			s3 = tmp;
		}
		if (s2 > s3)
		{
			tmp = s2;
			s2 = s3;
			s3 = tmp;
		}
	}


} // namespace BALL
