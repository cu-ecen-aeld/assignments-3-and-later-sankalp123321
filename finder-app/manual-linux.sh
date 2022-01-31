#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}
    
    # TODO: Add your kernel build steps here
    echo "1. Deep cleaning the kernel build tree"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    echo "2. Defconfig for the virtual QEMU emulator"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    echo "3. Build kernel to boot with QEMU"
    make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    echo "4. Build kernel modules"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    echo "5. Build device tree"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi 

cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# # TODO: Create necessary base directories
mkdir ${OUTDIR}/rootfs
cd ${OUTDIR}/rootfs
mkdir bin dev etc home lib proc sbin sys tmp usr vir lib64
mkdir usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and insatll busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
cd ${OUTDIR}/rootfs
echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot) 
cp $SYSROOT/lib/ld-linux-aarch64.so.* lib
cp $SYSROOT/lib64/libm.so.* lib64
cp $SYSROOT/lib64/libc.so.* lib64
cp $SYSROOT/lib64/libresolv.so.* lib64


# TODO: Make device nodes
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 600 dev/console c 5 1

# TODO: Clean and build the writer utility
cd ${FINDER_APP_DIR}
echo "Clean and build the writer utility"
make clean
make CROSS_COMPILE=${CROSS_COMPILE}

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
echo "Copying finder scripts to target rootfs"

cp ${FINDER_APP_DIR}/autorun-qemu.sh ${OUTDIR}/rootfs/home
cp ${FINDER_APP_DIR}/conf/ -r ${OUTDIR}/rootfs/home
cp ${FINDER_APP_DIR}/finder.sh ${OUTDIR}/rootfs/home
cp ${FINDER_APP_DIR}/finder-test.sh ${OUTDIR}/rootfs/home
cp ${FINDER_APP_DIR}/writer ${OUTDIR}/rootfs/home

# TODO: Chown the root directory
echo "Changing the ownership to root"
cd ${OUTDIR}/rootfs
sudo chown -R root:root *

# TODO: Create initramfs.cpio.gz
echo "Creating initramfs.cpio.gz"
cd "$OUTDIR/rootfs"
find . | cpio -H newc -ov --owner root:root > ../initramfs.cpio
cd ..
gzip -f initramfs.cpio
