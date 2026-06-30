# Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 2.8.12)

project(GRUbase)
enable_language(CXX)

if(DEBUG)
   set(CMAKE_BUILD_TYPE Debug)
   message(STATUS "CMake in DEBUG mode")
else()
   set(CMAKE_BUILD_TYPE Release)
   message(STATUS "CMake in RELEASE mode")
endif()


macro(DATE RESULT)
	if(UNIX)
		execute_process(COMMAND "date" "+%y.%m.%d" OUTPUT_VARIABLE ${RESULT})
		string(REGEX REPLACE "\n" "" ${RESULT} ${${RESULT}})
	else()
		message(SEND_ERROR "Unable to detect date")
		SET(${RESULT} 00.00.00)
	endif()
endmacro()

macro(TIME RESULT)
	if(UNIX)
		execute_process(COMMAND "date" "+%H:%M:%S" OUTPUT_VARIABLE ${RESULT})
		string(REGEX REPLACE "\n" "" ${RESULT} ${${RESULT}})
	else()
		message(SEND_ERROR "Unable to detect time")
		set(${RESULT} UNKNOWN)
	endif()
endmacro()

DATE(BUILD_DATE)
TIME(BUILD_TIME)

message(STATUS "Compilation date = ${BUILD_DATE} ${BUILD_TIME}")

configure_file(GRUconfig.in GRUconfig.in.h)

set(CLASSNAME     GRU)
set(CLASSNAMEVG   Vigru)
set(CLASSNAMECore ${CLASSNAME}Core)
set(CLASSNAMEBT   Blocktest)
set(GETMFMDIR $ENV{GETMFMDIR})
set(FDTDIR ${GETMFMDIR}/lib/)

set(GUSERDIR    ${CMAKE_SOURCE_DIR}/../UserCode/GUser/)
set(GUSERMFMDIR ${CMAKE_SOURCE_DIR}/../UserCode/GUserMFM/)
set(GRU_DICTIONARY   yes)


## look for ROOT version and apply differences
message("Looking for Root version:")
set(ROOT_SEARCHPATH 
    $ENV{ROOTSYS}/bin 
    )
set(myfirst 0)
set(mylength 2)

find_program(ROOT_CONFIG NAMES root-config PATHS ${ROOT_SEARCHPATH} )
if(ROOT_CONFIG MATCHES "NOTFOUND")
  message(FATAL_ERROR "ROOT is not found, check your installation")
endif()

execute_process (COMMAND  ${ROOT_CONFIG} --version 
		 OUTPUT_VARIABLE ROOTVERSION)
message("Root version:${ROOTVERSION}")		 
string(SUBSTRING "${ROOTVERSION}"  ${myfirst}  ${mylength}   ROOTVERSION2)
	

if("${ROOTVERSION2}" STREQUAL "5.")
        set (INTREPRETOR_LIB Cint)
        Set (DICT_INC ${CMAKE_BINARY_DIR}/${CLASSNAME}Dict.h)
else()
	set (INTREPRETOR_LIB Cling)
        Set (DICT_INC "")
endif()

if(NO_GSOAP)
else()
   add_definitions(-DNO_GSOAP)
   set(GRUSOAPDIR  ${CMAKE_SOURCE_DIR}/../gsoap/)
   set(GSCSOAPDIR  ${CMAKE_SOURCE_DIR}/../gsoapSC/)
endif()

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})


find_package(ROOT 
	REQUIRED COMPONENTS Core  ${INTREPRETOR_LIB} RIO Net Hist 
	Graf Graf3d Gpad   Tree TreePlayer Rint Postscript Matrix 
	Physics MathCore Thread MathMore  XMLIO XMLParser 
	GuiHtml Spectrum Gui FFTW)

message ( "-- $ROOT_USE_FILE = ${ROOT_USE_FILE}")
include(${ROOT_USE_FILE})

if(CMAKE_INSTALL_PREFIX)
   message (STATUS "-- found ${CMAKE_INSTALL_PREFIX}")
else()
   message (WARNING "-- CMAKE_INSTALL_PREFIX not found ")
endif()

## --- if  cmake Option -DNET_LIB=YES  then we compile with GANIL networks library 
## if you change this variable do a complete compilation and apply a clean



if(NET_LIB)
   set(NetOption "-DNET_LIB") 
   message (STATUS "-- ${NetOption} selected ")
   add_definitions(-DNET_LIB)

