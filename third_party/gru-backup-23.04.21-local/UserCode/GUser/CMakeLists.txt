# Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 2.8.12)
project(GUser)
set(CLASSNAME   GUser)
set(CMAKE_BUILD_TYPE Release)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})


set(GRUINCLUDE $ENV{GRUDIR}/include)
set(MFMINCLUDE $ENV{GRUDIR}/MFMlib)
set(VIGRUINCLUDE $ENV{GRUDIR}/vigru)


find_library(GRU_LIBRARIES GRU HINTS $ENV{GRUDIR}/lib/ )
	if(GRU_LIBRARIES)
   	message(STATUS "-- found GRU_LIBRARIES  ${GRU_LIBRARIES}")
	else()
   	message(FATAL_ERROR "-- GRU_LIBRARIES not found")
   	exit()
endif()




#find_package(ROOT REQUIRED)
find_package(ROOT 
	REQUIRED COMPONENTS Core  RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread
OPTIONAL_COMPONENTS  Cint)

message(STATUS "-- CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")

if(CMAKE_INSTALL_PREFIX)
   message (STATUS "-- found ${CMAKE_INSTALL_PREFIX}")
else()
   message (WARNING "-- CMAKE_INSTALL_PREFIX not found ")
endif()


include(${ROOT_USE_FILE})
message ( "-- $ROOT_USE_FILE = ${ROOT_USE_FILE}")

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}  $ENV{ROOTSYS}/include/ ${GRUINCLUDE} ${MFMINCLUDE} ${VIGRUINCLUDE})


# Créé des variables avec les fichiers à compiler
set(SRCSlib       ${CMAKE_BINARY_DIR}/${CLASSNAME}Dict   ${CMAKE_SOURCE_DIR}/${CLASSNAME}.C)
set(HEADERSlib	  ${CMAKE_BINARY_DIR}/${CLASSNAME}Dict.h ${CMAKE_SOURCE_DIR}/${CLASSNAME}.h)

ROOT_GENERATE_DICTIONARY(${CLASSNAME}Dict  ${CMAKE_SOURCE_DIR}/${CLASSNAME}.h LINKDEF ${CMAKE_SOURCE_DIR}/${CLASSNAME}_linkdef.h OPTIONS -p)


add_library(${CLASSNAME} SHARED  ${SRCSlib}  ${HEADERSlib})
#add_library(${CLASSNAME}a STATIC  ${SRCSlib}  ${HEADERSlib})
target_link_libraries(${CLASSNAME} ${ROOT_LIBRARIES} ${GRU_LIBRARIES})
set_target_properties(${CLASSNAME} PROPERTIES PREFIX "")
set_target_properties(${CLASSNAME} PROPERTIES OUTPUT_NAME ${CLASSNAME}_C)

target_include_directories(${CLASSNAME} PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}  $ENV{ROOTSYS}/include )

# messages
MESSAGE(STATUS "<<< Configuration >>>
Project:	${CMAKE_PROJECT_NAME}
Build type	${CMAKE_BUILD_TYPE}
Install path	${CMAKE_INSTALL_PREFIX}
Compiler:
C		${CMAKE_C_COMPILER}
C++		${CMAKE_CXX_COMPILER}
Linker:
Ld		${CMAKE_LINKER}
Compiler flags:
C		${CMAKE_C_FLAGS}
C++		${CMAKE_CXX_FLAGS}
Linker flags:
Executable      ${CMAKE_EXE_LINKER_FLAGS}
Module          ${CMAKE_MODULE_LINKER_FLAGS}
Shared          ${CMAKE_SHARED_LINKER_FLAGS}\n")


install(TARGETS ${CLASSNAME}     DESTINATION bin
		LIBRARY   DESTINATION lib
	        ARCHIVE   DESTINATION lib
	        INCLUDES  DESTINATION include
		COMPONENT libraries)
		
install(FILES ${HEADERSlib}  DESTINATION include)

