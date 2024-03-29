//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#define HAVE_REMOTE

#include <stdio.h>
#include <winsock.h>
#include "pcap.h"
#include "TAdderThread.h"
#include "THTTPSniffer.h"
#include "WarchaosMapperDecl.h"

#pragma comment (lib, "wpcap.lib")
#pragma comment (lib, "Packet.lib")

#pragma package(smart_init)
//---------------------------------------------------------------------------

#define MAX_PACKET_SIZE    0x10000
#define SIO_RCVALL         0x98000001

//��������� ��������� IP-������
typedef struct IPHeader {
  UCHAR   iph_verlen;         // ������ � ����� ���������
  UCHAR   iph_tos;            // ��� �������
  USHORT  iph_length;         // ����� ����� ������
  USHORT  iph_id;             // �������������
  USHORT  iph_offset;         // ����� � ��������
  UCHAR   iph_ttl;            // ����� ����� ������
  UCHAR   iph_protocol;       // ��������
  USHORT  iph_xsum;           // ����������� �����
  ULONG   iph_src;            // IP-����� �����������
  ULONG   iph_dest;           // IP-����� ����������
} IPHeader;
//��������� ��������� TCP-������
typedef struct TCPHeader {
  USHORT sourcePort;          // ���� �����������
  USHORT destinationPort;     // ���� ����������
  ULONG sequenceNumber;       // ����� ������������������
  ULONG acknowledgeNumber;    // ����� �������������
  UCHAR dataOffset;           // �������� �� ������� ������
  UCHAR flags;                // �����
  USHORT windows;             // ������ ����
  USHORT checksum;            // ����������� �����
  USHORT urgentPointer;       // ���������
} TCPHeader;

