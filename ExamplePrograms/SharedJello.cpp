/***********************************************************************
SharedJello - VR program to interact with "virtual Jell-O" in a
collaborative VR environment using a client/server approach and a
simplified force interaction model based on the Nanotech Construction
Kit.
Copyright (c) 2007 Oliver Kreylos

This file is part of the Virtual Jell-O interactive VR demonstration.

Virtual Jell-O is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

Virtual Jell-O is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with Virtual Jell-O; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <string.h>
#include <stdexcept>
#include <iostream>
#include <Misc/ThrowStdErr.h>
#include <Geometry/Plane.h>
#include <GL/gl.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Menu.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>
#include <GLMotif/Button.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/Slider.h>
#include <Vrui/Tools/DraggingTool.h>
#include <Vrui/Vrui.h>

#include "SharedJello.h"

/*****************************************
Methods of class SharedJello::AtomDragger:
*****************************************/

SharedJello::AtomDragger::AtomDragger(Vrui::DraggingTool* sTool,SharedJello* sApplication,unsigned int sDraggerID)
	:Vrui::DraggingToolAdapter(sTool),
	 application(sApplication),
	 draggerID(sDraggerID),
	 draggerRayBased(false),
	 active(false)
	{
	}

void SharedJello::AtomDragger::idleMotionCallback(Vrui::DraggingTool::IdleMotionCallbackData* cbData)
	{
	/* Update the dragger position: */
	draggerTransformation=ONTransform(cbData->currentTransformation.getTranslation(),cbData->currentTransformation.getRotation());
	}

void SharedJello::AtomDragger::dragStartCallback(Vrui::DraggingTool::DragStartCallbackData* cbData)
	{
	/* Store the dragger's selection ray if it is ray-based: */
	draggerRayBased=cbData->rayBased;
	if(draggerRayBased)
		draggerRay=cbData->ray;
	
	/* Activate this dragger: */
	active=true;
	}

void SharedJello::AtomDragger::dragCallback(Vrui::DraggingTool::DragCallbackData* cbData)
	{
	/* Update the dragger position: */
	draggerTransformation=ONTransform(cbData->currentTransformation.getTranslation(),cbData->currentTransformation.getRotation());
	}

void SharedJello::AtomDragger::dragEndCallback(Vrui::DraggingTool::DragEndCallbackData* cbData)
	{
	/* Deactivate this dragger: */
	active=false;
	}

/****************************
Methods of class SharedJello:
****************************/

