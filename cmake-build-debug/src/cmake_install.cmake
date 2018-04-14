# Install script for directory: /Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
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

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project/FinalProject")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project" TYPE EXECUTABLE FILES "/Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project/cmake-build-debug/FinalProject")
  if(EXISTS "$ENV{DESTDIR}/Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project/FinalProject" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project/FinalProject")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project/cmake-build-debug/ext/nanogui"
      "$ENV{DESTDIR}/Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project/FinalProject")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" "$ENV{DESTDIR}/Users/Nader/Google_Drive/Berkeley/Academics/2017-18_Junior_Year/Spring_Semester/Classes/CS184/FP/184-final-project/FinalProject")
    endif()
  endif()
endif()