if(NO_GSOAP)
   message(STATUS "compilation without GSOAP")
   add_definitions(-DNO_GSOAP)
   
else()     
   find_library(GSOAP_LIB gsoap++)
   if (GSOAP_LIB)
	message(STATUS "-- found gsoap  ${GSOAP_LIB}")
   else()
	message(FATAL_ERROR "-- GSOAP_LIB not found ")
   endif() 

 
   find_library(GRUSOAP_LIB GruSoap   HINTS ${GRUSOAPDIR}/lib/)
   if (GRUSOAP_LIB)
	message(STATUS "-- found GruSoap  ${GRUSOAP_LIB}")
   else()
	message(FATAL_ERROR "-- GRUSOAP_LIB not found in ${GRUSOAPDIR}/lib/")
   endif() 
   
   find_library(GSCClientSOAP_LIB   SCClientSoap HINTS ${GSCSOAPDIR}/lib/)
   if (GSCClientSOAP_LIB)
	message(STATUS "-- found  ${GSCClientSOAP_LIB}")
   else()
	message(FATAL_ERROR "-- GSCClientSOAP_LIB not found in ${GSCSOAPDIR}/lib/")
   endif()
endif() # if(NO_GSOAP)
   
    find_library(FDT_LIBRARIES fdt HINTS ${FDTDIR})
   if (FDT_LIBRARIES)
	  message(STATUS "-- found  ${FDT_LIBRARIES}")
   else()
	  message(FATAL_ERROR "-- FDT_LIBRARIES not found in ${FDTDIR}")
   endif()

else()
   message (STATUS "-- NET_LIB  not selected ")
   set(NO_GSOAP "YES") 
   add_definitions(-DNO_GSOAP)
   add_definitions(-DNO_FDT)
endif()

# force -fPIC option  even in .a library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# add definition to compiler 
add_definitions(${NetOption} -Wno-overloaded-virtual -Wno-unused-parameter -Wno-extra -fPIC)

set(MFMDIR $ENV{MFMDIR})

find_library(MFM_LIBRARIES libMFM.a  HINTS ${MFMDIR}/lib/ )
if (MFM_LIBRARIES)
	message(STATUS "-- found (1st test)  ${MFM_LIBRARIES}")
else()
	set(MFMDIR $ENV{GRUDIR}/../MFMlib/)
	find_library(MFM_LIBRARIES libMFM.a  HINTS ${MFMDIR}/lib/ )
	if (MFM_LIBRARIES)
		message(STATUS "-- found (2nd test) ${MFM_LIBRARIES}")
	else()
		message(FATAL_ERROR "-- MFM_LIBRARIES not found in ${MFMDIR}/lib")
	endif()
endif()



find_library(GETMFM_LIBRARIES libGetMFMDeviceStatic.a HINTS ${GETMFMDIR}/lib/ )
if (GETMFM_LIBRARIES)
	message(STATUS "-- found (1st test)  ${GETMFM_LIBRARIES}")
else()
	set(GETMFMDIR /home/acqexp/ActorsC/GetMFM/)
	find_library(GETMFM_LIBRARIES libGetMFMDeviceStatic.a  HINTS ${GETMFMDIR}/lib/ )
	if (GETMFM_LIBRARIES)
		message(STATUS "-- found (2nd test) ${GETMFM_LIBRARIES}")
	else()
		message(FATAL_ERROR "-- GETMFM_LIBRARIES not found in ${GETMFMDIR}/lib")
	endif()
endif()


find_library(thread_LIBRARIES pthread)
if(thread_LIBRARIES)	
	message(STATUS "-- found  ${thread_LIBRARIES}")
else()
		message(WARNING "-- thread_LIBRARIES not found")
endif()


if(NO_ZMQ)
   message(STATUS "compilation without ZMQ")
   add_definitions(-DNO_ZMQ)
else()

   find_library(zmq_LIBRARIES zmq)
   if(zmq_LIBRARIES)	
	message(STATUS "-- found  ${zmq_LIBRARIES}")
   else()
	message(WARNING "-- zmq_LIBRARIES not found")
	message(STATUS "compilation without ZMQ")
	add_definitions(-DNO_ZMQ)
   endif()		
endif()

find_library(Z_LIBRARIES z )
if (Z_LIBRARIES)
	message(STATUS "-- found  ${Z_LIBRARIES}")
else()
	message(WARNING "-- Z_LIBRARIES not found")
