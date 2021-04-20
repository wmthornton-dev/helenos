#!/bin/sh

#
# Copyright (c) 2009 Martin Decky
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# - Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# - The name of the author may not be used to endorse or promote products
#   derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

BINUTILS_GDB_GIT="https://github.com/HelenOS/binutils-gdb.git"

BINUTILS_BRANCH="binutils-2_31_1-helenos"
BINUTILS_VERSION="2.31.1"

GDB_BRANCH="gdb-8_2-helenos"
GDB_VERSION="8.2"

GCC_GIT="https://github.com/HelenOS/gcc.git"
GCC_BRANCH="8_2_0-helenos"
GCC_VERSION="8.2.0"

BASEDIR="$PWD"
SRCDIR="$(readlink -f $(dirname "$0"))"

SYSTEM_INSTALL=false
REAL_INSTALL=true
USE_HELENOS_TARGET=true

check_error() {
	if [ "$1" -ne "0" ] ; then
		echo
		echo "Script failed: $2"

		exit 1
	fi
}

show_usage() {
	echo "HelenOS cross-compiler toolchain build script"
	echo
	echo "Syntax:"
	echo " $0 [--system-wide] [--no-install] [--non-helenos-target] <platform>"
	echo " $0 [--system-wide] --test-version [<platform>]"
	echo
	echo "Possible target platforms are:"
	echo " amd64      AMD64 (x86-64, x64)"
	echo " arm32      ARM 32b"
	echo " arm64      AArch64"
	echo " ia32       IA-32 (x86, i386)"
	echo " ia64       IA-64 (Itanium)"
	echo " mips32     MIPS little-endian 32b"
	echo " mips32eb   MIPS big-endian 32b"
	echo " ppc32      PowerPC 32b"
	echo " riscv64    RISC-V 64b"
	echo " sparc64    SPARC V9"
	echo " all        build all targets"
	echo " essential  build only targets currently needed for HelenOS development"
	echo " parallel   same as 'all', but all in parallel"
	echo " 2-way      same as 'all', but 2-way parallel"
	echo
	echo "The toolchain target installation directory is determined by matching"
	echo "the first of the following conditions:"
	echo
	echo " (1) If the \$CROSS_PREFIX environment variable is set, then it is"
	echo "     used as the target installation directory."
	echo " (2) If the --system-wide option is used, then /opt/HelenOS/cross"
	echo "     is used as the target installation directory. This usually"
	echo "     requires running this script with super user privileges."
	echo " (3) In other cases, \$XDG_DATA_HOME/HelenOS/cross is used as the"
	echo "     target installation directory. If the \$XDG_DATA_HOME environment"
	echo "     variable is not set, then the default value of \$HOME/.local/share"
	echo "     is assumed."
	echo
	echo "If the --no-install option is used, the toolchain still uses the"
	echo "target installation directory as determined above, but the files"
	echo "are actually copied into the PKG/ subdirectory during the installation"
	echo "without affecting the actual target file system. This might be useful"
	echo "when preparing a system-wide installation, but avoiding running this"
	echo "script under the super user."
	echo
	echo "The --non-helenos-target option will build non-HelenOS-specific"
	echo "toolchain (i.e. it will use *-linux-* triplet instead of *-helenos)."
	echo "Using this toolchain for building HelenOS is not supported."
	echo
	echo "The --test-version mode tests the currently installed version of the"
	echo "toolchain."

	exit 3
}

set_cross_prefix() {
	if [ -z "$CROSS_PREFIX" ] ; then
		if $SYSTEM_INSTALL ; then
			CROSS_PREFIX="/opt/HelenOS/cross"
		else
			if [ -z "$XDG_DATA_HOME" ] ; then
				XDG_DATA_HOME="$HOME/.local/share"
			fi
			CROSS_PREFIX="$XDG_DATA_HOME/HelenOS/cross"
		fi
	fi
}

