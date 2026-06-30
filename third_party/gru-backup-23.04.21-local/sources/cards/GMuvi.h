// File :  GMuvi.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Description of a Muvi card
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


#ifndef __GMuvi__
#define __GMuvi__

// definition of constant muvi cards
#define NB_TELESCOPES 4
#define NB_MATES  18  // mates nb /telescop  
#define NB_CHANNELS_PER_MATE 16 // nb of channel by mates
#define CHANNEL_SIZE 16384 // Channel size
#define ENERGIE_AND_TIME 2 // 
#define X_AND_Y  2  // This done by mufee cards ( one for x and one for y)

#define NB_MAX_OF_PICS  1000 // 1000 is fully arbitrary



//    Mate00 -> 1/16 channel  -    
//    Mate01 -> 1/16 channel  |
//    Mate02 -> 1/16 channel  |
//    Mate03 -> 1/16 channel  |
//    Mate04 -> 1/16 channel  |  X ---
//    Mate05 -> 1/16 channel  |       |
//    Mate06 -> 1/16 channel  |       |
//    Mate07 -> 1/16 channel  |       |
//    Mate16 -> 1/16 channel  -       |
//   				      | - Telescope 0   = 288 channel (but  energy or in time =>*2 => 576
//    Mate08 -> 1/16 channel  -       |
//    Mate09 -> 1/16 channel  |       |
//    Mate10 -> 1/16 channel  |       |
//    Mate11 -> 1/16 channel  |       |
//    Mate12 -> 1/16 channel  |  Y ---
//    Mate13 -> 1/16 channel  |
//    Mate14 -> 1/16 channel  |
//    Mate15 -> 1/16 channel  |
//    Mate17 -> 1/16 channel  -

//.....  				  - Telescope 1

//.....  				  - Telescope 2

//.....  				  - Telescope 3

// note , each channel can be in energy or in time ( so *2)

#endif
 
