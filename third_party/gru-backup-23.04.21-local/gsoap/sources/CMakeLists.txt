# Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 2.8.12)
project(GruSoap)

if(DEBUG)
   set(CMAKE_BUILD_TYPE Debug)
   message(STATUS "CMake in DEBUG mode")
else()
   set(CMAKE_BUILD_TYPE Release)
   message(STATUS "CMake in RELEASE mode")
endif()

configure_file(grugsoap.in grugsoap.in.h)

#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -DWITH_NOANAMESPACES  -fPIC -m64")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -fPIC ")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -g -DWITH_NOANAMESPACES -m64" )
message(STATUS "-- CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")


# force -fPIC option n even in .a library
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

#get_target_property(TEMP GRUC COMPILE_FLAGS)
#message(STATUS "-- property = ${TEMP}")

find_library(Gsoap_LIBRARIES gsoap++)
if(Gsoap_LIBRARIES)
   message(STATUS "-- found gsoap ${Gsoap_LIBRARIES}")
else()
   message(WARNING "-- Gsoap_LIBRARIES not found")
endif()

find_library(Pthread_LIBRARIES pthread)

if(Pthread_LIBRARIES)
   message(STATUS "-- found Pthread ${Pthread_LIBRARIES}")
else()
   message(WARNING "-- Pthread_LIBRARIES not found")
endif()

if(CMAKE_INSTALL_PREFIX)
   message (STATUS "-- found ${CMAKE_INSTALL_PREFIX}")
else()
   message (WARNING "-- CMAKE_INSTALL_PREFIX not found ")
endif()

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

message(STATUS  "-- list of include dir : ${INCLUDE_DIRECTORIES} -- ${CMAKE_SOURCE_DIR} -- ${CMAKE_BINARY_DIR}")

# Créé des variables avec les fichiers à compiler

set(SRCSlibServer GruSoapServer.cpp GruSoapMyServer.cpp Utilities.cpp GSoapErrorCode.cpp  GruSoapC.cpp)
set(SRCSlibClient GruSoapClient.cpp GSoapErrorCode.cpp GruSoapC.cpp GruInterfaceSoap.cpp)	

set(HEADERSlibOriginal libGruSoap_service.h GruInterfaceSoap.h  GruSoapMyServer.h  GSoapErrorCode.h GSoapErrorCodef.h Main.h  Utilities.h)
set(HEADERSlibGenerated	 ${CMAKE_BINARY_DIR}/GruSoapH.h  ${CMAKE_BINARY_DIR}/GruSoapStub.h   ${CMAKE_BINARY_DIR}/ns.xsd  ${CMAKE_BINARY_DIR}/GruSoap.wsdl  ${CMAKE_BINARY_DIR}/GruSoap.nsmap )
set(HEADERSlib	${HEADERSlibOriginal} ${HEADERSlibGenerated})
set(SRCSexeServer
	${SRCSlibServer}
	Main.cpp
	)
		
set(SRCSexeClient
	GruSoapMyClient.cpp
	${SRCSlibClient}
	)
	
set(HEADERSexe
	${HEADERSlib}
	)

add_library(GruSoap STATIC GruSoapC.cpp GruSoapServer.cpp  GruSoapClient.cpp GruInterfaceSoap.cpp GSoapErrorCode.cpp Utilities.cpp ${HEADERSlib})

add_executable(GruSoapServer ${SRCSexeServer} ${HEADERSexe})
target_link_libraries(GruSoapServer ${Gsoap_LIBRARIES} ${Pthread_LIBRARIES})

add_executable(GRUC GruSoapClient.cpp GruSoapMyClient.cpp GruSoapC.cpp GSoapErrorCode.cpp Utilities.cpp ${HEADERSexe})
target_link_libraries(GRUC ${Gsoap_LIBRARIES} )

target_include_directories(GruSoapServer PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )
target_include_directories(GRUC          PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

	

add_custom_command(OUTPUT GruSoapC.cpp GruSoapH.h GruSoapStub.h GruSoapClient.cpp GruSoapServer.cpp  ns.xsd GruSoap.wsdl GruSoap.nsmap 
	DEPENDS libGruSoap_service.h GruSoapMyServer.cpp GruSoapMyClient.cpp
	COMMAND soapcpp2 -x -L -n -pGruSoap ${CMAKE_SOURCE_DIR}/libGruSoap_service.h
	COMMENT "Generating code with soapcpp2 from libGruSoap_service."
	)

install(TARGETS GruSoapServer GRUC GruSoap DESTINATION bin
		LIBRARY   DESTINATION lib
	        ARCHIVE   DESTINATION lib
	        INCLUDES  DESTINATION include
		COMPONENT libraries)
		
install(FILES ${HEADERSlibOriginal}  DESTINATION include)
install(FILES ${HEADERSlibGenerated} GruSoap2.nsmap DESTINATION include)
