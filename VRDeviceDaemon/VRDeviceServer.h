/***********************************************************************
VRDeviceServer - Class encapsulating the VR device protocol's server
side.
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

#include <vector>
#include <Threads/Thread.h>
#include <Threads/Mutex.h>
#include <Threads/MutexCond.h>
#include <Comm/TCPSocket.h>
#include <Vrui/VRDevicePipe.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFile;
}
class VRDeviceManager;

class VRDeviceServer
	{
	/* Embedded classes: */
	private:
	class ClientData // Class containing state of connected client
		{
		/* Elements: */
		public:
		Vrui::VRDevicePipe pipe; // Pipe connected to the client
		Threads::Thread communicationThread; // Client communication thread
		bool active; // Flag if the client is active
		bool streaming; // Flag if the client is streaming
		
		/* Constructors and destructors: */
		ClientData(const Comm::TCPSocket& socket)
			:pipe(socket),active(false),streaming(false)
			{
			};
		};
	
	typedef std::vector<ClientData*> ClientList; // Data type for lists of states of connected clients
	
	/* Elements: */
	private:
	VRDeviceManager* deviceManager; // Pointer to device manager running in server
	Comm::TCPSocket listenSocket; // Main socket the server listens on for incoming connections
	Threads::Thread listenThread; // Connection initiating thread
	Threads::Mutex clientListMutex; // Mutex serializing access to the client list
	ClientList clientList; // List of currently connected clients
	int numActiveClients; // Number of clients that are currently active
	//Threads trackerUpdateCompleteMutex; // Mutex to serialize access to tracker update notification condition variable
	Threads::MutexCond trackerUpdateCompleteCond; // Tracker update notification condition variable
	
	/* Private methods: */
	void* listenThreadMethod(void); // Connection initiating thread method
	void* clientCommunicationThreadMethod(ClientData* clientData); // Client communication thread method
	
	/* Constructors and destructors: */
	public:
	VRDeviceServer(VRDeviceManager* sDeviceManager,const Misc::ConfigurationFile& configFile); // Creates server associated with device manager
	~VRDeviceServer(void);
	};
