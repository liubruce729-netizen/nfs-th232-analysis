// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#include "General.h"
#include "Calimero.h"
#include "TROOT.h"
#include "TRint.h"

#include "TGraph.h"
#include "TCanvas.h"


#include <TCanvas.h>
#include <TRootGuiFactory.h>



char*  version = (char*)GRU_VERSION;
int  Bufsize = BUFSIZE;
int  BufSize = Bufsize ;   // for use of  libgan_tape library


int main(int argc, char **argv) {


      cout << "  ******************************************* " << "\n";
      cout << "  *                                         * " << "\n";
      cout << "  *  HELLO  -- You are Running  Calimero    * " << "\n";
      cout << "  *   Version :" << version << "                     * \n";
      cout << "  *                                         * " << "\n";
      cout << "  ******************************************* " << "\n";



      TRint* theApp = new TRint("Calimero", &argc, argv, NULL, 0);
      //      theApp->GetOptions(&argc, argv) ;
      theApp->SetPrompt("Calimero>");

      //Run interactive interface
      theApp->Run(true);
      cout << " \n Bye Bye\n";
      delete (theApp);


}
