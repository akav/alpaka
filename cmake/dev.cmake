#
# Copyright 2014-2015 Benjamin Worpitz
#
# This file is part of alpaka.
#
# alpaka is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# alpaka is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with alpaka.
# If not, see <http://www.gnu.org/licenses/>.
#

#-------------------------------------------------------------------------------
# Compiler settings.
#-------------------------------------------------------------------------------

#MSVC
IF(MSVC)
    # Force to always compile with W4
    IF(ALPAKA_DEV_COMPILE_OPTIONS MATCHES "/W[0-4]")
        STRING(REGEX REPLACE "/W[0-4]" "/W4" ALPAKA_DEV_COMPILE_OPTIONS "${ALPAKA_COMPILE_OPTIONS}")
    ELSE()
        LIST(APPEND ALPAKA_DEV_COMPILE_OPTIONS "/W4")
    ENDIF()
    # Improve debugging.
    IF(CMAKE_BUILD_TYPE MATCHES "Debug")
        LIST(APPEND ALPAKA_DEV_COMPILE_OPTIONS "-d2Zi+")
    ENDIF()
ELSE()
    # GNU
    IF(CMAKE_COMPILER_IS_GNUCXX)
        LIST(APPEND ALPAKA_DEV_COMPILE_OPTIONS "-Wall")
        LIST(APPEND ALPAKA_DEV_COMPILE_OPTIONS "-pedantic")
        LIST(APPEND ALPAKA_DEV_COMPILE_OPTIONS "-Wextra")
    # Clang or AppleClang
    ELSEIF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        LIST(APPEND ALPAKA_DEV_COMPILE_OPTIONS "-Wall")
    # ICC
    ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
        LIST(APPEND ALPAKA_DEV_COMPILE_OPTIONS "-Wall")
    # PGI
    ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL "PGI")
        LIST(APPEND ALPAKA_DEV_COMPILE_OPTIONS "-Minform=inform")
    ENDIF()
ENDIF()