# Install script for directory: /Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project/")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project/cmake-build-debug/ext/nanogui/cmake_install.cmake")
  include("/Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project/cmake-build-debug/CGL/cmake_install.cmake")
  include("/Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project/cmake-build-debug/src/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project/cmake-build-debug/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
