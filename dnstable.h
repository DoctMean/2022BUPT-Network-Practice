#ifndef DNSTABLE_H
#define DNSTABLE_H

typedef struct Trie {
    struct Trie *next[128];
    char ip[16];
    int cnt,ttl;
}trie;
void init(trie *node);
void insert(char *s, char *ip,int ttl);
char *query(char *s,int nowTime);
void load_dns_table(const char *filename);

#endif // DNSTABLE_H