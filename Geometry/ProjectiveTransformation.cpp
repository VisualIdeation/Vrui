/***********************************************************************
ProjectiveTransformation - Class for n-dimensional projective
transformations.
Copyright (c) 2001-2005 Oliver Kreylos

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

#define GEOMETRY_PROJECTIVETRANSFORMATION_IMPLEMENTATION

#ifndef METHODPREFIX
	#ifdef NONSTANDARD_TEMPLATES
		#define METHODPREFIX inline
	#else
		#define METHODPREFIX
	#endif
#endif

#include <Geometry/TranslationTransformation.h>
#include <Geometry/RotationTransformation.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/UniformScalingTransformation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/ScalingTransformation.h>
#include <Geometry/AffineTransformation.h>

#include <Geometry/ProjectiveTransformation.h>

namespace Geometry {

/***************************
Get matrix helper functions:
***************************/

#include <Geometry/MatrixHelperFunctions.h>

/*******************************************************
Methods of class ProjectiveTransformationOperationsBase:
*******************************************************/

template <class ScalarParam,int dimensionParam>
METHODPREFIX
Vector<ScalarParam,dimensionParam>
ProjectiveTransformationOperationsBase<ScalarParam,dimensionParam>::transformV(
	const Matrix<ScalarParam,dimensionParam+1,dimensionParam+1>& m,
	const Vector<ScalarParam,dimensionParam>& v)
	{
	Vector<ScalarParam,dimensionParam> result;
	for(int i=0;i<dimensionParam;++i)
		{
		ScalarParam c(0);
		for(int j=0;j<dimensionParam;++j)
			c+=m(i,j)*v[j];
		result[i]=c;
		}
	return result;
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
Point<ScalarParam,dimensionParam>
ProjectiveTransformationOperationsBase<ScalarParam,dimensionParam>::transformP(
	const Matrix<ScalarParam,dimensionParam+1,dimensionParam+1>& m,
	const Point<ScalarParam,dimensionParam>& p)
	{
	ScalarParam weight=m(dimensionParam,dimensionParam);
	for(int j=0;j<dimensionParam;++j)
		weight+=m(dimensionParam,j)*p[j];
	Point<ScalarParam,dimensionParam> result;
	for(int i=0;i<dimensionParam;++i)
		{
		ScalarParam c=m(i,dimensionParam);
		for(int j=0;j<dimensionParam;++j)
			c+=m(i,j)*p[j];
		result[i]=c/weight;
		}
	return result;
	}

#if 0
template <class ScalarParam,int dimensionParam>
METHODPREFIX
HVector<ScalarParam,dimensionParam>
ProjectiveTransformationOperationsBase<ScalarParam,dimensionParam>::transformHV(
	const Matrix<ScalarParam,dimensionParam+1,dimensionParam+1>& m,
	const HVector<ScalarParam,dimensionParam>& hv)
	{
	HVector<ScalarParam,dimensionParam> result;
	for(int i=0;i<=dimensionParam;++i)
		{
		ScalarParam c(0);
		for(int j=0;j<=dimensionParam;++j)
			c+=m(i,j)*hv[j];
		result[i]=c;
		}
	return result;
	}
#endif

template <class ScalarParam,int dimensionParam>
METHODPREFIX
Vector<ScalarParam,dimensionParam>
ProjectiveTransformationOperationsBase<ScalarParam,dimensionParam>::inverseTransformV(
	const Matrix<ScalarParam,dimensionParam+1,dimensionParam+1>& m,
	const Vector<ScalarParam,dimensionParam>& v)
	{
	/* Create the extended matrix: */
	double temp[dimensionParam+1][dimensionParam+2];
	for(int i=0;i<dimensionParam;++i)
		{
		for(int j=0;j<=dimensionParam;++j)
			temp[i][j]=m(i,j);
		temp[i][dimensionParam+1]=double(v[i]);
		}
	for(int j=0;j<=dimensionParam;++j)
		temp[dimensionParam][j]=m(dimensionParam,j);
	temp[dimensionParam][dimensionParam+1]=ScalarParam(0);
	
	/* Perform Gaussian elimination with column pivoting on the extended matrix: */
	gaussElimination<dimensionParam+1,dimensionParam+2>(temp);
	
	/* Return the result vector: */
	Vector<ScalarParam,dimensionParam> result;
	temp[dimensionParam][dimensionParam+1]/=temp[dimensionParam][dimensionParam];
	for(int i=dimensionParam-1;i>=0;--i)
		{
		for(int j=i+1;j<=dimensionParam;++j)
			temp[i][dimensionParam+1]-=temp[i][j]*temp[j][dimensionParam+1];
		temp[i][dimensionParam+1]/=temp[i][i];
		result[i]=ScalarParam(temp[i][dimensionParam+1]);
		}
	return result;
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
Point<ScalarParam,dimensionParam>
ProjectiveTransformationOperationsBase<ScalarParam,dimensionParam>::inverseTransformP(
	const Matrix<ScalarParam,dimensionParam+1,dimensionParam+1>& m,
	const Point<ScalarParam,dimensionParam>& p)
	{
	/* Create the extended matrix: */
	double temp[dimensionParam+1][dimensionParam+2];
	for(int i=0;i<dimensionParam;++i)
		{
		for(int j=0;j<=dimensionParam;++j)
			temp[i][j]=m(i,j);
		temp[i][dimensionParam+1]=double(p[i]);
		}
	for(int j=0;j<=dimensionParam;++j)
		temp[dimensionParam][j]=m(dimensionParam,j);
	temp[dimensionParam][dimensionParam+1]=ScalarParam(1);
	
	/* Perform Gaussian elimination with column pivoting on the extended matrix: */
	gaussElimination<dimensionParam+1,dimensionParam+2>(temp);
	
	/* Return the result point: */
	Point<ScalarParam,dimensionParam> result;
	temp[dimensionParam][dimensionParam+1]/=temp[dimensionParam][dimensionParam];
	for(int i=dimensionParam-1;i>=0;--i)
		{
		for(int j=i+1;j<=dimensionParam;++j)
			temp[i][dimensionParam+1]-=temp[i][j]*temp[j][dimensionParam+1];
		temp[i][dimensionParam+1]/=temp[i][i];
		result[i]=ScalarParam(temp[i][dimensionParam+1]/temp[dimensionParam][dimensionParam+1]);
		}
	return result;
	}

#if 0
template <class ScalarParam,int dimensionParam>
METHODPREFIX
HVector<ScalarParam,dimensionParam>
ProjectiveTransformationOperationsBase<ScalarParam,dimensionParam>::inverseTransformHV(
	const Matrix<ScalarParam,dimensionParam+1,dimensionParam+1>& m,
	const HVector<ScalarParam,dimensionParam>& hv)
	{
	/* Create the extended matrix: */
	double temp[dimensionParam+1][dimensionParam+2];
	for(int i=0;i<=dimensionParam;++i)
		{
		for(int j=0;j<=dimensionParam;++j)
			temp[i][j]=m(i,j);
		temp[i][dimensionParam+1]=double(hv[i]);
		}
	
	/* Perform Gaussian elimination with column pivoting on the extended matrix: */
	gaussElimination<dimensionParam+1,dimensionParam+2>(temp);
	
	/* Return the result vector: */
	HVector<ScalarParam,dimensionParam> result;
	for(int i=dimensionParam;i>=0;--i)
		{
		for(int j=i+1;j<=dimensionParam;++j)
			temp[i][dimensionParam+1]-=temp[i][j]*temp[j][dimensionParam+1];
		temp[i][dimensionParam+1]/=temp[i][i];
		result[i]=ScalarParam(temp[i][dimensionParam+1]);
		}
	return result;
	}
#endif

/*************************************************
Static elements of class ProjectiveTransformation:
*************************************************/

template <class ScalarParam,int dimensionParam>
const int ProjectiveTransformation<ScalarParam,dimensionParam>::dimension;
template <class ScalarParam,int dimensionParam>
const ProjectiveTransformation<ScalarParam,dimensionParam> ProjectiveTransformation<ScalarParam,dimensionParam>::identity; // Default constructor creates identity transformation!

/*****************************************
Methods of class ProjectiveTransformation:
*****************************************/

template <class ScalarParam,int dimensionParam>
template <class SourceScalarParam>
METHODPREFIX ProjectiveTransformation<ScalarParam,dimensionParam>::ProjectiveTransformation(const TranslationTransformation<SourceScalarParam,dimensionParam>& source)
	:matrix(Scalar(1))
	{
	source.writeMatrix(matrix);
	}

template <class ScalarParam,int dimensionParam>
template <class SourceScalarParam>
METHODPREFIX ProjectiveTransformation<ScalarParam,dimensionParam>::ProjectiveTransformation(const RotationTransformation<SourceScalarParam,dimensionParam>& source)
	:matrix(Scalar(1))
	{
	source.writeMatrix(matrix);
	}

template <class ScalarParam,int dimensionParam>
template <class SourceScalarParam>
METHODPREFIX ProjectiveTransformation<ScalarParam,dimensionParam>::ProjectiveTransformation(const OrthonormalTransformation<SourceScalarParam,dimensionParam>& source)
	:matrix(Scalar(1))
	{
	source.writeMatrix(matrix);
	}

template <class ScalarParam,int dimensionParam>
template <class SourceScalarParam>
METHODPREFIX ProjectiveTransformation<ScalarParam,dimensionParam>::ProjectiveTransformation(const UniformScalingTransformation<SourceScalarParam,dimensionParam>& source)
	:matrix(Scalar(1))
	{
	source.writeMatrix(matrix);
	}

template <class ScalarParam,int dimensionParam>
template <class SourceScalarParam>
METHODPREFIX ProjectiveTransformation<ScalarParam,dimensionParam>::ProjectiveTransformation(const OrthogonalTransformation<SourceScalarParam,dimensionParam>& source)
	:matrix(Scalar(1))
	{
	source.writeMatrix(matrix);
	}

template <class ScalarParam,int dimensionParam>
template <class SourceScalarParam>
METHODPREFIX ProjectiveTransformation<ScalarParam,dimensionParam>::ProjectiveTransformation(const ScalingTransformation<SourceScalarParam,dimensionParam>& source)
	:matrix(Scalar(1))
	{
	source.writeMatrix(matrix);
	}

template <class ScalarParam,int dimensionParam>
template <class SourceScalarParam>
METHODPREFIX ProjectiveTransformation<ScalarParam,dimensionParam>::ProjectiveTransformation(const AffineTransformation<SourceScalarParam,dimensionParam>& source)
	:matrix(Scalar(1))
	{
	source.writeMatrix(matrix);
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
ProjectiveTransformation<ScalarParam,dimensionParam>
ProjectiveTransformation<ScalarParam,dimensionParam>::rotateAround(
	const typename ProjectiveTransformation<ScalarParam,dimensionParam>::Point& pivot,
	const typename ProjectiveTransformation<ScalarParam,dimensionParam>::Rotation& sRotation)
	{
	Matrix sMatrix=Matrix::one;
	sRotation.writeMatrix(sMatrix);
	for(int i=0;i<dimension;++i)
		{
		sMatrix(i,dimension)=pivot[i];
		for(int j=0;j<dimensionParam;++j)
			sMatrix(i,dimension)-=sMatrix(i,j)*pivot[j];
		}
	return ProjectiveTransformation(sMatrix);
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
ProjectiveTransformation<ScalarParam,dimensionParam>
ProjectiveTransformation<ScalarParam,dimensionParam>::scaleAround(
	const typename ProjectiveTransformation<ScalarParam,dimensionParam>::Point& pivot,
	typename ProjectiveTransformation<ScalarParam,dimensionParam>::Scalar sScaling)
	{
	Matrix sMatrix=Matrix::one;
	for(int i=0;i<dimension;++i)
		sMatrix(i,i)=sScaling;
	for(int i=0;i<dimension;++i)
		sMatrix(i,dimension)=pivot[i]-sScaling*pivot[i];
	return ProjectiveTransformation(sMatrix);
	}

template <class ScalarParam,int dimensionParam>
METHODPREFIX
ProjectiveTransformation<ScalarParam,dimensionParam>
ProjectiveTransformation<ScalarParam,dimensionParam>::scaleAround(
	const typename ProjectiveTransformation<ScalarParam,dimensionParam>::Point& pivot,
	const typename ProjectiveTransformation<ScalarParam,dimensionParam>::Scale& sScaling)
	{
	Matrix sMatrix=Matrix::one;
	for(int i=0;i<dimension;++i)
		sMatrix(i,i)=sScaling[i];
	for(int i=0;i<dimension;++i)
		sMatrix(i,dimension)=pivot[i]-sScaling[i]*pivot[i];
	return ProjectiveTransformation(sMatrix);
	}

#if !defined(NONSTANDARD_TEMPLATES)

/**********************************************************************************
Force instantiation of all standard ProjectiveTransformation classes and functions:
**********************************************************************************/

template class ProjectiveTransformationOperations<float,2>;
template class ProjectiveTransformationOperations<double,2>;
template class ProjectiveTransformationOperationsBase<float,3>;
template class ProjectiveTransformationOperationsBase<double,3>;
template class ProjectiveTransformationOperations<float,3>;
template class ProjectiveTransformationOperations<double,3>;

template class ProjectiveTransformation<float,2>;
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const TranslationTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const RotationTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const OrthonormalTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const UniformScalingTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const OrthogonalTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const ScalingTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const AffineTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const TranslationTransformation<double,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const RotationTransformation<double,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const OrthonormalTransformation<double,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const UniformScalingTransformation<double,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const OrthogonalTransformation<double,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const ScalingTransformation<double,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const AffineTransformation<double,2>&);

template class ProjectiveTransformation<double,2>;
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const TranslationTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const RotationTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const OrthonormalTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const UniformScalingTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const OrthogonalTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const ScalingTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const AffineTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const TranslationTransformation<double,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const RotationTransformation<double,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const OrthonormalTransformation<double,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const UniformScalingTransformation<double,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const OrthogonalTransformation<double,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const ScalingTransformation<double,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const AffineTransformation<double,2>&);

template class ProjectiveTransformation<float,3>;
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const TranslationTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const RotationTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const OrthonormalTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const UniformScalingTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const OrthogonalTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const ScalingTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const AffineTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const TranslationTransformation<double,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const RotationTransformation<double,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const OrthonormalTransformation<double,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const UniformScalingTransformation<double,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const OrthogonalTransformation<double,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const ScalingTransformation<double,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const AffineTransformation<double,3>&);

template class ProjectiveTransformation<double,3>;
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const TranslationTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const RotationTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const OrthonormalTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const UniformScalingTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const OrthogonalTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const ScalingTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const AffineTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const TranslationTransformation<double,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const RotationTransformation<double,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const OrthonormalTransformation<double,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const UniformScalingTransformation<double,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const OrthogonalTransformation<double,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const ScalingTransformation<double,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const AffineTransformation<double,3>&);

#endif
}
