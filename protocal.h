#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef struct Segment{
    // DNS 头部
    int id, QR, opcode, AA, TC, RD, RA, Z, RCODE;
    int QDCOUNT, ANCOUNT, NSCOUNT, ARCOUNT;
    // DNS 问题
    unsigned char *qname;
    unsigned short qtype,qclass;
    // DNS 回答
    unsigned char *rrname;
    unsigned short rrtype, rrclass;
    unsigned int rrttl;
    unsigned short rdlength;
    unsigned char *rdata;
    
} Segment;
void init_segment(Segment *segment);
void free_segment(Segment *segment);
void extract_header(Segment *segment, char *buffer);
void extract_question(Segment *segment, char *buffer, int *offset);
void extract_response(Segment *segment, char *buffer, int *offset,int length);
void put16bits(char *buffer, int *offset, int value);
Segment create_response(Segment *query, char *ip);
Segment create_error_response(Segment *requestSegment);
int convert_segment_to_bytes(Segment *segment, char *buffer);

#endif // PROTOCOL_H