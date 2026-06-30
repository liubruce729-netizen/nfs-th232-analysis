// $Id: GScalerEvent.H,v 1.2 2001/07/07 19:18:43 patois /legeard Exp $
// Author: $Author: patois /legeard $
//-*************************************************************************
//                        GTGanilData.cpp  -  Main Header to ROOTGAnilTape
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
// Scaler class for the scaler structure.
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

#ifndef __GScalerEvent__
#define __GScalerEvent__

#include <TObject.h>
#include "GBufferIn2p3.h"
// ---------------------------------------------------------------------------
class GScalerEvent : public TObject
{

private:
  UInt_t fLabel;      // channel label
  Int_t  fStatus;     // etat de la voie echelle =0 when stop , = 1 when started , -1 offline
  UInt_t fCount;      // value
  UInt_t fFreq;       // frequency
  UInt_t fTics;       // espace de temps pour calculer la frequence
  UInt_t fReserve[3]; // Comment here
  UInt_t fEventCount; // nb of physical event (not accurate)


public:
  GScalerEvent(void);
  GScalerEvent(GBufferIn2p3* _buffer ,int i=0,UInt_t nbevent =0);
  ~GScalerEvent();
  virtual void Raz();
  virtual UInt_t GetLabel(){return fLabel;};
  virtual Int_t  GetStatus(){return fStatus;};
  virtual UInt_t GetCount(){return fCount;};
  virtual UInt_t GetFreq(){return fFreq;};
  virtual UInt_t GetTics(){return fTics;};
  virtual UInt_t GetEventCount(){return fEventCount;};

  virtual UInt_t* GetLabelPoint(){return &fLabel;};
  virtual Int_t*  GetStatusPoint(){return &fStatus;};
  virtual UInt_t* GetCountPoint(){return &fCount;};
  virtual UInt_t* GetFreqPoint(){return &fFreq;};
  virtual UInt_t* GetTicsPoint(){return &fTics;};
  virtual UInt_t* GetEventCountPoint(){return &fEventCount;};
  virtual void Set(GBufferIn2p3* _buffer,int i=0,UInt_t nbevent =0);

  ClassDef(GScalerEvent,1); // Event scaler structure

};


#endif

