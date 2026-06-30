# Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 2.8.12)
project(GUser)
set(CLASSNAME   GUser)
set(CMAKE_BUILD_TYPE Release)


message(STATUS "-- CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
get_target_property(TEMP GRUC COMPILE_FLAGS)
message(STATUS "-- property = ${TEMP}")


if(CMAKE_INSTALL_PREFIX)
   message (STATUS "-- found ${CMAKE_INSTALL_PREFIX}")
else()
   message (WARNING "-- CMAKE_INSTALL_PREFIX not found ")
endif()

find_library(MFM_LIBRARIES libMFM.a  HINTS ${MFMDIR}/lib/ )
if (MFM_LIBRARIES)
	message(STATUS "-- found  ${MFM_LIBRARIES}")
else()
	set(MFMDIR     $ENV{GRUDIR}/../MFMlib/)
	find_library(MFM_LIBRARIES libMFM.a  HINTS ${MFMDIR}/lib/ )
	if (MFM_LIBRARIES)
		message(STATUS "-- found  ${MFM_LIBRARIES}")
	else()
		message(FATAL_ERROR "-- MFM_LIBRARIES not found in ${MFMDIR}/lib")
	endif()
endif()


set(GRUINCLUDE  $ENV{GRUDIR}/include/) 
set(VIGRUDIR    $ENV{GRUDIR}/vigru/)
set(GSOAPDIR    $ENV{GRUDIR}/gsoap/)
set(MFMINCLUDE  ${MFMDIR}/include/)

Set (GRUFLAGS     -I${GRUINCLUDE}  -I${MFMINCLUDE}  -I${VIGRUDIR}   -I${GSOAPDIR})
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${GRUINCLUDE} ${MFMINCLUDE} $ENV{ROOTSYS}/include )

# Créé des variables avec les fichiers à compiler

set(SRCSlib ${CLASSNAME}Dict.C ${CLASSNAME}.C)

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
	set (ROOTCINTCMD1 rootcint -f ${CLASSNAME}Dict.C -c -p ${GRUFLAGS}  ${CMAKE_SOURCE_DIR}/${CLASSNAME}.h   ${CMAKE_SOURCE_DIR}/${CLASSNAME}_linkdef.h)
	set (ROOTCINTCMD2 "")
else()
	set (ROOTCINTCMD1 rootcint -f ${CLASSNAME}Dict.C -c -p -rml ${CLASSNAME}_C.so -rmf ${CLASSNAME}_C.rootmap ${GRUFLAGS} ${CMAKE_SOURCE_DIR}/${CLASSNAME}.h ${CLASSNAME}_linkdef.h )
	set (ROOTCINTCMD2 rootcint -f ${CLASSNAME}Dict.C -c -p ${GRUFLAGS}  ${CMAKE_SOURCE_DIR}/${CLASSNAME}.h   ${CLASSNAME}_linkdef.h)
endif()

set(HEADERSlib	  ${CMAKE_BINARY_DIR}/${CLASSNAME}Dict.h ${HEADERSlib} ${CMAKE_SOURCE_DIR}/${CLASSNAME}.h )
#message("command rootcint1 : ${ROOTCINTCMD1}")	
#message("command rootcint2 : ${ROOTCINTCMD2}")	

add_library(${CLASSNAME} SHARED  ${SRCSlib} )
set_target_properties(${CLASSNAME} PROPERTIES PREFIX "")
set_target_properties(${CLASSNAME} PROPERTIES OUTPUT_NAME ${CLASSNAME}_C)

add_custom_command(OUTPUT ${CLASSNAME}Dict.C  ${CLASSNAME}Dict.h ${CLASSNAME}_C.d
	DEPENDS ${GRUINCLUDE}/General.h ${GRUINCLUDE}/GAcq.h ${GRUINCLUDE}/GDevice.h ${CMAKE_SOURCE_DIR}/${CLASSNAME}.h ${CMAKE_SOURCE_DIR}/${CLASSNAME}_linkdef.h
	COMMAND ${ROOTCINTCMD1}
	COMMAND ${ROOTCINTCMD2}
	COMMENT "Generating dictionnary code with rootcint for ${CLASSNAME} with root ${ROOTVERSION}"
	)


install(TARGETS ${CLASSNAME}     DESTINATION bin
		LIBRARY   DESTINATION lib
	        ARCHIVE   DESTINATION lib
	        INCLUDES  DESTINATION include
		COMPONENT libraries)
		
install(FILES ${HEADERSlib}  DESTINATION include)

