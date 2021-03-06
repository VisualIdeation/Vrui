/***********************************************************************
SplinePatch - Class for n-dimensional non-uniform, non-rational tensor-
product B-spline patches.
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

#define GEOMETRY_SPLINEPATCH_IMPLEMENTATION

#ifndef METHODPREFIX
	#ifdef NONSTANDARD_TEMPLATES
		#define METHODPREFIX inline
	#else
		#define METHODPREFIX
	#endif
#endif

#include <Geometry/SplinePatch.h>

namespace Geometry {

/****************************
Methods of class SplinePatch:
****************************/

template <class ScalarParam,int dimensionParam>
METHODPREFIX
typename SplinePatch<ScalarParam,dimensionParam>::Index
SplinePatch<ScalarParam,dimensionParam>::findInterval(
	const typename SplinePatch<ScalarParam,dimensionParam>::Parameter& u) const
	{
	Index result;
	
	/* Perform binary search in each of the knot arrays independently: */
	for(int i=0;i<2;++i)
		{
		int l=degree[i]-1;
		int r=numPoints[i]-1;
		while(r-l>1)
			{
			int t=(l+r)>>1;
			if(u[i]<knots[i][t])
				r=t;
			else
				l=t;
			}
		result[i]=l;
		}
	
	return result;
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
void
SplinePatch<ScalarParam,dimensionParam>::deBoorStage0(
	const typename SplinePatch<ScalarParam,dimensionParam>::Parameter& u,
	typename SplinePatch<ScalarParam,dimensionParam>::EvaluationCache* cache,
	const typename SplinePatch<ScalarParam,dimensionParam>::Index& iv,
	const typename SplinePatch<ScalarParam,dimensionParam>::Size& subDegree) const
	{
	/* Perform deBoor's algorithm independently for each row of intermediate points: */
	for(int i=0;i<=subDegree[1];++i)
		{
		/* Perform deBoor's algorithm: */
		const Scalar* knotBase=&knots[0][iv[0]-subDegree[0]+1];
		Point* pointBase=&cache->points[i*(degree[0]+1)];
		for(int j=0;j<subDegree[0];++j,++knotBase,++pointBase)
			{
			Scalar alpha=(u[0]-knotBase[0])/(knotBase[subDegree[0]]-knotBase[0]);
			pointBase[0]=affineCombination(pointBase[0],pointBase[1],alpha);
			}
		}
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
void
SplinePatch<ScalarParam,dimensionParam>::deBoorStage1(
	const typename SplinePatch<ScalarParam,dimensionParam>::Parameter& u,
	typename SplinePatch<ScalarParam,dimensionParam>::EvaluationCache* cache,
	const typename SplinePatch<ScalarParam,dimensionParam>::Index& iv,
	const typename SplinePatch<ScalarParam,dimensionParam>::Size& subDegree) const
	{
	/* Perform deBoor's algorithm independently for each column of intermediate points: */
	for(int j=0;j<=subDegree[0];++j)
		{
		/* Perform deBoor's algorithm: */
		const Scalar* knotBase=&knots[1][iv[1]-subDegree[1]+1];
		Point* pointBase=&cache->points[j];
		for(int i=0;i<subDegree[1];++i,++knotBase,pointBase+=degree[0]+1)
			{
			Scalar alpha=(u[1]-knotBase[0])/(knotBase[subDegree[1]]-knotBase[0]);
			pointBase[0]=affineCombination(pointBase[0],pointBase[degree[0]+1],alpha);
			}
		}
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
SplinePatch<ScalarParam,dimensionParam>::SplinePatch(
	const typename SplinePatch<ScalarParam,dimensionParam>::Size& sDegree,
	const typename SplinePatch<ScalarParam,dimensionParam>::Size& sNumPoints,
	const typename SplinePatch<ScalarParam,dimensionParam>::Scalar* const sKnots[2],
	const typename SplinePatch<ScalarParam,dimensionParam>::Point* sPoints)
	:degree(sDegree),numPoints(sNumPoints),
	 points(new Point[numPoints[0]*numPoints[1]])
	{
	/* Copy the knot arrays: */
	for(int i=0;i<2;++i)
		{
		knots[i]=new Scalar[numPoints[i]+degree[i]-1];
		for(int j=0;j<numPoints[i]+degree[i]-1;++j)
			knots[i][j]=sKnots[i][j];
		}
	
	if(sPoints!=0)
		{
		/* Copy the control point array: */
		Point* dPtr=points;
		const Point* sPtr=sPoints;
		for(int i=0;i<numPoints[1];++i)
			for(int j=0;j<numPoints[0];++j,++dPtr,++sPtr)
				*dPtr=*sPtr;
		}
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
SplinePatch<ScalarParam,dimensionParam>::~SplinePatch(
	void)
	{
	delete[] knots[0];
	delete[] knots[1];
	delete[] points;
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
typename SplinePatch<ScalarParam,dimensionParam>::Point
SplinePatch<ScalarParam,dimensionParam>::evaluate(
	const typename SplinePatch<ScalarParam,dimensionParam>::Parameter& u,
	typename SplinePatch<ScalarParam,dimensionParam>::EvaluationCache* cache) const
	{
	/* Find the knot interval containing the given parameter: */
	Index iv=findInterval(u);
	
	/* Copy the control points defining the respective curve segment into the evaluation cache: */
	for(int i=0;i<=degree[1];++i)
		for(int j=0;j<=degree[0];++j)
			cache->points[i*(degree[0]+1)+j]=points[(i+iv[1]-degree[1]+1)*numPoints[0]+(j+iv[0]-degree[0]+1)];
	
	/* Perform degree[1] stages of deBoor's algorithm along the patch's major direction: */
	Size subDegree=degree;
	for(;subDegree[1]>0;--subDegree[1])
		deBoorStage1(u,cache,iv,subDegree);
	
	/* Perform degree[0] stages of deBoor's algorithm along the patch's minor direction: */
	for(;subDegree[0]>0;--subDegree[0])
		deBoorStage0(u,cache,iv,subDegree);
	
	/* Return the result point: */
	return cache->points[0];
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
typename SplinePatch<ScalarParam,dimensionParam>::Point
SplinePatch<ScalarParam,dimensionParam>::evaluate(
	const typename SplinePatch<ScalarParam,dimensionParam>::Parameter& u,
	typename SplinePatch<ScalarParam,dimensionParam>::EvaluationCache* cache,
	typename SplinePatch<ScalarParam,dimensionParam>::Vector& deriv0,
	typename SplinePatch<ScalarParam,dimensionParam>::Vector& deriv1) const
	{
	/* Find the knot interval containing the given parameter: */
	Index iv=findInterval(u);
	
	/* Copy the control points defining the respective curve segment into the evaluation cache: */
	for(int i=0;i<=degree[1];++i)
		for(int j=0;j<=degree[0];++j)
			cache->points[i*(degree[0]+1)+j]=points[(i+iv[1]-degree[1]+1)*numPoints[0]+(j+iv[0]-degree[0]+1)];
	
	/* Perform degree[1]-1 stages of deBoor's algorithm along the patch's major direction: */
	Size subDegree=degree;
	for(;subDegree[1]>1;--subDegree[1])
		deBoorStage1(u,cache,iv,subDegree);
	
	/* Perform degree[0]-1 stages of deBoor's algorithm along the patch's minor direction: */
	for(;subDegree[0]>1;--subDegree[0])
		deBoorStage0(u,cache,iv,subDegree);
	
	/* Calculate the derivative vectors: */
	Scalar base0=knots[0][iv[0]+1]-knots[0][iv[0]];
	Scalar alpha0=(u[0]-knots[0][iv[0]])/base0;
	Scalar base1=knots[1][iv[1]+1]-knots[1][iv[1]];
	Scalar alpha1=(u[1]-knots[1][iv[1]])/base1;
	deriv0=(cache->points[1]-cache->points[0])*(Scalar(1)-alpha1)+(cache->points[degree[0]+2]-cache->points[degree[0]+1])*alpha1;
	deriv0*=Scalar(degree[0])/base0;
	deriv1=(cache->points[degree[0]+1]-cache->points[0])*(Scalar(1)-alpha0)+(cache->points[degree[0]+2]-cache->points[1])*alpha0;
	deriv1*=Scalar(degree[1])/base1;
	
	/* Perform last stages of deBoor's algorithm in both directions: */
	deBoorStage1(u,cache,iv,subDegree);
	--subDegree[1];
	deBoorStage0(u,cache,iv,subDegree);
	
	/* Return the result point: */
	return cache->points[0];
	}

#if !defined(NONSTANDARD_TEMPLATES)

/*********************************************************************
Force instantiation of all standard SplinePatch classes and functions:
*********************************************************************/

template class SplinePatch<float,2>;
template class SplinePatch<double,2>;
template class SplinePatch<float,3>;
template class SplinePatch<double,3>;

#endif

}
