cmake_minimum_required(VERSION 3.16)
project(qtjack
  DESCRIPTION "a qt jack lib"
  LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/cmake")  

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package( Qt5Core REQUIRED)
find_package(Jack REQUIRED)

set(QTJACK_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/AudioBuffer
  ${CMAKE_CURRENT_SOURCE_DIR}/include/AudioPort
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Buffer
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Client
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Driver
  ${CMAKE_CURRENT_SOURCE_DIR}/include/LICENSE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/MidiBuffer
  ${CMAKE_CURRENT_SOURCE_DIR}/include/MidiEvent
  ${CMAKE_CURRENT_SOURCE_DIR}/include/MidiPort
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Parameter
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Port
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Processor
  ${CMAKE_CURRENT_SOURCE_DIR}/include/RingBuffer
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Server
  ${CMAKE_CURRENT_SOURCE_DIR}/include/System

  ${CMAKE_CURRENT_SOURCE_DIR}/include/audiobuffer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/audioport.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/buffer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/client.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/driver.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/global.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/midibuffer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/midievent.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/midiport.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/parameter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/port.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/processor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/ringbuffer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/server.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/system.h
)
set(QTJACK_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/audiobuffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/audioport.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/buffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/driver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/midibuffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/midievent.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/midiport.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/parameter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/port.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/server.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/system.cpp
)

QT5_WRAP_CPP(QTJACK_MOCrcs 
    ${QTJACK_SOURCES}
    ${QTJACK_HEADERS})

add_library(qtjack SHARED
    ${QTJACK_SOURCES}
    ${QTJACK_HEADERS}
    ${QTJACK_MOCrcs})

target_include_directories(qtjack PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(qtjack PUBLIC include)
target_include_directories(qtjack SYSTEM
  PUBLIC ${Qt5Core_INCLUDE_DIRS})
target_include_directories(qtjack SYSTEM
  PUBLIC ${JACK_INCLUDE_DIRS})

target_link_libraries(qtjack PRIVATE 
    ${JACK_LIBRARIES}
    Qt5::Core)

set_target_properties(qtjack PROPERTIES PUBLIC_HEADER "${QTJACK_HEADERS}")

install(TARGETS qtjack
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/qtjack)