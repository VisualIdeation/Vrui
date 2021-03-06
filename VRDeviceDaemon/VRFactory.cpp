/***********************************************************************
VRFactory - Class to create objects of a specific class class derived
from a common base class.
Copyright (c) 2002-2005 Oliver Kreylos

This file is part of the Vrui VR Device Driver Daemon (VRDeviceDaemon).

The Vrui VR Device Driver Daemon is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Vrui VR Device Driver Daemon is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui VR Device Driver Daemon; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#define VRFACTORY_IMPLEMENTATION

#include "VRFactory.h"

#include <string.h>
#include <stdio.h>
#include <dlfcn.h>
#include <Misc/ThrowStdErr.h>

namespace {

/************************************************************************
Helper function to retrieve function pointers from DSOs without warnings:
************************************************************************/

typedef void (*FunctionPointer)(void);

inline FunctionPointer funcdlsym(void* dsoHandle,const char* functionName)
	{
	/* Use a workaround to get rid of warnings in g++'s pedantic mode: */
	ptrdiff_t intermediate=reinterpret_cast<ptrdiff_t>(dlsym(dsoHandle,functionName));
	return reinterpret_cast<FunctionPointer>(intermediate);
	}

}

/**************************
Methods of class VRFactory:
**************************/

template <class BaseClassParam>
inline
VRFactory<BaseClassParam>::VRFactory(
	VRFactory<BaseClassParam>::FactoryManager* sFactoryManager,
	const std::string& sClassName,
	const std::string& dsoPath)
	:factoryManager(sFactoryManager),className(sClassName),dsoHandle(0),createObjectFunc(0),destroyObjectFunc(0)
	{
	/* Construct DSO name: */
	char dsoName[2048];
	size_t dsoNameLen=sizeof(dsoName);
	char* dsoNamePtr=dsoName;
	memcpy(dsoNamePtr,dsoPath.c_str(),dsoPath.size());
	dsoNamePtr+=dsoPath.size();
	dsoNameLen-=dsoPath.size();
	dsoNamePtr[0]='/';
	++dsoNamePtr;
	--dsoNameLen;
	snprintf(dsoNamePtr,dsoNameLen,SYSDSONAMETEMPLATE,className.c_str());
	
	/* Load DSO: */
	dsoHandle=dlopen(dsoName,RTLD_LAZY|RTLD_GLOBAL);
	if(dsoHandle==0)
		Misc::throwStdErr("VRFactory: Could not load dynamic library %s due to %s",dsoName,dlerror());
	
	/* Get address to class creation function: */
	std::string createClassFuncName=std::string("createClass")+className;
	createClassFunc=CreateClassFuncType(funcdlsym(dsoHandle,createClassFuncName.c_str()));
	
	/* Get address to object creation function: */
	std::string createObjectFuncName=std::string("createObject")+className;
	createObjectFunc=CreateObjectFuncType(funcdlsym(dsoHandle,createObjectFuncName.c_str()));
	if(createObjectFunc==0)
		Misc::throwStdErr("VRFactory: Could not resolve function %s in dynamic library %s due to %s",createObjectFuncName.c_str(),dsoName,dlerror());

	/* Get address to object destruction function: */
	std::string destroyObjectFuncName=std::string("destroyObject")+className;
	destroyObjectFunc=DestroyObjectFuncType(funcdlsym(dsoHandle,destroyObjectFuncName.c_str()));
	if(destroyObjectFunc==0)
		Misc::throwStdErr("VRFactory: Could not resolve function %s in dynamic library %s due to %s",destroyObjectFuncName.c_str(),dsoName,dlerror());
	
	/* Get address to class destruction function: */
	std::string destroyClassFuncName=std::string("destroyClass")+className;
	destroyClassFunc=DestroyClassFuncType(funcdlsym(dsoHandle,destroyClassFuncName.c_str()));
	
	/* Call class creation function: */
	if(createClassFunc!=0)
		createClassFunc(this,factoryManager);
	}

template <class BaseClassParam>
inline
VRFactory<BaseClassParam>::~VRFactory(void)
	{
	/* Call class destruction function: */
	if(destroyClassFunc!=0)
		destroyClassFunc(this,factoryManager);
	
	/* Unload DSO: */
	dlclose(dsoHandle);
	}
