#
# Copyright (c) 2017, 2018, Dmitry Salychev <darkness.bsd@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the <organization> nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# RGB to palette - program to find a nearest color from the given palette
# to the selected 24-bit RGB color. It calculates a distance between two
# colors in YUV colorspace in order to find similar ones according to the
# "average human perception".
#
# Usage: rgb2pal bc7575 terminal256.palette
#
cmake_minimum_required(VERSION 3.2)
project(rgb2pal C)

# Program name and version
set(RTP "rgb2pal")
set(RTP_VERSION "0.1.0")
add_definitions(-DRTP_VERSION="${RTP_VERSION}")

include(CheckFunctionExists)
include(CheckIncludeFiles)

# Set flags here
if (CMAKE_BUILD_TYPE MATCHES Debug)
	message(STATUS "Debug version of ${RTP} ${RTP_VERSION} will be built by default.")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG")
	if (CMAKE_COMPILER_IS_GNUCC OR
	    ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -std=iso9899:1999")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wpointer-arith -Wcast-qual")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align -Wstrict-prototypes")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes -Wconversion")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wsign-compare")
	endif()
else()
	message(STATUS "Release version of ${RTP} ${MSIM_VERSION} will be built.")
	message(STATUS "Set CMAKE_BUILD_TYPE=Debug to build a debug version.")
	if (CMAKE_COMPILER_IS_GNUCC OR
	    ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -pedantic -std=iso9899:1999")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wshadow -Wpointer-arith -Wcast-qual")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wcast-align -Wstrict-prototypes")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wmissing-prototypes -Wconversion")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wsign-compare")
	endif()
endif()

# Set sources here
set(RTP_SRC		main.c)

# Look for libraries, packages, etc.
# ...

# Set target libraries here
# ...

# Check functions, include files here
if (NOT MSVC)
	check_function_exists(fmax RESULT)
	if (NOT RESULT)
		unset(RESULT)
		list(APPEND TARGET_LIBS -lm)
		message(STATUS "Math library added to the TARGET_LIBS")
	endif()
endif()

# Define include files and directories here
# ...

add_executable(${RTP} ${RTP_SRC})
install(TARGETS ${RTP} DESTINATION bin)
file(COPY "terminal256.palette" DESTINATION ${CMAKE_BINARY_DIR})
