
-------------------------------------------------------------------------------------------
Informations :

These files allow to generate Soap server and Clients to test server.
GRU do not use gsoap . GRUCore (and ex Calimero) use gsoap to communicate with GECO (.
GRUCore have a server saop and wait command from GECO ( which is client).

-------------------------------------------------------------------------------------------
To build (options are in brackets) :

cmake -DCMAKE_INSTALL_PREFIX=../ ../sources/  [-DDEBUG=YES] 
make

copy GruSoap.nsmap in GruSoap2.nsmap ( in sources directory)  and in GruSoap2.nsmap change   "Namespace GruSoap_namespaces[]"  in  "Namespace GruSoap_namespaces2[]" 
make install

add option "-DDEBUG=YES"  to activate debug compilation mode 

-------------------------------------------------------------------------------------------

Test:
You can launch the stansalone server "GruSoapServer".
and test it with client GRUC

Ex of command :
GRUC localhost:6603 GRU TEST 3
GRUC localhost:6603 GRU GET SPECTRA LIST
GRUC localhost:6603 GRU GET SPECTRUM   SoapSpectra16384 MyFamilySoap
GRUC localhost:6603 GRU GET INFO

-------------------------------------------------------------------------------------------
