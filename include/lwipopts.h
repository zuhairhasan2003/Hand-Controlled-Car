#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

#define LWIP_TIMEVAL_PRIVATE 0

// 1. Enable OS Support (FreeRTOS)
#define NO_SYS                      0
#define LWIP_SOCKET                 1
#define LWIP_NETCONN                1

// 2. Threading & Stack Settings
// These sizes are in words/bytes depending on your FreeRTOS port; 
// 2048 is generally safe for the Pico networking background task.
#define TCPIP_THREAD_STACKSIZE      2048
#define DEFAULT_THREAD_STACKSIZE    2048
#define TCPIP_THREAD_PRIO           3

// 3. Mailbox Sizes (Required for Sockets/Netconn)
#define TCPIP_MBOX_SIZE             8
#define DEFAULT_RAW_RECVMBOX_SIZE   8
#define DEFAULT_UDP_RECVMBOX_SIZE   8
#define DEFAULT_TCP_RECVMBOX_SIZE   8
#define DEFAULT_ACCEPTMBOX_SIZE     8

// 4. Memory & Buffer Tuning
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    (16 * 1024)
#define MEMP_NUM_TCP_SEG            32
#define PBUF_POOL_SIZE              24

// 5. TCP/UDP Protocol Settings
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_IPV4                   1

// 6. Access Point / DHCP Settings
// Enable DHCP server support if you want to assign IPs to clients
#define LWIP_UDP                    1
#define LWIP_DHCP                   1

// 7. Synchronization
// Crucial for FreeRTOS: use core locking to prevent race conditions
#define LWIP_TCPIP_CORE_LOCKING     1

#define LWIP_COMPAT_SOCKETS 0

#endif
