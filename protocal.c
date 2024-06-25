/*
这个文件包含了 DNS 协议的相关函数，
包括提取 DNS 报文的头部、问题部分、回答部分，
创建 DNS 查询报文和 DNS 应答报文，以及将 DNS 报文转换为字节数组。
*/

#include "protocal.h"
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <stdint.h>

void init_segment(Segment *segment){
    segment->id=0;
    segment->QR=0;
    segment->opcode=0;
    segment->AA=0;
    segment->TC=0;
    segment->RD=0;
    segment->RA=0;
    segment->Z=0;
    segment->RCODE=0;
    segment->QDCOUNT=0;
    segment->ANCOUNT=0;
    segment->NSCOUNT=0;
    segment->ARCOUNT=0;
    segment->qname=NULL;
    segment->qtype=0;
    segment->qclass=0;
    segment->rrname=NULL;
    segment->rrtype=0;
    segment->rrclass=0;
    segment->rrttl=0;
    segment->rdlength=0;
    segment->rdata=NULL;
}

void free_segment(Segment *segment) {
    if (segment->qname != NULL) {
        free(segment->qname);
    }
    if (segment->rrname != NULL) {
        free(segment->rrname);
    }
    if (segment->rdata != NULL) {
        free(segment->rdata);
    }
}

void extract_name(char *buffer, int *offset, char *output) {
    int original_offset = *offset;
    int output_offset = 0;
    int len;

    while ((len = buffer[(*offset)++]) != 0) {
        // 检查是否是一个指针
        if ((len & 0xc0) == 0xc0) {
            // 提取指针的偏移量
            int pointer_offset = ((len & 0x3f) << 8) | buffer[(*offset)++];
            // 保存当前的偏移量
            int saved_offset = *offset;
            // 跳转到指针指向的位置
            *offset = pointer_offset;
            // 提取域名
            extract_name(buffer, offset, output + output_offset);
            // 恢复偏移量
            *offset = saved_offset;
            return;
        } else {
            // 提取标签
            memcpy(output + output_offset, buffer + *offset, len);
            output_offset += len;
            *offset += len;
            output[output_offset++] = '.';
        }
    }

    output[output_offset - 1] = '\0';  // 替换最后一个'.'为'\0'
}

void extract_header(Segment *segment,char *buffer){
<<<<<<< HEAD
    segment->id = ((short)(buffer[0]&0xff) << 8) | (buffer[1]&0xff); // 获取 DNS 查询 ID
=======
    segment->id = ((buffer[0]&0xff) << 8) | (buffer[1]&0xff); // 获取 DNS 查询 ID
>>>>>>> b94af2dfa32919ec78fda763533f7693eb045d62
    segment->QR = ((buffer[2]&0xff) >> 7) & 0x01; // 获取 DNS 查询 QR 标志
    segment->opcode = ((buffer[2]&0xff) >> 3) & 0x0F; // 获取 DNS 查询 Opcode
    segment->AA = ((buffer[2]&0xff) >> 2) & 0x01; // 获取 DNS 查询 AA 标志
    segment->TC = ((buffer[2]&0xff) >> 1) & 0x01; // 获取 DNS 查询 TC 标志
    segment->RD = (buffer[2]&0xff) & 0x01; // 获取 DNS 查询 RD 标志
    segment->RA = ((buffer[3]&0xff) >> 7) & 0x01; // 获取 DNS 查询 RA 标志
    segment->Z = ((buffer[3]&0xff) >> 4) & 0x07; // 获取 DNS 查询 Z 标志
    segment->RCODE = (buffer[3]&0xff) & 0x0F; // 获取 DNS 查询 RCODE
<<<<<<< HEAD
    segment->QDCOUNT = ((short)(buffer[4]&0xff) << 8) | (buffer[5]&0xff); // 获取 DNS 查询 QDCOUNT
    segment->ANCOUNT = ((short)(buffer[6]&0xff) << 8) | (buffer[7]&0xff); // 获取 DNS 查询 ANCOUNT
    segment->NSCOUNT = ((short)(buffer[8]&0xff) << 8) | (buffer[9]&0xff); // 获取 DNS 查询 NSCOUNT
    segment->ARCOUNT = ((short)(buffer[10]&0xff) << 8) | (buffer[11]&0xff); // 获取 DNS 查询 ARCOUNT
=======
    segment->QDCOUNT = ((buffer[4]&0xff) << 8) | (buffer[5]&0xff); // 获取 DNS 查询 QDCOUNT
    segment->ANCOUNT = ((buffer[6]&0xff) << 8) | (buffer[7]&0xff); // 获取 DNS 查询 ANCOUNT
    segment->NSCOUNT = ((buffer[8]&0xff) << 8) | (buffer[9]&0xff); // 获取 DNS 查询 NSCOUNT
    segment->ARCOUNT = ((buffer[10]&0xff) << 8) | (buffer[11]&0xff); // 获取 DNS 查询 ARCOUNT
>>>>>>> b94af2dfa32919ec78fda763533f7693eb045d62
}

