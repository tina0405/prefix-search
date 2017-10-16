#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "testbench.h"
#include "tst.h"
#define FILE_REF "out_ref.txt"
/** constants insert, delete, max word(s) & stack nodes */
enum { INS, DEL, WRDMAX = 256, STKMAX = 512, LMAX = 1024 };
#define REF INS
#define CPY DEL
#define POOL_SIZE
/* timing helper function */

typedef struct pool {
    char * next;
    char * tail;
} MEMPOOL;

MEMPOOL *pool_initial(size_t size)
{
    MEMPOOL * p = (MEMPOOL*)malloc(size);
    p->next = (char *) calloc(1, size);
    p->tail = p->next + size;
    return p;
}

void pool_free(MEMPOOL *p)
{
    free(p);
}

void *pool_access(MEMPOOL *p,size_t size)
{
    if( p->tail - p->next < size )
        return NULL;
    void *tmp = (void*)p->next;
    p->next += size;
    return tmp;
}

static double tvgetf(void)
{
    struct timespec ts;
    double sec;

    clock_gettime(CLOCK_REALTIME, &ts);
    sec = ts.tv_nsec;
    sec /= 1e9;
    sec += ts.tv_sec;

    return sec;
}

/* simple trim '\n' from end of buffer filled by fgets */
static void rmcrlf(char *s)
{
    size_t len = strlen(s);
    if (len && s[len - 1] == '\n')
        s[--len] = 0;
}

#define IN_FILE "cities_3000.txt"

int main(int argc, char **argv)
{
    char word[WRDMAX]="";
    char *sgl[LMAX] = {NULL};
    tst_node *root = NULL, *res = NULL;
    MEMPOOL* poolmemory = pool_initial(POOL_SIZE);
    int rtn = 0, idx = 0, sidx = 0;
    FILE *fp = fopen(IN_FILE, "r");
    double t1, t2;
    int b_flag = 0;
    if(argc>1 ) {
        if(!strcmp("--bench", argv[1]))
            b_flag = 1;
        else
            b_flag = 2;
    }
    if (!fp) { /* prompt, open, validate file for reading */
        fprintf(stderr, "error: file open failed '%s'.\n", argv[1]);
        return 1;
    }
    t1 = tvgetf();
    while ((rtn = fscanf(fp, "%s", word)) != EOF) {
        char *p = pool_access( poolmemory , sizeof(word));
        /* FIXME: insert reference to each string */
        if (!tst_ins_del(&root, &p, INS, REF)) {
            fprintf(stderr, "error: memory exhausted, tst_insert.\n");
            fclose(fp);
            return 1;
        }
        idx++;
    }
    t2 = tvgetf();

    fclose(fp);
    printf("ternary_tree, loaded %d words in %.6f sec\n", idx, t2 - t1);

    for (;;) {
        printf(
            "\nCommands:\n"
            " a  add word to the tree\n"
            " f  find word in tree\n"
            " s  search words matching prefix\n"
            " d  delete word from the tree\n"
            " q  quit, freeing all data\n\n"
            "choice: ");
        if(b_flag == 1) {
            strcpy(word , argv[2]);
        } else if (b_flag == 2) {
            strcpy(word , argv[1]);
        } else {
            fgets(word, sizeof word, stdin);
        }
        printf("%s\n",word);
        switch (*word) {
            char *p = NULL;
        case 'a':
            printf("enter word to add: ");
            if(b_flag == 1) {
                //tst_ins_del_bench
                break;
            } else if (b_flag == 2) {
                strcpy(word , argv[2]);
            } else {
                if (!fgets(word, sizeof word, stdin)) {
                    fprintf(stderr, "error: insufficient input.\n");
                    break;
                }
            }
            printf("%s\n", word);
            rmcrlf(word);
            p = word;
            t1 = tvgetf();
            /* FIXME: insert reference to each string */
            res = tst_ins_del(&root, &p, INS, REF);
            t2 = tvgetf();
            if (res) {
                idx++;
                printf("  %s - inserted in %.6f sec. (%d words in tree)\n",
                       (char *) res, t2 - t1, idx);
            } else
                printf("  %s - already exists in list.\n", (char *) res);
            break;
        case 'f':
            printf("find word in tree: ");
            if(b_flag == 1) {
                //tst_search_bench
                break;
            } else if (b_flag == 2) {
                strcpy(word , argv[2]);
            } else {
                if (!fgets(word, sizeof word, stdin)) {
                    fprintf(stderr, "error: insufficient input.\n");
                    break;
                }
            }
            printf("%s\n", word);
            rmcrlf(word);
            t1 = tvgetf();
            res = tst_search(root, word);
            t2 = tvgetf();
            if (res)
                printf("  found %s in %.6f sec.\n", (char *) res, t2 - t1);
            else
                printf("  %s not found.\n", word);
            break;
        case 's':
            printf("find words matching prefix (at least 1 char): ");
            if(b_flag == 1) {
                prefix_search_testbench(root,FILE_REF);
                break;
            } else if (b_flag == 2) {
                strcpy(word , argv[2]);
            } else {
                if (!fgets(word, sizeof word, stdin)) {
                    fprintf(stderr, "error: insufficient input.\n");
                    break;
                }
            }
            printf("%s\n", word);
            rmcrlf(word);
            t1 = tvgetf();
            res = tst_search_prefix(root, word, sgl, &sidx, LMAX);
            t2 = tvgetf();
            if (res) {
                printf("  %s - searched prefix in %.6f sec\n\n", word, t2 - t1);
                for (int i = 0; i < sidx; i++)
                    printf("suggest[%d] : %s\n", i, sgl[i]);
            } else
                printf("  %s - not found\n", word);
            break;
        case 'd':
            printf("enter word to del: ");
            if(b_flag == 1) {
                //tst_ins_del_bench
                break;
            } else if (b_flag == 2) {
                strcpy(word , argv[2]);
            } else {
                if (!fgets(word, sizeof word, stdin)) {
                    fprintf(stderr, "error: insufficient input.\n");
                    break;
                }
            }
            rmcrlf(word);
            p = word;
            printf("  deleting %s\n", word);
            t1 = tvgetf();
            /* FIXME: remove reference to each string */
            res = tst_ins_del(&root, &p, DEL, REF);
            t2 = tvgetf();
            if (res)
                printf("  delete failed.\n");
            else {
                printf("  deleted %s in %.6f sec\n", word, t2 - t1);
                idx--;
            }
            break;
        case 'q':
            tst_free_all(root);
            return 0;
            break;
        default:
            fprintf(stderr, "error: invalid selection.\n");
            break;
        }
        if(b_flag) {
            pool_free(poolmemory);
            break;
        }
    }

    return 0;
}


