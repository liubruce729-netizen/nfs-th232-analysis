# Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.12)
	      
#-- path to CMake modules
list(APPEND CMAKE_MODULE_PATH
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMakeListsUtilities
    )

#-- Define project version here (and here only)!
set(MFM_VERSION_STRING "25.11.07")

include(ParseVersionString)

#-- name and version of project
project(MFMlib
        VERSION ${MFM_VERSION}
       )

#-- configure C header with MFM_VERSION macro
configure_file(${PROJECT_SOURCE_DIR}/Version.h.in
               ${CMAKE_BINARY_DIR}/Version.h
	      )

#-- set correct RPATH in all libraries and executables
set(CMAKE_INSTALL_PKGLIBDIR lib)
include(StandardRpathHandling)
   
# Prepare test sets
include (CTest )
ENABLE_TESTING()
ADD_TEST ( MFMlibrary  MFMtest.exe "-test -v=6")


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

DATE(BUILD_MFM_DATE)
TIME(BUILD_MFM_TIME)

message(STATUS "Compilation date = ${BUILD_MFM_DATE} ${BUILD_MFM_TIME}")

configure_file(MFMlib.in MFMlib.in.h)

# force -fPIC option n even in .a library
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

if(NO_MFMNXML)
message(STATUS "compilation without tinyxml")

#target_compile_options(MFMlib NO_MFMNXML)
#target_compile_options(MFMliba NO_MFMNXML)
#target_compile_options(MFMtest.exe NO_MFMNXML)

add_definitions(-DNO_MFMXML)

else()
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
endif()

if(CMAKE_INSTALL_PREFIX)
   message(STATUS "-- found CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
else()
   message(WARNING "-- CMAKE_INSTALL_PREFIX not found ")
endif()

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/GanTape/)
#message("-- CMAKE_CXX_FLAGS_RELEASE =  ${CMAKE_CXX_FLAGS_RELEASE}")
#message("-- CMAKE_CXX_FLAGS_DEBUG =    ${CMAKE_CXX_FLAGS_DEBUG}")

# Créé des variables avec les fichiers à compiler
set(SRCSlib
DataGenParameters.cc  MFMBlobFrame.cc      MFMEbyedatFrame.cc   MFMNedaFrame.cc     MFMS3RuthFrame.cc      MFMXmlDataDescriptionFrame.cc
DataParameters.cc     MFMBoxDiagFrame.cc   MFMExogamFrame.cc    MFMNumExoFrame.cc   MFMS3SynchroFrame.cc   MFMXmlFileHeaderFrame.cc
DataPar.cc            MFMChimeraFrame.cc   MFMOscilloFrame.cc   MFMScalerDataFrame.cc
DataScal.cc           MFMCoboFrame.cc      MFMHelloFrame.cc     MFMRibfFrame.cc     DataScalers.cc
MFMCoboTopoFrame.cc   MFMMergeFrame.cc     MFMS3AlphaFrame.cc   MFMVamosICFrame.cc  MFMParisFrame.cc 
MError.cc             MFMCommonFrame.cc    MFMMutantFrame.cc    MFMS3BaF2Frame.cc   MFMVamosPDFrame.cc    MFMReaGenericFrame.cc
MFMBasicFrame.cc      MFMDiamantFrame.cc   MFMNedaCompFrame.cc  MFMS3eGUNFrame.cc   MFMVamosTACFrame.cc   MFMReaTraceFrame.cc 
MFMSiriusFrame.cc     MFMS3DeflectorFrame.cc  MFMFasterFrame.cc MFMFasterDTSFrame.cc MFMMesytecFrame.cc   MFMFaziaFrame.cc  
MFMGeneral.cc       GanTape/acq_ebyedat_get_next_event.cxx  GanTape/gan_acq_swap_buf.c 
)
	
set(HEADERSlib ${CMAKE_BINARY_DIR}/Version.h
Cobo.h               MError.h           MFMCoboFrame.h      MFMHelloFrame.h     MFMNumExoFrame.h   MFMS3RuthFrame.h      MFMVamosTACFrame.h
DataGenParameters.h  MFMAllFrames.h     MFMCoboTopoFrame.h  MFMMergeFrame.h     MFMOscilloFrame.h  MFMS3SynchroFrame.h   MFMXmlDataDescriptionFrame.h
DataParameters.h     MFMBasicFrame.h    MFMCommonFrame.h    MFMMutantFrame.h    MFMRibfFrame.h     MFMScalerDataFrame.h  MFMXmlFileHeaderFrame.h
DataPar.h            MFMBlobFrame.h     MFMDiamantFrame.h   MFMNedaCompFrame.h  MFMS3AlphaFrame.h  MFMTypes.h            XmlTags.h
DataScalers.h        MFMBoxDiagFrame.h  MFMEbyedatFrame.h   MFMNedaFrame.h      MFMS3BaF2Frame.h   MFMVamosICFrame.h     MFMReaTraceFrame.h
DataScal.h           MFMChimeraFrame.h  MFMExogamFrame.h    MFMS3eGUNFrame.h    MFMVamosPDFrame.h  MFMReaGenericFrame.h  MFMSiriusFrame.h  
MFMParisFrame.h      MFMS3DeflectorFrame.h                  MFMFasterFrame.h    MFMFasterDTSFrame.h   MFMMesytecFrame.h   MFMFaziaFrame.h      MFMGeneral.h
CUtilities.h         ArgInterpretor.h   MFMFewDefines.h
GanTape/GEN_TYPE.H   GanTape/gan_acq_buf.h   GanTape/acq_ebyedat_get_next_event.h    GanTape/STR_EVT.H   GanTape/gan_tape_erreur.h  GanTape/gan_acq_swap_buf.h
)

add_library(MFMa STATIC ${SRCSlib} ${HEADERSlib})
set_target_properties(MFMa PROPERTIES OUTPUT_NAME MFM)
target_link_libraries(MFMa)

add_library(MFM SHARED    ${SRCSlib} ${HEADERSlib})
target_link_libraries(MFM  ${TinyXML_LIBRARIES})

add_executable(MFMtest.exe MFMtest.cc ${HEADERSlib})
target_link_libraries(MFMtest.exe MFMa ${TinyXML_LIBRARIES} )


install(TARGETS MFMtest.exe MFMa MFM
                EXPORT    ${PROJECT_NAME}Exports 
                RUNTIME   DESTINATION bin
		          LIBRARY   DESTINATION lib
	             ARCHIVE   DESTINATION lib
	             INCLUDES  DESTINATION include
		 )

install(FILES ${HEADERSlib} DESTINATION include)

install(FILES ${HEADERSlib} DESTINATION src_bis)
install(FILES ${SRCSlib} MFMtest.cc Doxyfile.MFM DESTINATION src_bis)
install(FILES ${CMAKE_SOURCE_DIR}/../README.md  DESTINATION src_bis/..)

#-- set up CMake installation for others to find us with find_package(MFMlib)
set(CMAKE_INSTALL_PKGCONFIGDIR lib/cmake/${PROJECT_NAME})
set(NO_PROJECT_TARGETS_NAMESPACE TRUE)
include(InstallConfigPackageVersion)