endif()

find_library(TinyXML_LIBRARIES tinyxml HINTS ${MYTINYXMLDIR} )
	if(TinyXML_LIBRARIES)
   	message(STATUS "-- found local ${TinyXML_LIBRARIES}")
	else()
	find_library(TinyXML_LIBRARIES tinyxml  )
		if (TinyXML_LIBRARIES)
		message(STATUS "-- found  ${TinyXML_LIBRARIES}")
		else()
   		message(WARNING "-- TinyXML_LIBRARIES not found")
   		endif()
	endif()

set (ROOTLIBS ${ROOTLIBS}  ${Html_LIBRARIES} ${Spectrum_LIBRARIES}) # -lHtml -lSpectrum

include_directories(${MFMDIR}/include/ ${GETMFMDIR}/include/ ${GETMFMDIR}/gsoapGECO/include/  ${CMAKE_SOURCE_DIR}  ${CMAKE_SOURCE_DIR}/GanTape2
 ${CMAKE_SOURCE_DIR}/cards/ ${CMAKE_BINARY_DIR} ${GRUSOAPDIR}/include/ ${GSCSOAPDIR}/include/ $ENV{ROOTSYS}/include/  ${CMAKE_CURRENT_SOURCE_DIR}/include ${FDTDIR} )
	 
Set (SRCVIGRUlib Vigru.C GVDialogAbout.C GVDialogSource.C GVMenuBar.C  GVToolBar.C GVToolBarVZ.C GVToolBarVS.C    
		 GVSetRefreshTimer.C GVSetPeakFind.C GVTab.C GVPrintDialog.C   GVClassManual.C GVSpectraInfo.C 
		 GVConsol.C  GVSetPage.C  GVSetStat.C  GVPad.C GVMyQuestion.C  GVSetDuplication.C GVSetReset.C
		 GVSpectraChooser.C  GVSetRangeUser.C GVMeter.C)
		
Set (HEADVIGRUlib Vigru.h GVDialogAbout.h GVDialogSource.h GVMenuBar.h  GVToolBar.h GVToolBarVZ.h GVToolBarVS.h    
		  GVSetRefreshTimer.h GVSetPeakFind.h GVTab.h GVPrintDialog.h   GVClassManual.h GVSpectraInfo.h 
		  GVConsol.h  GVSetPage.h   GVSetStat.h  GVPad.h GVMyQuestion.h  GVSetDuplication.h GVSetReset.h
		  GVSpectraChooser.h GVSetRangeUser.h GVMeter.h)

if(NET_LIB)
Set (SRCNetlib     GNetServerSoap.C GNetClientSoap.C  GNetClientNarval.C )
Set (HEADERSNetlib GNetServerSoap.h GNetClientSoap.h  GNetClientNarval.h )
endif()	
	 
Set (SRClib      GBase.C GAcq.C GAcqNumexo.C GDevice.C GListDevice.C GFile.C GetMFM.C 
		 GTape.C GMFMFile.C GBuffer.C GBufferIn2p3.C GBufferMFM.C GEventBase.C
		 GEvent.C   GEventMFM.C  GScaler.C GScalerEvent.C   GError.C GClientMemory.C
		 GSpectra.C GParaCaliXml.C GSpectrumIdentity.C GSpectraDB.C GCommand.C  
		 GNetServer.C GNetServerRoot.C GNetClient.C GNetClientRoot.C GSubEvent.C 
		 GEtalonnageMatacq.C GEtalonnageMust.C GGeneBuffer.C GTTree.C
		 GGeneBufferActor.C GClientMemoryNarval.C TMayaHisto.C GStateMachine.C 
		 GSort.C MyRun.C   GInterrupt.C  GDemo.C 
		 GVClassManual.C GVBrowser.C GVListTree.C  GVSpectraInfo.C GVClassAuto.C GVClass.C ${SRCNetlib} ${SRCVIGRUlib})
 
