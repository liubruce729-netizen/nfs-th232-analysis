#include "MFMMesytecFrame.h"

//_______________________________________________________________________________  
  MFMMesytecFrame::MFMMesytecFrame(){}
//_______________________________________________________________________________
  MFMMesytecFrame::~MFMMesytecFrame(){}
//_______________________________________________________________________________
void MFMMesytecFrame::SetAttributs(void *pt)
{
    MFMBlobFrame::SetAttributs(pt);
    SetEventSizeFromFrameData();
   // SetFrameTypeFromFrameData();
    //SetFrameSizeFromFrameData();
    //SetTimeStampFromFrameData();
    //SetEventNumberFromFrameData();
}
//_______________________________________________________________________________
void MFMMesytecFrame::SetUserDataPointer()
{
   set_user_data_pointer(MFM_Mesytec_frame,Data);
}

//_______________________________________________________________________________
void MFMMesytecFrame::SetTimeStampFromMesytecFrameData()
{
   /// Compute time stamp from  frame
   fTimeStamp = 0;
   uint64_t * timeStamp = &(fTimeStamp);
   fTimeStamp = ((MFM_Mesytec_frame*) pHeader)->EventInfo.timestamp;
   if (fLocalIsBigEndian != fFrameIsBigEndian)
      SwapInt64((timeStamp), 6);
}
//_______________________________________________________________________________
void MFMMesytecFrame::SetTimeStamp(uint64_t timestamp)
{
   /// Set value of Time Stamp in frame

   timestamp = timestamp & 0x0000ffffffffffff;
   ((MFM_Mesytec_frame*) pHeader)->EventInfo.timestamp = timestamp;
   MFMBlobFrame::SetTimeStamp(timestamp);
}
//_______________________________________________________________________________
void MFMMesytecFrame::SetEventNumberFromMesytecFrameData()
{
   /// set value of event number from  frame
   fEventNumber = 0;
   char * eventNumber = (char*) &(fEventNumber);
   fEventNumber = ((MFM_Mesytec_frame*) pHeader)->EventInfo.eventnumber;
   if (fLocalIsBigEndian != fFrameIsBigEndian)
      SwapInt32((uint32_t *) (eventNumber), 4);
}
//_______________________________________________________________________________
void MFMMesytecFrame::SetEventNumber(uint32_t eventnumber)
{
   /// Set Event Number of frame
   ((MFM_Mesytec_frame*) pHeader)->EventInfo.eventnumber = eventnumber;
    MFMCommonFrame::SetEventNumber(eventnumber);
}
//_______________________________________________________________________________
void MFMMesytecFrame::SetEventSizeFromFrameData()
{
    /// Read from the frame the value of MFM_Fazia_EventInfo::eventsize
    eventsize=0;
    char * eventSize = (char*) &(eventsize);
    eventsize = ((MFM_Mesytec_frame*) pHeader)->EventInfo.eventsize;
    if (fLocalIsBigEndian != fFrameIsBigEndian)
       SwapInt32((uint32_t *) (eventSize), 4);
}
//_______________________________________________________________________________
void MFMMesytecFrame::FillDataWithRamdomValue(uint64_t timestamp,
		uint32_t enventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	char i = 0;
	char * pchar =(char*)&(((MFM_Mesytec_frame*) pHeader)->Data);
	int usersize =MESYTEC_USERSIZE;
	SetEventNumber(enventnumber);
	SetTimeStamp(timestamp);
	for (i=0;i< usersize;i++)
		*(pchar+i) =i;
}
