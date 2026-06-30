#ifndef __GVSpectraInfo__
#define __GVSpectraInfo__

#include <TGFrame.h>
#include <TGTextView.h>
#include <TGButton.h>

class GVSpectraInfo: public TGCompositeFrame
{
  
  
private:
  
  TGTextView *textName, *textSourceType, *textSource;

  TGCheckButton *fCheck;
  
public:
  
  GVSpectraInfo(const TGWindow *p,TString name, TString source,TString sourceName);

  virtual ~GVSpectraInfo();

  void Select(Bool_t down);

  void ClickCheck();

  Bool_t IsSelected();

  TString GetSpectraName();

  TString GetSourceType();

  Bool_t HandleButton(Event_t *event);

  Pixel_t white, orange, lightgrey, green;

  ClassDef(GVSpectraInfo,1)//Graphic row displaying informations about a spectrum

};
#endif
