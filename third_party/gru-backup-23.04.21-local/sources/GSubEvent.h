// File : GSubEvent.h
// Author: Luc Legeard  (spring 2010)
//////////////////////////////////////////////////////////////////////////////
//
// Class GEvent
//
// This class manger Ganil Sub-Event 
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

#ifndef __GSubEvent__
#define __GSubEvent__

#include "General.h"
#include "GBuffer.h"
#include "TRandom2.h"
#include "DataParameters.h"
#include "GEN_TYPE.H"
#include "GBase.h"
#include <sstream>



//______________________________________________________________________________________


class GSubEvent : public GBase
{

protected:

	Char_t		fSystemId;		//Id of the detector system (Exogam, Vamos, Tiara ... )
	Char_t		fClockWords;		//Number of 16bits words used to represent the sub-event clock
	Char_t		fStatusWords;		//Number of 16bits words used to represent the sub-event status
	Char_t		fSubEventNumberWords;	//Number of 16bits words used to represent the sub-event number
	Char_t		fFormatType;		// Format type of the sub-event
	Long64_t	fClock;			//fClock, fStatus and fSubEventNumber are Long64_t because they cas be represented
	Long64_t	fStatus;		//by 3 words of 16 bits. So only a 8bytes type can store it.
	Long64_t	fSubEventNumber;	// Number of the sub-event
	Int_t		fSubEvent_size;		// Size of the brut data buffer
	Int_t		fHeader_size;		// Size of the sub-event header
	Int_t		randSeed;		// Seed for function rand()
	UShort_t	*fSubEventBrut;		//! Brut data sub-event with header 
	UShort_t	*fSubEventBrutData;	//! Brut data sub-event
	TRandom2     *fRand;//
	int        fLaw;          // if true , random value follow binomial law
	DataParameters *fParameter;		// list of parameters name and label index
	Int_t		fNbDatas; // number of couples Label/Data of sub-event
    Float_t     fProba; // probability of appearance of a parameter
private:


public :

	GSubEvent(DataParameters* _fParameter);
	~GSubEvent(); 

	virtual void ClearData(Int_t);
	Int_t GetNbofDatas();
	Int_t GetSize();

	void SetSystem_id(Char_t system_id);
	void SetClockWords(Char_t clockWords);
	void SetStatusWords(Char_t statusWords);
	void SetNumberWords(Char_t numberWords);
	void SetFormatType(Char_t formatType);
	void SetRandomLaw(int law){ fLaw = law;};
	void SetRandomProba(Float_t proba){ fProba = proba;}
	UShort_t* GetSubEventBrut();
	virtual Int_t  GetSubEventNumber(){return fSubEventNumber;}
	virtual void  RazSubEvent();
	virtual void MakeSubEvent(Long64_t clock, Long64_t status, Long64_t number, Int_t* subTabOfLabels, Int_t sizeOfSubTab);
	virtual  void DumpSubEvent();

private :
	  virtual void ToDoInCaseOfInterrupt(){};
	  protected :

	virtual void MakeSubEventHeader();
	ClassDef (GSubEvent ,1); // Class to manage sub-event

};
#endif