__fastcall THTTPSniffer::THTTPSniffer(bool CreateSuspended) : TThread(CreateSuspended)
{
      snifPause = imstopped = false;
      packets = 0;
}
//---------------------------------------------------------------------------
void __fastcall THTTPSniffer::Execute()
{
      WSADATA           wsadata;    // ������������� WinSock.
      SOCKET            s;          // C�������� �����.
      char              name[128];  // ��� ����� (����������).
      HOSTENT* phe;
      SOCKADDR_IN       sa;         // ����� �����
      IN_ADDR           sa1;        //
      unsigned long     flag = 1;   // ���� PROMISC ���/����.

      IPHeader* iphdr;
      TCPHeader* tcphdr;
      int count, datlen, tcphdrlen;
      WORD size;
      bool SYN, FIN, goodpacket;
                                                                                // �������������
      s = NULL;
      Buffer = new char[MAX_PACKET_SIZE];
      AnsiString str;
      try
      {

pcap_if_t *alldevs;
pcap_if_t *d;
int inum;
int i=0;
pcap_t *adhandle;
int res;
char errbuf[PCAP_ERRBUF_SIZE];
struct tm ltime;
char timestr[16];
struct pcap_pkthdr *header;
const u_char *pkt_data;
time_t local_tv_sec;

    /* Retrieve the device list on the local machine */
    if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1) {
        fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
        exit(1);
    }

    /* Print the list */
    for (d=alldevs; d; d=d->next) {
        printf("%d. %s", ++i, d->name);
        if (d->description)
            printf(" (%s)\n", d->description);
        else
            printf(" (No description available)\n");
    }

    if (i==0) {
        printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
//        return -1;
    }

    inum = 2;

    if (inum < 1 || inum > i) {
        printf("\nInterface number out of range.\n");
        /* Free the device list */
        pcap_freealldevs(alldevs);
//        return -1;
    }
    
    /* Jump to the selected adapter */
    for(d=alldevs, i=0; i< inum-1 ;d=d->next, i++);
    
    /* Open the device */
    if ( (adhandle= pcap_open(d->name,          // name of the device
                              65536,            // portion of the packet to capture. 
                                                // 65536 guarantees that the whole packet will be captured on all the link layers
                              PCAP_OPENFLAG_PROMISCUOUS,    // promiscuous mode
                              1000,             // read timeout
                              NULL,             // authentication on the remote machine
                              errbuf            // error buffer
                              ) ) == NULL)
    {
        fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
        /* Free the device list */
        pcap_freealldevs(alldevs);
//        return -1;
    }
    
    printf("\nlistening on %s...\n", d->description);
    
    /* At this point, we don't need any more the device list. Free it */
    pcap_freealldevs(alldevs);
/*
    while((res = pcap_next_ex( adhandle, &header, &pkt_data)) >= 0){

        if(res == 0)
            continue; // Timeout elapsed

        // convert the timestamp to readable format
        local_tv_sec = header->ts.tv_sec;
        localtime_s(&ltime, &local_tv_sec);
        strftime( timestr, sizeof timestr, "%H:%M:%S", &ltime);

        printf("%s,%.6d len:%d\n", timestr, header->ts.tv_usec, header->len);
    }
*/
/*
         bool hh = WSAStartup(MAKEWORD(2,2), &wsadata) == NOERROR;
         s = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
//         gethostname(name, sizeof(name));
//         phe = gethostbyname( name );
         ZeroMemory(&sa, sizeof(sa));
         sa.sin_family = AF_INET;
         sa.sin_addr.s_addr = inet_addr(Host.c_str());
//         sa.sin_addr.s_addr = ((struct in_addr *)phe->h_addr_list[0])->s_addr;
         int he = bind(s, (SOCKADDR *)&sa, sizeof(SOCKADDR));

         ioctlsocket(s, SIO_RCVALL, &flag);                                     // ��������� promiscuous mode.
*/
         while(!Terminated)
         {
            if (snifPause && imstopped) continue;
            if (snifPause) {
               imstopped = true;
               continue;
            }

            res = pcap_next_ex(adhandle, &header, &pkt_data);
            if (res == 0) continue;

            count = header->len;
//            count = recv(s, Buffer, MAX_PACKET_SIZE, 0);
            // ��������� IP-������
            if(count >= (sizeof(IPHeader)))
            {
               Buffer = ((char*) pkt_data) + 14;
               iphdr = (IPHeader *) Buffer;
               if(iphdr->iph_protocol != IPPROTO_TCP) continue;                 // ������ �� ��������� (TCP)
//               str = "";
               //�������� ������ ������...

               str += " To ";
               sa1.s_addr = iphdr->iph_dest;
               str += inet_ntoa(sa1);

               // ����������� � �������� ��� ����� �����������.
               if (FilterByIP && iphdr->iph_src != FilterIPAddress) continue;   // ������ �� IP ������
//               if (FilterByIP && !(iphdr->iph_src == FilterIPAddress || iphdr->iph_dest == FilterIPAddress)) continue;
//               if (FilterByIP && iphdr->iph_dest != inet_addr("192.168.1.11")) continue;
               str += "From ";
               sa1.s_addr = iphdr->iph_src;
               str += inet_ntoa(sa1);
               // ����������� � �������� ��� ����� ����������.
               str += " To ";
               sa1.s_addr = iphdr->iph_dest;
               str += inet_ntoa(sa1);
               // ��������� ��������. ������ ������ ���� �������� ���������� � ����� winsock2.h
               str += " Protocol: TCP";
               // ��������� ������. ��� ��� � ���� ������ ������ ������� ������, � �� ��������, �������� ����� �������
               str += " Size: ";
               size = (iphdr->iph_length << 8) + (iphdr->iph_length >> 8);
               str += IntToStr(size);
               // ��������� ����� ����� ������.
               str += " TTL:";
               str += IntToStr(iphdr->iph_ttl);
               tcphdr = (TCPHeader *)(Buffer+sizeof(IPHeader));
               tcphdrlen = (tcphdr->dataOffset>>4)*4;
               SYN = tcphdr->flags&2;
               FIN = tcphdr->flags&1;

//TMemoryStream* bysh = new TMemoryStream();
//bysh->Write(pkt_data, count);
//bysh->SaveToFile("debug\\packet("+tcphdr->sourcePort+")"+(SYN?"SYN":"")+""+(FIN?"FIN":"")+".bin");

               if (SYN)
               {
                  AdderThread->CreateSocket(tcphdr->destinationPort, tcphdr->sequenceNumber);
               }
               else
               {
                  datlen = size-sizeof(IPHeader)-tcphdrlen;                     // � TCP ������ ����� �� IP ���������� ���� TCP ���������
                  goodpacket = AdderThread->AddPacketsToSocket(tcphdr->destinationPort, tcphdr->sequenceNumber, Buffer+sizeof(IPHeader)+tcphdrlen, datlen, FIN);
                  if (goodpacket) packets++;
               }

            }
         }
      }
      __finally
      {
         if (s) closesocket(s);
         WSACleanup();
         delete[] Buffer;
      }
}
//---------------------------------------------------------------------------
