cmake_minimum_required(VERSION 3.16)
project(qtjack
  DESCRIPTION "a qt jack lib"
  VERSION 1.0.1
  LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/cmake")  

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

find_package( Qt5Core REQUIRED)
find_package(Jack REQUIRED)

set(QTJACK_HEADERS
  include/AudioBuffer
  include/AudioPort
  include/Buffer
  include/Client
  include/Driver
  include/LICENSE
  include/MidiBuffer
  include/MidiEvent
  include/MidiPort
  include/Parameter
  include/Port
  include/Processor
  include/RingBuffer
  include/Server
  include/System

  include/audiobuffer.h
  include/audioport.h
  include/buffer.h
  include/client.h
  include/driver.h
  include/global.h
  include/midibuffer.h
  include/midievent.h
  include/midiport.h
  include/parameter.h
  include/port.h
  include/processor.h
  include/ringbuffer.h
  include/server.h
  include/system.h
)
set(QTJACK_SOURCES
  src/audiobuffer.cpp
  src/audioport.cpp
  src/buffer.cpp
  src/client.cpp
  src/driver.cpp
  src/midibuffer.cpp
  src/midievent.cpp
  src/midiport.cpp
  src/parameter.cpp
  src/port.cpp
  src/server.cpp
  src/system.cpp
)

QT5_WRAP_CPP(QTJACK_MOCrcs 
    ${QTJACK_SOURCES}
    ${QTJACK_HEADERS}
)

add_library(qtjack SHARED
    ${QTJACK_SOURCES}
    ${QTJACK_HEADERS}
)

target_sources(qtjack PRIVATE
    ${QTJACK_SOURCES}
    ${QTJACK_HEADERS}
    ${QTJACK_MOCrcs}
)

target_include_directories(qtjack PRIVATE include)
target_include_directories(qtjack PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(qtjack PUBLIC 
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>/qtjack)
target_include_directories(qtjack SYSTEM
  PUBLIC ${Qt5Core_INCLUDE_DIRS})
target_include_directories(qtjack SYSTEM
  PUBLIC ${JACK_INCLUDE_DIRS})

target_link_libraries(qtjack PUBLIC 
    ${JACK_LIBRARIES}
    Qt5::Core)

set_target_properties(qtjack PROPERTIES PUBLIC_HEADER "${QTJACK_HEADERS}")

install(TARGETS 
    qtjack
    EXPORT qtjackConfig
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION include/qtjack
)

export(TARGETS qtjack
    NAMESPACE qtjack::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/qtjackConfig.cmake"
)

install(EXPORT qtjackConfig
    DESTINATION "${CMAKE_INSTALL_DATADIR}/qtjack/cmake"
    NAMESPACE qtjack::
)
