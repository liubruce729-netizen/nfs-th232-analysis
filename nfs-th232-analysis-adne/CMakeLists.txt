cmake_minimum_required(VERSION 3.10)
project(ADNE)

# Définir les arguments pour GRUSYS et MFMSYS
set(GRUSYS "" CACHE PATH "Chemin vers GRUSYS")
set(MFMSYS "" CACHE PATH "Chemin vers MFMSYS")

if(NOT GRUSYS)
    message(FATAL_ERROR "Le chemin GRUSYS doit être spécifié : -DGRUSYS=/chemin/vers/GRUSYS")
endif()

if(NOT MFMSYS)
    message(FATAL_ERROR "Le chemin MFMSYS doit être spécifié : -DMFMSYS=/chemin/vers/MFMSYS")
endif()

# Détection de ROOT
find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

# Inclusion des répertoires
include_directories(
    ${ROOT_INCLUDE_DIRS}
    ${GRUSYS}/include
    ${MFMSYS}/include  # Ce chemin inclut DataParameters.h
)

message(STATUS "[INFO]  GRUSYS:: ${GRUSYS}")
message(STATUS "[INFO]  MFMSYS:: ${MFMSYS}")

# Ajouter les répertoires des bibliothèques
link_directories(
    ${GRUSYS}/lib
    ${MFMSYS}/lib
)

# Options de compilation pour TLib
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Macro pour générer les dictionnaires ROOT
macro(generate_root_dictionary class_name)
    set(CLASS_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/${class_name}.h")
    set(LINKDEF "${CMAKE_CURRENT_SOURCE_DIR}/${class_name}LinkDef.h")
    set(DICT_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/${class_name}Dict.cxx")
    
    add_custom_command(
        OUTPUT ${DICT_SOURCE}
        COMMAND rootcling
        ARGS -I${MFMSYS}/include -I${GRUSYS}/include -I${ROOT_INCLUDE_DIRS} -f ${DICT_SOURCE} -c ${CLASS_HEADER} ${LINKDEF}
        DEPENDS ${CLASS_HEADER} ${LINKDEF} 
        COMMENT "Génération du dictionnaire pour ${class_name}"
    )

    list(APPEND T_DICT_SOURCES ${DICT_SOURCE})
endmacro()

# Liste des classes pour lesquelles générer un dictionnaire
set(CLASSES_WITH_DICT
    TGenericData
    TExogam2Data
    TExogam2READata
    TVamosICData
    TParisData
    TTriggerData
    TNWallData
    TCsIData
    TEbyEData
    TCsI
    TExogam2
    TExogam2REA
    TGeneric
    TVamosIC
    TParis
    TEbyE
    TNWall
    TTrigger
    TDetector
    GUser
)

# Générer les dictionnaires pour chaque classe
foreach(class ${CLASSES_WITH_DICT})
    generate_root_dictionary(${class})
endforeach()

# Compilation des classes T* (inclure les fichiers .cxx des classes T* et leurs dictionnaires)
file(GLOB T_SOURCES "T*.cxx")
add_library(TLib STATIC ${T_SOURCES} ${T_DICT_SOURCES})
set_target_properties(TLib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(TLib ${ROOT_LIBRARIES} GRU MFM)

# Compilation de GUser
add_library(GUser SHARED GUser.C)
target_link_libraries(GUser TLib ${ROOT_LIBRARIES} GRU MFM)

# Compilation des exécutables
add_executable(AnalysisADNE AnalysisADNE.C)
target_link_libraries(AnalysisADNE TLib GUser ${ROOT_LIBRARIES} GRU MFM -lyaml-cpp)

add_executable(SpyAnalysisADNE SpyAnalysisADNE.C)
target_link_libraries(SpyAnalysisADNE TLib GUser ${ROOT_LIBRARIES} GRU MFM -lyaml-cpp)

# Installation (optionnel)
install(TARGETS TLib GUser AnalysisADNE SpyAnalysisADNE DESTINATION bin)