Set (HEADERSlib  GVersion.h GBase.h GAcq.h GAcqNumexo.h GDevice.h GListDevice.h GFile.h  GetMFM.h
		 GTape.h GMFMFile.h GBuffer.h GBufferIn2p3.h GBufferMFM.h GEventBase.h
		 GEvent.h   GEventMFM.h  GScaler.h GScalerEvent.h   GError.h GClientMemory.h
		 GSpectra.h GParaCaliXml.h GSpectrumIdentity.h  GSpectraDB.h GCommand.h GTTree.h
		 GNetServer.h GNetServerRoot.h GNetClient.h GNetClientRoot.h GSubEvent.h 
		 GEtalonnageMatacq.h GEtalonnageMust.h GGeneBuffer.h ${CMAKE_BINARY_DIR}/GRUconfig.in.h
		 GGeneBufferActor.h GClientMemoryNarval.h TMayaHisto.h GStateMachine.h 
		 GSort.h MyRun.h   GInterrupt.h  GDemo.h General.h GTtape_erreur.h GanTape2/acq_tcp_parametres_def.h
		 GanTape2/acq_tcp_status_struc.h GanTape2/acq_tcp_struct.h GanTape2/acq_codes_erreur.h 
		 GVClassManual.h  GVBrowser.h  GVListTree.h GVSpectraInfo.h  GVClassAuto.h  GVClass.h ${HEADERSNetlib} ${HEADVIGRUlib})


#source for Blocktest		 
Set (SRClibBT BControl.C  BProgressBar.cpp   CError.cc)
Set (HEADERSlibBT  BProgressBar.h BT.h  CError.h  BControl.h  Binclude.h  ${CLASSNAMEBT}.h  )	
	 
set (HEADERSlib	 ${HEADERSlib}  ${DICT_INC}  ${CMAKE_SOURCE_DIR}/${CLASSNAME}.h)

# Generation of GRU dictionnary
ROOT_GENERATE_DICTIONARY(${CLASSNAME}Dict  ${CMAKE_SOURCE_DIR}/${CLASSNAME}.h   OPTIONS -p               LINKDEF ${CMAKE_SOURCE_DIR}/${CLASSNAME}_linkdef.h  )
#ROOT_GENERATE_DICTIONARY(${CLASSNAME}Dict   ${CMAKE_SOURCE_DIR}/${CLASSNAME}.h   OPTIONS -p -DGRU_DICTIONARY LINKDEF ${CMAKE_SOURCE_DIR}/${CLASSNAME}_linkdef.h  )

# Generation of library libGRU.a
add_library(${CLASSNAME}static STATIC   ${SRClib}  ${HEADERSlib})
set_target_properties(${CLASSNAME}static PROPERTIES OUTPUT_NAME ${CLASSNAME} )
target_link_libraries(${CLASSNAME}static)

# Generation of library libGRU.so
add_library(${CLASSNAME}shared SHARED  ${CMAKE_BINARY_DIR}/${CLASSNAME}Dict  ${HEADERSlibBT})
set_target_properties(${CLASSNAME}shared PROPERTIES OUTPUT_NAME ${CLASSNAME} ) 
target_link_libraries(${CLASSNAME}shared ${CLASSNAME}static ${GRUSOAP_LIB} ${GSCClientSOAP_LIB}  ${ROOT_LIBRARIES} ${Z_LIBRARIES}
   ${GETMFM_LIBRARIES} ${MFM_LIBRARIES} ${GSOAP_LIB} ${zmq_LIBRARIES} ${TinyXML_LIBRARIES} ${FDT_LIBRARIES} ${thread_LIBRARIES} )

# Generation of executable GRU
add_executable(${CLASSNAME}exe  ${CLASSNAME}.C ${CMAKE_BINARY_DIR}/${CLASSNAME}Dict  ${HEADERSlib})
target_link_libraries(${CLASSNAME}exe ${CLASSNAME}static ${GRUSOAP_LIB} ${GSCClientSOAP_LIB}   ${ROOT_LIBRARIES} 
${Z_LIBRARIES} ${GETMFM_LIBRARIES} ${MFM_LIBRARIES} ${zmq_LIBRARIES}   ${TinyXML_LIBRARIES} ${FDT_LIBRARIES} ${thread_LIBRARIES} )
set_target_properties(${CLASSNAME}exe PROPERTIES OUTPUT_NAME ${CLASSNAME} )

# Generation of executable vigru
add_executable(${CLASSNAMEVG}exe  ${CLASSNAMEVG}Main.C ${CMAKE_BINARY_DIR}/${CLASSNAME}Dict ${CLASSNAMEVG}Main.h ${HEADERSlib})
target_link_libraries(${CLASSNAMEVG}exe ${CLASSNAME}static ${GSOAP_LIB} ${GRUSOAP_LIB} ${GSCClientSOAP_LIB} ${ROOT_LIBRARIES}
${Z_LIBRARIES}   ${GETMFM_LIBRARIES} ${MFM_LIBRARIES} ${FDT_LIBRARIES}  ${zmq_LIBRARIES}   ${TinyXML_LIBRARIES} ${thread_LIBRARIES})
set_target_properties(${CLASSNAMEVG}exe PROPERTIES OUTPUT_NAME vigru ) 

