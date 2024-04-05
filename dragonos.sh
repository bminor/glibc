bash install_deps.sh
bash default_configure.sh
cd build
make -j $(nproc)
DESTDIR=$DADK_CURRENT_BUILD_DIR make install -j $(nproc)