/***********************************************************************
GLTransformationWrappers - Wrapper functions to use templatized geometry
library transformation objects as parameters to OpenGL matrix functions.
Copyright (c) 2002-2005 Oliver Kreylos

This file is part of the OpenGL Wrapper Library for the Templatized
Geometry Library (GLGeometry).

The OpenGL Wrapper Library for the Templatized Geometry Library is free
software; you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any
later version.

The OpenGL Wrapper Library for the Templatized Geometry Library is
distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Wrapper Library for the Templatized Geometry Library; if
not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite
330, Boston, MA 02111-1307 USA
***********************************************************************/

#define GLTRANSFORMATIONWRAPPERS_IMPLEMENTATION

#ifndef METHODPREFIX
	#ifdef NONSTANDARD_TEMPLATES
		#define METHODPREFIX inline
	#else
		#define METHODPREFIX
	#endif
#endif

#include <Math/Math.h>
#include <Geometry/TranslationTransformation.h>
#include <Geometry/RotationTransformation.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/UniformScalingTransformation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/ScalingTransformation.h>
#include <Geometry/AffineTransformation.h>
#include <Geometry/ProjectiveTransformation.h>
#include <GL/gl.h>
#include <GL/GLMatrixTemplates.h>
#include <GL/GLGetMatrixTemplates.h>

#include <GL/GLTransformationWrappers.h>

/***********************************
Overloaded versions of matrix calls:
***********************************/

namespace GLTransformationWrappers { // PO'Leary

template <class ScalarParam>
METHODPREFIX
void
glLoadMatrix(
	const Geometry::TranslationTransformation<ScalarParam,3>& t)
	{
	glLoadIdentity();
	glTranslate(t.getTranslation().getComponents());
	}

template <class ScalarParam>
METHODPREFIX
void
glMultMatrix(
	const Geometry::TranslationTransformation<ScalarParam,3>& t)
	{
	glTranslate(t.getTranslation().getComponents());
	}

template <class ScalarParam>
METHODPREFIX
void
glLoadMatrix(
	const Geometry::RotationTransformation<ScalarParam,3>& t)
	{
	glLoadIdentity();
	glRotate(Math::deg(t.getRotation().getAngle()),t.getRotation().getAxis().getComponents());
	}

template <class ScalarParam>
METHODPREFIX
void
glMultMatrix(
	const Geometry::RotationTransformation<ScalarParam,3>& t)
	{
	glRotate(Math::deg(t.getRotation().getAngle()),t.getRotation().getAxis().getComponents());
	}

template <class ScalarParam>
METHODPREFIX
void
glLoadMatrix(
	const Geometry::OrthonormalTransformation<ScalarParam,3>& t)
	{
	glLoadIdentity();
	glTranslate(t.getTranslation().getComponents());
	glRotate(Math::deg(t.getRotation().getAngle()),t.getRotation().getAxis().getComponents());
	}

template <class ScalarParam>
METHODPREFIX
void
glMultMatrix(
	const Geometry::OrthonormalTransformation<ScalarParam,3>& t)
	{
	glTranslate(t.getTranslation().getComponents());
	glRotate(Math::deg(t.getRotation().getAngle()),t.getRotation().getAxis().getComponents());
	}

template <class ScalarParam>
METHODPREFIX
void
glLoadMatrix(
	const Geometry::UniformScalingTransformation<ScalarParam,3>& t)
	{
	glLoadIdentity();
	glScale(t.getScaling());
	}

template <class ScalarParam>
METHODPREFIX
void
glMultMatrix(
	const Geometry::UniformScalingTransformation<ScalarParam,3>& t)
	{
	glScale(t.getScaling());
	}

template <class ScalarParam>
METHODPREFIX
void
glLoadMatrix(
	const Geometry::OrthogonalTransformation<ScalarParam,3>& t)
	{
	glLoadIdentity();
	glTranslate(t.getTranslation().getComponents());
	glRotate(Math::deg(t.getRotation().getAngle()),t.getRotation().getAxis().getComponents());
	glScale(t.getScaling());
	}

template <class ScalarParam>
METHODPREFIX
void
glMultMatrix(
	const Geometry::OrthogonalTransformation<ScalarParam,3>& t)
	{
	glTranslate(t.getTranslation().getComponents());
	glRotate(Math::deg(t.getRotation().getAngle()),t.getRotation().getAxis().getComponents());
	glScale(t.getScaling());
	}

template <class ScalarParam>
METHODPREFIX
void
glLoadMatrix(
	const Geometry::ScalingTransformation<ScalarParam,3>& t)
	{
	glLoadIdentity();
	glScale(t.getScaling().getComponents());
	}

template <class ScalarParam>
METHODPREFIX
void
glMultMatrix(
	const Geometry::ScalingTransformation<ScalarParam,3>& t)
	{
	glScale(t.getScaling().getComponents());
	}

template <class ScalarParam>
METHODPREFIX
void
glLoadMatrix(
	const Geometry::AffineTransformation<ScalarParam,3>& t)
	{
	/* Copy the transformation coefficients into a temporary array: */
	ScalarParam temp[16];
	const typename Geometry::AffineTransformation<ScalarParam,3>::Matrix& m=t.getMatrix();
	ScalarParam* tPtr=temp;
	for(int j=0;j<4;++j)
		{
		for(int i=0;i<3;++i,++tPtr)
			*tPtr=m(i,j);
		*tPtr=ScalarParam(0);
		++tPtr;
		}
	temp[15]=ScalarParam(1);
	
	/* Upload the temporary array: */
	GLMatrixTemplates::glLoadMatrix(temp); // PO'Leary
	}

template <class ScalarParam>
METHODPREFIX
void
glMultMatrix(
	const Geometry::AffineTransformation<ScalarParam,3>& t)
	{
	/* Copy the transformation coefficients into a temporary array: */
	ScalarParam temp[16];
	const typename Geometry::AffineTransformation<ScalarParam,3>::Matrix& m=t.getMatrix();
	ScalarParam* tPtr=temp;
	for(int j=0;j<4;++j)
		{
		for(int i=0;i<3;++i,++tPtr)
			*tPtr=m(i,j);
		*tPtr=ScalarParam(0);
		++tPtr;
		}
	temp[15]=ScalarParam(1);
	
	/* Upload the temporary array: */
	GLMatrixTemplates::glMultMatrix(temp); // PO'Leary
	}

template <class ScalarParam>
METHODPREFIX
void
glLoadMatrix(
	const Geometry::ProjectiveTransformation<ScalarParam,3>& t)
	{
	/* Copy the transformation coefficients into a temporary array: */
	ScalarParam temp[16];
	const typename Geometry::ProjectiveTransformation<ScalarParam,3>::Matrix& m=t.getMatrix();
	ScalarParam* tPtr=temp;
	for(int j=0;j<4;++j)
		for(int i=0;i<4;++i,++tPtr)
			*tPtr=m(i,j);
	
	/* Upload the temporary array: */
	GLMatrixTemplates::glLoadMatrix(temp); // PO'Leary
	}

template <class ScalarParam>
METHODPREFIX
void
glMultMatrix(
	const Geometry::ProjectiveTransformation<ScalarParam,3>& t)
	{
	/* Copy the transformation coefficients into a temporary array: */
	ScalarParam temp[16];
	const typename Geometry::ProjectiveTransformation<ScalarParam,3>::Matrix& m=t.getMatrix();
	ScalarParam* tPtr=temp;
	for(int j=0;j<4;++j)
		for(int i=0;i<4;++i,++tPtr)
			*tPtr=m(i,j);
	
	/* Upload the temporary array: */
	GLMatrixTemplates::glMultMatrix(temp); // PO'Leary
	}

} // PO'Leary

