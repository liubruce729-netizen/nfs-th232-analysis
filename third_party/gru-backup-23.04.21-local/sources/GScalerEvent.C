// $Id: GScalerEvent.cpp,v 1.1 2001/07/04 09:42:44 patois/legeard Exp $
// Author: $Author: patois/legeard $
//***************************************************************************
//                             -------------------
//    begin                : Thu Jun 14 2001
//    copyright            : (C) 2001 by Garp
//    email                : patois@ganil.fr legeard@ganil.fr
//////////////////////////////////////////////////////////////////////////
//
// Part of GRU
//
// GScalerEvent
//
// Scaler class for scaler events.
//
//////////////////////////////////////////////////////////////////////////

// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************

extern "C" {
#include <GEN_TYPE.H>
}
#include "GScalerEvent.h"
#include "GBufferIn2p3.h"

ClassImp( GScalerEvent);

//______________________________________________________________________________
GScalerEvent::GScalerEvent(void) {
	// Default constructor. dont do anything
	Raz();
}

//______________________________________________________________________________
GScalerEvent::GScalerEvent(GBufferIn2p3* _buffer, int i, UInt_t nbevent) {
	Raz();
	Set(_buffer, i, nbevent);

}

//______________________________________________________________________________
GScalerEvent::~GScalerEvent(void) {

}
//______________________________________________________________________________
void GScalerEvent::Raz(){
	fLabel =  0;
	fStatus = 0;
	fCount =  0;
	fFreq =   0;
	fTics =   0;
	fEventCount = 0;
	for (int j=0;j<3;j++)
	fReserve[j]= 0;
}
//___
//______________________________________________________________________________
void GScalerEvent::Set(GBufferIn2p3*_buffer, int i, UInt_t nbevent) {
	fLabel
			= (UInt_t) _buffer -> GetBuffer_map_in2p3()->les_donnees.cas.scale.jbus_scale.UnScale[i].Label;
	fStatus
			= (Int_t) _buffer -> GetBuffer_map_in2p3()->les_donnees.cas.scale.jbus_scale.UnScale[i].Status;
	fCount
			= (UInt_t) _buffer -> GetBuffer_map_in2p3()->les_donnees.cas.scale.jbus_scale.UnScale[i].Count;
	fFreq
			= (UInt_t) _buffer -> GetBuffer_map_in2p3()->les_donnees.cas.scale.jbus_scale.UnScale[i].Freq;
	fTics
			= (UInt_t) _buffer -> GetBuffer_map_in2p3()->les_donnees.cas.scale.jbus_scale.UnScale[i].Tics;
	fEventCount = nbevent;
	for (int j = 0; j < 3; j++)
		fReserve[j]
				= _buffer->GetBuffer_map_in2p3()->les_donnees.cas.scale.jbus_scale.UnScale[j].Reserve[i];

}

