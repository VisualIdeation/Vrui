/***********************************************************************
PolygonMesh - Base class for polygon meshes represented by a split-edge
data structure.
Copyright (c) 2003-2005 Oliver Kreylos

This file is part of the Templatized Geometry Library (TGL).

The Templatized Geometry Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Templatized Geometry Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Templatized Geometry Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#define GEOMETRY_POLYGONMESH_IMPLEMENTATION

#include <Misc/HashTable.h>
#include <Geometry/AffineCombiner.h>

#include "PolygonMesh.h"

namespace Geometry {

namespace {

/***************
Utility classes:
***************/

class VertexPair // Helper class to create face connectivity during polyhedron construction
	{
	/* Elements: */
	private:
	int index[2]; // Vertex indices of pair; first index is always smaller

	/* Constructors and destructors: */
	public:
	VertexPair(int sIndex1,int sIndex2)
		{
		if(sIndex1<sIndex2)
			{
			index[0]=sIndex1;
			index[1]=sIndex2;
			}
		else
			{
			index[0]=sIndex2;
			index[1]=sIndex1;
			}
		}

	/* Methods: */
	friend bool operator==(const VertexPair& vp1,const VertexPair& vp2)
		{
		return vp1.index[0]==vp2.index[0]&&vp1.index[1]==vp2.index[1];
		}
	friend bool operator!=(const VertexPair& vp1,const VertexPair& vp2)
		{
		return vp1.index[0]!=vp2.index[0]&&vp1.index[1]!=vp2.index[1];
		}
	static size_t rawHash(const VertexPair& vp)
		{
		return vp.index[0]*31+vp.index[1]*17;
		}
	static size_t hash(const VertexPair& vp,size_t tableSize)
		{
		return (vp.index[0]*31+vp.index[1]*17)%tableSize;
		}
	};

}
	
/**************************************
Methods of class PolygonMeshBaseVertex:
**************************************/

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
inline
bool
PolygonMeshBaseVertex<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::isOnBoundary(
	void) const
	{
	/* Walk clockwise around vertex until start edge or null is hit: */
	const Edge* ePtr=edge;
	do
		{
		ePtr=ePtr->getVertexPred();
		}
	while(ePtr!=0&&ePtr!=edge);
	return ePtr!=0;
	}

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
inline
int
PolygonMeshBaseVertex<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::calcNumEdges(
	void) const
	{
	int result=0;
	
	/* Walk clockwise around vertex until start edge or null is hit: */
	const Edge* ePtr=edge;
	do
		{
		++result;
		ePtr=ePtr->getVertexPred();
		}
	while(ePtr!=0&&ePtr!=edge);
	
	if(ePtr==0) // Vertex is boundary vertex
		{
		/* Walk counter-clockwise around vertex until null is hit: */
		ePtr=edge;
		while(edge!=0)
			{
			++result;
			ePtr=ePtr->getVertexSucc();
			}
		}
	
	return result;
	}

/************************************
Methods of class PolygonMeshBaseEdge:
************************************/

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
inline
const typename PolygonMeshBaseEdge<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::Edge*
PolygonMeshBaseEdge<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::getFacePred(
	void) const
	{
	/* Walk around the face counter-clockwise until start edge is encountered again: */
	const Edge* ePtr=faceSucc;
	while(ePtr->faceSucc!=this)
		ePtr=ePtr->faceSucc;
	return ePtr;
	}

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
inline
typename PolygonMeshBaseEdge<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::Edge*
PolygonMeshBaseEdge<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::getFacePred(
	void)
	{
	/* Walk around the face counter-clockwise until start edge is encountered again: */
	Edge* ePtr=faceSucc;
	while(ePtr->faceSucc!=this)
		ePtr=ePtr->faceSucc;
	return ePtr;
	}

