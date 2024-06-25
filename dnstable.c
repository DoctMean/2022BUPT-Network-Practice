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
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Cannot open file: %s\n", filename);
        return;
    }

    char ip[16];
    char domain[256];
    while (fscanf(file, "%15s %255s", ip, domain) == 2) {
        insert(domain, ip, 0);
    }

    fclose(file);
}