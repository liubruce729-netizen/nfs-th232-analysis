

#ifndef  __GVCompositeFrame__
#define  __GVCompositeFrame__


#include <TGFrame.h>

#include <TRootEmbeddedCanvas.h>


class GVCompositeFrame:public TGCompositeFrame {
    
private: 

  TRootEmbeddedCanvas *fCanvas;

public:
  
  GVCompositeFrame(const TGWindow *p);

  TRootEmbeddedCanvas* GetRootCanvas();
  
  virtual ~GVCompositeFrame();
   
  ClassDef(GVCompositeFrame,1)
};

#endif