/************************************
Methods of class PolygonMeshBaseFace:
************************************/

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
inline
int
PolygonMeshBaseFace<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::calcNumVertices(
	void) const
	{
	int result=0;
	
	/* Walk counter-clockwise around face: */
	const Edge* ePtr=edge;
	do
		{
		++result;
		ePtr=ePtr->getFaceSucc();
		}
	while(ePtr!=edge);
	return result;
	}

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
inline
typename PolygonMeshBaseFace<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::Point
PolygonMeshBaseFace<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::calcCentroid(
	void) const
	{
	/* Walk counter-clockwise around face: */
	typename Point::AffineCombiner centroidCombiner;
	const Edge* ePtr=edge;
	do
		{
		centroidCombiner.addPoint(ePtr->getStart()->getPosition());
		ePtr=ePtr->getFaceSucc();
		}
	while(ePtr!=edge);
	return centroidCombiner.getPoint();
	}

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
inline
typename PolygonMeshBaseFace<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::Vector
PolygonMeshBaseFace<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::calcNormal(
	void) const
	{
	Vector normal=Vector::zero;
	
	/* Walk counter-clockwise around face: */
	const Edge* ePtr=edge;
	Vector d1=ePtr->getEnd()->getPosition()-ePtr->getStart()->getPosition();
	ePtr=ePtr->getFaceSucc();
	do
		{
		Vector d2=ePtr->getEnd()->getPosition()-ePtr->getStart()->getPosition();
		normal+=Geometry::cross(d1,d2);
		d1=d2;
		ePtr=ePtr->getFaceSucc();
		}
	while(ePtr!=edge->getFaceSucc());
	return normal;
	}

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
inline
typename PolygonMeshBaseFace<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::PlaneEquation
PolygonMeshBaseFace<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::calcPlaneEquation(
	void) const
	{
	/* Walk counter-clockwise around face: */
	Vector normal=Vector::zero;
	typename Point::AffineCombiner centroidCombiner;
	const Edge* ePtr=edge;
	Vector d1=ePtr->getEnd()->getPosition()-ePtr->getStart()->getPosition();
	ePtr=ePtr->getFaceSucc();
	do
		{
		Vector d2=ePtr->getEnd()->getPosition()-ePtr->getStart()->getPosition();
		normal+=Geometry::cross(d1,d2);
		d1=d2;
		centroidCombiner.addPoint(ePtr->getStart()->getPosition());
		ePtr=ePtr->getFaceSucc();
		}
	while(ePtr!=edge->getFaceSucc());
	return PlaneEquation(normal,normal*centroidCombiner.getPoint());
	}

/****************************
Methods of class PolygonMesh:
****************************/

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
inline
PolygonMesh<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::~PolygonMesh(
	void)
	{
	delete[] vertices;
	delete[] edges;
	delete[] faces;
	}

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
inline
void
PolygonMesh<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::set(
	const typename PolygonMesh<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::Point sVertices[],
	const int faceVertexIndices[])
	{
	/* Some local typedefs: */
	typedef Misc::HashTable<VertexPair,Edge*> EdgeHasher;
	
	/* Destroy existing mesh: */
	numVertices=0;
	delete[] vertices;
	vertices=0;
	numEdges=0;
	delete[] edges;
	edges=0;
	numFaces=0;
	delete[] faces;
	faces=0;
	
	/* Determine number of vertices, faces and edges: */
	for(const int* viPtr=faceVertexIndices;viPtr[0]>=0;++viPtr)
		{
		/* Process a single face: */
		int numFaceVertices=0;
		while(viPtr[0]>=0)
			{
			if(numVertices<viPtr[0]+1)
				numVertices=viPtr[0]+1;
			++numFaceVertices;
			++viPtr;
			}
		++numFaces;
		numEdges+=numFaceVertices;
		}
	
	/* Create vertices: */
	vertices=new Vertex[numVertices];
	for(int i=0;i<numVertices;++i)
		vertices[i].position=sVertices[i];
	
	/* Create edges and faces: */
	edges=new Edge[numEdges];
	faces=new Face[numFaces];
	Edge* ePtr=edges;
	Face* fPtr=faces;
	EdgeHasher edgeHash((numEdges*2)/3);
	for(const int* viPtr=faceVertexIndices;viPtr[0]>=0;++viPtr)
		{
		/* Process a single face: */
		Edge* firstEdge=ePtr;
		Edge* lastEdge=0;
		int startVertexIndex=viPtr[0];
		int firstVertexIndex=startVertexIndex;
		while(viPtr[0]>=0)
			{
			ePtr->start=&vertices[startVertexIndex];
			vertices[startVertexIndex].edge=ePtr;
			ePtr->face=fPtr;
			if(lastEdge!=0)
				lastEdge->faceSucc=ePtr;
			lastEdge=ePtr;
			int endVertexIndex=viPtr[1]>=0?viPtr[1]:firstVertexIndex;
			VertexPair vp(startVertexIndex,endVertexIndex);
			typename EdgeHasher::Iterator opIt=edgeHash.findEntry(vp);
			if(opIt.isFinished())
				{
				/* Add edge to hash table: */
				edgeHash.setEntry(typename EdgeHasher::Entry(vp,ePtr));
				ePtr->opposite=0;
				}
			else
				{
				/* Link edge to opposite edge: */
				ePtr->opposite=opIt->getDest();
				opIt->getDest()->opposite=ePtr;
				}
			startVertexIndex=endVertexIndex;
			++ePtr;
			++viPtr;
			}
		lastEdge->faceSucc=firstEdge;
		fPtr->edge=firstEdge;
		++fPtr;
		}
	}

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
inline
typename PolygonMesh<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::Box
PolygonMesh<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>::calcBoundingBox(
	void) const
	{
	Box result=Box::empty;
	for(int i=0;i<numVertices;++i)
		result.addPoint(vertices[i].getPosition());
	return result;
	}

}
