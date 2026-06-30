Author : Luc Legeard

legeard @ ganil.fr

---------------
## Intoduction


This package MFMlib is a complete  C++ library to decode or encode MFM frame

It generates a libMFM.a which can be linked to your own code.

A usefull executable "MFMtest.exe" can tests you MFM run files or generate MFM frames in a file with random data.

-------------------------------------------------------------------------------------------------

## Build and Installation (options are in brackets)

cd build ( if build isn't created , make it! (in this case, beside source directory))

cmake -DCMAKE_INSTALL_PREFIX=../ ../sources/  [-DDEBUG=YES] [-DNO_MFMNXML=YES] [-DMYTINYXMLDIR=/the/directory/where/libtinyxml.so/is/] 

make

make install-DDEBUG=YES


### Build Options : 


- add "-DDEBUG=YES" to activate debug compilation mode
- If tinyxml library is not installed or not necessary MFM can be compiled without tinyxml add option "-DNO_MFMNXML=YES" 
- to add your own tynixml library 
download tynixml from  http://www.grinninglizard.com/tinyxml/.
In tinyxml directory,
edit Makefile and for adding -fPIC option change lines 


Rules for compiling source files to object files :
~~~
%.o : %.cpp
	${CXX}  -c ${CXXFLAGS} ${INCS} $< -o $@

   %.o : %.c
	${CC}  -c ${CFLAGS} ${INCS} $< -o $@
~~~

to

Rules for compiling source files to object files
~~~
%.o : %.cpp
	${CXX} -fPIC -c ${CXXFLAGS} ${INCS} $< -o $@

   %.o : %.c
	${CC} -fPIC -c ${CFLAGS} ${INCS} $< -o $@

lib:	tinyxml.o tinyxmlparser.o tinyxmlerror.o tinystr.o
	${CC} -shared -fPIC ${CFLAGS} ${INCS} tinyxml.o tinyxmlparser.o tinyxmlerror.o tinystr.o -o libtinyxml.so
~~~


compile with a "make lib" tynixml  
to produce tynixml.so

The new cmake command become
cmake -DMYTINYXMLDIR=/the/directory/where/libtinyxml.so/is/  -DCMAKE_INSTALL_PREFIX=../ ../sources/

------------------------------------------------------------------------------------------------- 
## Usage and Documentation 

 
 MFMtest.exe --help

 In a tuto directory, you have a example of MFMlib usage for you own C++ code
 
 For code documentation , browse  https://ganil-acq.pages.in2p3.fr/Analysis/MFMlib/


 
------------------------------------------------------------------------------------------------- 
         




