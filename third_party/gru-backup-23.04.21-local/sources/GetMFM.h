//--      FILE : GetMFM.h 		  --
//--    AUTHOR : Luc Legeard    --

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#ifndef __GetMFM__
#define __GetMFM__

#include <sys/time.h>
#include "acq_codes_erreur.h"
#include "GDevice.h"
#include "acq_codes_erreur.h"
#include <stdlib.h>
#include <TString.h>
#include "General.h"
#include "GTtape_erreur.h"
#include "GBuffer.h"
#include "GBufferMFM.h"

// test with these devices to begin
#include "DeviceRandMFM.h"
#include "DeviceRFSoc.h"
#include "DevicePureTCP.h"
#include "DevicePureUDP.h"
#include "DeviceZMQ.h"
#include "DeviceFDT.h"
#include "DeviceNarval.h"
#include "DeviceXdaq.h"
#include "DeviceFile.h"
#include "DeviceEbyBlockFile.h"


#if defined(__CLING__) && !defined(__ROOTCLING__)
#include "DeviceGECO.h"
#include "DeviceEbyBlockTcp.h"
#include "DeviceGeneric.h"
#endif

/* to do
#include "DeviceEbyBlockTcp.h"--> thread  problème avec #include <pthread.h>
#include "DeviceGECO.h" --> u_int16_t problème avec #include "GECOSoapH.h"
when we 
#include "DeviceGeneric.h"
so we have introduce GRU_DICTIONARY

these errors is du to cint in root5 , no pb on root6 so i introduce #defined(__CLING__) && !defined(__ROOTCLING__)

*/


//#include "DeviceGeneric.h"
#ifndef GRU_DICTIONARY
#endif

//____________________________________________________________________________________________
class GetMFM : public GDevice
{

 private:

//DeviceGeneric * fGetMFMDevice; //!!! TO_CHANGE
   Device        * fGetMFMDevice; //!!!
   DeviceRandMFM * fGetMFMDeviceRand; //!!!
   DeviceRFSoc   * fGetMFMDeviceRF; //!!!
   DevicePureTCP * fGetMFMDeviceTCP; //!!!
   DevicePureUDP * fGetMFMDeviceUDP; //!!!
   DeviceFDT     * fGetMFMDeviceFDT; //!!!
   DeviceZMQ     * fGetMFMDeviceZMQ; //!!!
   DeviceNarval  * fGetMFMDeviceNarval; //!!!
   DeviceXdaq    * fGetMFMDeviceXdaq; //!!!
   DeviceFile    * fGetMFMDeviceFile; //!!!
   DeviceEbyBlockFile *  fGetMFMDeviceEbyBlockFile; //!!!
#if defined(__CLING__) && !defined(__ROOTCLING__)
   DeviceEbyBlockTcp  *  fGetMFMDeviceEbyBlockTcp;  //!!!
   DeviceGeneric       * fGetMFMDeviceGeneric;
#endif
    
   UtilVector_c  * MyVector_c;   //!!!
 public:
  GetMFM (void);				// constructeur par defaut d'un objet GetMFM 
  GetMFM (const char* host);		        // constructeur par defaut d'un objet GetMFM avec le nom de la machine hote connecter
  ~GetMFM ();
  virtual void InitReceiver (const char* host,int port,char* type );
  virtual void GetMFMInit(const char* host);
 // These following functions are  abstract in GDevice
  virtual void Open(char mod = 'r') ;
  virtual void Open(char* mod);
  virtual void Close ();
  virtual void ReadBuffer (); // recuperation d'un buffer de donnees brutes
  virtual void Rewind(bool quiet=false) { };
  virtual void Inquire(char* Exp_Name ) {cout<<" This Inquire do nothing " <<  Exp_Name << "\n";}
  virtual TNamed* GetSpectrum(const char* histoname=NULL,TNamed* old_sp=NULL){cout<<" This GetSpectrum do nothing\n";return NULL;};
  virtual TString* GetListSpectra(){cout<<" This GetListSpectra do nothing\n";return NULL;}; // get a Liste name of histograms
  virtual void WriteBuffer(GBuffer* buffer ) {cout<<" This WriteBuffer do nothing\n";}
  ClassDef (GetMFM, 1); // GetMFM client device (inherits from GDevice)
};

#endif

