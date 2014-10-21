#!/bin/sh

die () { echo $*; exit -1; }

set -x

[[ -a configure ]] || ./bootstrap.sh || die "GammaRay bootstrap failed."

if [[ ! -d toolchain ]]; then
  # Using NDK r10b.
  $ANDROID_NDK/build/tools/make-standalone-toolchain.sh \
    --platform=android-18 \
    --toolchain=arm-linux-androideabi-4.8 \
    --install-dir=$PWD/toolchain
fi

# Add the standalone toolchain to the path.
pathadd() { PATH="${PATH:+"$PATH:"}$1"; }
pathadd "$PWD/toolchain/bin"
command -v arm-linux-androideabi-gcc &> /dev/null || exit -1
export SYSROOT=$PWD/toolchain/sysroot
export CC="arm-linux-androideabi-gcc --sysroot=$SYSROOT"
export CXX=arm-linux-androideabi-g++
export LD=arm-linux-androideabi-ld
export RANLIB=arm-linux-androideabi-ranlib
export AR=arm-linux-androideabi-ar
export CROSS_PREFIX=arm-linux-androideabi-
export CFLAGS='-march=armv7-a -mfloat-abi=softfp -mfpu=neon'
export LDFLAGS='-Wl,--fix-cortex-a8'

if [[ ! -a libiconv-1.14/lib/.libs/libiconv.a ]]; then
  [[ -a libiconv-1.14.tar.gz ]] || \
    wget http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.14.tar.gz
  tar xvfz libiconv-1.14.tar.gz
  cd libiconv-1.14
  ./configure --enable-static --host=arm-linux || die "Configure failed."
  make -j8 || die "Make failed."
  cd ..
fi

./configure --enable-static --host=arm-linux || die "Configure failed."
make -j8 bin/gray-crawler || die "Make failed."
