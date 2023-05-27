# Mao_DPDK_Framework_Packet_Processing
Packet processing framework based on DPDK, with KNI &amp; interrupt mode PMD supports.

```
# dpdk-stable-20.11.6
# http://fast.dpdk.org/rel/dpdk-20.11.6.tar.xz


sudo su

apt install libnuma-dev pkg-config meson
meson setup build
cd build
ninja
ninja install

echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

echo 1 > /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages
mkdir /hugepages-1G
mount -thugetlbfs hugetlbfs /hugepages-1G -o "pagesize=1G"


# option 1. using uio_pci_generic driver

sudo modprobe uio_pci_generic

sudo dpdk-devbind.py -b uio_pci_generic 0b:00.0 13:00.0
sudo dpdk-devbind.py -s

Network devices using DPDK-compatible driver
============================================
0000:0b:00.0 '82574L Gigabit Network Connection 10d3' drv=uio_pci_generic unused=e1000e,vfio-pci
0000:13:00.0 '82574L Gigabit Network Connection 10d3' drv=uio_pci_generic unused=e1000e,vfio-pci

Network devices using kernel driver
===================================
0000:02:01.0 '82545EM Gigabit Ethernet Controller (Copper) 100f' if=ens33 drv=e1000 unused=vfio-pci,uio_pci_generic *Active*



# option 2. build and install igb_uio driver module
git clone git@github.com:MaoJianwei/dpdk_with_igb_uio_module.git
cd dpdk_with_igb_uio_module/
git submodule update --init --recursive

cd dpdk-kmods-igb_uio/linux/igb_uio/
make
sudo insmod ./igb_uio.ko

# 
```