GLMotif::PopupMenu* SharedJello::createMainMenu(void)
	{
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Collaborative Virtual Jell-O");
	
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	GLMotif::Button* centerDisplayButton=new GLMotif::Button("CenterDisplayButton",mainMenu,"Center Display");
	centerDisplayButton->getSelectCallbacks().add(this,&SharedJello::centerDisplayCallback);
	
	GLMotif::ToggleButton* showSettingsDialogToggle=new GLMotif::ToggleButton("ShowSettingsDialogToggle",mainMenu,"Show Settings Dialog");
	showSettingsDialogToggle->getValueChangedCallbacks().add(this,&SharedJello::showSettingsDialogCallback);
	
	GLMotif::Button* createInputDeviceButton=new GLMotif::Button("CreateInputDeviceButton",mainMenu,"Create Input Device");
	createInputDeviceButton->getSelectCallbacks().add(this,&SharedJello::createInputDeviceCallback);
	
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

void SharedJello::updateSettingsDialog(void)
	{
	/* Update the atom mass slider: */
	double jiggliness=(Math::log(double(atomMass))/Math::log(1.1)+32.0)/64.0;
	jigglinessTextField->setValue(jiggliness);
	jigglinessSlider->setValue(jiggliness);
	
	/* Update the viscosity slider: */
	viscosityTextField->setValue(1.0-double(attenuation));
	viscositySlider->setValue(1.0-double(attenuation));
	
	/* Update the gravity slider: */
	gravityTextField->setValue(double(gravity));
	gravitySlider->setValue(double(gravity));
	}

GLMotif::PopupWindow* SharedJello::createSettingsDialog(void)
	{
	const GLMotif::StyleSheet& ss=*Vrui::getWidgetManager()->getStyleSheet();
	
	settingsDialog=new GLMotif::PopupWindow("SettingsDialog",Vrui::getWidgetManager(),"Settings Dialog");
	
	GLMotif::RowColumn* settings=new GLMotif::RowColumn("Settings",settingsDialog,false);
	settings->setNumMinorWidgets(3);
	
	double jiggliness=(Math::log(double(atomMass))/Math::log(1.1)+32.0)/64.0;
	
	new GLMotif::Label("JigglinessLabel",settings,"Jiggliness",ss.font);
	
	jigglinessTextField=new GLMotif::TextField("JigglinessTextField",settings,6);
	jigglinessTextField->setFieldWidth(6);
	jigglinessTextField->setPrecision(4);
	jigglinessTextField->setValue(jiggliness);
	
	jigglinessSlider=new GLMotif::Slider("JigglinessSlider",settings,GLMotif::Slider::HORIZONTAL,ss.fontHeight*10.0f);
	jigglinessSlider->setValueRange(0.0,1.0,0.01);
	jigglinessSlider->setValue(jiggliness);
	jigglinessSlider->getValueChangedCallbacks().add(this,&SharedJello::jigglinessSliderCallback);
	
	double viscosity=1.0-double(attenuation);
	
	new GLMotif::Label("ViscosityLabel",settings,"Viscosity");
	
	viscosityTextField=new GLMotif::TextField("ViscosityTextField",settings,6);
	viscosityTextField->setFieldWidth(6);
	viscosityTextField->setPrecision(2);
	viscosityTextField->setValue(viscosity);
	
	viscositySlider=new GLMotif::Slider("ViscositySlider",settings,GLMotif::Slider::HORIZONTAL,ss.fontHeight*10.0f);
	viscositySlider->setValueRange(0.0,1.0,0.01);
	viscositySlider->setValue(viscosity);
	viscositySlider->getValueChangedCallbacks().add(this,&SharedJello::viscositySliderCallback);
	
	new GLMotif::Label("GravityLabel",settings,"Gravity");
	
	gravityTextField=new GLMotif::TextField("GravityTextField",settings,6);
	gravityTextField->setFieldWidth(6);
	gravityTextField->setPrecision(2);
	gravityTextField->setValue(double(gravity));
	
	gravitySlider=new GLMotif::Slider("GravitySlider",settings,GLMotif::Slider::HORIZONTAL,ss.fontHeight*10.0f);
	gravitySlider->setValueRange(0.0,40.0,0.5);
	gravitySlider->setValue(double(gravity));
	gravitySlider->getValueChangedCallbacks().add(this,&SharedJello::gravitySliderCallback);
	
	settings->manageChild();
	
	updateSettingsDialog();
	
	return settingsDialog;
	}

void SharedJello::sendParamUpdate(void)
	{
	/* Send a parameter state update to the server: */
	{
	Threads::Mutex::Lock pipeLock(pipe->getMutex());
	pipe->writeMessage(SharedJelloPipe::CLIENT_PARAMUPDATE);
	pipe->write<Scalar>(atomMass);
	pipe->write<Scalar>(attenuation);
	pipe->write<Scalar>(gravity);
	
	pipe->flush();
	}
	}

void* SharedJello::communicationThreadMethod(void)
	{
	/* Enable immediate cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Run the server communication protocol machine: */
	try
		{
		while(true)
			{
			/* Wait for the next message: */
			SharedJelloPipe::MessageIdType message=pipe->readMessage();
			
			/* Bail out if disconnecting or an unexpected message was received: */
			if(message==SharedJelloPipe::DISCONNECT_REPLY)
				{
				break;
				}
			else if(message==SharedJelloPipe::SERVER_PARAMUPDATE)
				{
				/* Read the new simulation parameters: */
				atomMass=pipe->read<Scalar>();
				attenuation=pipe->read<Scalar>();
				gravity=pipe->read<Scalar>();
				++newParameterVersion;
				
				/* Request a redraw: */
				Vrui::requestUpdate();
				}
			else if(message==SharedJelloPipe::SERVER_UPDATE)
				{
				/* Lock the next free crystal slot: */
				int nextIndex=(lockedIndex+1)%3;
				if(nextIndex==mostRecentIndex)
					nextIndex=(nextIndex+1)%3;
				
				/* Process the server update message: */
				crystals[nextIndex]->readAtomStates(*pipe);
				
				/* Update the slot's crystal renderer: */
				renderers[nextIndex]->update();
				
				/* Mark the client update slot as most recent: */
				mostRecentIndex=nextIndex;
				
				/* Request a redraw: */
				Vrui::requestUpdate();
				}
			else
				Misc::throwStdErr("SharedJello::communicationThreadMethod: Protocol error");
			}
		}
	catch(std::runtime_error err)
		{
		/* Ignore any connection errors; just disconnect the client */
		std::cerr<<"Caught exception "<<err.what()<<std::endl<<std::flush;
		}
	catch(...)
		{
		/* Ignore any connection errors; just disconnect the client */
		std::cerr<<"Caught spurious exception"<<std::endl<<std::flush;
		}
	
	return 0;
	}

SharedJello::SharedJello(int& argc,char**& argv,char**& appDefaults)
	:Vrui::Application(argc,argv,appDefaults),
	 pipe(0),
	 newParameterVersion(1),parameterVersion(1),
	 lockedIndex(0),mostRecentIndex(1),
	 nextDraggerID(0),
	 mainMenu(0),settingsDialog(0)
	{
	/* Parse the command line: */
	const char* serverHostName=0;
	int serverPortID=-1;
	bool renderDomainBox=true;
	for(int i=0;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"host")==0)
				{
				++i;
				if(i<argc)
					serverHostName=argv[i];
				else
					std::cerr<<"SharedJello::SharedJello: Ignored dangling -host option"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"port")==0)
				{
				++i;
				if(i<argc)
					serverPortID=atoi(argv[i]);
				else
					std::cerr<<"SharedJello::SharedJello: Ignored dangling -port option"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"nobox")==0)
				renderDomainBox=false;
			}
		}
	if(serverHostName==0||serverPortID<0)
		Misc::throwStdErr("SharedJello::SharedJello: No server host name or port ID provided");
	
	/* Connect to the shared Jell-O server: */
	pipe=new SharedJelloPipe(serverHostName,serverPortID,Vrui::openPipe());
	
	/* Initiate the connection: */
	if(pipe->readMessage()!=SharedJelloPipe::CONNECT_REPLY)
		{
		/* Bail out: */
		Misc::throwStdErr("SharedJello::SharedJello: Connection refused by shared Jell-O server");
		}
	
	/* Read the Jell-O crystal's domain box: */
	Point domainMin=pipe->readPoint();
	Point domainMax=pipe->readPoint();
	domain=Box(domainMin,domainMax);
	
	/* Read the number of atoms in the Jell-O crystal: */
	JelloCrystal::Index numAtoms;
	pipe->read<int>(numAtoms.getComponents(),3);
	
	/* Create triple buffer of Jell-O crystals: */
	for(int i=0;i<3;++i)
		crystals[i]=new JelloCrystal(numAtoms);
	
	/* Wait for the first parameter update message to get the initial simulation parameters: */
	if(pipe->readMessage()!=SharedJelloPipe::SERVER_PARAMUPDATE)
		{
		/* Bail out: */
		Misc::throwStdErr("SharedJello::SharedJello: Connection refused by shared Jell-O server");
		}
	atomMass=pipe->read<Scalar>();
	attenuation=pipe->read<Scalar>();
	gravity=pipe->read<Scalar>();
	
	/* Wait for the first server update message to get initial atom positions: */
	if(pipe->readMessage()!=SharedJelloPipe::SERVER_UPDATE)
		{
		/* Bail out: */
		Misc::throwStdErr("SharedJello::SharedJello: Connection refused by shared Jell-O server");
		}
	crystals[mostRecentIndex]->readAtomStates(*pipe);
	
	/* Calculate the domain box color: */
	GLColor<GLfloat,3> domainBoxColor;
	for(int i=0;i<3;++i)
		domainBoxColor[i]=1.0f-Vrui::getBackgroundColor()[i];
	
	/* Create the triple buffer of Jell-O renderers: */
	for(int i=0;i<3;++i)
		{
		renderers[i]=new JelloRenderer(*crystals[i]);
		renderers[i]->setRenderDomainBox(renderDomainBox);
		renderers[i]->setDomainBoxColor(domainBoxColor);
		}
	renderers[mostRecentIndex]->update();
	
	/* Start the server communication thread: */
	communicationThread.start(this,&SharedJello::communicationThreadMethod);
	
	/* Create the program's user interface: */
	mainMenu=createMainMenu();
	Vrui::setMainMenu(mainMenu);
	settingsDialog=createSettingsDialog();
	
	/* Initialize the navigation transformation: */
	centerDisplayCallback(0);
	}

