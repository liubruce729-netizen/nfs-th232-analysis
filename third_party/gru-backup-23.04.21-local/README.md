Author Luc Legeard 

legeard @ ganil.fr

---------------


### GRU (Ganil Root Utilities)


(GRU)Ganil Root Utilities is a software including ROOT.

GRU has libraries to convert, copy , verify , analyze and  get Ganil format data from acquisition.
Three ways are available to get data :
	-from runs (tapes or disk)
	-from ganil acquisition (so GRU is client of ganil acquisition)
	-from networks (so GRU is a spy of networks)


---------------

### GRUCore

 Same as GRU , but have no C++ interpretor. The commands ares sent via Soap protocole using GRUC tools.
 
---------------

### GRUC

 Client tool to send commands to GRUCore
 
  
---------------

### Blocktest


Blocktest is a part of GRU.

Blocktest is a utility witch verify runs on a disk. It replaces the old "Ganil_tape"

Help : Blocktest -h

--------------


### vigru 

vigru is a visualisation of histogram of Ganil Acquisition and GRU.
vigru gets histogram via network.
vigru can also load root histograms or Ganil Acquisition histogram


Usage : vigru
        (and began to use menu  : setup->source )


---------- 
 
### Licence                                                                       

  This program is free software; you can redistribute it and/or modify 
  
   it under the terms of the GNU General Public License as published by 

   the Free Software Foundation; either version 2 of the License, or
  
   (at your option) any later version.
 
----------    
 
### Installation 

#### Requirement

- Centos7 (tested on , but other linux can be used)
- ZMQ libraries
- tinyxml
- cmake
- gsoap
- root 

- and also but not absolute necessary
 doxygen 
 
#### Install

- to build (options are in brackets):
- cd build ( if build isn't created , make it! (in this case, beside source directory))
- cmake3   -DCMAKE_INSTALL_PREFIX=../install ../sources/  -DNET_LIB=YES [-DMFMDIR=/whereMFMlibIs/] [-DDEBUG=YES] [-DNO_GSOAP=YES]
- make
- make install


#### Options :

- if $MFMDIR is not defined or if we want to change MFMlib   add option  -DMFMDIR=/whereMFMlibIs/
- add option "-DDEBUG=YES"  to activate debug compilation mode 
- if you want a GRU without network library (for exemple to analyse only files and no oneline data) change   "-DNET_LIB=YES"  by "-DNET_LIB=NO"
- to compile without gsoap and devalidate all devices using this protocol add option -DNO_GSOAP=YES , in this case GetMFM must be compiled with this same option ( -DNO_GSOAP=YES )
--------- 

### Example of quick installation 


Quick install GRU 


1) In a empty folder named "MYGRU", get MFMlib, GRU and GetMFM  packages from https://gitlab.in2p3.fr/Ganil-acq .  Unzip, gunzip,untar.... the packages

mkdir  INSTALL_DIR
setenv INSTALL_DIR INSTALL_DIR

2) MFMlib

- cd MFMlib
- mkdir -p build
- cd build
- cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}/MFMlib ../sources/
- make install
- cd ../..
  
3) GetMFM

- cd GetMFM
- cd FDTlib
- make 
- cd ..
- cd gsoapGECO
- mkdir -p build
- cd build
- cmake -DCMAKE_INSTALL_PREFIX=../  ../sources/
- make install
- cd ../..
  
- mkdir build 
- cd build
- cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}/GetMFM/ ../sources/ -DMYMFMLIBDIR=${INSTALL_DIR}/MFMlib
- make install
  
4) GRU

- cd gru
- cd gsoap
- mkdir build
- cd build
- cmake -DCMAKE_INSTALL_PREFIX=../ ../sources/
- make install
- cd ../../

- cd gsoapSC
- mkdir build
- cd build
- cmake -DCMAKE_INSTALL_PREFIX=../ ../sources/
- make install
- cd ../../ 

- mkdir build
- cd build
- cmake3 -DNET_LIB=YES -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}/GRU/ ../sources/ -DMFMDIR=${INSTALL_DIR}/MFMlib/ -DGETMFMDIR=${INSTALL_DIR}/GetMFM/
- make -j8
- make install
- cd ../

- cp build/*.pcm  ${INSTALL_DIR}/GRU/lib (if ROOT version 6)
- cp build/*.pcm  ${INSTALL_DIR}/GRU/bin (if ROOT version 6)


- add your $PATH with  ${INSTALL_DIR}/GRU/bin, ${INSTALL_DIR}/MFMlib/bin and  ${INSTALL_DIR}/GetMFM/bin  directories
- add your ${INSTALL_DIR}/GRU/lib, ${INSTALL_DIR}/MFMlib/lib and  ${INSTALL_DIR}/GetMFM/lib directories
  
 
 try "GRU"
 
 --------- 
 
### Class Information
   
 https://ganil-acq.pages.in2p3.fr/Analysis/gru/
 
 ---------
