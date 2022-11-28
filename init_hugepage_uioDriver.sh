#sudo su
echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
modprobe uio_pci_generic
ip link set ens192 down
ip link set ens224 down
dpdk-devbind.py -b uio_pci_generic 0b:00.0 13:00.0
dpdk-devbind.py -s