SharedJello::~SharedJello(void)
	{
	if(pipe!=0)
		{
		{
		/* Ask the server to disconnect: */
		Threads::Mutex::Lock pipeLock(pipe->getMutex());
		pipe->writeMessage(SharedJelloPipe::DISCONNECT_REQUEST);
		pipe->flush();
		pipe->shutdown(false,true);
		}
		}
	
	/* Wait until the communication thread receives the disconnect reply and terminates: */
	communicationThread.join();
	
	/* Close the server pipe: */
	delete pipe;
	
	/* Delete the crystal and renderer triple-buffer: */
	for(int i=0;i<3;++i)
		{
		delete renderers[i];
		delete crystals[i];
		}
	
	/* Delete all atom draggers: */
	for(AtomDraggerList::iterator adIt=atomDraggers.begin();adIt!=atomDraggers.end();++adIt)
		delete *adIt;
	
	/* Delete the user interface: */
	delete mainMenu;
	delete settingsDialog;
	}

void SharedJello::toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData)
	{
	/* Check if the new tool is a dragging tool: */
	Vrui::DraggingTool* tool=dynamic_cast<Vrui::DraggingTool*>(cbData->tool);
	if(tool!=0)
		{
		/* Create an atom dragger object and associate it with the new tool: */
		AtomDragger* newDragger=new AtomDragger(tool,this,nextDraggerID);
		
		/* Add new dragger to list: */
		++nextDraggerID;
		atomDraggers.push_back(newDragger);
		}
	}

