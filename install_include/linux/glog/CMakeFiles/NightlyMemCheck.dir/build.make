# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/hxf0223/work/libcontrolcan2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/hxf0223/work/libcontrolcan2/build

# Utility rule file for NightlyMemCheck.

# Include the progress variables for this target.
include glog/CMakeFiles/NightlyMemCheck.dir/progress.make

glog/CMakeFiles/NightlyMemCheck:
	cd /home/hxf0223/work/libcontrolcan2/build/glog && /usr/bin/ctest -D NightlyMemCheck

NightlyMemCheck: glog/CMakeFiles/NightlyMemCheck
NightlyMemCheck: glog/CMakeFiles/NightlyMemCheck.dir/build.make

.PHONY : NightlyMemCheck

# Rule to build all files generated by this target.
glog/CMakeFiles/NightlyMemCheck.dir/build: NightlyMemCheck

.PHONY : glog/CMakeFiles/NightlyMemCheck.dir/build

glog/CMakeFiles/NightlyMemCheck.dir/clean:
	cd /home/hxf0223/work/libcontrolcan2/build/glog && $(CMAKE_COMMAND) -P CMakeFiles/NightlyMemCheck.dir/cmake_clean.cmake
.PHONY : glog/CMakeFiles/NightlyMemCheck.dir/clean

glog/CMakeFiles/NightlyMemCheck.dir/depend:
	cd /home/hxf0223/work/libcontrolcan2/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hxf0223/work/libcontrolcan2 /home/hxf0223/work/libcontrolcan2/glog /home/hxf0223/work/libcontrolcan2/build /home/hxf0223/work/libcontrolcan2/build/glog /home/hxf0223/work/libcontrolcan2/build/glog/CMakeFiles/NightlyMemCheck.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : glog/CMakeFiles/NightlyMemCheck.dir/depend

