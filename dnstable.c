/*
这个文件是一个简单的 DNS 表实现，用于将域名映射到 IP 地址。
用于本地 DNS 查询的 DNS 表是一个简单的 trie 树，
每个节点有 128 个子节点，每个子节点对应一个 ASCII 字符。
*/
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dnstable.h"

<<<<<<< HEAD
trie *root;

void init(trie *node){
    for(int i=0;i<128;i++){
        node->next[i]=NULL;
    }
    node->cnt=0;
    node->ttl=0;
}

void insert(char *s, char *ip,int ttl) {
    trie *now=root;
    for (int i = 0; s[i]; i++) {
        if (!now->next[s[i]]) {
            now->next[s[i]] = malloc(sizeof(trie));
            init(now->next[s[i]]);
        }
        now=now->next[s[i]];
        now->cnt++;
    }
    now->cnt++;
    strcpy(now->ip,ip);
    now->ttl=ttl;
}

void erase(char *s) {
    trie *now=root;
    for (int i = 0; s[i]; i++) {
        if (!now->next[s[i]]) {
            return;
        }
        trie *tmp=now;
        now=now->next[s[i]];
        now->cnt--;
        if(tmp->cnt==0){
            free(tmp);
        }
    }
    now->cnt--;
    if(now->cnt==0){
        free(now);
    }
}

char *query(char *s,int nowTime) {
    trie *now=root;
    for (int i = 0; s[i]; i++) {
        if (!now->next[s[i]]) {
            return NULL;
        }
        now=now->next[s[i]];
    }
    if(now->ttl&&now->ttl<nowTime){
        return NULL;
        erase(s);
    }
    return now->ip;
}

void load_dns_table(const char *filename) {
    root = malloc(sizeof(trie));
    init(root);
=======
HashTable *LRUhead,*hashTableHead[mod];
int cntBuffer;

void initHashNode(HashTable *node){
    node->name=NULL;
    node->ip=NULL;
    node->cname=NULL;
    node->ttl=0;
    node->next=node->prev=NULL;
    node->LRUnext=node->LRUprev=NULL;
}

void initHashTable(){
    LRUhead=malloc(sizeof(HashTable));
    LRUhead->LRUnext=LRUhead->LRUprev=LRUhead;
    for(int i=0;i<mod;i++){
        hashTableHead[i]=malloc(sizeof(HashTable));
        initHashNode(hashTableHead[i]);
        hashTableHead[i]->next=hashTableHead[i]->prev=hashTableHead[i];
    }
}

void moveToFront(HashTable *node){
    if(cntBuffer<=2)return;
    node->LRUprev->LRUnext=node->LRUnext;
    node->LRUnext->LRUprev=node->LRUprev;
    node->LRUnext=LRUhead->LRUnext;
    node->LRUprev=LRUhead;
    LRUhead->LRUnext->LRUprev=node;
    LRUhead->LRUnext=node;
}

void removeNode(HashTable *node){
    node->prev->next=node->next;
    node->next->prev=node->prev;
    node->LRUprev->LRUnext=node->LRUnext;
    node->LRUnext->LRUprev=node->LRUprev;
    free(node->name);
    free(node->ip);
    free(node->cname);
    free(node);
}

void insertIp(char *s, char *ip,int ttl) {
    if(cntBuffer>2*mod)removeNode(LRUhead->LRUprev);
    int len=strlen(s);
    int hash=0;
    for(int i=0;i<len;i++){
        hash=(hash*P+s[i])%mod;
    }
    HashTable *now=hashTableHead[hash];
    while(now->next!=hashTableHead[hash]){
        now=now->next;
        if(strcmp(now->name,s)==0){
            now->ip=strdup(ip);
            now->ttl=ttl;
            if(ttl!=-1)moveToFront(now);
            return;
        }
    }
    HashTable *newNode=malloc(sizeof(HashTable));
    initHashNode(newNode);
    newNode->name=strdup(s);
    newNode->ip=strdup(ip);
    newNode->cname=NULL;
    newNode->ttl=ttl;
    newNode->next=hashTableHead[hash];
    newNode->prev=now;
    now->next=newNode;
    hashTableHead[hash]->prev=newNode;
    if(ttl!=-1){
        cntBuffer++;
        newNode->LRUnext=LRUhead->LRUnext;
        newNode->LRUprev=LRUhead;
        LRUhead->LRUnext->LRUprev=newNode;
        LRUhead->LRUnext=newNode;
    }
}

void insertCname(char *s,char *cname,int ttl){
    if(cntBuffer>2*mod)removeNode(LRUhead->LRUprev);
    int len=strlen(s);
    int hash=0;
    for(int i=0;i<len;i++){
        hash=(hash*P+s[i])%mod;
    }
    HashTable *now=hashTableHead[hash];
    while(now->next!=hashTableHead[hash]){
        now=now->next;
        if(strcmp(now->name,s)==0){
            now->cname=strdup(cname);
            now->ttl=ttl;
            if(ttl!=-1)moveToFront(now);
            return;
        }
    }
    HashTable *newNode=malloc(sizeof(HashTable));
    initHashNode(newNode);
    newNode->name=strdup(s);
    newNode->cname=strdup(cname);
    newNode->ttl=ttl;
    newNode->next=hashTableHead[hash];
    newNode->prev=now;
    now->next=newNode;
    hashTableHead[hash]->prev=newNode;
    if(ttl!=-1){
        cntBuffer++;
        newNode->LRUnext=LRUhead->LRUnext;
        newNode->LRUprev=LRUhead;
        LRUhead->LRUnext->LRUprev=newNode;
        LRUhead->LRUnext=newNode;
    }
}

char *queryIp(char *s,int nowTime) {
    int len=strlen(s);
    int hash=0;
    for(int i=0;i<len;i++){
        hash=(hash*P+s[i])%mod;
    }
    HashTable *now=hashTableHead[hash];
    while(now->next!=hashTableHead[hash]){
        now=now->next;
        if(strcmp(now->name,s)==0){
            if((now->ttl)!=-1&&(now->ttl)<nowTime){
                removeNode(now);
                return NULL;
            }
            // moveToFront(now);
            if((now->ip)!=NULL)return now->ip;
            else if((now->cname)!=NULL)return queryIp(now->cname,nowTime);
        }
    }
    return NULL;
}

char *queryCname(char *s,int nowTime){
    int len=strlen(s);
    int hash=0;
    for(int i=0;i<len;i++){
        hash=(hash*P+s[i])%mod;
    }
    HashTable *now=hashTableHead[hash];
    while(now->next!=hashTableHead[hash]){
        now=now->next;
        if(strcmp(now->name,s)==0){
            if((now->ttl)!=-1&&(now->ttl)<nowTime){
                removeNode(now);
                return NULL;
            }
            moveToFront(now);
            return now->cname;
        }
    }
    return NULL;
}

void load_dns_table(const char *filename) {
    initHashTable();
>>>>>>> b94af2dfa32919ec78fda763533f7693eb045d62
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Cannot open file: %s\n", filename);
        return;
    }

    char ip[16];
    char domain[256];
    while (fscanf(file, "%15s %255s", ip, domain) == 2) {
<<<<<<< HEAD
        insert(domain, ip, 0);
=======
        insertIp(domain, ip, -1);
>>>>>>> b94af2dfa32919ec78fda763533f7693eb045d62
    }

    fclose(file);
}