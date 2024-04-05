mkdir -p build
mkdir -p install
if [ -z "$ARCH" ] || [ "$ARCH" != "x86_64" ] && [ "$ARCH" != "riscv64" ] &&  [ "$ARCH" != "aarch64" ]; then
    echo "No ARCH specified, use x86_64 as default"
    export ARCH="x86_64"
fi
if [ "$ARCH" == "x86_64" ] || [ "$ARCH" == "riscv64" ] ||  [ "$ARCH" == "aarch64" ]; then
    export TRIPLET=${ARCH}-linux-gnu
fi
export BUILD=`uname -m`
cd build
../configure \
  --prefix=/ \
  --host=${TRIPLET} \
  --build=${BUILD}-linux-gnu \
  CC="${TRIPLET}-gcc -m64" \
  CXX="${TRIPLET}-g++ -m64" \
  CFLAGS="-O2" \
  CXXFLAGS="-O2"