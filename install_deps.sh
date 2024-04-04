sudo apt-get install -y gcc make gdb
sudo apt-get install -y texinfo gawk bison sed
sudo apt-get install -y python3-dev python3-pip python-is-python3
pip install pexpect
mkdir -p build
mkdir -p install
cd build
../configure \
  --prefix=/usr \
  --host=x86_64-linux-gnu \
  --build=x86_64-linux-gnu \
  CC="gcc -m64" \
  CXX="g++ -m64" \
  CFLAGS="-O2" \
  CXXFLAGS="-O2"