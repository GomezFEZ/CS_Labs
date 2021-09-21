// Name: Gaomez
// Login ID: Gaomez

/* LRU算法实现
 * 创建S个队列,每个都对应一个高速缓存组,队列的元素为int值,表示高速缓存组中的行的索引,组索引为i对应的队列记为LRU[i]
 * 对于给出的每个地址,处理时会把它指定的第i组高速缓存行索引j入队,即LRU[i].push(j)
 * 注意,这会使得LRU[i]可能含有多个相同的行索引
 * 当需要替换时,选择LRU[i].front()返回的行索引
 * 但是考虑情形:在第i组按顺序访问了第 a b b a 行,这时front返回的是a,这并不是LRU
 * 解决方案是当新的a入队时,把先前已有的a改为无效值-1,即 -1 b b a
 * front()修改为,首先将队首的-1全都出队,直到队首不再是-1,返回队首
 * 这样一来就实现了LRU算法
 */
// 没有完整地进行内存管理!

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cachelab.h"

int hit_count, miss_count, eviction_count;
typedef unsigned long long ULL;
struct queue_node {
    ULL val;
    struct queue_node *nxt, *pre;
};
struct queue {
    struct queue_node *head;
    struct queue_node *end;
};

void simulate(int, int, int, int);
void proc_instr(ULL, ULL, struct queue *, ULL **, int, int);
void push(struct queue *, ULL);
void pop(struct queue *);
ULL front(struct queue *);

int main(int argc, char **argv) {
    // check arguments
    int s, E, b, is_verbose = 0;
    char fname[20];
    for (int i = 1; i < argc; i++) {
        if (!strcmp("-s", argv[i]))
            sscanf(argv[i + 1], "%d", &s);
        else if (!strcmp("-E", argv[i]))
            sscanf(argv[i + 1], "%d", &E);
        else if (!strcmp("-b", argv[i]))
            sscanf(argv[i + 1], "%d", &b);
        else if (!strcmp("-t", argv[i]))
            sscanf(argv[i + 1], "%s", fname);
        else if (!strcmp("-v", argv[i]))
            is_verbose = 1;
    }
    // debug area
    // is_verbose = 0;
    // strcpy(fname, "traces/yi.trace");
    // s = 4, E = 1, b = 4;
    // printf("s=%d,E=%d,b=%d,fname=%s,verbose=%d", s, E, b, fname, is_verbose);
    // getchar(),getchar();

    freopen(fname, "r", stdin);

    simulate(s, E, b, is_verbose);
    printSummary(hit_count, miss_count, eviction_count);

    return 0;
}

void simulate(int s, int E, int b, int v) {
    // initialize
    int S = (int)pow(2, s);
    ULL **cache = (ULL **)malloc(S * sizeof(ULL *));  // cache[S][E]
    for (int i = 0; i < S; i++) {
        cache[i] = (ULL *)malloc(E * sizeof(ULL));
        memset(cache[i], -1, E * sizeof(ULL));
    }
    struct queue *LRU =
        (struct queue *)malloc(S * sizeof(struct queue));  // LRU[S]
    for (int i = 0; i < S; i++) LRU[i].head = LRU[i].end = NULL;

    // process instructions
    char instr, waste[20];
    ULL address;
    while (scanf("%c", &instr) == 1) {
        if (instr != 'M' && instr != 'L' && instr != 'S' && instr != 'I')
            continue;
        scanf("%llx %s", &address, waste);
        if (instr == 'I') continue;
        // done receiving instruction data
        ULL mask = 0x0;
        for (int i = 0; i < b; i++) mask = (mask << 1) + 0x1;
        // ULL b_val = address & mask;
        for (int i = 0; i < s; i++) mask = (mask << 1) + 0x1;
        ULL s_val = (address & mask) >> b;
        ULL t_val = address >> (s + b);
        // process with instr data
        // no need to know is it 'L' or 'S', but to know is it 'M' ?
        if (v) printf("%c %llx,%d ", instr, address, b);
        proc_instr(t_val, s_val, LRU, cache, S, E);
        if (instr == 'M') proc_instr(t_val, s_val, LRU, cache, S, E);
        puts("");
    }
}

void proc_instr(ULL tv, ULL sv, struct queue *lru, ULL **ca, int S, int E) {
    // tv - int value of tag bit; sv - set index
    // 1. check if cache with tv exist
    for (int i = 0; i < E; i++)
        if (ca[sv][i] == tv) {
            // hit
            hit_count++;
            printf("hit ");
            printf(":LRU[%lld][%d]", sv, i);
            push(&lru[sv], i);
            return;
        }

    // 2. not hit, load into cache
    // find empty cache line?
    miss_count++;
    printf("miss ");
    for (int i = 0; i < E; i++)
        if (ca[sv][i] == -1) {
            // found
            ca[sv][i] = tv;
            printf(":LRU[%lld][%d]", sv, i);
            push(&lru[sv], i);
            return;
        }
    // not found, replace lru cache
    eviction_count++;
    printf("eviction ");
    int vict_index = front(&lru[sv]);
    printf(":LRU[%lld][%d]", sv, vict_index);
    pop(&lru[sv]);
    push(&lru[sv], vict_index);
    ca[sv][vict_index] = tv;
}

void push(struct queue *elem, ULL val) {
    if (elem->head == NULL) {
        elem->head = elem->end =
            (struct queue_node *)malloc(sizeof(struct queue_node));
        elem->head->nxt = elem->head->pre = NULL;
        elem->head->val = val;
    } else {
        // set all the existing 'val' in the queue to dummy value '-1'
        // '-1' will be ignored when calling front()
        for (struct queue_node *cur = elem->head; cur != elem->end;
             cur = cur->nxt)
            if (cur->val == val) cur->val = -1;

        // enqueue 'val'
        elem->end->nxt = (struct queue_node *)malloc(sizeof(struct queue_node));
        elem->end->nxt->pre = elem->end;
        elem->end = elem->end->nxt;
        elem->end->val = val;
        elem->end->nxt = NULL;
    }
}

void pop(struct queue *elem) {
    if (elem->end == elem->head) {
        free(elem->head);
        elem->head = NULL;
    } else {
        struct queue_node *tmp = elem->head->nxt;
        free(elem->head);
        elem->head = tmp;
        elem->head->pre = NULL;
    }
}

ULL front(struct queue *elem) {
    while (elem->head->val == -1) pop(elem);
    return elem->head->val;
}

