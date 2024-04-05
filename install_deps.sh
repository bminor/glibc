sudo apt-get install -y make gdb
sudo apt-get install -y texinfo gawk bison sed
sudo apt-get install -y python3-dev python-is-python3
if [ -z "$ARCH" ] || [ "$ARCH" != "x86_64" ] && [ "$ARCH" != "riscv" ] &&  [ "$ARCH" != "aarch64" ]; then
    echo "No ARCH specified, use x86_64 as default"
    export ARCH="x86_64"
fi

if [ "$ARCH" == "riscv" ] ||  [ "$ARCH" == "aarch64" ]; then
    export TRIPLET=${ARCH}-linux-gnu
fi

if [ "$ARCH" == "x86_64" ] ; then
    export TRIPLET=x86-64-linux-gnu
fi

sudo apt-get install -y gcc-${TRIPLET}