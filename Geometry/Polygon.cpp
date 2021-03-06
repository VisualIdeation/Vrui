/***********************************************************************
Polygon - Class for planar polygons in affine space.
Copyright (c) 2004-2007 Oliver Kreylos

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

#define GEOMETRY_POLYGON_IMPLEMENTATION

#ifndef METHODPREFIX
	#ifdef NONSTANDARD_TEMPLATES
		#define METHODPREFIX inline
	#else
		#define METHODPREFIX
	#endif
#endif

#include <Misc/Utility.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/AffineCombiner.h>

#include <Geometry/Polygon.h>

namespace Geometry {

/************************************
Static elements of class PolygonBase:
************************************/

template <class ScalarParam,int dimensionParam>
const int PolygonBase<ScalarParam,dimensionParam>::dimension;

/****************************
Methods of class PolygonBase:
****************************/

template <class ScalarParam,int dimensionParam>
METHODPREFIX
PolygonBase<ScalarParam,dimensionParam>::PolygonBase(
	int sNumVertices)
	:numVertices(sNumVertices),vertices(new Point[numVertices])
	{
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
PolygonBase<ScalarParam,dimensionParam>::PolygonBase(
	int sNumVertices,
	const typename PolygonBase<ScalarParam,dimensionParam>::Point sVertices[])
	:numVertices(sNumVertices),vertices(new Point[numVertices])
	{
	/* Copy the source vertices: */
	for(int i=0;i<numVertices;++i)
		vertices[i]=sVertices[i];
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
PolygonBase<ScalarParam,dimensionParam>::PolygonBase(
	const PolygonBase<ScalarParam,dimensionParam>& source)
	:numVertices(source.numVertices),vertices(new Point[numVertices])
	{
	/* Copy the source vertices: */
	for(int i=0;i<numVertices;++i)
		vertices[i]=source.vertices[i];
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
PolygonBase<ScalarParam,dimensionParam>&
PolygonBase<ScalarParam,dimensionParam>::operator=(
	const PolygonBase<ScalarParam,dimensionParam>& source)
	{
	/* Check for aliasing: */
	if(this!=&source)
		{
		/* Re-allocate the vertex array: */
		if(numVertices!=source.numVertices)
			{
			delete[] vertices;
			numVertices=source.numVertices;
			vertices=new Point[numVertices];
			}
		
		/* Copy the source vertices: */
		for(int i=0;i<numVertices;++i)
			vertices[i]=source.vertices[i];
		}
	
	return *this;
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
PolygonBase<ScalarParam,dimensionParam>::~PolygonBase(
	void)
	{
	delete[] vertices;
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
typename PolygonBase<ScalarParam,dimensionParam>::Point
PolygonBase<ScalarParam,dimensionParam>::calcCentroid(
	void) const
	{
	/* Calculate the centroid: */
	typename Point::AffineCombiner ac;
	for(int i=0;i<numVertices;++i)
		ac.addPoint(vertices[i]);
	return ac.getPoint();
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
void
PolygonBase<ScalarParam,dimensionParam>::doClip(
	const typename PolygonBase<ScalarParam,dimensionParam>::Plane& plane)
	{
	/* Create temporary array of vertices: */
	int numClipVertices=0;
	Point* clipVertices=new Point[numVertices+1];
	
	/* Process each edge of the polygon: */
	Point* p1=&vertices[numVertices-1];
	Scalar d1=plane.calcDistance(*p1);
	for(int i=0;i<numVertices;++i)
		{
		Point* p2=&vertices[i];
		Scalar d2=plane.calcDistance(*p2);
		
		/* Check if the edge intersects the plane: */
		if(d1*d2<Scalar(0))
			{
			/* Add the intersection point to the clipped polygon: */
			clipVertices[numClipVertices]=affineCombination(*p1,*p2,-d1/(d2-d1));
			++numClipVertices;
			}
		
		/* Check if the edge's end point is behind the plane: */
		if(d2<=Scalar(0))
			{
			/* Add the end point to the clipped polygon: */
			clipVertices[numClipVertices]=*p2;
			++numClipVertices;
			}
		
		/* Go to the next vertex: */
		p1=p2;
		d1=d2;
		}
	
	/* Create the final polygon: */
	if(numClipVertices!=numVertices)
		{
		delete[] vertices;
		numVertices=numClipVertices;
		vertices=new Point[numVertices];
		}
	for(int i=0;i<numVertices;++i)
		vertices[i]=clipVertices[i];
	delete[] clipVertices;
	}

/************************
Methods of class Polygon:
************************/

template <class ScalarParam,int dimensionParam>
METHODPREFIX
typename Polygon<ScalarParam,dimensionParam>::Vector
Polygon<ScalarParam,dimensionParam>::calcNormal(
	void) const
	{
	/* Calculate the normal vector by adding cross products of each pair of adjacent edges: */
	Vector normal=Vector::zero;
	const Point* p2=&vertices[numVertices-1];
	Vector d1=*p2-vertices[numVertices-2];
	for(int i=0;i<numVertices;++i)
		{
		/* Calculate the cross product of the vertex' two incident edges: */
		const Point* p3=&vertices[i];
		Vector d2=*p3-*p2;
		normal+=cross(d1,d2);
		
		/* Go to the next vertex: */
		p2=p3;
		d1=d2;
		}
	
	/* Return the normal vector: */
	return normal;
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
typename Polygon<ScalarParam,dimensionParam>::Plane
Polygon<ScalarParam,dimensionParam>::calcPlane(
	void) const
	{
	/* Calculate the centroid, and the normal vector by adding cross products of each pair of adjacent edges: */
	typename Point::AffineCombiner ac;
	Vector normal=Vector::zero;
	const Point* p2=&vertices[numVertices-1];
	Vector d1=*p2-vertices[numVertices-2];
	for(int i=0;i<numVertices;++i)
		{
		/* Calculate the cross product of the vertex' two incident edges: */
		const Point* p3=&vertices[i];
		ac.addPoint(*p3);
		Vector d2=*p3-*p2;
		normal+=cross(d1,d2);
		
		/* Go to the next vertex: */
		p2=p3;
		d1=d2;
		}
	
	/* Return the resulting plane: */
	return Plane(normal,ac.getPoint());
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
int*
Polygon<ScalarParam,dimensionParam>::calcProjectionAxes(
	const typename Polygon<ScalarParam,dimensionParam>::Vector& polygonNormal,
	int projectionAxes[2]) const
	{
	/* Find the two axes corresponding to the smallest normal vector components: */
	Scalar axis[2];
	for(int i=0;i<2;++i)
		{
		projectionAxes[i]=i;
		axis[i]=Math::abs(polygonNormal[i]);
		}
	if(axis[0]>axis[1])
		{
		Misc::swap(projectionAxes[0],projectionAxes[1]);
		Misc::swap(axis[0],axis[1]);
		}
	for(int i=2;i<dimension;++i)
		{
		Scalar a=Math::abs(polygonNormal[i]);
		if(a<axis[0])
			{
			projectionAxes[1]=projectionAxes[0];
			projectionAxes[0]=i;
			axis[1]=axis[0];
			axis[0]=a;
			}
		else if(a<axis[1])
			{
			projectionAxes[1]=i;
			axis[1]=a;
			}
		}
	
	return projectionAxes;
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
bool
Polygon<ScalarParam,dimensionParam>::isInside(
	const typename Polygon<ScalarParam,dimensionParam>::Point& p,
	const int projectionAxes[2]) const
	{
	/* Test a ray going along the first projection axis against all polygon edges: */
	int numIntersections=0;
	Scalar pa0=p[projectionAxes[0]];
	Scalar pa1=p[projectionAxes[1]];
	const Point* p1=&vertices[numVertices-1];
	Scalar p1a1=(*p1)[projectionAxes[1]];
	for(int i=0;i<numVertices;++i)
		{
		/* Get the endpoint of the current edge: */
		const Point* p2=&vertices[i];
		Scalar p2a1=(*p2)[projectionAxes[1]];
		
		/* Check for region change: */
		if(p1a1<pa1&&p2a1>=pa1)
			{
			/* Check if the edge intersection is to the right of p: */
			Scalar p1a0=(*p1)[projectionAxes[0]];
			Scalar p2a0=(*p2)[projectionAxes[0]];
			if((pa1-p1a1)*(p2a0-p1a0)>(pa0-p1a0)*(p2a1-p1a1))
				++numIntersections;
			}
		else if(p2a1<pa1&&p1a1>=pa1)
			{
			/* Check if the edge intersection is to the right of p: */
			Scalar p1a0=(*p1)[projectionAxes[0]];
			Scalar p2a0=(*p2)[projectionAxes[0]];
			if((pa1-p1a1)*(p2a0-p1a0)<(pa0-p1a0)*(p2a1-p1a1))
				++numIntersections;
			}
		
		/* Go to the next edge: */
		p1=p2;
		p1a1=p2a1;
		}
	
	/* The point is inside if the number of intersections is odd: */
	return (numIntersections&0x1)==0x1;
	}

/***************************
Methods of class Polygon<2>:
***************************/

template <class ScalarParam>
METHODPREFIX
bool
Polygon<ScalarParam,2>::isInside(
	const typename Polygon<ScalarParam,2>::Point& p) const
	{
	/* Test a ray going along the first projection axis against all polygon edges: */
	int numIntersections=0;
	const Point* p1=&Base::vertices[Base::numVertices-1];
	for(int i=0;i<Base::numVertices;++i)
		{
		/* Get the endpoint of the current edge: */
		const Point* p2=&Base::vertices[i];
		
		/* Check for region change: */
		if((*p1)[1]<p[1]&&(*p2)[1]>=p[1])
			{
			/* Check if the edge intersection is to the right of p: */
			if((p[1]-(*p1)[1])*((*p2)[0]-(*p1)[0])>(p[0]-(*p1)[0])*((*p2)[1]-(*p1)[1]))
				++numIntersections;
			}
		else if((*p2)[1]<p[1]&&(*p1)[1]>=p[1])
			{
			/* Check if the edge intersection is to the right of p: */
			if((p[1]-(*p1)[1])*((*p2)[0]-(*p1)[0])<(p[0]-(*p1)[0])*((*p2)[1]-(*p1)[1]))
				++numIntersections;
			}
		
		/* Go to the next edge: */
		p1=p2;
		}
	
	/* The point is inside if the number of intersections is odd: */
	return (numIntersections&0x1)==0x1;
	}

#if !defined(NONSTANDARD_TEMPLATES)

/*****************************************************************
Force instantiation of all standard Polygon classes and functions:
*****************************************************************/

template class PolygonBase<float,2>;
template class PolygonBase<float,3>;

template class PolygonBase<double,2>;
template class PolygonBase<double,3>;

template class Polygon<float,2>;
template class Polygon<float,3>;

template class Polygon<double,2>;
template class Polygon<double,3>;

#endif

}
