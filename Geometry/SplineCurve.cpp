/***********************************************************************
SplineCurve - Class for n-dimensional non-uniform, non-rational B-spline
curves.
Copyright (c) 2001-2007 Oliver Kreylos

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

#define GEOMETRY_SPLINECURVE_IMPLEMENTATION

#ifndef METHODPREFIX
	#ifdef NONSTANDARD_TEMPLATES
		#define METHODPREFIX inline
	#else
		#define METHODPREFIX
	#endif
#endif

#include <Geometry/SplineCurve.h>

namespace Geometry {

/****************************
Methods of class SplineCurve:
****************************/

template <class ScalarParam,int dimensionParam>
METHODPREFIX
SplineCurve<ScalarParam,dimensionParam>::SplineCurve(
	int sDegree,
	int sNumPoints)
	:degree(sDegree),numPoints(sNumPoints),
	 knots(new Scalar[numPoints+degree-1]),points(new Point[numPoints])
	{
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
SplineCurve<ScalarParam,dimensionParam>::SplineCurve(
	int sDegree,
	int sNumPoints,
	const typename SplineCurve<ScalarParam,dimensionParam>::Point* sPoints,
	typename SplineCurve<ScalarParam,dimensionParam>::KnotVectorType knotVectorType)
	:degree(sDegree),numPoints(sNumPoints),
	 knots(new Scalar[numPoints+degree-1]),points(new Point[numPoints])
	{
	/* Create the knot vector: */
	switch(knotVectorType)
		{
		case UNIFORM:
			for(int i=0;i<numPoints+degree-1;++i)
				knots[i]=Scalar(i);
			break;
		
		case NATURAL:
			{
			Scalar* knotPtr=knots;
			for(int i=0;i<degree;++i,++knotPtr)
				*knotPtr=Scalar(0);
			for(int i=1;i<numPoints-degree;++i,++knotPtr)
				*knotPtr=Scalar(i);
			for(int i=0;i<degree;++i,++knotPtr)
				*knotPtr=Scalar(numPoints-degree);
			break;
			}
		}
	
	/* Copy the control point array: */
	for(int i=0;i<numPoints;++i)
		points[i]=sPoints[i];
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
SplineCurve<ScalarParam,dimensionParam>::SplineCurve(
	int sDegree,
	int sNumPoints,
	const typename SplineCurve<ScalarParam,dimensionParam>::Scalar* sKnots,
	const typename SplineCurve<ScalarParam,dimensionParam>::Point* sPoints)
	:degree(sDegree),numPoints(sNumPoints),
	 knots(new Scalar[numPoints+degree-1]),points(new Point[numPoints])
	{
	/* Copy the knot and control point arrays: */
	for(int i=0;i<numPoints+degree-1;++i)
		knots[i]=sKnots[i];
	for(int i=0;i<numPoints;++i)
		points[i]=sPoints[i];
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
SplineCurve<ScalarParam,dimensionParam>::SplineCurve(
	const SplineCurve<ScalarParam,dimensionParam>& source)
	:degree(source.degree),numPoints(source.numPoints),
	 knots(new Scalar[numPoints+degree-1]),points(new Point[numPoints])
	{
	/* Copy the knot and control point arrays: */
	for(int i=0;i<numPoints+degree-1;++i)
		knots[i]=source.knots[i];
	for(int i=0;i<numPoints;++i)
		points[i]=source.points[i];
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
SplineCurve<ScalarParam,dimensionParam>::~SplineCurve(
	void)
	{
	delete[] knots;
	delete[] points;
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
SplineCurve<ScalarParam,dimensionParam>&
SplineCurve<ScalarParam,dimensionParam>::operator=(
	const SplineCurve<ScalarParam,dimensionParam>& source)
	{
	/* Check for self-assignment: */
	if(&source!=this)
		{
		/* Delete current knot and control point arrays: */
		delete[] knots;
		delete[] points;
		
		/* Create new spline: */
		degree=source.degree;
		numPoints=source.numPoints;
		knots=new Scalar[numPoints+degree-1];
		points=new Point[numPoints];
		
		/* Copy the knot and control point arrays: */
		for(int i=0;i<numPoints+degree-1;++i)
			knots[i]=source.knots[i];
		for(int i=0;i<numPoints;++i)
			points[i]=source.points[i];
		}
	
	/* Return result of assignment: */
	return *this;
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
typename SplineCurve<ScalarParam,dimensionParam>::Point
SplineCurve<ScalarParam,dimensionParam>::evaluate(
	typename SplineCurve<ScalarParam,dimensionParam>::Scalar u,
	typename SplineCurve<ScalarParam,dimensionParam>::EvaluationCache* cache) const
	{
	/* Find the knot interval containing the given parameter: */
	int iv=findInterval(u);
	
	/* Copy the control points defining the respective curve segment into the evaluation cache: */
	const Point* pointBase=&points[iv-degree+1];
	for(int i=0;i<=degree;++i)
		cache->points[i]=pointBase[i];
	
	/* Perform degree stages of deBoor's algorithm: */
	for(int k=0;k<degree;++k)
		deBoorStage(u,cache,iv,k);
	
	/* Return the result point: */
	return cache->points[0];
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
typename SplineCurve<ScalarParam,dimensionParam>::Point
SplineCurve<ScalarParam,dimensionParam>::evaluate(
	typename SplineCurve<ScalarParam,dimensionParam>::Scalar u,
	typename SplineCurve<ScalarParam,dimensionParam>::EvaluationCache* cache,
	typename SplineCurve<ScalarParam,dimensionParam>::Vector& deriv1) const
	{
	/* Find the knot interval containing the given parameter: */
	int iv=findInterval(u);
	
	/* Copy the control points defining the respective curve segment into the evaluation cache: */
	const Point* pointBase=&points[iv-degree+1];
	for(int i=0;i<=degree;++i)
		cache->points[i]=pointBase[i];
	
	/* Perform degree-1 stages of deBoor's algorithm: */
	for(int k=0;k<degree-1;++k)
		deBoorStage(u,cache,iv,k);
	
	/* Calculate the first derivative: */
	deriv1=cache->points[1]-cache->points[0];
	deriv1*=Scalar(degree)/(knots[iv+1]-knots[iv]);
	
	/* Perform the last stage of deBoor's algorithm: */
	deBoorStage(u,cache,iv,degree-1);
	
	/* Return the result point: */
	return cache->points[0];
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
typename SplineCurve<ScalarParam,dimensionParam>::Point
SplineCurve<ScalarParam,dimensionParam>::evaluate(
	typename SplineCurve<ScalarParam,dimensionParam>::Scalar u,
	typename SplineCurve<ScalarParam,dimensionParam>::EvaluationCache* cache,
	typename SplineCurve<ScalarParam,dimensionParam>::Vector& deriv1,
	typename SplineCurve<ScalarParam,dimensionParam>::Vector& deriv2) const
	{
	/* Find the knot interval containing the given parameter: */
	int iv=findInterval(u);
	
	/* Copy the control points defining the respective curve segment into the evaluation cache: */
	const Point* pointBase=&points[iv-degree+1];
	for(int i=0;i<=degree;++i)
		cache->points[i]=pointBase[i];
	
	/* Perform degree-2 stages of deBoor's algorithm: */
	for(int k=0;k<degree-2;++k)
		deBoorStage(u,cache,iv,k);
	
	/* Calculate the second derivative: */
	Vector dc[2];
	dc[0]=cache->points[1]-cache->points[0];
	dc[0]*=Scalar(2)/(knots[iv+1]-knots[iv-1]);
	dc[1]=cache->points[2]-cache->points[1];
	dc[1]*=Scalar(2)/(knots[iv+2]-knots[iv]);
	deriv2=dc[1]-dc[0];
	deriv2*=Scalar(1)/(knots[iv+1]-knots[iv]);
	
	/* Perform the second last stage of deBoor's algorithm: */
	deBoorStage(u,cache,iv,degree-2);
	
	/* Calculate the first derivative: */
	deriv1=cache->points[1]-cache->points[0];
	deriv1*=Scalar(degree)/(knots[iv+1]-knots[iv]);
	
	/* Perform the last stage of deBoor's algorithm: */
	deBoorStage(u,cache,iv,degree-1);
	
	/* Return the result point: */
	return cache->points[0];
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
SplineCurve<ScalarParam,dimensionParam>&
SplineCurve<ScalarParam,dimensionParam>::insertKnot(
	typename SplineCurve<ScalarParam,dimensionParam>::Scalar newKnot)
	{
	/* Insert new knot into knot array: */
	int numKnots=numPoints+degree-1;
	Scalar* newKnots=new Scalar[numKnots+1];
	int i;
	for(i=0;i<numKnots&&knots[i]<=newKnot;++i)
		newKnots[i]=knots[i];
	int iv=i-degree;
	newKnots[i]=newKnot;
	for(;i<numKnots;++i)
		newKnots[i+1]=knots[i];
	
	/* Copy initial part of control point array: */
	Point* newPoints=new Point[numPoints+1];
	for(i=0;i<=iv;++i)
		newPoints[i]=points[i];
	
	/* Calculate new control points influenced by new knot as affine combinations of old ones: */
	for(;i<=iv+degree;++i)
		{
		Scalar alpha=(newKnot-knots[i-1])/(knots[i+degree-1]-knots[i-1]);
		newPoints[i]=affineCombination(points[i-1],points[i],alpha);
		}
	
	/* Copy rest of control point array: */
	for(;i<=numPoints;++i)
		newPoints[i]=points[i-1];
	
	/* Store new knot and control point arrays: */
	++numPoints;
	delete[] knots;
	knots=newKnots;
	delete[] points;
	points=newPoints;
	
	/* Return resulting spline curve: */
	return *this;
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
SplineCurve<ScalarParam,dimensionParam>&
SplineCurve<ScalarParam,dimensionParam>::elevateDegree(
	void)
	{
	/* Return resulting spline curve: */
	return *this;
	}

#if !defined(NONSTANDARD_TEMPLATES)

/*********************************************************************
Force instantiation of all standard SplineCurve classes and functions:
*********************************************************************/

template class SplineCurve<float,2>;
template class SplineCurve<double,2>;
template class SplineCurve<float,3>;
template class SplineCurve<double,3>;

#endif

}
