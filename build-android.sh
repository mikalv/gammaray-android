#!/bin/sh

if [[ ! -d toolchain ]]; then
  $ANDROID_NDK/build/tools/make-standalone-toolchain.sh \
    --install-dir=$PWD/toolchain
fi

# Add the standalone toolchain to the path.
pathadd() { PATH="${PATH:+"$PATH:"}$1"; }
pathadd "$PWD/toolchain/bin"
command -v arm-linux-androideabi-gcc &> /dev/null || exit -1
export CC=arm-linux-androideabi-gcc
export CXX=arm-linux-androideabi-g++
export LD=arm-linux-androideabi-ld
export RANLIB=arm-linux-androideabi-ranlib
export AR=arm-linux-androideabi-ar
export CROSS_PREFIX=arm-linux-androideabi-

if [[ ! -a libiconv-1.14/lib/.libs/libiconv.a ]]; then
  [[ -a libiconv-1.14.tar.gz ]] || \
    wget http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.14.tar.gz
  tar xvfz libiconv-1.14.tar.gz
  cd libiconv-1.14
  ./configure --enable-static --host=arm-linux
  make -j8
fi

[[ -a configure ]] || autoreconf -i
./configure --enable-static --host=arm-linux
make -j8 bin/gray-crawler