void SharedJello::toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData)
	{
	/* Check if the to-be-destroyed tool is a dragging tool: */
	Vrui::DraggingTool* tool=dynamic_cast<Vrui::DraggingTool*>(cbData->tool);
	if(tool!=0)
		{
		/* Find the atom dragger associated with the tool in the list: */
		AtomDraggerList::iterator adIt;
		for(adIt=atomDraggers.begin();adIt!=atomDraggers.end()&&(*adIt)->getTool()!=tool;++adIt)
			;
		if(adIt!=atomDraggers.end())
			{
			/* Remove the atom dragger: */
			delete *adIt;
			atomDraggers.erase(adIt);
			}
		}
	}

void SharedJello::frame(void)
	{
	/* Check if there was a parameter update since the last frame: */
	if(parameterVersion!=newParameterVersion)
		{
		/* Update the settings dialog: */
		updateSettingsDialog();
		parameterVersion=newParameterVersion;
		}
	
	/* Check if there was a server state update since the last frame: */
	if(lockedIndex!=mostRecentIndex)
		{
		/* Deactivate the previously locked crystal state: */
		renderers[lockedIndex]->setActive(false);
		
		/* Lock the most recent crystal state: */
		lockedIndex=mostRecentIndex;
		
		/* Activate the previously locked crystal state: */
		renderers[lockedIndex]->setActive(true);
		}
	
	/* Send a state update to the server: */
	{
	Threads::Mutex::Lock pipeLock(pipe->getMutex());
	pipe->writeMessage(SharedJelloPipe::CLIENT_UPDATE);
	pipe->write<int>(atomDraggers.size());
	for(AtomDraggerList::iterator adIt=atomDraggers.begin();adIt!=atomDraggers.end();++adIt)
		{
		pipe->write<unsigned int>((*adIt)->draggerID);
		pipe->write<int>((*adIt)->draggerRayBased?1:0);
		pipe->writePoint((*adIt)->draggerRay.getOrigin());
		pipe->writeVector((*adIt)->draggerRay.getDirection());
		pipe->writeONTransform((*adIt)->draggerTransformation);
		pipe->write<char>((*adIt)->active?char(1):char(0));
		}
	pipe->flush();
	}
	}

