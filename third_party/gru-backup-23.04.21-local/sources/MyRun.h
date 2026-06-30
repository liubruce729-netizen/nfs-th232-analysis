

#ifndef __MyRun__
#define __MyRun__
#include "TTask.h"
class MyRun: public TTask {
public:
	MyRun() {
		;
	}
	MyRun(const char *name, const char *title);
	virtual ~MyRun() {
		;
	}
	void Exec(Option_t *option = "");
ClassDef(MyRun,1) // Run Reconstruction task
};


#endif