# Generation of executable Blocktest
add_executable(${CLASSNAMEBT}  ${BTDIR}${CLASSNAMEBT}.C  ${CMAKE_BINARY_DIR}/${CLASSNAME}Dict  ${BTDIR}${CLASSNAMEBT}.h ${SRClibBT} ${HEADERSlibBT} ${HEADERSlib})
target_link_libraries(${CLASSNAMEBT} ${CLASSNAME}static ${GSOAP_LIB} ${GRUSOAP_LIB} ${GSCClientSOAP_LIB}  ${ROOT_LIBRARIES} ${Z_LIBRARIES} 
 ${GETMFM_LIBRARIES} ${MFM_LIBRARIES} ${FDT_LIBRARIES} ${TinyXML_LIBRARIES} ${zmq_LIBRARIES}  ${thread_LIBRARIES} )
set_target_properties(${CLASSNAMEBT} PROPERTIES OUTPUT_NAME ${CLASSNAMEBT})

if(NET_LIB)
# Generation of executable GRUCore
add_executable(${CLASSNAMECore}.exe  ${CLASSNAMECore}.C ${CMAKE_BINARY_DIR}/${CLASSNAME}Dict ${CLASSNAMECore}.h ${HEADERSlib})
target_link_libraries(${CLASSNAMECore}.exe ${CLASSNAME}static ${GSOAP_LIB} ${GRUSOAP_LIB} ${GSCClientSOAP_LIB} ${ROOT_LIBRARIES} ${Z_LIBRARIES} 
${GETMFM_LIBRARIES} ${MFM_LIBRARIES}  ${TinyXML_LIBRARIES} ${zmq_LIBRARIES} ${FDT_LIBRARIES} ${thread_LIBRARIES})
set_target_properties(${CLASSNAMECore}.exe PROPERTIES OUTPUT_NAME ${CLASSNAMECore}) 
endif()

# Installation of files

if(NET_LIB)
install(TARGETS ${CLASSNAME}static   ${CLASSNAME}shared  ${CLASSNAMECore}.exe ${CLASSNAME}exe ${CLASSNAMEVG}exe ${CLASSNAMEBT} DESTINATION bin
		LIBRARY   DESTINATION lib
	        ARCHIVE   DESTINATION lib
	        INCLUDES  DESTINATION include
		COMPONENT libraries)
else()
install(TARGETS ${CLASSNAME}static   ${CLASSNAME}shared   ${CLASSNAME}exe ${CLASSNAMEVG}exe ${CLASSNAMEBT} DESTINATION bin
		LIBRARY   DESTINATION lib
	        ARCHIVE   DESTINATION lib
	        INCLUDES  DESTINATION include
		COMPONENT libraries)
endif()
		

		
install(FILES ${HEADERSlib}  DESTINATION include)

install(DIRECTORY ${GUSERDIR}  DESTINATION GUser)

install(DIRECTORY ${GUSERMFMDIR}  DESTINATION GUserMFM)

install(FILES ${HEADERSlib} ${SRClib} ${HEADERSNetlib} ${SRCNetlib} ${HEADVIGRUlib} ${SRCVIGRUlib} ${SRClibBT} ${HEADERSlibBT} Doxyfile.GRU DESTINATION src_bis)

install(FILES ${CMAKE_SOURCE_DIR}/../README.md  DESTINATION src_bis/..)


if("${ROOTVERSION2}" STREQUAL "5.")

else()
install(FILES 	${CMAKE_BINARY_DIR}/libGRUDict_rdict.pcm   DESTINATION bin)
install(FILES 	${CMAKE_BINARY_DIR}/libGRUDict_rdict.pcm   DESTINATION lib)
endif()

if(NO_GSOAP)
 
else()  
install(FILES   ${GSCSOAPDIR}/bin/SCClient ${GSCSOAPDIR}/bin/SCSoapServer  DESTINATION bin)
install(FILES   ${GRUSOAPDIR}/bin/GRUC     ${GRUSOAPDIR}/bin/GruSoapServer DESTINATION bin)
endif()