template <class ScalarParam>
METHODPREFIX
Geometry::ProjectiveTransformation<ScalarParam,3>
glGetMatrix(
	GLMatrixEnums::Pname pname)
	{
	ScalarParam temp[16];
	glGetMatrix(pname,temp);
	return Geometry::ProjectiveTransformation<ScalarParam,3>::fromColumnMajor(temp);
	}

#if !defined(NONSTANDARD_TEMPLATES)

/********************************************************
Force instantiation of all standard GLGeometry functions:
********************************************************/

namespace GLTransformationWrappers { // PO'Leary

template void glLoadMatrix(const Geometry::TranslationTransformation<float,3>&);
template void glLoadMatrix(const Geometry::TranslationTransformation<double,3>&);
template void glMultMatrix(const Geometry::TranslationTransformation<float,3>&);
template void glMultMatrix(const Geometry::TranslationTransformation<double,3>&);

template void glLoadMatrix(const Geometry::RotationTransformation<float,3>&);
template void glLoadMatrix(const Geometry::RotationTransformation<double,3>&);
template void glMultMatrix(const Geometry::RotationTransformation<float,3>&);
template void glMultMatrix(const Geometry::RotationTransformation<double,3>&);

template void glLoadMatrix(const Geometry::OrthonormalTransformation<float,3>&);
template void glLoadMatrix(const Geometry::OrthonormalTransformation<double,3>&);
template void glMultMatrix(const Geometry::OrthonormalTransformation<float,3>&);
template void glMultMatrix(const Geometry::OrthonormalTransformation<double,3>&);

template void glLoadMatrix(const Geometry::UniformScalingTransformation<float,3>&);
template void glLoadMatrix(const Geometry::UniformScalingTransformation<double,3>&);
template void glMultMatrix(const Geometry::UniformScalingTransformation<float,3>&);
template void glMultMatrix(const Geometry::UniformScalingTransformation<double,3>&);

template void glLoadMatrix(const Geometry::OrthogonalTransformation<float,3>&);
template void glLoadMatrix(const Geometry::OrthogonalTransformation<double,3>&);
template void glMultMatrix(const Geometry::OrthogonalTransformation<float,3>&);
template void glMultMatrix(const Geometry::OrthogonalTransformation<double,3>&);

template void glLoadMatrix(const Geometry::ScalingTransformation<float,3>&);
template void glLoadMatrix(const Geometry::ScalingTransformation<double,3>&);
template void glMultMatrix(const Geometry::ScalingTransformation<float,3>&);
template void glMultMatrix(const Geometry::ScalingTransformation<double,3>&);

template void glLoadMatrix(const Geometry::AffineTransformation<float,3>&);
template void glLoadMatrix(const Geometry::AffineTransformation<double,3>&);
template void glMultMatrix(const Geometry::AffineTransformation<float,3>&);
template void glMultMatrix(const Geometry::AffineTransformation<double,3>&);

template void glLoadMatrix(const Geometry::ProjectiveTransformation<float,3>&);
template void glLoadMatrix(const Geometry::ProjectiveTransformation<double,3>&);
template void glMultMatrix(const Geometry::ProjectiveTransformation<float,3>&);
template void glMultMatrix(const Geometry::ProjectiveTransformation<double,3>&);

} // PO'Leary

template Geometry::ProjectiveTransformation<float,3> glGetMatrix<float>(GLMatrixEnums::Pname pname);
template Geometry::ProjectiveTransformation<double,3> glGetMatrix<double>(GLMatrixEnums::Pname pname);

#endif