test_version() {
	set_cross_prefix

	echo "HelenOS cross-compiler toolchain build script"
	echo
	echo "Testing the version of the installed software in $CROSS_PREFIX"
	echo

	if [ -z "$1" ] || [ "$1" = "all" ] ; then
		PLATFORMS='amd64 arm32 arm64 ia32 ia64 mips32 mips32eb ppc32 riscv64 sparc64'
	else
		PLATFORMS="$1"
	fi

	for PLATFORM in $PLATFORMS ; do
		set_target_from_platform "$PLATFORM"
		PREFIX="${CROSS_PREFIX}/bin/${HELENOS_TARGET}"

		echo "== $PLATFORM =="
		test_app_version "Binutils" "ld" "GNU ld (.*) \([.0-9]*\)" "$BINUTILS_VERSION"
		test_app_version "GCC" "gcc" "gcc version \([.0-9]*\)" "$GCC_VERSION"
		test_app_version "GDB" "gdb" "GNU gdb (.*)[[:space:]]\+\([.0-9]*\)" "$GDB_VERSION"
	done
}

test_app_version() {
	PKGNAME="$1"
	APPNAME="$2"
	REGEX="$3"
	INS_VERSION="$4"

	APP="${PREFIX}-${APPNAME}"
	if [ ! -e $APP ]; then
		echo "- $PKGNAME is missing"
	else
		VERSION=`${APP} -v 2>&1 | sed -n "s:^${REGEX}.*:\1:p"`

		if [ -z "$VERSION" ]; then
			echo "- $PKGNAME Unexpected output"
			return 1
		fi

		if [ "$INS_VERSION" = "$VERSION" ]; then
			echo "+ $PKGNAME is up-to-date ($INS_VERSION)"
		else
			echo "- $PKGNAME ($VERSION) is outdated ($INS_VERSION)"
		fi
	fi
}

change_title() {
	printf "\e]0;$1\a"
}

show_countdown() {
	TM="$1"

	if [ "${TM}" -eq 0 ] ; then
		echo
		return 0
	fi

	printf "${TM} "
	change_title "${TM}"
	sleep 1

	TM="`expr "${TM}" - 1`"
	show_countdown "${TM}"
}

show_dependencies() {
	set_cross_prefix

	echo "HelenOS cross-compiler toolchain build script"
	echo
	echo "Installing software to $CROSS_PREFIX"
	echo
	echo
	echo "IMPORTANT NOTICE:"
	echo
	echo "For a successful compilation and use of the cross-compiler"
	echo "toolchain you need at least the following dependencies."
	echo
	echo "Please make sure that the dependencies are present in your"
	echo "system. Otherwise the compilation process might fail after"
	echo "a few seconds or minutes."
	echo
	echo " - SED, AWK, Flex, Bison, gzip, bzip2, Bourne Shell"
	echo " - gettext, zlib, Texinfo, libelf, libgomp"
	echo " - GNU Make, Coreutils, Sharutils, tar"
	echo " - native C and C++ compiler, assembler and linker"
	echo " - native C and C++ standard library with headers"
	echo
}

cleanup_dir() {
	DIR="$1"

	if [ -d "${DIR}" ] ; then
		change_title "Removing ${DIR}"
		echo " >>> Removing ${DIR}"
		rm -fr "${DIR}"
	fi
}

create_dir() {
	DIR="$1"
	DESC="$2"

	change_title "Creating ${DESC}"
	echo ">>> Creating ${DESC}"

	mkdir -p "${DIR}"
	test -d "${DIR}"
	check_error $? "Unable to create ${DIR}."
}

check_dirs() {
	OUTSIDE="$1"
	BASE="$2"
	ORIGINAL="$PWD"

	cd "${BASE}"
	check_error $? "Unable to change directory to ${BASE}."
	ABS_BASE="$PWD"
	cd "${ORIGINAL}"
	check_error $? "Unable to change directory to ${ORIGINAL}."

	mkdir -p "${OUTSIDE}"
	cd "${OUTSIDE}"
	check_error $? "Unable to change directory to ${OUTSIDE}."

	while [ "${#PWD}" -gt "${#ABS_BASE}" ]; do
		cd ..
	done

	if [ "$PWD" = "$ABS_BASE" ]; then
		echo
		echo "CROSS_PREFIX cannot reside within the working directory."

		exit 5
	fi

	cd "${ORIGINAL}"
	check_error $? "Unable to change directory to ${ORIGINAL}."
}