void SharedJello::display(GLContextData& contextData) const
	{
	/* Render the Jell-O crystal: */
	renderers[lockedIndex]->glRenderAction(contextData);
	}

void SharedJello::centerDisplayCallback(Misc::CallbackData* cbData)
	{
	/* Set up navigation transformation to transform local physical coordinates to canonical shared Jell-O space: */
	Vrui::Point floorDisplayCenter=Vrui::getFloorPlane().project(Vrui::getDisplayCenter());
	Vrui::Vector floorForward=Geometry::normalize(Vrui::getFloorPlane().project(Vrui::getForwardDirection()));
	Vrui::Vector floorRight=Geometry::normalize(Geometry::cross(floorForward,Vrui::getFloorPlane().getNormal()));
	Vrui::Rotation rot=Vrui::Rotation::fromBaseVectors(floorRight,floorForward);
	Vrui::setNavigationTransformation(Vrui::NavTransform(floorDisplayCenter-Vrui::Point::origin,rot,Vrui::getInchFactor()));
	}

void SharedJello::showSettingsDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Hide or show settings dialog based on toggle button state: */
	if(cbData->set)
		{
		/* Pop up the settings dialog at the same position as the main menu: */
		Vrui::getWidgetManager()->popupPrimaryWidget(settingsDialog,Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
		}
	else
		Vrui::popdownPrimaryWidget(settingsDialog);
	}

void SharedJello::createInputDeviceCallback(Misc::CallbackData* cbData)
	{
	}

void SharedJello::jigglinessSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Compute and set the atom mass: */
	double jiggliness=cbData->value;
	atomMass=Scalar(Math::exp(Math::log(1.1)*(jiggliness*64.0-32.0)));
	
	/* Send a parameter update to the server: */
	sendParamUpdate();
	}

void SharedJello::viscositySliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Set the attenuation: */
	attenuation=Scalar(1)-Scalar(cbData->value);
	
	/* Send a parameter update to the server: */
	sendParamUpdate();
	}

void SharedJello::gravitySliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Set the gravity: */
	gravity=Scalar(cbData->value);
	
	/* Send a parameter update to the server: */
	sendParamUpdate();
	}

/*********************
Main program function:
*********************/

int main(int argc,char* argv[])
	{
	try
		{
		char** appDefaults=0;
		SharedJello sj(argc,argv,appDefaults);
		sj.run();
		}
	catch(std::runtime_error err)
		{
		std::cerr<<"Caught exception "<<err.what()<<std::endl;
		return 1;
		}
	
	return 0;
	}
