
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h> 
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

/* PCAPNG Block Types */
#define BLOCK_TYPE_SHB  0x0A0D0D0A  /* Section Header Block */
#define BLOCK_TYPE_EPB  0x00000006  /* Enhanced Packet Block */

/* Generic Block Header (Common to all blocks) */
struct block_header {
    uint32_t type;
    uint32_t length; /* Total length of the block including header and trailer */
};

/* Enhanced Packet Block (EPB) specific fields */
/* Structure: [Block Header] [Interface ID] [Timestamp High] [Timestamp Low] [Cap Len] [Orig Len] [Packet Data] [Padding] [Options] [Block Length] */
struct epb_header {
    uint32_t interface_id;
    uint32_t timestamp_high;
    uint32_t timestamp_low;
    uint32_t captured_len;
    uint32_t original_len;
};

void process_packet(unsigned char *buffer, int len) {
    struct ether_header *eth = (struct ether_header *)buffer;
    
    /* Check if it is an IP packet (0x0800) */
    if (ntohs(eth->ether_type) == ETHERTYPE_IP) {
        /* Jump over 14 bytes of Ethernet header to get to IP header */
        struct ip *ip = (struct ip *)(buffer + 14); 
        int ip_header_len = ip->ip_hl * 4;

        /* Check if the protocol inside IP is ICMP (1) */
        if (ip->ip_p == IPPROTO_ICMP) {
            /* Jump over Ethernet + IP header to get to ICMP header */
            struct icmp *icmp = (struct icmp *)(buffer + 14 + ip_header_len);
            
            printf("\n--- Packet Analyzed ---\n");

            /* Layer 2: Ethernet */
            printf("L2 (Ethernet): MAC Src: %02x:%02x:%02x:%02x:%02x:%02x -> MAC Dst: %02x:%02x:%02x:%02x:%02x:%02x\n",
                   eth->ether_shost[0], eth->ether_shost[1], eth->ether_shost[2],
                   eth->ether_shost[3], eth->ether_shost[4], eth->ether_shost[5],
                   eth->ether_dhost[0], eth->ether_dhost[1], eth->ether_dhost[2],
                   eth->ether_dhost[3], eth->ether_dhost[4], eth->ether_dhost[5]);
            printf("               EtherType: 0x%04x\n", ntohs(eth->ether_type));

            /* Layer 3: IP */
            /* Using two printfs for IPs to avoid static buffer overwrite issues with inet_ntoa */
            printf("L3 (IP):       Src: %s", inet_ntoa(ip->ip_src));
            printf(" -> Dst: %s\n", inet_ntoa(ip->ip_dst));
            printf("               Protocol: %d (ICMP) | TTL: %d | Header Len: %d bytes\n", 
                   ip->ip_p, ip->ip_ttl, ip_header_len);

            /* Layer 4: ICMP */
            printf("L4 (ICMP):     Type: %d", icmp->icmp_type);
            if (icmp->icmp_type == ICMP_ECHO) printf(" (Echo Request)");
            else if (icmp->icmp_type == ICMP_ECHOREPLY) printf(" (Echo Reply)");
            printf("\n");
            
            printf("               Code: %d | ID: %d | Seq: %d\n", 
                   icmp->icmp_code, ntohs(icmp->icmp_id), ntohs(icmp->icmp_seq));
        }
    }
}

int main(int argc, char *argv[]) {
    FILE *fp;
    struct block_header bh;
    struct epb_header epb;
    unsigned char buffer[65535]; /* Buffer to hold packet data */
    long block_remaining;

    if (argc != 2) {
        printf("Usage: %s <file.pcapng>\n", argv[0]);
        return 1;
    }

    fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("Error opening file: %s\n", argv[1]);
        return 1;
    }

    printf("--- PCAPNG Packet Analysis ---\n");

    /* Loop through blocks until EOF */
    while (fread(&bh, sizeof(struct block_header), 1, fp) == 1) {
        
        /* PCAPNG blocks are 32-bit aligned. 
           We read the type and length (8 bytes total). 
           'length' includes these 8 bytes. */
        
        if (bh.type == BLOCK_TYPE_EPB) {
            /* Found an Enhanced Packet Block */
            
            /* Read EPB specific fixed fields (20 bytes) */
            fread(&epb, sizeof(struct epb_header), 1, fp);

            /* Read Packet Data */
            if (epb.captured_len > 65535) {
                printf("Packet too large, skipping.\n");
            } else {
                fread(buffer, 1, epb.captured_len, fp);
                process_packet(buffer, epb.captured_len);
            }

            /* Handle Padding: Packet data is padded to 32-bit (4 byte) boundary */
            int padding = (4 - (epb.captured_len % 4)) % 4;
            if (padding > 0) fseek(fp, padding, SEEK_CUR);

            /* Skip Options + Trailer Length.
               Total Read So Far = 8 (Block Header) + 20 (EPB Header) + CapLen + Padding */
            int read_so_far = 8 + 20 + epb.captured_len + padding;
            int remaining_to_skip = bh.length - read_so_far;
            
            if (remaining_to_skip > 0) {
                fseek(fp, remaining_to_skip, SEEK_CUR);
            }

        } else {
            /* Unknown or Irrelevant Block (SHB, IDB, etc.) - Skip it */
            /* We already read 8 bytes (Type + Length), so skip (Length - 8) */
            fseek(fp, bh.length - 8, SEEK_CUR);
        }
    }

    fclose(fp);
    return 0;
}
