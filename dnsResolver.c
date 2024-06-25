#include <stdio.h>
#include <winsock2.h> // 包含 Winsock2 头文件
#include <time.h> // 包含 time.h 头文件

#include "protocal.h" // 包含 DNS 协议头文件
#include "dnstable.h" // 包含 DNS 表头文件

#pragma comment(lib, "ws2_32.lib") // 链接到 ws2_32.lib 库

#define PORT 53 // 定义服务器监听的端口号
#define BUF_SIZE 1024 // 定义接收数据的缓冲区大小
#define TIMELIMIT 1000 //定义等待时间
/*
等待响应的报文列表
*/
typedef struct WaitResponseList{
    Segment segment;
    struct sockaddr_in clientAddr;
    int recved,timer;
}WaitResponseList;
WaitResponseList waitResponseList[1<<10];
int l=0,r=0;
int debug=0;
int main(int argc, char *argv[]) {
    // freopen("debug.txt","w",stdout);
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            debug = 2;
        }
    }

    load_dns_table("dns-table.txt"); // 加载 DNS 表
    WSADATA wsaData; // Winsock 数据结构
    SOCKET serverSocket; // 服务器套接字
    struct sockaddr_in serverAddr, clientAddr, remoteAddr; // 服务器和客户端地址结构
    int clientAddrLen = sizeof(clientAddr); // 客户端地址结构的长度
    char buffer[BUF_SIZE]; // 接收数据的缓冲区

    // 初始化 Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // 使用 2.2 版本的 Winsock
        printf("WSAStartup failed.\n"); // 初始化失败
        while(1);
    }

    // 创建 UDP 套接字
    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // 创建 UDP 套接字
    if (serverSocket == INVALID_SOCKET) { // 套接字创建失败
        printf("Error creating socket: %d\n", WSAGetLastError()); // 输出错误信息
        WSACleanup(); // 清理 Winsock
        while(1);
    }

    // // 定义超时时间
    // DWORD timeout = 1000;  // 超时时间为1000毫秒，即1秒

    // // 设置socket的接收超时选项
    // if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
    //     printf("setsockopt failed with error: %d\n", WSAGetLastError());
    //     WSACleanup();
    //     return 1;
    // }

    // 设置服务器地址
    serverAddr.sin_family = AF_INET; // IPv4 地址
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 使用任意可用 IP 地址
    serverAddr.sin_port = htons(PORT); // 设置端口号，使用 htons 将主机字节序转换为网络字节序

    // 绑定套接字到端口
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) { // 绑定套接字失败
        printf("Bind failed with error: %d\n", WSAGetLastError()); // 输出错误信息
        closesocket(serverSocket); // 关闭套接字
        WSACleanup(); // 清理 Winsock
        while(1);
    }

    // 定义远程 DNS 服务器的地址
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(53); // DNS 服务器通常使用端口 53
    remoteAddr.sin_addr.s_addr = inet_addr("10.3.9.4"); // 远程 DNS 服务器的 IP 地址

    printf("DNS server listening on port %d...\n", PORT); // 输出监听端口信息

    // 接收 DNS 查询并输出
    while (1) {
        int bytesReceived = recvfrom(serverSocket, buffer, BUF_SIZE, 0, (struct sockaddr*)&clientAddr, &clientAddrLen); // 接收数据
        if (bytesReceived == SOCKET_ERROR) { // 接收数据出错
            if(debug)printf("recvfrom failed with error: %d\n", WSAGetLastError()); // 输出错误信息
            continue;
        }
        // 解析 DNS 查询
        Segment segment;
        init_segment(&segment); // 初始化 DNS 报文
        extract_header(&segment, buffer); // 提取 DNS 头部
        int offset = 12; // 偏移量
        extract_question(&segment, buffer, &offset); // 提取 DNS 问题

        if(segment.QR==0){
            //这是一个查询报文
            if(debug)printf("Received DNS query from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port)); // 输出 DNS 查询来源信息
            
            if(debug>1)printf("%s: type %d, class %d\n", segment.qname, segment.qtype, segment.qclass); // 输出 DNS 查询信息
            // 查询 DNS 表
            if(segment.qtype==1){
                char *ip = queryIp(segment.qname,clock()); // 查询 DNS 表
                if (ip != NULL) { // 查询到 IP 地址
                    if(debug>1)printf("Found IP: %s\n", ip); // 输出 IP 地址
                    Segment responseSegment;// 创建 DNS 应答
                    init_segment(&responseSegment); // 初始化 DNS 报文
                    if(strcmp(ip,"0.0.0.0")==0){
                        responseSegment=create_error_response(&segment);
                    }else{
                        responseSegment= create_response(&segment, ip); 
                    } 
                    int bytesSent = convert_segment_to_bytes(&responseSegment, buffer);
                    // 发送应答报文
                    int result = sendto(serverSocket, buffer, bytesSent, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
                    if (result == SOCKET_ERROR) {
                        printf("sendto failed with error: %d\n", WSAGetLastError());
                        free_segment(&responseSegment);
                        free_segment(&segment);
                        continue;
                    }
                } else { // 未查询到 IP 地址
                    if(debug>1)printf("Not found in DNS table\n"); // 输出未查询到信息
                    //在远程服务器上查询
                    // 创建 DNS 查询报文
                    waitResponseList[r].segment=segment;
                    waitResponseList[r].clientAddr=clientAddr;
                    waitResponseList[r].recved=0;
                    waitResponseList[r].timer=clock();
                    segment.id=r;
                    r=(r+1)%1024;
                    
                    // 将 DNS 查询报文转换为字节数组
                    int bytesToSend = convert_segment_to_bytes(&segment, buffer);
                    // 定义远程 DNS 服务器的地址
                    struct sockaddr_in remoteAddr;
                    remoteAddr.sin_family = AF_INET;
                    remoteAddr.sin_port = htons(53); // DNS 服务器通常使用端口 53
                    remoteAddr.sin_addr.s_addr = inet_addr("10.3.9.4"); // 远程 DNS 服务器的 IP 地址
                    
                    // 向远程 DNS 服务器发送查询请求
                    int result = sendto(serverSocket, buffer, bytesToSend, 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr));
                    if (result == SOCKET_ERROR) {
                        printf("sendto failed with error: %d\n", WSAGetLastError());
                        free_segment(&segment);
                        continue;
                    }
                }
            }else if(segment.qtype==28){

            }
            
        }else if(segment.QR==1){
            //这是一个应答报文,来自远程DNS服务器
            if(debug)printf("Received DNS response from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port)); // 输出 DNS 查询来源信息
            if(waitResponseList[segment.id].recved)continue;
            waitResponseList[segment.id].recved=1;
            int fakeOffset=0;
            put16bits(buffer, &fakeOffset, waitResponseList[segment.id].segment.id);
            int bytesSent = bytesReceived;
            //从远程服务器接收到的报文
            //将其中的映射关系添加到cache中
            extract_response(&segment, buffer, &offset, bytesReceived); // 提取 DNS 头部
            //将DNS回答添加到cache中
            if(segment.rdata){
                insertIp(segment.qname,segment.rdata,segment.rrttl*1000+clock());
            }
            // 发送应答报文
            int sendResult = sendto(serverSocket, buffer, bytesSent, 0, (struct sockaddr *)&waitResponseList[segment.id].clientAddr, sizeof(clientAddr));
            if (sendResult == SOCKET_ERROR) {
                printf("sendto failed with error: %d\n", WSAGetLastError());
                free_segment(&segment);
                continue;
            }
        }
        if(r!=l&&clock()-waitResponseList[l].timer>TIMELIMIT){
            //超时
            if(debug)printf("Cant find ip for the domain %s\n",waitResponseList[l].segment.qname);
            waitResponseList[l].recved=1;
            // 创建一个错误应答
            Segment errorSegment = create_error_response(&waitResponseList[l].segment);

            // 转换为字节流
            int bytesSent = convert_segment_to_bytes(&errorSegment, buffer);

            // 发送应答报文
            int sendResult = sendto(serverSocket, buffer, bytesSent, 0, (struct sockaddr *)&waitResponseList[l].clientAddr, sizeof(clientAddr));
            if (sendResult == SOCKET_ERROR) {
                printf("sendto failed with error: %d\n", WSAGetLastError());
                continue;
            }
        }
        while(l!=r&&waitResponseList[l].recved){
            l=(l+1)%1024;
        }
    }
    // 关闭套接字
    closesocket(serverSocket); // 关闭套接字
    WSACleanup(); // 清理 Winsock

    return 0; // 程序正常退出
}
