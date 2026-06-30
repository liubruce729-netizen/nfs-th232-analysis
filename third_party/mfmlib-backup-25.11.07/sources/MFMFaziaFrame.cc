#include "MFMFaziaFrame.h"
//_______________________________________________________________________________

MFMFaziaFrame::MFMFaziaFrame(){


 }
//_______________________________________________________________________________ 
MFMFaziaFrame::~MFMFaziaFrame(){
   

 }
 /*
//_______________________________________________________________________________ 
void MFMFaziaFrame::SetAttributs(void *pt)
{
    MFMBlobFrame::SetAttributs(pt);
    SetEventSizeFromFrameData();
}*/
//_______________________________________________________________________________ 
void MFMFaziaFrame::SetUserDataPointer()
{
   set_user_data_pointer(MFM_Fazia_Header,Data);
}
//_______________________________________________________________________________ 
void MFMFaziaFrame::SetTimeStampFromFaziaFrameData()
{
   /// Compute time stamp from  frame
   fTimeStamp = 0;
   uint64_t * timeStamp = &(fTimeStamp);
   memcpy(((char*) (&fTimeStamp)),
          ((MFM_Fazia_Header*) pHeader)->EventInfo.timestamp, 6);
   if (fLocalIsBigEndian != fFrameIsBigEndian)
      SwapInt64((timeStamp), 6);
}
//_______________________________________________________________________________ 
void MFMFaziaFrame::SetTimeStamp(uint64_t timestamp)
{
   /// Set value of Time Stamp in frame

   char* pts = (char*) &timestamp;
   timestamp = timestamp & 0x0000ffffffffffff;
   memcpy(((MFM_Fazia_Header*) pHeader)->EventInfo.timestamp, pts, 6);
   MFMBlobFrame::SetTimeStamp(timestamp);
}
//_______________________________________________________________________________ 
void MFMFaziaFrame::SetEventNumberFromFaziaFrameData()
{
   /// set value of event number from  frame
   fEventNumber = 0;
   char * eventNumber = (char*) &(fEventNumber);
   fEventNumber = ((MFM_Fazia_Header*) pHeader)->EventInfo.eventnumber;
   if (fLocalIsBigEndian != fFrameIsBigEndian)
      SwapInt32((uint32_t *) (eventNumber), 4);
}
//_______________________________________________________________________________ 
void MFMFaziaFrame::SetEventNumber(uint32_t eventnumber)
{
   /// Set Event Number of frame
   ((MFM_Fazia_Header*) pHeader)->EventInfo.eventnumber = eventnumber;
    MFMCommonFrame::SetEventNumber(eventnumber);
}
//_______________________________________________________________________________ 
void MFMFaziaFrame::FillDataWithRamdomValue(uint64_t timestamp,
		uint32_t enventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	char i = 0;
	char * pchar =(char*)&(((MFM_Fazia_Header*) pHeader)->Data);
	int usersize =FAZIA_USERSIZE;
	SetEventNumber(enventnumber);
	SetTimeStamp(timestamp);
	for (i=0;i< usersize;i++)
		*(pchar+i) =i;
}
//_______________________________________________________________________________ 
void MFMFaziaFrame::SetEventSizeFromFrameData()
{
    /// Read from the frame the value of MFM_Fazia_EventInfo::eventsize
    ///
    /// warning! 2017 data written in little-endian format, although the
    /// frame endianness is big-endian!
    eventsize=0;
    char * eventSize = (char*) &(eventsize);
    eventsize = ((MFM_Fazia_Header*) pHeader)->EventInfo.eventsize;
//    if (fLocalIsBigEndian != fFrameIsBigEndian)
//       SwapInt32((uint32_t *) (eventSize), 4);
}
//_______________________________________________________________________________ 
