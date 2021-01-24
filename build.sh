#!/bin/sh

PWD=$(pwd)
SRC_DIR=$PWD
BUILD_DIR=$PWD/build
BINDIR_X86=$BUILD_DIR/x86
BINDIR_X64=$BUILD_DIR/x64

check_mingw(){
	if [ ! -x "$(command -v i686-w64-mingw32-gcc)" ] || [ ! -x "$(command -v x86_64-w64-mingw32-gcc)" ]; then
		echo 'Error: mingw-w64 is not installed.' >&2
		exit 1
	fi
}

prepare_dirs(){
	if [ ! -d $BUILD_DIR ]; then
		mkdir -p $BUILD_DIR || exit 1
	fi
	if [ ! -d $BINDIR_X86 ]; then
		mkdir -p $BINDIR_X86 || exit 1
	fi
	if [ ! -d $BINDIR_X64 ]; then
		mkdir -p $BINDIR_X64 || exit 1
	fi
	return 0
}

build_target_x86(){
	make -C $SRC_DIR CC=i686-w64-mingw32-gcc STRIP=i686-w64-mingw32-strip CONFIG_STATIC=yes TARGET_OS=MinGW strip && mv $SRC_DIR/*.exe $BINDIR_X86
	return 0
}

build_target_x64(){
	make -C $SRC_DIR CC=x86_64-w64-mingw32-gcc STRIP=x86_64-w64-mingw32-strip CONFIG_STATIC=yes TARGET_OS=MinGW strip && mv $SRC_DIR/*.exe $BINDIR_X64
	return 0
}

clean_all(){
	if [ -d $BUILD_DIR ]; then
		rm -rf $BUILD_DIR || exit 1
	fi
	return 0
}

check_mingw
prepare_dirs
build_target_x86
build_target_x64
#clean_all
