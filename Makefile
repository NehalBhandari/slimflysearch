#--------------------- Basic Settings -----------------------------------------#
PROGRAM_NAME  := slimflysearch
BINARY_BASE   := bin
BUILD_BASE    := bld
SOURCE_BASE   := src
MAIN_FILE     := src/search.cc

#--------------------- External Libraries -------------------------------------#
HEADER_DIRS   := \
	../libprim/inc \
	../libstrop/inc \
	../libgrid/inc

STATIC_LIBS   := \
	../libprim/bld/libprim.a \
	../libstrop/bld/libstrop.a \
	../libgrid/bld/libgrid.a

#--------------------- Cpp Lint -----------------------------------------------#
LINT          := ../makeccpp/cpplint/cpplint.py
LINT_FLAGS    :=

#--------------------- Unit Tests ---------------------------------------------#
TEST_SUFFIX   := _TEST
GTEST_BASE    := ../makeccpp/gtest

#--------------------- Compilation and Linking --------------------------------#
CXX           := g++
SRC_EXTS      := .cc
HDR_EXTS      := .h .tcc
CXX_FLAGS     := -Wall -Wextra -pedantic -Wfatal-errors -std=c++11
CXX_FLAGS     +=  -g -O3 -flto
LINK_FLAGS    :=

#--------------------- Auto Makefile ------------------------------------------#
include ../makeccpp/auto_bin.mk
