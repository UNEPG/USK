# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = "/Users/azat/Library/Application Support/JetBrains/Toolbox/apps/CLion/ch-0/202.6948.80/CLion.app/Contents/bin/cmake/mac/bin/cmake"

# The command to remove a file.
RM = "/Users/azat/Library/Application Support/JetBrains/Toolbox/apps/CLion/ch-0/202.6948.80/CLion.app/Contents/bin/cmake/mac/bin/cmake" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/azat/Projects/unisat/sensorboard

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/azat/Projects/unisat/sensorboard/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/uno_SoftwareSerial.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/uno_SoftwareSerial.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/uno_SoftwareSerial.dir/flags.make

CMakeFiles/uno_SoftwareSerial.dir/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp.obj: CMakeFiles/uno_SoftwareSerial.dir/flags.make
CMakeFiles/uno_SoftwareSerial.dir/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp.obj: /Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/azat/Projects/unisat/sensorboard/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/uno_SoftwareSerial.dir/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp.obj"
	/Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/avr-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/uno_SoftwareSerial.dir/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp.obj -c /Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp

CMakeFiles/uno_SoftwareSerial.dir/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/uno_SoftwareSerial.dir/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp.i"
	/Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/avr-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp > CMakeFiles/uno_SoftwareSerial.dir/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp.i

CMakeFiles/uno_SoftwareSerial.dir/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/uno_SoftwareSerial.dir/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp.s"
	/Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/avr-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp -o CMakeFiles/uno_SoftwareSerial.dir/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp.s

# Object files for target uno_SoftwareSerial
uno_SoftwareSerial_OBJECTS = \
"CMakeFiles/uno_SoftwareSerial.dir/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp.obj"

# External object files for target uno_SoftwareSerial
uno_SoftwareSerial_EXTERNAL_OBJECTS =

libuno_SoftwareSerial.a: CMakeFiles/uno_SoftwareSerial.dir/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp.obj
libuno_SoftwareSerial.a: CMakeFiles/uno_SoftwareSerial.dir/build.make
libuno_SoftwareSerial.a: CMakeFiles/uno_SoftwareSerial.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/azat/Projects/unisat/sensorboard/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libuno_SoftwareSerial.a"
	$(CMAKE_COMMAND) -P CMakeFiles/uno_SoftwareSerial.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/uno_SoftwareSerial.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/uno_SoftwareSerial.dir/build: libuno_SoftwareSerial.a

.PHONY : CMakeFiles/uno_SoftwareSerial.dir/build

CMakeFiles/uno_SoftwareSerial.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/uno_SoftwareSerial.dir/cmake_clean.cmake
.PHONY : CMakeFiles/uno_SoftwareSerial.dir/clean

CMakeFiles/uno_SoftwareSerial.dir/depend:
	cd /Users/azat/Projects/unisat/sensorboard/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/azat/Projects/unisat/sensorboard /Users/azat/Projects/unisat/sensorboard /Users/azat/Projects/unisat/sensorboard/cmake-build-debug /Users/azat/Projects/unisat/sensorboard/cmake-build-debug /Users/azat/Projects/unisat/sensorboard/cmake-build-debug/CMakeFiles/uno_SoftwareSerial.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/uno_SoftwareSerial.dir/depend