prepare() {
	show_dependencies
	show_countdown 10

	mkdir -p "${BASEDIR}/downloads"
	cd "${BASEDIR}/downloads"
	check_error $? "Change directory failed."

	change_title "Downloading sources"
	echo ">>> Downloading sources"
	git clone --depth 1 -b "$BINUTILS_BRANCH" "$BINUTILS_GDB_GIT" "binutils-$BINUTILS_VERSION"
	git clone --depth 1 -b "$GDB_BRANCH" "$BINUTILS_GDB_GIT" "gdb-$GDB_VERSION"
	git clone --depth 1 -b "$GCC_BRANCH" "$GCC_GIT" "gcc-$GCC_VERSION"

	# If the directory already existed, pull upstream changes.
	git -C "binutils-$BINUTILS_VERSION" pull
	git -C "gdb-$GDB_VERSION" pull
	git -C "gcc-$GCC_VERSION" pull

	change_title "Downloading GCC prerequisites"
	echo ">>> Downloading GCC prerequisites"
	cd "gcc-${GCC_VERSION}"
	./contrib/download_prerequisites
	cd ..
}

set_target_from_platform() {
	case "$1" in
		"arm32")
			GNU_ARCH="arm"
			;;
		"arm64")
			GNU_ARCH="aarch64"
			;;
		"ia32")
			GNU_ARCH="i686"
			;;
		"mips32")
			GNU_ARCH="mipsel"
			;;
		"mips32eb")
			GNU_ARCH="mips"
			;;
		"ppc32")
			GNU_ARCH="ppc"
			;;
		*)
			GNU_ARCH="$1"
			;;
	esac

	HELENOS_TARGET="${GNU_ARCH}-helenos"

	case "$1" in
		"arm32")
			LINUX_TARGET="${GNU_ARCH}-linux-gnueabi"
			;;
		*)
			LINUX_TARGET="${GNU_ARCH}-linux-gnu"
			;;
	esac
}

