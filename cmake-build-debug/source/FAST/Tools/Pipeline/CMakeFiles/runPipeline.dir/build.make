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
CMAKE_COMMAND = /snap/clion/126/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/126/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/andrep/workspace/forks/FAST

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/andrep/workspace/forks/FAST/cmake-build-debug

# Include any dependencies generated for this target.
include source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/depend.make

# Include the progress variables for this target.
include source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/progress.make

# Include the compile flags for this target's objects.
include source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/flags.make

source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/runPipeline_autogen/mocs_compilation.cpp.o: source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/flags.make
source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/runPipeline_autogen/mocs_compilation.cpp.o: source/FAST/Tools/Pipeline/runPipeline_autogen/mocs_compilation.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/andrep/workspace/forks/FAST/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/runPipeline_autogen/mocs_compilation.cpp.o"
	cd /home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Tools/Pipeline && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/runPipeline.dir/runPipeline_autogen/mocs_compilation.cpp.o -c /home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Tools/Pipeline/runPipeline_autogen/mocs_compilation.cpp

source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/runPipeline_autogen/mocs_compilation.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/runPipeline.dir/runPipeline_autogen/mocs_compilation.cpp.i"
	cd /home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Tools/Pipeline && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Tools/Pipeline/runPipeline_autogen/mocs_compilation.cpp > CMakeFiles/runPipeline.dir/runPipeline_autogen/mocs_compilation.cpp.i

source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/runPipeline_autogen/mocs_compilation.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/runPipeline.dir/runPipeline_autogen/mocs_compilation.cpp.s"
	cd /home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Tools/Pipeline && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Tools/Pipeline/runPipeline_autogen/mocs_compilation.cpp -o CMakeFiles/runPipeline.dir/runPipeline_autogen/mocs_compilation.cpp.s

source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/main.cpp.o: source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/flags.make
source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/main.cpp.o: ../source/FAST/Tools/Pipeline/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/andrep/workspace/forks/FAST/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/main.cpp.o"
	cd /home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Tools/Pipeline && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/runPipeline.dir/main.cpp.o -c /home/andrep/workspace/forks/FAST/source/FAST/Tools/Pipeline/main.cpp

source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/runPipeline.dir/main.cpp.i"
	cd /home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Tools/Pipeline && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/andrep/workspace/forks/FAST/source/FAST/Tools/Pipeline/main.cpp > CMakeFiles/runPipeline.dir/main.cpp.i

source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/runPipeline.dir/main.cpp.s"
	cd /home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Tools/Pipeline && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/andrep/workspace/forks/FAST/source/FAST/Tools/Pipeline/main.cpp -o CMakeFiles/runPipeline.dir/main.cpp.s

# Object files for target runPipeline
runPipeline_OBJECTS = \
"CMakeFiles/runPipeline.dir/runPipeline_autogen/mocs_compilation.cpp.o" \
"CMakeFiles/runPipeline.dir/main.cpp.o"

# External object files for target runPipeline
runPipeline_EXTERNAL_OBJECTS =

bin/runPipeline: source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/runPipeline_autogen/mocs_compilation.cpp.o
bin/runPipeline: source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/main.cpp.o
bin/runPipeline: source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/build.make
bin/runPipeline: lib/libFAST.so.3.1.1
bin/runPipeline: /usr/lib/x86_64-linux-gnu/libopenslide.so
bin/runPipeline: /usr/lib/x86_64-linux-gnu/libOpenCL.so
bin/runPipeline: /usr/lib/x86_64-linux-gnu/libGL.so
bin/runPipeline: /usr/lib/x86_64-linux-gnu/libGLU.so
bin/runPipeline: /usr/lib/x86_64-linux-gnu/libSM.so
bin/runPipeline: /usr/lib/x86_64-linux-gnu/libICE.so
bin/runPipeline: /usr/lib/x86_64-linux-gnu/libX11.so
bin/runPipeline: /usr/lib/x86_64-linux-gnu/libXext.so
bin/runPipeline: source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/andrep/workspace/forks/FAST/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../../../../bin/runPipeline"
	cd /home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Tools/Pipeline && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/runPipeline.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/build: bin/runPipeline

.PHONY : source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/build

source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/clean:
	cd /home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Tools/Pipeline && $(CMAKE_COMMAND) -P CMakeFiles/runPipeline.dir/cmake_clean.cmake
.PHONY : source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/clean

source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/depend:
	cd /home/andrep/workspace/forks/FAST/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/andrep/workspace/forks/FAST /home/andrep/workspace/forks/FAST/source/FAST/Tools/Pipeline /home/andrep/workspace/forks/FAST/cmake-build-debug /home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Tools/Pipeline /home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : source/FAST/Tools/Pipeline/CMakeFiles/runPipeline.dir/depend

