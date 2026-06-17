#ifndef __TRIGGERDATA__
#define __TRIGGERDATA__

#include <vector>

#include "TObject.h"
#include "TObjString.h"


class TTriggerData : public TObject {
 private:
   // TDC
   UShort_t	fTRIG_1;
   UShort_t	fTRIG_2;
   UShort_t	fTRIG_3;
   UShort_t	fTRIG_4;
   UShort_t	fAGAVADATA[30];
   
   //Time Stamp
   long long   fTimeStampsGlobal[10] ;
 
 public:
   TTriggerData();
   virtual ~TTriggerData();

      void	Clear();
      void	Dump();



   /////////////////////           GETTERS           ////////////////////////
   // (T)
   UShort_t	GetTRIG1()        {return fTRIG_1;} //vamos
   UShort_t	GetTRIG2()        {return fTRIG_2;} //vamos
   UShort_t	GetTRIG3()        {return fTRIG_3;} //vamos
   UShort_t	GetTRIG4()        {return fTRIG_4;} //EXOGAM MK2
   UShort_t	GetAGAVADATA(int i)        {return fAGAVADATA[i];}
   

   unsigned long long    GetfTimeStamps(int i)   {return fTimeStampsGlobal[i];}


   /////////////////////           SETTERS           ////////////////////////
   // (T)
   void	SetTRIG1(UShort_t T)     {fTRIG_1 = T;}
   void	SetTRIG2(UShort_t T)     {fTRIG_2 = T;}
   void	SetTRIG3(UShort_t T)     {fTRIG_3 = T;}
   void	SetTRIG4(UShort_t T)     {fTRIG_4 = T;}
   void	SetAGAVADATA(UShort_t T, int i)     {fAGAVADATA[i]=T;}
   
   
   
   
   
   void SetfTimeStamps(unsigned long long T, int i){fTimeStampsGlobal[i] = T; }

   ClassDef(TTriggerData,1)  // TriggerData structure
};

#endif
