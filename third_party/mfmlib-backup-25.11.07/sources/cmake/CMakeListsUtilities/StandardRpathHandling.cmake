# RPATH handling - executables can be used without setting LD_LIBRARY_PATH
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling

# use, i.e. don't skip the full RPATH for the build or install tree
set(CMAKE_SKIP_RPATH FALSE)

# use, i.e. don't skip the full RPATH for the install tree
set(CMAKE_SKIP_INSTALL_RPATH FALSE)

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# add the automatically determined parts of the RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(APPLE)
  # macOS: dyld does NOT support $ORIGIN; use Mach-O tokens instead
  set(CMAKE_MACOSX_RPATH ON)

  # Install-time rpath: relative to the executable location
  # (equivalent of $ORIGIN/../<pkglibdir> on Linux)
  set(CMAKE_INSTALL_RPATH
      "@executable_path"
      "@executable_path/../${CMAKE_INSTALL_PKGLIBDIR}"
  )
else()
  # Linux/ELF: $ORIGIN is supported
  set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
  set(CMAKE_INSTALL_RPATH "$ORIGIN" "$ORIGIN/../${CMAKE_INSTALL_PKGLIBDIR}")
endif()





