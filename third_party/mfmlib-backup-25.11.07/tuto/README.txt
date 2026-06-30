


Example of using MFMlib in a pure C++ software.

It read a file "mfmfile.dat"  and read few data inside.

Generate exectable with 

 g++  -c test.cc -I ../include/ -o test.o -std=c++11;
 g++   test.o  ../lib/libMFM.a  -L /usr/lib/ -L /usr/lib64/   -lm -lc -o test.exe
 
 
