#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
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
    echo "Cleaning the Kernel"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    echo "Config the kernel -> using default config"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    echo "Building the Kernel Image"
    make -j 4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    echo "Bulding the Module" 
    make -j 4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    echo "Bulding the Device tree"
    make -j 4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"
# Remove the Image if it alreadd exist 
if [ -e ${OUTDIR}/Image ]; then
    rm ${OUTDIR}/Image
fi 
# Then copy it
cp  ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR} 

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

#TODO: Create necessary base directories
mkdir rootfs && cd rootfs
mkdir bin sbin dev etc home lib lib64 proc sys tmp usr var
mkdir -p usr/bin usr/sbin usr/lib
mkdir -p var/log 

cd "$OUTDIR"
#Clone only if the repository does not exist.
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

# TODO: Make and install busybox
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

# TODO: Add library dependencies to rootfs
echo "Add dependent libraries to root filesystem"
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
touch ${OUTDIR}/temp.txt

cd ${OUTDIR}/rootfs
${CROSS_COMPILE}readelf -a bin/busybox | grep -i "Program interpreter" | grep -oP '/[^][]*(?=])' > ${OUTDIR}/temp.txt
${CROSS_COMPILE}readelf -a bin/busybox | grep -i "Shared Library" | grep -oP '\[\K[^][]*(?=])' >> ${OUTDIR}/temp.txt

file=${OUTDIR}/temp.txt
counter=0
while read -r line; do
    echo -e $line
    if [ ${counter} -eq 0 ]; then
        cp -a ${SYSROOT}$line ${OUTDIR}/rootfs/lib/
    else
        cp -a ${SYSROOT}/lib64/$line ${OUTDIR}/rootfs/lib64/
    fi
    counter=$((counter+1))
done <$file
rm -f ${OUTDIR}/temp.txt

# TODO: Make device nodes
echo "Add null, console device node"
cd ${OUTDIR}/rootfs
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 600 dev/console c 5 1

# TODO: Clean and build the writer utility (cross compile)
#then copy it to /home directory in rootfs 
echo "Clean and build the writer uility"
cd ${FINDER_APP_DIR}
if [ -e writer ]; then
    make clean
fi
make CROSS_COMPILE=${CROSS_COMPILE} writer
cp writer ${OUTDIR}/rootfs/home
make clean

# TODO: Copy the finder related scripts and executables
#(finder.sh, conf/username.txt, conf/assignment.txt and finder-test.sh) to the /home directory
# on the target rootfs
echo "Copy the finder related scripts and executables"
cp finder.sh finder-test.sh ${OUTDIR}/rootfs/home 
cp -r conf/ ${OUTDIR}/rootfs/home
cp autorun-qemu.sh ${OUTDIR}/rootfs/home  

# TODO: Chown the root directory
# TODO: Create initramfs.cpio.gz
echo "Remove old initramfs"
cd ${OUTDIR}
if [ -e ${OUTDIR}/initramfs.cpio ];then
    rm -f initramfs.cpio
fi

if [ -e ${OUTDIR}/initramfs.cpio.gz ];then
    rm -f initramfs.cpio.gz
fi
echo "Create initramfs"
cd ${OUTDIR}/rootfs
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ${OUTDIR}
gzip initramfs.cpio
