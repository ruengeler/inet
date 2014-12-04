//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2009 Thomas Reschka
// Copyright (C) 2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <errno.h>

#include "inet/common/packet/PcapDump.h"
#include "inet/common/serializer/pcap.h"

#include "inet/networklayer/common/IPProtocolId_m.h"

#include "inet/common/serializer/headerserializers/ethernet/EthernetSerializer.h"
#include "inet/common/serializer/headerserializers/ieee80211/Ieee80211Serializer.h"

#ifdef WITH_UDP
#include "inet/transportlayer/udp/UDPPacket_m.h"
#endif // ifdef WITH_UDP

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/common/serializer/ipv4/IPv4Serializer.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#include "inet/common/serializer/ipv6/IPv6Serializer.h"
#endif // ifdef WITH_IPv6

namespace inet {

using namespace serializer;

PcapDump::PcapDump()
{
    dumpfile = NULL;
}

PcapDump::~PcapDump()
{
    closePcap();
}

void PcapDump::openPcap(const char* filename, unsigned int snaplen_par, unsigned int linktype)
{
    struct pcap_hdr fh;

    if (!filename || !filename[0])
        throw cRuntimeError("Cannot open pcap file: file name is empty");

    dumpfile = fopen(filename, "wb");

    if (!dumpfile)
        throw cRuntimeError("Cannot open pcap file [%s] for writing: %s", filename, strerror(errno));

    snaplen = snaplen_par;

    fh.magic = PCAP_MAGIC;
    fh.version_major = 2;
    fh.version_minor = 4;
    fh.thiszone = 0;
    fh.sigfigs = 0;
    fh.snaplen = snaplen;
    fh.network = linktype;
    fwrite(&fh, sizeof(fh), 1, dumpfile);
}

void PcapDump::writeFrame(simtime_t stime, const IPv4Datagram *ipPacket)
{
    if (!dumpfile)
        throw cRuntimeError("Cannot write frame: pcap output file is not open");

#ifdef WITH_IPv4
    uint8 buf[MAXBUFLENGTH];
    memset((void *)&buf, 0, sizeof(buf));

    struct pcaprec_hdr ph;

    simtime_t stime_usec;
    int64 temp_sec;
    stime.split(SIMTIME_S, temp_sec, stime_usec);
    ph.ts_sec = (int32)temp_sec;
    ph.ts_usec = stime_usec.inUnit(SIMTIME_US);

     // Write Ethernet header
    uint32 hdr = 2; //AF_INET

    int32 serialized_ip = serializer::IPv4Serializer().serialize(ipPacket, buf, sizeof(buf), true);
    ph.orig_len = serialized_ip + sizeof(uint32);

    ph.incl_len = ph.orig_len > snaplen ? snaplen : ph.orig_len;
    fwrite(&ph, sizeof(ph), 1, dumpfile);
    fwrite(&hdr, sizeof(uint32), 1, dumpfile);
    fwrite(buf, ph.incl_len - sizeof(uint32), 1, dumpfile);
#else
    throw cRuntimeError("Cannot write frame: INET compiled without IPv4 feature");
#endif
}

void PcapDump::writeEtherFrame(simtime_t stime, const EthernetIIFrame *etherPacket)
{
    if (!dumpfile)
        throw cRuntimeError("Cannot write frame: pcap output file is not open");

    uint8 buf[MAXBUFLENGTH];
    memset((void *)&buf, 0, sizeof(buf));

    struct pcaprec_hdr ph;

    simtime_t stime_usec;
    int64 temp_sec;
    stime.split(SIMTIME_S, temp_sec, stime_usec);
    ph.ts_sec = (int32)temp_sec;
    ph.ts_usec = stime_usec.inUnit(SIMTIME_US);

    int32 serialized_ethernet = EthernetSerializer().serialize(etherPacket, buf, sizeof(buf));
    if (serialized_ethernet > 0) {
        ph.orig_len = serialized_ethernet;

        ph.incl_len = ph.orig_len > snaplen ? snaplen : ph.orig_len;
        fwrite(&ph, sizeof(ph), 1, dumpfile);
        fwrite(buf, ph.incl_len, 1, dumpfile);
    }
}

void PcapDump::writeIeee80211Frame(simtime_t stime, Ieee80211Frame *ieee80211Packet)
{
    if (!dumpfile)
        throw cRuntimeError("Cannot write frame: pcap output file is not open");

    uint8 buf[MAXBUFLENGTH];
    memset((void*)&buf, 0, sizeof(buf));

    struct pcaprec_hdr ph;

    simtime_t stime_usec;
    int64 temp_sec;
    stime.split(SIMTIME_S, temp_sec, stime_usec);
    ph.ts_sec = (int32)temp_sec;
    ph.ts_usec = stime_usec.inUnit(SIMTIME_US);

    int32 serialized_ieee80211 = Ieee80211Serializer().serialize(ieee80211Packet, buf, sizeof(buf));
    if (serialized_ieee80211 > 0) {
        ph.orig_len = serialized_ieee80211;

        ph.incl_len = ph.orig_len > snaplen ? snaplen : ph.orig_len;
        fwrite(&ph, sizeof(ph), 1, dumpfile);
        fwrite(buf, ph.incl_len, 1, dumpfile);
    }
}

void PcapDump::closePcap()
{
    if (dumpfile) {
        fclose(dumpfile);
        dumpfile = NULL;
    }
}

} // namespace inet

