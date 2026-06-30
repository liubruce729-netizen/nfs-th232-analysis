// File : GListDevice.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GListDevice
//
// This class manger tape and file in Ganil format
// The associated methods can do copies,duplications, verifications, dumps...
//
/////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/


#ifndef __GListDevice__
#define __GListDevice__

#include <sstream>
using std::ostream;


#include <TThread.h>
#include <TH1.h>
#include "General.h"
#include "GBuffer.h"

#include "GDevice.h"
/*
#include "GTape.h"
#include "GNetClientRoot.h"
#include "GClientMemory.h"

#ifdef NET_LIB
#include "GNetClientSoap.h"
#include "GNetClientNarval.h"
#endif

*/
#define NO_MYDEVICE 0
#define NO_GTape 1
#define NO_GNetClientRoot 2
#define NO_GClientMemory 3
#define NO_GNetClientSoap 4
#define NO_GNetClientGanil 5
#define NO_GNetClientNarval 6

#define NB_OF_DEVICES 7

//_________________________________________________________________________________________


class GListDevice : public TObject{

	GDevice *fListDevices[NB_OF_DEVICES];

	public:
	GListDevice(void);
	~GListDevice();
	    virtual GDevice* GetMyDevice()             {return ((GDevice*)(fListDevices[NO_MYDEVICE]));};
		virtual GDevice* GetTape()                   {return ((GDevice*)(fListDevices[NO_GTape]));};
		virtual GDevice* GetNetClientRoot() {return ((GDevice*)(fListDevices[NO_GNetClientRoot]));};
		virtual GDevice* GetClientMemory()  {return ((GDevice*)(fListDevices[NO_GClientMemory]));}

		virtual void SetMyDevice(GDevice* dev)      {fListDevices[NO_MYDEVICE]       = dev;};
		virtual void SetTape(GDevice* dev)          {fListDevices[NO_GTape]          = dev;};
		virtual void SetNetClientRoot(GDevice* dev) {fListDevices[NO_GNetClientRoot] = dev;};
		virtual void SetClientMemory(GDevice* dev)  {fListDevices[NO_GClientMemory]  = dev;};

	#ifdef NET_LIB
		virtual GDevice* GetNetClientGanil()   {return (GDevice*)(fListDevices[NO_GNetClientGanil]);};
		virtual GDevice* GetNetClientSoap()    {return (GDevice*)(fListDevices[NO_GNetClientSoap]);};
		virtual GDevice* GetNetClientNarval()  {return (GDevice*)(fListDevices[NO_GNetClientNarval]);};
		virtual void SetNetClientGanil(GDevice* dev)   {fListDevices[NO_GNetClientGanil]  = dev;};
		virtual void SetNetClientSoap(GDevice* dev)    {fListDevices[NO_GNetClientSoap]   = dev;};
		virtual void SetNetClientNarval(GDevice* dev)  {fListDevices[NO_GNetClientNarval] = dev;};
	#else
		virtual GDevice* GetNetClientGanil()   {return NULL;};
		virtual GDevice* GetNetClientSoap()    {return NULL;};
		virtual GDevice* GetNetClientNarval()  {return NULL;};
		virtual void SetNetClientGanil(GDevice* dev)   {};
		virtual void SetNetClientSoap(GDevice* dev)    {};
		virtual void SetNetClientNarval(GDevice* dev)  {};
	#endif

	virtual void ClearList();

  ClassDef (GListDevice ,1); // Liste of device

};

#endif
