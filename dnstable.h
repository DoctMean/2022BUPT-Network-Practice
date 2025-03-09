#ifndef DNSTABLE_H
#define DNSTABLE_H

#define P 31
#define mod 1000003
typedef struct HashTable{
    /*
    使用hash链表和LRU链表的交叉链表结构
    */
    char *name;
    char *ip;
    char *ipv6;
    char *cname;
    int ttl;
    struct HashTable *next,*prev;
    struct HashTable *LRUnext,*LRUprev;
}HashTable;


void initHashNode(HashTable *node);//初始化节点
void initHashTable();//初始化哈希表
void moveToFront(HashTable *node);//LRU,将节点移动到链表头部
void removeNode(HashTable *node);//删除节点
void insertIp(char *s, char *ip,int ttl);//插入IP记录
void insertIpv6(char *s, char *ip,int ttl);
void insertCname(char *s,char *cname,int ttl);//插入CNAME记录
char *queryIp(char *s,int nowTime);//查询IP记录
char *queryCname(char *s,int nowTime);//查询CNAME记录
char *queryIpv6(char *s,int nowTime);
void load_dns_table(const char *filename);//加载本地DNS表

#endif // DNSTABLE_H