void extract_question(Segment *segment, char *buffer, int *offset) {
    // 提取QNAME
    char qname[256];
    // int qname_length = 0;
    // while (1) {
    //     unsigned char label_length;
    //     memcpy(&label_length, buffer + *offset, 1);
    //     (*offset)++;
    //     if (label_length == 0) {
    //         break;
    //     }
    //     memcpy(qname + qname_length, buffer + *offset, label_length);
    //     qname_length += label_length;
    //     qname[qname_length] = '.';
    //     qname_length++;
    //     *offset += label_length;
    // }
    // qname[qname_length - 1] = '\0';  // 替换最后一个点为null字符
    extract_name(buffer, offset, qname);
    if(segment->qname!=NULL){
        free(segment->qname);
    }
    segment->qname = strdup(qname);

    // 提取QTYPE
    unsigned short qtype;
    memcpy(&qtype, buffer + *offset, 2);
    *offset += 2;
    segment->qtype = ntohs(qtype);  // 注意网络字节序到主机字节序的转换

    // 提取QCLASS
    unsigned short qclass;
    memcpy(&qclass, buffer + *offset, 2);
    *offset += 2;
    segment->qclass = ntohs(qclass);  // 注意网络字节序到主机字节序的转换
}

void extract_response(Segment *segment, char *buffer, int *offset,int length) {
    // 提取回答部分
    // 使用头部中的ANCOUNT字段来获取应答记录的数量
    int answer_count = segment->ANCOUNT;

    // 处理每个应答记录
    for (int i = 0; i < answer_count; i++) {
        // 提取域名
        segment->rrname=malloc(256);
        extract_name(buffer, offset, segment->rrname);

        // 提取类型
        segment->rrtype = (buffer[*offset] << 8) | buffer[*offset + 1];
        *offset += 2;
        // 提取类
        segment->rrclass = (buffer[*offset] << 8) | buffer[*offset + 1];
        *offset += 2;
        // 提取TTL
        segment->rrttl = (buffer[*offset] << 24) | (buffer[*offset + 1] << 16) | (buffer[*offset + 2] << 8) | buffer[*offset + 3];
        *offset += 4;
        // 提取数据长度
        segment->rdlength = (buffer[*offset] << 8) | buffer[*offset + 1];
        *offset += 2;

        // 处理数据
        if (segment->rrtype == 1) {  // A记录
            // 将数据转换为IP地址格式
            unsigned ip = ntohl(*(unsigned *)(buffer + *offset));
            segment->rdata = malloc(16);
            sprintf(segment->rdata, "%d.%d.%d.%d",
                    ip >> 24 & 0xFF, ip >> 16 & 0xFF, ip >> 8 & 0xFF, ip & 0xFF);
            *offset += 4;
            //我储存一个ip地址
            //用于cache
            return;
        }

        // 跳过数据
        *offset += segment->rdlength;
    }
}

Segment create_response(Segment *query, char *ip) {
    Segment response;
    init_segment(&response);
    // 复制查询报文的部分字段到应答报文
    response.id = query->id;
    response.QR = 1;
    response.opcode = query->opcode;
    response.AA = 0;
    response.TC = query->TC;
    response.RD = query->RD;
    response.RA = 1;
    response.Z = query->Z;
    response.RCODE = 0;
    response.QDCOUNT = query->QDCOUNT;
    response.ANCOUNT = ip != NULL ? 1 : 0;
    response.NSCOUNT = query->NSCOUNT;
    response.ARCOUNT = query->ARCOUNT;
    response.qname = query->qname;
    response.qtype = query->qtype;
    response.qclass = query->qclass;

    // 设置应答报文的答案部分
    if (ip != NULL) {
        response.rrname = query->qname;
        response.rrtype = query->qtype;
        response.rrclass = query->qclass;
        response.rrttl = 3600;
        response.rdlength = 4;
        response.rdata = ip;
    }

    return response;
}

