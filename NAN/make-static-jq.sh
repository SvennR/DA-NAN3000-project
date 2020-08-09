#!/bin/sh
echo "******* Building static jq binary *******"
echo ""
echo "Build dependencies:"
echo "flex bison libtool make automake autoconf"
echo ""

sudo apt-get install flex bison libtool make automake autoconf -qq

if [ ! -d "bin-static" ]; then
    mkdir bin-static
fi

if [ -d "jq" ]; then
    echo "Directory \"jq\" already exists!"
    echo "(delete it if you want to clone from GITHUB repo)"
else
    echo "Cloning from GITHUB repo.."
    git clone https://github.com/stedolan/jq.git
fi


cd jq
git submodule update --init # if building from git to get oniguruma
autoreconf -fi              # if building from git
./configure --with-oniguruma=builtin --disable-maintainer-mode
make LDFLAGS=-all-static -j8
make check
if [ -f "jq" ]; then
    echo "\n\n"
    echo "Build succeeded!"
    echo "Verify that jq is statically built.."
    file jq
    ldd jq
    echo "Copying jq binary to projectroot/bin-static/"
    cp jq ../bin-static
    ls -la ../bin-static | grep "jq"
    echo "Deleting jq directory.."
    cd ..
    rm -rf jq 
    echo "Done!"
    return 0
else
    echo "Build failed.."
    return 1
fi