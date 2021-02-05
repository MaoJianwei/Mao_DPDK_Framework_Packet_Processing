cmake_minimum_required(VERSION 3.10)
project(Mao_DPDK_Framework_Packet_Processor)

set(CMAKE_CXX_STANDARD 11)

include_directories(src)


add_executable(
        Mao_DPDK_Framework_Packet_Processor
        src/main.c
        src/include/MaoConstant.h
        #Demo/main.c
        #Demo/IpSrTunnel/MaoIpRouteTableLPM.c
        #Demo/IpSrTunnel/MaoIpRouteTableLPM.h
        #Demo/IpSrTunnel/MaoMplsTunnel.h
        #Demo/SegmentRouting/mplsForwardTable.c
        #Demo/SegmentRouting/mplsForwardTable.h
        #Demo/RouterParam/PortConfigure.h
        #Demo/RouterParam/PortLcoreBinding.h
        #Demo/PacketInspector/packetInspector.c Demo/PacketInspector/packetInspector.h dpdk-example/mainpower.c Demo/mainnew.c)
        # dpdk-example/main.c
        src/include/main.h)
