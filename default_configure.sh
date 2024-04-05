mkdir -p build
mkdir -p install
if [ -z "$ARCH" ] || [ "$ARCH" != "x86_64" ] && [ "$ARCH" != "riscv" ] &&  [ "$ARCH" != "aarch64" ]; then
    echo "No ARCH specified, use x86_64 as default"
    export ARCH="x86_64"
fi
echo "TARGET ARCH is: $ARCH"
export HOST=`uname -m`
echo "HOST ARCH is: $HOST"
cd build
../configure \
  --prefix=/usr \
  --host=${HOST}-linux-gnu \
  --build=${ARCH}-linux-gnu \
  CC="gcc -m64" \
  CXX="g++ -m64" \
  CFLAGS="-O2" \
  CXXFLAGS="-O2"