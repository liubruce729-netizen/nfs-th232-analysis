
cmake_minimum_required(VERSION 2.8.12)
project(SCSoap)

if(DEBUG)
   set(CMAKE_BUILD_TYPE Debug)
   message(STATUS "CMake in DEBUG mode")
else()
   set(CMAKE_BUILD_TYPE Release)
   message(STATUS "CMake in RELEASE mode")
endif()


configure_file(SCgsoap.in SCgsoap.in.h)
SET(CMAKE_C_FLAGS  "${CMAKE_CXX_FLAGS}  -DWITH_NOANAMESPACES  -fPIC -m64")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}  -DWITH_NOANAMESPACES  -fPIC -m64")
#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}   -fPIC ")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS}  -DWITH_NOANAMESPACES -fPIC -m64" )
message(STATUS "-- CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")


find_library(Gsoap_LIBRARIES gsoap++)
if(Gsoap_LIBRARIES)
   message(STATUS "-- found gsoap ${Gsoap_LIBRARIES}")
else()
   message(WARNING "-- Gsoap_LIBRARIES not found")
endif()

find_library(Pthread_LIBRARIES pthread)

if(Pthread_LIBRARIES)
   message(STATUS "-- found  Pthread ${Pthread_LIBRARIES}")
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

set(SRSClibServer Utilities.cpp SCSoapMyServer.cpp SCSoapServer.cpp SCSoapC.cpp  )
set(SRSClibClient Utilities.cpp SCInterfaceSoap.cpp SCSoapC.cpp SCSoapClient.cpp )	

set(HEADERSlibOriginal   GSoapErrorCode.h Utilities.h sc_soap_services.hh SCInterfaceSoap.h)
set(HEADERSlibGenerated	 ${CMAKE_BINARY_DIR}/SCSoap.nsmap  ${CMAKE_BINARY_DIR}/SCSoapH.h  )

set(HEADERSlib	${HEADERSlibOriginal} ${HEADERSlibGenerated})


add_library(SCSoap STATIC SCInterfaceSoap.cpp Utilities.cpp SCSoapC.cpp SCSoapServer.cpp  SCSoapClient.cpp  ${HEADERSlib})	
add_library(SCClientSoap STATIC  SCSoapClientLib.cpp Utilities.cpp  SCInterfaceSoap.cpp ${HEADERSlib})

add_executable(SCClient  SCSoapClient.cpp SCSoapMyClient.cpp SCSoapC.cpp Utilities.cpp ${HEADERlib})
target_link_libraries(SCClient  ${Gsoap_LIBRARIES} )
target_include_directories(SCClient      PUBLIC  ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )


add_executable(SCSoapServer SCSoapMyServer.cpp SCSoapC.cpp SCSoapServer.cpp Utilities.cpp ${HEADERLib})
target_link_libraries(SCSoapServer ${Gsoap_LIBRARIES} ${Pthread_LIBRARIES})
target_include_directories(SCSoapServer  PUBLIC  ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )
	
	
add_custom_command(OUTPUT sc.wsdl SCSoap.nsmap sc.xsd SCSoapClientLib.cpp SCSoap.wsdl SCSoapscProxy.cpp SCSoapscService.cpp  soapClientLib.cpp soapServer.cpp soapServerLib.cpp sc_soap_services.hh SCSoapH.h SCSoapC.cpp SCSoapClient.cpp SCSoapServer.cpp 
	DEPENDS sc_soap_services.hh
	COMMAND soapcpp2   -n -pSCSoap  ${CMAKE_SOURCE_DIR}/sc_soap_services.hh
	COMMENT "Generating code with soapcpp2 from sc_soap_services."
	)	


install(TARGETS SCSoapServer SCClient SCSoap SCClientSoap DESTINATION bin
		LIBRARY   DESTINATION lib
	        ARCHIVE   DESTINATION lib
	        INCLUDES  DESTINATION include
		COMPONENT libraries)
		
install(FILES ${HEADERSlibOriginal}  DESTINATION include)
install(FILES ${HEADERSlibGenerated} DESTINATION include)