build_target() {
	PLATFORM="$1"

	# This sets the *_TARGET variables
	set_target_from_platform "$PLATFORM"
	if $USE_HELENOS_TARGET ; then
		TARGET="$HELENOS_TARGET"
	else
		TARGET="$LINUX_TARGET"
	fi

	WORKDIR="${BASEDIR}/${TARGET}"
	INSTALL_DIR="${BASEDIR}/PKG"
	BINUTILSDIR="${WORKDIR}/binutils-${BINUTILS_VERSION}"
	GCCDIR="${WORKDIR}/gcc-${GCC_VERSION}"
	GDBDIR="${WORKDIR}/gdb-${GDB_VERSION}"

	# This sets the CROSS_PREFIX variable
	set_cross_prefix

	if [ -z "$JOBS" ] ; then
		JOBS="`nproc`"
	fi

	PREFIX="${CROSS_PREFIX}"

	echo ">>> Removing previous content"
	cleanup_dir "${WORKDIR}"
	mkdir -p "${WORKDIR}"
	check_dirs "${PREFIX}" "${WORKDIR}"

	if $USE_HELENOS_TARGET ; then
		change_title "Creating build sysroot"
		echo ">>> Creating build sysroot"
		mkdir -p "${WORKDIR}/sysroot/include"
		mkdir "${WORKDIR}/sysroot/lib"
		ARCH="$PLATFORM"
		if [ "$ARCH" = "mips32eb" ]; then
			ARCH=mips32
		fi

		cp -r -L -t "${WORKDIR}/sysroot/include" \
			${SRCDIR}/../abi/include/* \
			${SRCDIR}/../uspace/lib/c/arch/${ARCH}/include/* \
			${SRCDIR}/../uspace/lib/c/include/*
		check_error $? "Failed to create build sysroot."
	fi


	echo ">>> Processing binutils (${PLATFORM})"
	mkdir -p "${BINUTILSDIR}"
	cd "${BINUTILSDIR}"
	check_error $? "Change directory failed."

	change_title "binutils: configure (${PLATFORM})"
	CFLAGS="-Wno-error -fcommon" "${BASEDIR}/downloads/binutils-${BINUTILS_VERSION}/configure" \
		"--target=${TARGET}" \
		"--prefix=${PREFIX}" \
		"--program-prefix=${TARGET}-" \
		--disable-nls \
		--disable-werror \
		--enable-gold \
		--enable-deterministic-archives \
		--disable-gdb \
		--with-sysroot
	check_error $? "Error configuring binutils."

	change_title "binutils: make (${PLATFORM})"
	make all -j$JOBS
	check_error $? "Error compiling binutils."

	change_title "binutils: install (${PLATFORM})"
	make install "DESTDIR=${INSTALL_DIR}"
	check_error $? "Error installing binutils."


	echo ">>> Processing GCC (${PLATFORM})"
	mkdir -p "${GCCDIR}"
	cd "${GCCDIR}"
	check_error $? "Change directory failed."

	if $USE_HELENOS_TARGET ; then
		SYSROOT=--with-sysroot --with-build-sysroot="${WORKDIR}/sysroot"
	else
		SYSROOT=--without-headers
	fi

	change_title "GCC: configure (${PLATFORM})"
	PATH="$PATH:${INSTALL_DIR}/${PREFIX}/bin" "${BASEDIR}/downloads/gcc-${GCC_VERSION}/configure" \
		"--target=${TARGET}" \
		"--prefix=${PREFIX}" \
		"--program-prefix=${TARGET}-" \
		--with-gnu-as \
		--with-gnu-ld \
		--disable-nls \
		--enable-languages=c,c++,go \
		--enable-lto \
		--disable-shared \
		--disable-werror \
		$SYSROOT
	check_error $? "Error configuring GCC."

	change_title "GCC: make (${PLATFORM})"
	PATH="${PATH}:${PREFIX}/bin:${INSTALL_DIR}/${PREFIX}/bin" make all-gcc -j$JOBS
	check_error $? "Error compiling GCC."

	if $USE_HELENOS_TARGET ; then
		PATH="${PATH}:${PREFIX}/bin:${INSTALL_DIR}/${PREFIX}/bin" make all-target-libgcc -j$JOBS
		check_error $? "Error compiling libgcc."
		# TODO: libatomic and libstdc++ need some extra care
		#    PATH="${PATH}:${PREFIX}/bin:${INSTALL_DIR}/${PREFIX}/bin" make all-target-libatomic -j$JOBS
		#    check_error $? "Error compiling libatomic."
		#    PATH="${PATH}:${PREFIX}/bin:${INSTALL_DIR}/${PREFIX}/bin" make all-target-libstdc++-v3 -j$JOBS
		#    check_error $? "Error compiling libstdc++."
	fi

	change_title "GCC: install (${PLATFORM})"
	PATH="${PATH}:${INSTALL_DIR}/${PREFIX}/bin" make install-gcc "DESTDIR=${INSTALL_DIR}"
	if $USE_HELENOS_TARGET ; then
		PATH="${PATH}:${INSTALL_DIR}/${PREFIX}/bin" make install-target-libgcc "DESTDIR=${INSTALL_DIR}"
		#    PATH="${PATH}:${INSTALL_DIR}/${PREFIX}/bin" make install-target-libatomic "DESTDIR=${INSTALL_DIR}"
		#    PATH="${PATH}:${INSTALL_DIR}/${PREFIX}/bin" make install-target-libstdc++-v3 "DESTDIR=${INSTALL_DIR}"
	fi
	check_error $? "Error installing GCC."


	echo ">>> Processing GDB (${PLATFORM})"
	mkdir -p "${GDBDIR}"
	cd "${GDBDIR}"
	check_error $? "Change directory failed."

	change_title "GDB: configure (${PLATFORM})"
	CFLAGS="-fcommon" PATH="$PATH:${INSTALL_DIR}/${PREFIX}/bin" "${BASEDIR}/downloads/gdb-${GDB_VERSION}/configure" \
		"--target=${TARGET}" \
		"--prefix=${PREFIX}" \
		"--program-prefix=${TARGET}-" \
		--enable-werror=no
	check_error $? "Error configuring GDB."

	change_title "GDB: make (${PLATFORM})"
	PATH="${PATH}:${PREFIX}/bin:${INSTALL_DIR}/${PREFIX}/bin" make all-gdb -j$JOBS
	check_error $? "Error compiling GDB."

	change_title "GDB: make (${PLATFORM})"
	PATH="${PATH}:${INSTALL_DIR}/${PREFIX}/bin" make install-gdb "DESTDIR=${INSTALL_DIR}"
	check_error $? "Error installing GDB."

	# Symlink clang and lld to the install path.
	CLANG="`which clang 2> /dev/null || echo "/usr/bin/clang"`"
	LLD="`which ld.lld 2> /dev/null || echo "/usr/bin/ld.lld"`"

	ln -s $CLANG "${INSTALL_DIR}/${PREFIX}/bin/${TARGET}-clang"
	ln -s $LLD "${INSTALL_DIR}/${PREFIX}/bin/${TARGET}-ld.lld"

	if $REAL_INSTALL ; then
		echo ">>> Moving to the destination directory."
		cp -r -t "${PREFIX}" "${INSTALL_DIR}/${PREFIX}/"*
	fi

	cd "${BASEDIR}"
	check_error $? "Change directory failed."

	echo ">>> Cleaning up"
	cleanup_dir "${WORKDIR}"

	echo
	echo ">>> Cross-compiler for ${TARGET} installed."
}

while [ "$#" -gt 1 ] ; do
	case "$1" in
		--system-wide)
			SYSTEM_INSTALL=true
			shift
			;;
		--test-version)
			test_version "$2"
			exit
			;;
		--no-install)
			REAL_INSTALL=false
			shift
			;;
		--non-helenos-target)
			USE_HELENOS_TARGET=false
			shift
			;;
		*)
			show_usage
			;;
	esac
done

if [ "$#" -lt "1" ] ; then
	show_usage
fi

case "$1" in
	--test-version)
		test_version
		exit
		;;
	amd64|arm32|arm64|ia32|ia64|mips32|mips32eb|ppc32|riscv64|sparc64)
		prepare
		build_target "$1"
		;;
	"all")
		prepare
		build_target "amd64"
		build_target "arm32"
		build_target "arm64"
		build_target "ia32"
		build_target "ia64"
		build_target "mips32"
		build_target "mips32eb"
		build_target "ppc32"
		build_target "riscv64"
		build_target "sparc64"
		;;
	"essential")
		prepare
		build_target "amd64"
		build_target "arm32"
		build_target "arm64"
		build_target "ia32"
		build_target "ia64"
		build_target "mips32"
		build_target "mips32eb"
		build_target "ppc32"
		build_target "sparc64"
		;;
	"parallel")
		prepare
		build_target "amd64" &
		build_target "arm32" &
		build_target "arm64" &
		build_target "ia32" &
		build_target "ia64" &
		build_target "mips32" &
		build_target "mips32eb" &
		build_target "ppc32" &
		build_target "riscv64" &
		build_target "sparc64" &
		wait
		;;
	"2-way")
		prepare
		build_target "amd64" &
		build_target "arm32" &
		wait

		build_target "arm64" &
		build_target "ia32" &
		wait

		build_target "ia64" &
		build_target "mips32" &
		wait

		build_target "mips32eb" &
		build_target "ppc32" &
		wait

		build_target "riscv64" &
		build_target "sparc64" &
		wait
		;;
	*)
		show_usage
		;;
esac
