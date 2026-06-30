// File : GSort.h
// Author: Luc Legeard  (winter 2004)
//////////////////////////////////////////////////////////////////////////////
//
// Class GSort
//
// This Class sort
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

#ifndef __GSort__
#define __GSort__
#include "GBase.h"
//______________________________________________________________________________________

class GSort : public GBase{
public:
	 GSort();
	  ~GSort();
	  void Help();
	  void Merge(char *liste_files);
	  virtual void ToDoInCaseOfInterrupt(){};
  ClassDef (GSort ,1) // Merge of run on time stamp

};
#endif