Segment create_error_response(Segment *requestSegment) {
    Segment errorSegment;
    init_segment(&errorSegment);
    // 复制请求报文的id和QDCOUNT字段
    errorSegment.id = requestSegment->id;
    errorSegment.QDCOUNT = requestSegment->QDCOUNT;

    // 设置错误代码为3（名称错误）
    errorSegment.RCODE = 3;

    // 设置其他字段为0
    errorSegment.QR = 1; // 这是一个应答报文
    errorSegment.opcode = 0;
    errorSegment.AA = 0;
    errorSegment.TC = 0;
    errorSegment.RD = 0;
    errorSegment.RA = 0;
    errorSegment.Z = 0;
    errorSegment.ANCOUNT = 0;
    errorSegment.NSCOUNT = 0;
    errorSegment.ARCOUNT = 0;

    // 设置问题部分为请求报文的问题部分
    errorSegment.qname = requestSegment->qname;
    errorSegment.qtype = requestSegment->qtype;
    errorSegment.qclass = requestSegment->qclass;

    // 设置回答部分为空
    errorSegment.rrname = NULL;
    errorSegment.rrtype = 0;
    errorSegment.rrclass = 0;
    errorSegment.rrttl = 0;
    errorSegment.rdlength = 0;
    errorSegment.rdata = NULL;

    return errorSegment;
}

void put16bits(char *buffer, int *offset, int value) {
    // 将整数转换为网络字节序
    short netValue = htons(value);

    // 将网络字节序的整数放入字节数组中
    memcpy(buffer + *offset, &netValue, sizeof(netValue));
    // 更新偏移量
    *offset += sizeof(netValue);
}

void put_string(char *buffer, int *offset, char *value) {
    char *start = value;
    char *end = strchr(value, '.');

    while (end != NULL) {
        // 计算部分的长度，并将其放入字节数组中
        int length = end - start;
        buffer[(*offset)++] = length;

        // 将部分的字符放入字节数组中
        memcpy(buffer + *offset, start, length);
        *offset += length;

        // 移动到下一个部分
        start = end + 1;
        end = strchr(start, '.');
    }

    // 处理最后一个部分
    int length = strlen(start);
    buffer[(*offset)++] = length;
    memcpy(buffer + *offset, start, length);
    *offset += length;

    // 添加结束标记
    buffer[(*offset)++] = 0;
}

void put32bits(char *buffer, int *offset, int value) {
    // 将整数转换为网络字节序

    // 将网络字节序的整数放入字节数组中
    memcpy(buffer + *offset, &value, sizeof(value));

    // 更新偏移量
    *offset += sizeof(value);
}

int convert_segment_to_bytes(Segment *segment, char *buffer) {
    int offset = 0;

    // 头部
    put16bits(buffer, &offset, segment->id);
    put16bits(buffer, &offset, (segment->QR << 15) | (segment->opcode << 11) | (segment->AA << 10) | (segment->TC << 9) | (segment->RD << 8) | (segment->RA << 7) | (segment->Z << 4) | segment->RCODE);
    put16bits(buffer, &offset, segment->QDCOUNT);
    put16bits(buffer, &offset, segment->ANCOUNT);
    put16bits(buffer, &offset, segment->NSCOUNT);
    put16bits(buffer, &offset, segment->ARCOUNT);

    // 问题部分
    put_string(buffer, &offset, segment->qname);
    put16bits(buffer, &offset, segment->qtype);
    put16bits(buffer, &offset, segment->qclass);

    // 答案部分
    if (segment->ANCOUNT > 0) {
        put_string(buffer, &offset, segment->rrname);
        put16bits(buffer, &offset, segment->rrtype);
        put16bits(buffer, &offset, segment->rrclass);
        put32bits(buffer, &offset, segment->rrttl);
        put16bits(buffer, &offset, segment->rdlength);
        put32bits(buffer, &offset, inet_addr(segment->rdata));
    }

    return offset;
}