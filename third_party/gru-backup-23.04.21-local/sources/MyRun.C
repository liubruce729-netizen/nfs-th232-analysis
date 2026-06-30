
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mtio.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <TObject.h>
#include "MyRun.h"
//______________________________________________________________________________
ClassImp(MyRun)

MyRun::MyRun(const char *name, const char *title) :
	TTask(name, title) {
}

void MyRun::Exec(Option_t *option){
		printf("GRU executing \n");
		};



