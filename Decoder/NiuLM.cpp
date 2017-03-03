/* NiuTrans - SMT platform
 * Copyright (C) 2011, NEU-NLPLab (http://www.nlplab.com/). All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

/*
 * $Id:
 * language model; OurLM.cpp
 * The following macros change the behavior of this library
 * (1). LMTRAIN_LIB
 *      determines whether this file is used as a standalone library or a lib
 * (2). CACHE_ENABLE
 *      determines whether the cache mechanism is enabled or not, cache is
 *      used when this file is used as a lib
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Rushan Chen (email: chenrsster@gmail.com)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); June 19th, 2011; fix bugs in "MKVOCAB" process
 * Ji Ma (email: majineu@gmail.com); June 13th, 2011; fix bugs about using "va_list"
 *
 */


#include "NiuLM.h"
#define LMTRAIN_LIB

#ifndef WIN32
#define _FILE_OFFSET_BITS 64
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#else /* WIN32 */
#include <windows.h>
#endif

namespace niu_lm
{


#define ISSPACE(c)      (strchr(WORDDELIM, (c)) ? 1 : 0)


#define LMSIGNATURE     0x19801201

#define HTABMEMBLKSZ    (2 * 1024 * 1024)
#define HTABSZ          10000
#define GROW_THRESH     0.75
#define GROW_FACTOR     1.414
#define WSEQMEMBLKSZ    (4 * 1024 * 1024)
#define WORDDELIM       " \r\n\t"
#define INFOSTEP        (1024 * 1024)


typedef int (*mapfunc_t)(void *key, size_t klen, void *val,
                                size_t vlen, va_list vl);
typedef void (*ngram_func_t)(int *ngram, int *count, int last,
                                int is_cutoff, va_list va);
typedef long long offset_t;

enum action {
        MKVOCAB,
        CNTNGRRAM,
        MKMODEL,
};

struct memblock {
        void *mem;
        size_t size;
        size_t used;            /* how many bytes have been used */
        struct memblock *next;
};

/* --------------------------------------------------------------------------- */
/*                                util functions                               */
/* --------------------------------------------------------------------------- */

static void
read_one_line(FILE *fp, char **buf, int *buf_len)
{
        if (! *buf) {
                *buf_len = 20;
                *buf = (char *)calloc(*buf_len, sizeof(char));
                if (! *buf) {
                        fprintf(stderr, "[ERROR]: out of memory\n");
                        exit(1);
                }
        }

        if (! fgets(*buf, *buf_len, fp)) {
                if (ferror(fp)) {
                        fprintf(stderr, "[ERROR]: read file\n");
                        exit(1);
                }
                (*buf)[0] = '\0';
                return;
        }

        while ((*buf)[strlen(*buf) - 1] != '\n') {
                char *p = NULL;
                int len = *buf_len + 1;
                *buf_len *= 2;
                *buf = (char *)realloc(*buf, *buf_len);
                if (! *buf) {
                        fprintf(stderr, "[ERROR]: out of memory\n");
                        exit(1);
                }
                p = *buf + strlen(*buf);
                if (! fgets(p, len, fp)) {
                        if (ferror(fp)) {
                                fprintf(stderr, "[ERROR]: read file\n");
                                exit(1);
                        } else if (feof(fp)) {
                                return;
                        }
                }
        }
}

static size_t
get_line_word_cnt(const char *line)
{
        const char *q = line;
        size_t cnt = 0;

        while (*q != '\0') {
                if (! ISSPACE(*q)) {
                        cnt++;
                        while (! ISSPACE(*q) && *q != '\0')
                                q++;
                }
                while (ISSPACE(*q) && *q != '\0')
                        q++;
        }

        return cnt;
}

#ifdef WIN32

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

/* 
 * From http://www.suacommunity.com/dictionary/gettimeofday-entry.php#id36065483
 * the second argument is not used
 */
int gettimeofday(struct timeval *tv, void *NULLptr)
{
        FILETIME ft;

        unsigned __int64 tmpres = 0;
        static int tzflag = 0;

        if (tv != NULL) {
                GetSystemTimeAsFileTime(&ft);

                tmpres |= ft.dwHighDateTime;
                tmpres <<= 32;
                tmpres |= ft.dwLowDateTime;
                tmpres /= 10;
                tmpres -= DELTA_EPOCH_IN_MICROSECS;

                tv->tv_sec = (long)(tmpres / 1000000UL);
                tv->tv_usec = (long)(tmpres % 1000000UL);
        }

        return 0;
}

int
timersub(struct timeval *end, struct timeval *beg, struct timeval *sub)
{
    if (end->tv_usec >= beg->tv_usec) {
        sub->tv_sec = end->tv_sec - beg->tv_sec;
        sub->tv_usec = end->tv_usec - beg->tv_usec;
    } else {
        sub->tv_usec = (long)1e6 + end->tv_usec - beg->tv_usec;
        sub->tv_sec = end->tv_sec - (long)1.0 - beg->tv_sec;
    }
    return 0;
}

#endif /* WIN32 */

static void *
mmap_file(const char *fn, fsize_t filesz, fsize_t &memsize)
{
#ifndef WIN32
        void *file = NULL;
        int fd = 0, err = 0;

        if (filesz == 0) {
                struct stat statbuf;

                fd = open(fn, O_RDONLY);
                if (fd < 0) {
                        fprintf(stderr, "[ERROR]: open file %s\n", fn);
                        exit(1);
                }
                err = fstat(fd, &statbuf);
                if (err < 0) {
                        fprintf(stderr, "[ERROR]: stat file %s\n", fn);
                        exit(1);
                }
                filesz = statbuf.st_size;
                file = mmap(NULL, filesz, PROT_READ, MAP_SHARED, fd, 0);
                if (file == MAP_FAILED) {
                        fprintf(stderr, "[ERROR]: mmap file %s\n", fn);
                        exit(1);
                }
        } else { /* create file */
                char c = '\0';
                fd = open(fn, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                if (fd < 0) {
                        fprintf(stderr, "[ERROR]: open file %s\n", fn);
                        exit(1);
                }
                /* extend file to filesz
                 */
                lseek(fd, filesz - 1, SEEK_SET);
                if (write(fd, &c, 1) != 1) {
                        fprintf(stderr, "[ERROR]: write file %s\n", fn);
                        exit(1);
                }
                file = mmap(NULL, filesz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                if (file == MAP_FAILED) {
                        fprintf(stderr, "[ERROR]: mmap file\n");
                        exit(1);
                }
        }
        return file;
#else /* WIN32 */
        HANDLE fd = NULL, map = NULL;
        void *file = NULL;
        wchar_t *wfn = NULL;
        int wfn_len = 0;

        wfn_len = (int)mbstowcs(NULL, fn, strlen(fn));
        wfn = (wchar_t *)calloc(wfn_len + 1, sizeof(*wfn));
        if (! wfn) {
            fprintf(stderr, "[ERROR]: out of memory");
            exit(1);
        }
        mbstowcs(wfn, fn, strlen(fn));
                
        if (filesz > 0) {
                remove(fn);
                fd = CreateFileW(wfn, GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
                if (fd == (HANDLE)(-1)) {
                        fprintf(stderr, "[ERROR]: CreateFile");
                        exit(1);
                }
                int t[1];
                t[0] = filesz >> 32;
                if (SetFilePointer(fd, (int)(filesz & 0xFFFFFFFF), (PLONG)t,
                                   FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
                    fprintf(stderr, "[ERROR]: SetFilePointer\n");
                    exit(1);
                }
                if (SetEndOfFile(fd) == 0) {
                        fprintf(stderr, "[ERROR]: SetEndOfFile\n");
                        exit(1);
                }
        } else {
                fd = CreateFileW(wfn, GENERIC_READ, FILE_SHARE_READ, NULL,
                                OPEN_EXISTING, 0, NULL);
                if (fd == (HANDLE)(-1)) {
                        fprintf(stderr, "[ERROR]: CreateFile\n");
                        exit(1);
                }
        }

        map = CreateFileMapping(fd, NULL,
                                filesz > 0 ? PAGE_READWRITE : PAGE_READONLY,
                                0, 0, NULL);
        if (! map) {
                fprintf(stderr, "[ERROR]: CreateFileMapping\n");
                exit(1);
        }

        file = MapViewOfFile(map, filesz > 0 ? FILE_MAP_WRITE : FILE_MAP_READ,
                             0, 0, 0);
        if (! file) {
                fprintf(stderr, "[ERROR]: MapViewOfFile");
                exit(1);
        }

        return file;
#endif /* WIN32 */
}

/* --------------------------------------------------------------------------- */
/*                                   hashtable                                 */
/* --------------------------------------------------------------------------- */

struct hashnode {
        void *key;
        void *val;
        size_t key_len;
        size_t val_len;
        struct hashnode *next;
};

struct hashtable {
        struct hashnode **htab;
        size_t size;
        size_t nnodes;
        struct memblock *memblk;
};

/*
 * <from coreutils-7.6/lib/hash.c>
 * Return true if CANDIDATE is a prime number.  CANDIDATE should be an odd
 * number at least equal to 11.
 */
static int
is_prime(size_t candidate)
{
        size_t divisor = 3;
        size_t square = divisor * divisor;

        while (square < candidate && (candidate % divisor)) {
                divisor++;
                square += 4 * divisor;
                divisor++;
        }

        return (candidate % divisor ? 1 : 0);
}

/*
 * <from coreutils-7.6/lib/hash.c>
 * Round a given CANDIDATE number up to the nearest prime, and return that
 * prime.  Primes lower than 10 are merely skipped.
 */
static size_t
next_prime(size_t candidate)
{
        /* Skip small primes.
         */
        if (candidate < 10)
                candidate = 10;

        /* Make it definitely odd.
         */
        candidate |= 1;

        while (((size_t)-1) != candidate && !is_prime (candidate))
                candidate += 2;

        return candidate;
}

static size_t
hashfunc(const void *bytes, size_t len)
{
        unsigned hval = 0;
        unsigned g = 0;
        const char *str = (const char *) bytes;
        size_t l = 0;

        hval = 0;
        while (l < len) {
                l++;
                hval <<= 4;
                hval += (unsigned long) *str++;
                g = hval & ((unsigned long) 0xf << (32 - 4));
                if (g != 0) {
                        hval ^= g >> (32 - 8);
                        hval ^= g;
                }
        }
        
        return hval;
}

static int
htab_memblk_new(struct hashtable *htab, size_t sz)
{
        size_t memsz = sizeof(struct memblock) + sz;
        struct memblock *m = NULL;

        m = (struct memblock *)calloc(1, memsz);
        if (! m)
                return -1;
        m->mem = (void *)(m + 1);
        m->size = sz;
        m->next = htab->memblk;
        htab->memblk = m;

        return 0;
}

static int
htab_grow(struct hashtable *htab)
{
        size_t newsz = next_prime((size_t)(htab->size * GROW_FACTOR));
        struct hashnode **newhtab = NULL;
        int i = 0;

        if (newsz < htab->size) /* overflow? */
                return -1;

        newhtab = (struct hashnode **)calloc(newsz, sizeof *newhtab);
        if (! newhtab)
                return -1;

        for (i = 0; i < htab->size; i++) {
                struct hashnode *pn = NULL, *qn = NULL;
                for (pn = htab->htab[i]; pn; pn = qn) {
                        qn = pn->next;
                        size_t bkt = hashfunc(pn->key, pn->key_len) % newsz;
                        pn->next = newhtab[bkt];
                        newhtab[bkt] = pn;
                }
        }

        free(htab->htab);
        htab->htab = newhtab;
        htab->size = newsz;

        return 0;
}

int
htab_new(struct hashtable **phtab)
{
        if (! phtab)
                return -1;

        struct hashtable *h = NULL;
        int err = 0;

        *phtab = NULL;

        h = (struct hashtable *)calloc(1, sizeof *h);
        if (! h)
                return -1;
        h->size = next_prime(HTABSZ);
        h->htab = (struct hashnode **)calloc(h->size, sizeof *h->htab);
        if (! h->htab) {
                free(h);
                return -1;
        }
        err = htab_memblk_new(h, HTABMEMBLKSZ);
        if (err < 0) {
                free(h->htab);
                free(h);
                return -1;
        }

        *phtab = h;

        return 0;
}

int
htab_size(struct hashtable *htab)
{
        if (! htab)
                return 0;
        return (int)(htab->nnodes);
}

int
htab_insert(struct hashtable *htab, void *key, size_t klen, void *val, size_t vlen)
{
        if (! htab)
                return -1;

        size_t bkt = hashfunc(key, klen) % htab->size;
        size_t memsz = 0;
        int err = 0;
        struct hashnode *pn = NULL;
        struct memblock *memblk = htab->memblk;

        memsz = sizeof(struct hashnode) + klen + vlen;
        if (memsz > HTABMEMBLKSZ)
                err = htab_memblk_new(htab, memsz);
        else if (memsz > memblk->size - memblk->used)
                err = htab_memblk_new(htab, HTABMEMBLKSZ);
        if (err < 0)
                return -1;

        memblk = htab->memblk;
        pn = (struct hashnode *)((char *)(memblk->mem) + memblk->used);
        pn->key = (void *)(pn + 1);
        pn->val = (char *)(pn->key) + klen;
        pn->key_len = klen;
        pn->val_len = vlen;
        memcpy(pn->key, key, klen);
        memcpy(pn->val, val, vlen);

        memblk->used += memsz;

        pn->next = htab->htab[bkt];
        htab->htab[bkt] = pn;
        htab->nnodes++;

        float r = (float)(htab->nnodes) / htab->size;
        if (r > GROW_THRESH) {
                err = htab_grow(htab);
                if (err < 0)
                        return -1;
        }

        return 0;
}

/* 
 * when no such <key> existing in htab, 1 is return
 */
int
htab_search(struct hashtable *htab, const void *key, size_t klen, void **val)
{
        if (! htab)
                return -1;

        int bkt = (int)(hashfunc(key, klen) % htab->size);
        struct hashnode *pn = NULL;

        for (pn = htab->htab[bkt]; pn; pn = pn->next) {
                if (pn->key_len != klen)
                        continue;
                if (! memcmp(key, pn->key, klen)) {
                        if (val)
                                *val = pn->val;
                        return 0;
                }
        }

        if (val)
                *val = NULL;
        return 1;
}

int
htab_map(struct hashtable *htab, mapfunc_t mf, ...)
{
        struct hashnode *pn = NULL;
        int i = 0, err = 0;
        va_list vl;

        for (i = 0; i < htab->size; i++) {
                for (pn = htab->htab[i]; pn; pn = pn->next) {
            va_start(vl, mf);
                        err = mf(pn->key, pn->key_len, pn->val, pn->val_len, vl);
            va_end(vl);
                        if (err < 0)
                                return err;
                }
        }

        return 0;
}

void
htab_free(struct hashtable **phtab)
{
        if (! phtab || ! *phtab)
                return;
        struct memblock *memblk = NULL, *pm = NULL;
        struct hashtable *htab = *phtab;
        
        for (memblk = htab->memblk; memblk; memblk = pm) {
                pm = memblk->next;
                free(memblk);
        }
        free(htab->htab);
        free(htab);
        *phtab = NULL;
}

/* --------------------------------------------------------------------------- */
/*                                   mkvocab                                   */
/* --------------------------------------------------------------------------- */

/* 
 * <vl>: int cutoff, FILE *ofp
 */
static int
map_dumpvocab(void *key, size_t klen, void *val, size_t vlen, va_list vl)
{
        int cutoff = 0;
        int freq = *(int *)val;
        char *word = (char *)key;
        FILE *ofp = NULL;

        cutoff = va_arg(vl, int);
        ofp = va_arg(vl, FILE *);

        if (freq > cutoff)
                fprintf(ofp, "%s\n", word);

        return 0;
}

static void
mkvocab(const char *ifn, const char *ofn, int cutoff)
{
        FILE *ifp = stdin, *ofp = stdout;
        int err = 0, nlines = 0, i = 0, line_len = 0;
        char *line = NULL;
        struct hashtable *htab = NULL;

        if (ifn) {
                ifp = fopen(ifn, "r");
                if (! ifp) {
                        fprintf(stderr, "[ERROR]: open %s\n", ifn);
                        exit(1);
                }
        }
        if (ofn) {
                ofp = fopen(ofn, "w");
                if (! ofp) {
                        fprintf(stderr, "[ERROR]: open %s\n", ofn);
                        exit(1);
                }
        }

        err = htab_new(&htab);
        if (err < 0) {
                fprintf(stderr, "[ERROR]: htab new error\n");
                exit(1);
        }

        while (1) {
                read_one_line(ifp, &line, &line_len);
                if (line[0] == '\0')
                        break;
                if (line[strlen(line) - 1] == '\n')
                        line[strlen(line) - 1] = '\0';
                if (line[strlen(line) - 1] == '\r')
                        line[strlen(line) - 1] = '\0';

                char *word = NULL;
                int *pfreq = NULL;

                word = strtok(line, WORDDELIM);
                if (! word)
                        continue;
                do {
                        err = htab_search(htab, word, strlen(word) + 1,
                                          (void **)&pfreq);
                        if (err < 0) {
                                fprintf(stderr, "[ERROR]: search htab\n");
                                exit(1);
                        } else if (err == 1) { /* no such word */
                                int freq = 1;
                                err = htab_insert(htab, word, strlen(word) + 1,
                                                  &freq, sizeof freq);
                                if (err < 0) {
                                        fprintf(stderr, "[ERROR]: htab insert\n");
                                        exit(1);
                                }
                        } else {
                                (*pfreq)++;
                        }
                        word = strtok(NULL, WORDDELIM);
                } while (word);

                if( ++nlines % 100000 == 0 )
                    fprintf(stderr, "\b..");
                int tmpC = nlines / 10000;
                if(tmpC % 4 == 0)
                    fprintf(stderr, "\b/");
                else if(tmpC % 4 ==1)
                    fprintf(stderr, "\b|");
                else if(tmpC % 4 == 2)
                    fprintf(stderr, "\b\\");
                else
                    fprintf(stderr, "\b-");
        }
        fprintf(stderr, "\b.\n");
        if( ifp != stdin )
            fclose( ifp );

        const char *xx[] = {"<s>", "</s>", "<unk>"};
        for (i = 0; i < sizeof xx / sizeof xx[0]; i++) {
                err = htab_search(htab, xx[i], strlen(xx[i]) + 1, NULL);
                if (err < 0) {
                        fprintf(stderr, "[ERROR]: htab search\n");
                        exit(1);
                } else if (err == 1) {
                        fprintf(ofp, "%s\n", xx[i]);
                }
        }
        htab_map(htab, map_dumpvocab, cutoff, ofp);
        if( ofp != stdout )
            fclose( ofp );
        free(line);
        htab_free( &htab );
}

static void
ldvocab(const char *ifn, struct hashtable **phtab)
{
        struct hashtable *htab = NULL;
        FILE *ifp = stdin;
        char *line = NULL;
        int err = 0, wid = 0, line_len = 0;

        if (ifn) {
                ifp = fopen(ifn, "r");
                if (! ifp) {
                        fprintf(stderr, "[ERROR]: open file %s\n", ifn);
                        exit(1);
                }
        }
        err = htab_new(&htab);
        if (err < 0) {
                fprintf(stderr, "[ERROR]: htab new\n");
                exit(1);
        }

        while (1) {
                read_one_line(ifp, &line, &line_len);
                if (line[0] == '\0')
                        break;
                if (line[strlen(line) - 1] == '\n')
                        line[strlen(line) - 1] = '\0';
                if (line[strlen(line) - 1] == '\r')
                        line[strlen(line) - 1] = '\0';

                err = htab_search(htab, line, strlen(line) + 1, NULL);
                if (err < 0) {
                        fprintf(stderr, "[ERROR]: htab search\n");
                        exit(1);
                }
                if (err == 0) {
                        fprintf(stderr, "[ERROR]: duplicate words in vocabulary\n");
                        exit(1);
                }
                err = htab_insert(htab, line, strlen(line) + 1, &wid, sizeof wid);
                if (err < 0) {
                        fprintf(stderr, "[ERROR]: htab insert\n");
                        exit(1);
                }
                wid++;
        }

        *phtab = htab;
        free(line);
        if (ifp != stdin)
                fclose(ifp);
}

/* --------------------------------------------------------------------------- */
/*                                   cntngram                                  */
/* --------------------------------------------------------------------------- */

struct ngram {
        int *start;
        int *start_bak;         /* used when sorting ngrams */
        int wid;                /* just for speed improvement */
};

struct parfn {
        char fn[256];           /* the name of the partial ngram-count file */
        struct parfn *next;
};

struct parfbuf {
        FILE *fp;
        int *ngram;
        int N;
        int count;
};

static void
wseq_free(struct memblock **pwseq)
{
        struct memblock *p = *pwseq, *q = NULL;

        for (; p != NULL; p = q) {
                q = p->next;
                free(p);
        }
        *pwseq = NULL;
}

static int *
wseq_add(struct memblock *wseq, int wid)
{
        int *p = (int *)((char *)(wseq->mem) + wseq->used);
        *p = wid;
        wseq->used += sizeof(wid);
        return p;
}

static void
wseq_alloc_memblk_if_needed(struct memblock **pwseq, size_t sz, int nwords)
{
        struct memblock *mb = NULL;
        size_t memsz = 0;
        
        mb = *pwseq;
        if (mb && (mb->size - mb->used) / sizeof(int) >= nwords)
                return;

        /* allocate new memory block
         */
        memsz = sizeof(*mb) + sz;
        mb = (struct memblock *)calloc(1, memsz);
        if (! mb) {
                fprintf(stderr, "[ERROR]: out of memory\n");
                exit(1);
        }
        mb->mem = (void *)(mb + 1);
        mb->size = sz;
        mb->next = *pwseq;
        *pwseq = mb;
}

/* 
 * Determine (roughly of course) the maximum number of ngrams that can be handled
 * at once given the memory available
 * <memsz> is in unit of Megabyte
 */
static size_t
ngram_max(size_t memsz)
{
        size_t m = (size_t)(memsz * 1024 * 1024 * 0.8 / (sizeof(struct ngram) + 4));
        if (m > INT_MAX)
                return INT_MAX;
        return m;
}

static void
ngram_sort(struct ngram *ngram_arr, size_t ngram_arr_sz, int wcnt, int N)
{
        int *count = NULL, *offset = NULL;
        int i = 0, j = 0, wid = 0;

        count = (int *)calloc(wcnt, sizeof(int));
        offset = (int *)calloc(wcnt, sizeof(int));
        if (! count || ! offset) {
                fprintf(stderr, "[ERROR]: out of memory\n");
                exit(1);
        }

        //fprintf(stderr, "Sorting ngram ... ");
        for (i = N - 1; i >= 0; i--) {
                memset(count, 0, sizeof(int) * wcnt);
                memset(offset, 0, sizeof(int) * wcnt);

                for (j = 0; j < ngram_arr_sz; j++) {
                        wid = *(ngram_arr[j].start + i);
                        ngram_arr[j].wid = wid;
                        ngram_arr[j].start_bak = ngram_arr[j].start;
                        count[wid]++;
                }

                for (j = 1; j < wcnt; j++)
                        offset[j] = offset[j - 1] + count[j - 1];

                for (j = 0; j < ngram_arr_sz; j++) {
                        wid = ngram_arr[j].wid;
                        ngram_arr[offset[wid]].start = ngram_arr[j].start_bak;
                        offset[wid]++;
                }
        }
        //fprintf(stderr, "done\n");

        free(count);
        free(offset);
}

static void
ngram_dump(struct ngram *ngram_arr, size_t ngram_arr_sz, int N, const char *ofn)
{
        FILE *ofp = stdout;
        int i = 0, j = 0;
        int *ngrambuf = NULL, *ngramcnt = NULL, *ngramcur = NULL;
        fsize_t filesz = 0;

        if (ofn) {
                ofp = fopen(ofn, "wb");
                if (! ofp) {
                        fprintf(stderr, "[ERROR]: open file %s\n", ofn);
                        exit(1);
                }
        }
        ngrambuf = (int *)calloc(3 * N, sizeof(*ngrambuf));
        if (! ngrambuf) {
                fprintf(stderr, "[ERROR]: out of memory\n");
                exit(1);
        }
        ngramcnt = ngrambuf + N;
        ngramcur = ngramcnt + N;

        for (i = 0; i < N; i++) {
                ngrambuf[i] = *(ngram_arr[0].start + i);
                ngramcnt[i] = 1;
        }
        for (i = 1; i < ngram_arr_sz; i++) {
                int diffat = -1;

                for (j = 0; j < N; j++)
                        ngramcur[j] = *(ngram_arr[i].start + j);
                for (j = 0; j < N; j++) {
                        if (ngramcur[j] != ngrambuf[j]) {
                                diffat = j;
                                break;
                        } else {
                                ngramcnt[j]++;
                        }
                }
                if (diffat == -1)
                        continue;
                /* output in binary format
                 */
                fwrite(ngrambuf, sizeof(ngrambuf[0]), N, ofp);
                fwrite(ngramcnt + N - 1, sizeof(ngramcnt[0]), 1, ofp);
                int sz = N * sizeof(ngrambuf[0]) + sizeof(ngramcnt[0]);
                filesz += sz;
                /*if (filesz % INFOSTEP < sz)
                        fprintf(stderr, "\rDumping to %s %lldM ...",
                                ofn ? ofn : "stdout", filesz / INFOSTEP);*/

                for (j = diffat; j < N; j++) {
                        ngrambuf[j] = ngramcur[j];
                        ngramcnt[j] = 1;
                }
        }
        /* output in binary format
         */
        fwrite(ngrambuf, sizeof(ngrambuf[0]), N, ofp);
        fwrite(ngramcnt + N - 1, sizeof(ngramcnt[0]), 1, ofp);
        /*fprintf(stderr, "\rDumping to %s %lldM ... done\n", ofn ? ofn : "stdout",
                                                filesz / INFOSTEP);*/

        free(ngrambuf);
        if (ofp != stdout)
                fclose(ofp);
}

static int
parfbuf_cmp(const void *_pb1, const void *_pb2)
{
        struct parfbuf *pb1 = (struct parfbuf *)_pb1;
        struct parfbuf *pb2 = (struct parfbuf *)_pb2;
        int i = 0;

        if (feof(pb1->fp))
                return 1;
        if (feof(pb2->fp))
                return -1;

        for (i = 0; i < pb1->N; i++) {
                if (pb1->ngram[i] > pb2->ngram[i])
                        return 1;
                else if (pb1->ngram[i] < pb2->ngram[i])
                        return -1;
        }

        return 0;
}

static void
ngram_merge(struct parfn *pflst, int npf, int N, const char *ofn)
{
        struct parfbuf *pfbuf = NULL;
        struct parfn *pfn = pflst;
        int *ngrambuf = NULL, count = 0, i = 0, err = 0;
        FILE *ofp = stdout;
        fsize_t filesz = 0;

        /* remove is necessary on Window$, Window$ fails to
         * rename if the destination file already exists
         */
        remove(ofn);

        if (npf == 1) {
                if (ofn) {
                        //fprintf(stderr, "Rename %s to %s\n", pflst->fn, ofn);
                        err = rename(pflst->fn, ofn);
                        if (err) {
                                fprintf(stderr, "[ERROR]: rename %s to %s\n",
                                                pflst->fn, ofn);
                                exit(1);
                        }
                }
                return;
        }

        if (ofn) {
                ofp = fopen(ofn, "wb");
                if (! ofp) {
                        fprintf(stderr, "[ERROR]: open file %s\n", ofn);
                        exit(1);
                }
        }

        pfbuf = (struct parfbuf *)calloc(1, sizeof(*pfbuf) * npf +
                                            sizeof(*(pfbuf->ngram)) * N * npf);
        ngrambuf = (int *)calloc(N, sizeof(*ngrambuf));
        if (! pfbuf || ! ngrambuf) {
                fprintf(stderr, "[ERROR]: out of memroy\n");
                exit(1);
        }
        /* initialization
         */
        for (i = 0; i < npf; i++) {
                int *base = (int *)(pfbuf + npf);
                pfbuf[i].ngram = base + N * i;
                pfbuf[i].N = N;
                pfbuf[i].fp = fopen(pfn->fn, "rb");
                if (! pfbuf[i].fp) {
                        fprintf(stderr, "[ERROR]: open file %s\n", pfn->fn);
                        exit(1);
                }
                pfn = pfn->next;
        }
        /* read in first ngram of each partial file
         */
        for (i = 0; i < npf; i++) {
                fread(pfbuf[i].ngram, sizeof(*pfbuf[i].ngram), N, pfbuf[i].fp);
                fread(&pfbuf[i].count, sizeof(pfbuf[i].count), 1, pfbuf[i].fp);
        }
        qsort(pfbuf, npf, sizeof(*pfbuf), parfbuf_cmp);
        memcpy(ngrambuf, pfbuf[0].ngram, sizeof(*ngrambuf) * N);
        count = pfbuf[0].count;

        while (1) {
                fread(pfbuf[0].ngram, sizeof(*pfbuf[0].ngram), N, pfbuf[0].fp);
                fread(&pfbuf[0].count, sizeof(pfbuf[0].count), 1, pfbuf[0].fp);

                /* sort <pfbuf>
                 */
                struct parfbuf t = pfbuf[0];
                int pos = 0;
                for (i = 1; i < npf; i++) {
                        if (parfbuf_cmp(&t, pfbuf + i) < 0) {
                                pos = i - 1;
                                break;
                        }
                }
                if (i == npf)
                        pos = npf - 1;
                if (pos != 0)
                        memmove(pfbuf, pfbuf + 1, pos * sizeof(struct parfbuf));
                pfbuf[pos] = t;

                if (feof(pfbuf[0].fp)) /* no more ngrams ? */
                        break;

                if (! memcmp(pfbuf[0].ngram, ngrambuf, N * sizeof(*ngrambuf))) {
                        count += pfbuf[0].count;
                } else {
                        fwrite(ngrambuf, sizeof(*ngrambuf), N, ofp);
                        fwrite(&count, sizeof(count), 1, ofp);
                        int sz = N * sizeof(ngrambuf[0]) + sizeof(count);
                        filesz += sz;
                        /*if (filesz % INFOSTEP < sz)
                                fprintf(stderr, "\rMerge %lldM ... ",
                                        filesz / INFOSTEP);*/

                        /* update ngrambuf
                         */
                        memcpy(ngrambuf, pfbuf[0].ngram, sizeof(*ngrambuf) * N);
                        count = pfbuf[0].count;
                }
        }
        fwrite(ngrambuf, sizeof(*ngrambuf), N, ofp);
        fwrite(&count, sizeof(count), 1, ofp);
        //fprintf(stderr, "\rMerge %lldM ... done\n", filesz / INFOSTEP);

        if (ofp != stdout)
                fclose(ofp);
        for (i = 0; i < npf; i++)
                fclose(pfbuf[i].fp);
        free(pfbuf);
        free(ngrambuf);
}

static void
cntngram(const char *vfn, const char *ifn, const char *ofn,
         int N, int cutoff, int memsz)
{
        struct memblock *wseq = NULL;
        struct ngram *ngram_arr = NULL;
        size_t max_ngrams = ngram_max(memsz);
        struct hashtable *htab = NULL;
        char *line = NULL, *word = NULL;
        FILE *ifp = stdin;
        int err = 0, unkid = 0, osid = 0, csid = 0, *pid = NULL;
        const char *unk = "<unk>", *os = "<s>", *cs = "</s>";
        int ngram_idx = 0, nlines = 0, nparfn = 0, line_len = 0;
        const char *parfn_prefix = "ngcnt_";
        struct parfn *pflst = NULL;

        /* initialization
         */
        ldvocab(vfn, &htab);
        ngram_arr = (struct ngram *)calloc(max_ngrams, sizeof(*ngram_arr));
        if (! ngram_arr) {
                fprintf(stderr, "[ERROR]: out of memory\n");
                exit(1);
        }

        /* get word id of <unk>, <s>, </s>
         */
        err = htab_search(htab, unk, strlen(unk) + 1, (void **)&pid);
        if (err < 0) {
                fprintf(stderr, "[ERROR]: htab search\n");
                exit(1);
        } else if (err == 1) {
                fprintf(stderr, "[ERROR]: no <unk> in vocabulary\n");
                exit(1);
        }
        unkid = *pid;
        err = htab_search(htab, os, strlen(os) + 1, (void **)&pid);
        if (err == 1) {
                fprintf(stderr, "[ERROR]: no <s> in vocabulary\n");
                exit(1);
        }
        osid = *pid;
        err = htab_search(htab, cs, strlen(cs) + 1, (void **)&pid);
        if (err == 1) {
                fprintf(stderr, "[ERROR]: no </s> in vocabulary\n");
                exit(1);
        }
        csid = *pid;

        if (ifn) {
                ifp = fopen(ifn, "r");
                if (! ifp) {
                        fprintf(stderr, "[ERROR]: open file %s\n", ifn);
                        exit(1);
                }
        }

        while (1) {
                int is_lastline = 0;
                int wc = 0, widx = 0, *pos = NULL, wid = 0, i = 0;

                read_one_line(ifp, &line, &line_len);
                if (line[0] == '\0')
                        is_lastline = 1;
                if (! is_lastline && line[strlen(line) - 1] == '\n')
                        line[strlen(line) - 1] = '\0';
                if (! is_lastline && line[strlen(line) - 1] == '\r')
                        line[strlen(line) - 1] = '\0';

                if (! is_lastline) {
                        wc = (int)get_line_word_cnt(line);
                        if (wc == 0)
                                continue;
                }
                /* dump current n-grams
                 */
                if (is_lastline || ngram_idx + wc + 1 > max_ngrams) {
                        /*if (is_lastline)
                                fprintf(stderr, "\r%d lines read ... done\n",
                                                                nlines);
                        else
                                fprintf(stderr, "\r%d lines read ... part %d\n",
                                                        nlines, nparfn + 1);*/
                        /* construct partial file name list
                         */
                        struct parfn *pf = NULL;
                        pf = (struct parfn *)calloc(1, sizeof(*pf));
                        if (! pf) {
                                fprintf(stderr, "[ERROR]: out of memory\n");
                                exit(1);
                        }
                        nparfn++;
                        sprintf(pf->fn, "%s%03d", parfn_prefix, nparfn);
                        pf->next = pflst;
                        pflst = pf;

                        ngram_sort(ngram_arr, ngram_idx, htab_size(htab), N);
                        ngram_dump(ngram_arr, ngram_idx, N, pf->fn);

                        /* make some clear
                         */
                        ngram_idx = 0;
                        wseq_free(&wseq);

                        if (is_lastline)
                                break;
                }

                wc += N; /* 1 for <s>, N - 1 for </s> */
                wseq_alloc_memblk_if_needed(&wseq, WSEQMEMBLKSZ, wc);

                wseq_add(wseq, osid);
                widx++;
                word = strtok(line, WORDDELIM);
                do {
                        err = htab_search(htab, word, strlen(word) + 1,
                                                        (void **)&pid);
                        wid = err == 1 ? unkid : *pid;
                        pos = wseq_add(wseq, wid);
                        widx++;
                        if (widx >= N)
                                ngram_arr[ngram_idx++].start = pos - N + 1;
                } while ((word = strtok(NULL, WORDDELIM)));

                /* append N - 1 </s>
                 */
                for (i = 1; i <= N - 1; i++) {
                        pos = wseq_add(wseq, csid);
                        widx++;
                        if (widx >= N)
                                ngram_arr[ngram_idx++].start = pos - N + 1;
                }

                if( ++nlines % 100000 == 0 )
                    fprintf(stderr, "\b..");

                int tmpC = nlines / 10000;
                if(tmpC % 4 == 0)
                    fprintf(stderr, "\b/");
                else if(tmpC % 4 ==1)
                    fprintf(stderr, "\b|");
                else if(tmpC % 4 == 2)
                    fprintf(stderr, "\b\\");
                else
                    fprintf(stderr, "\b-");
        }
        fprintf(stderr, "\b.\n");

        ngram_merge(pflst, nparfn, N, ofn);

        /* free resource
         */
        struct parfn *p = NULL, *q = NULL;
        if (nparfn > 1) {
                for (p = pflst; p; p = p->next)
                        remove(p->fn);
        }
        for (p = pflst; p; p = q) {
                q = p->next;
                free(p);
        }
        htab_free(&htab);
        free(ngram_arr);
        free(line);
        if (ifp != stdin)
                fclose(ifp);
}

/* --------------------------------------------------------------------------- */
/*                                   mkmodel                                   */
/* --------------------------------------------------------------------------- */

struct model_header {
        char mark[1024];
        int signature;
        int order;
        int unkid;
};

struct inner_node {
        int wid;
        float prob;
        float backoff;
        int link;
};

struct final_node {
        int wid;
        float prob;
};

struct sptags{          /* special tags */
        int osid;
        int csid;
        int unkid;
};
 
static int
is_good_ngram(int *ngram, int last, struct sptags *stags)
{
        int i = 0;

        if (last == 0)
                return 1;
        if (ngram[last - 1] == stags->csid)
                return 0;
        for (i = 0; i <= last; i++)
                if (ngram[i] == stags->unkid)
                        return 0;
        return 1;
}

static void
do_each_ngram(const char *cfn, int N, int *cutoff, struct sptags *stags,
                        const char *msg_prefix, ngram_func_t ngfun, ...)
{
        FILE *cfp = NULL;
        int *ngramcnt = NULL, *ngrambuf = NULL, *ngramcur = NULL;
        int *cutofftype = NULL; /* number of types of ngram being cut off */
        int i = 0, count = 0, diffat = 0, is_cutoff = 0;
        fsize_t readbytes = 0, recordsize = 0;
        va_list va;


        cfp = fopen(cfn, "rb");
        if (! cfp) {
                fprintf(stderr, "[ERROR]: open file %s\n", cfn);
                exit(1);
        }
        ngramcnt = (int *)calloc(4 * N, sizeof(*ngramcnt));
        if (! ngramcnt) {
                fprintf(stderr, "[ERROR]: out of memory\n");
                exit(1);
        }
        ngrambuf = ngramcnt + N;
        ngramcur = ngrambuf + N;
        cutofftype = ngramcur + N;

        /* read in 1st ngram
         */
        if (fread(ngrambuf, sizeof(*ngrambuf), N, cfp) != N) {
                if (! feof(cfp))
                        fprintf(stderr, "[ERROR]: read %s\n", cfn);
                else
                        fprintf(stderr, "[ERROR]: ngram count file empty!\n");
                exit(1);
        }
        if (fread(&count, sizeof(count), 1, cfp) != 1) {
                if (! feof(cfp))
                        fprintf(stderr, "[ERROR]: read %s\n", cfn);
                else
                        fprintf(stderr, "[ERROR]: ngram count file format error\n");
                exit(1);
        }
        for (i = 0; i < N; i++)
                ngramcnt[i] = count;
        readbytes = sizeof(*ngrambuf) * N + sizeof(count);
        recordsize = readbytes;

        while (fread(ngramcur, sizeof(*ngramcur), N, cfp)) {
                if (fread(&count, sizeof(count), 1, cfp) != 1) {
                        if (! feof(cfp))
                                fprintf(stderr, "[ERROR]: read %s\n", cfn);
                        else
                                fprintf(stderr, "[ERROR]: ngram count file "
                                                "format error\n");
                        exit(1);
                }
                readbytes += sizeof(*ngramcur) * N + sizeof(count);

                diffat = -1;
                for (i = 0; i < N; i++) {
                        if (ngramcur[i] < ngrambuf[i]) {
                                fprintf(stderr, "[ERROR]: ngram not in correct "
                                                "order\n");
                                exit(1);
                        }
                        if (ngramcur[i] != ngrambuf[i]) {
                                diffat = i;
                                break;
                        }
                        ngramcnt[i] += count;
                }
                if (diffat == -1) {
                        fprintf(stderr, "[ERROR]: duplicated ngram found\n");
                        exit(1);
                }
                for (i = N - 1; i >= diffat; i--) {
                        if (! is_good_ngram(ngrambuf, i, stags)) {
                                ngrambuf[i] = ngramcur[i];
                                ngramcnt[i] = count;
                                continue;
                        }
                        if (ngramcnt[i] <= cutoff[i] && i != 0) {
                                cutofftype[i]++;
                                is_cutoff = 1;
                        }

                va_start(va, ngfun);
                        ngfun(ngrambuf, ngramcnt, i, is_cutoff, va);
                va_end(va);

                        ngrambuf[i] = ngramcur[i];
                        ngramcnt[i] = count;
                        is_cutoff = 0;
                }
                /*if (readbytes % INFOSTEP < recordsize)
                        fprintf(stderr, "\r%s: %lldM processed ...", msg_prefix,
                                                        readbytes / INFOSTEP);*/
        }
        if (! feof(cfp)) {
                fprintf(stderr, "[ERROR]: read %s\n", cfn);
                exit(1);
        }
        /* handle last ngram
         */
        for (i = N - 1; i >= 0; i--) {
                if (! is_good_ngram(ngrambuf, i, stags))
                        continue;
                if (ngramcnt[i] <= cutoff[i] && i != 0) {
                        cutofftype[i]++;
                        is_cutoff = 1;
                }

        va_start(va, ngfun);
                ngfun(ngrambuf, ngramcnt, i, is_cutoff, va);
        va_end(va);

                is_cutoff = 0;
        }
        /*fprintf(stderr, "\r%s: %lld bytes processed ... done\n",
                                        msg_prefix, readbytes);*/

        free(ngramcnt);
        fclose(cfp);
}

/* 
 * <va>: ngramtype, Nr, gtmax
 */
static void
_stat_ngram_count(int *ngram, int *count, int last, int is_cutoff, va_list va)
{
        int *ngramtype = NULL, **Nr = NULL, *gtmax = NULL;

        ngramtype = va_arg(va, int *);
        Nr = va_arg(va, int **);
        gtmax = va_arg(va, int *);

        if (! is_cutoff)
                ngramtype[last]++;
        if (count[last] <= gtmax[last] + 1)
                Nr[last][count[last]]++;
}

static void
stat_ngram_count(const char *cfn, int *ngramtype, int **Nr, int *gtmax,
                              int *cutoff, struct sptags *stags, int N)
{
        do_each_ngram(cfn, N, cutoff, stags, "stat", _stat_ngram_count,
                                                ngramtype, Nr, gtmax);
}

static void
create_model_file(const char *mfn, int *ngramtype, int N, int unkid,
                  struct inner_node **inode, struct final_node **pfnode)
{
        fsize_t memsz = 0, tmp = 0;
        int i = 0;
        char *model = NULL;

        /* get the size of model file
         */
        memsz += sizeof(struct model_header);   /* header section */
        memsz += sizeof(int) * N;               /* ngram count section */
        memsz += sizeof(offset_t) * N;          /* ngram offset section */
        for (i = 0; i < N - 1; i++)             /* 1 to N-1 gram section */
                memsz += sizeof(struct inner_node) * (ngramtype[i] + 1);
        memsz += sizeof(struct final_node) * ngramtype[N - 1]; /* N gram section */
        /*fprintf(stderr, "Create model file %lldM (%lld Bytes)\n",
                                memsz / (1024 * 1024), memsz);*/
        
        /* create model file
         */
        model = (char *)mmap_file(mfn, memsz, tmp);

        /* fill header section
         */
        struct model_header *hdr = (struct model_header *)model;
        sprintf( hdr->mark, "Copyright (C) 2011, NEU-NLPLab (http://www.nlplab.com/). All rights reserved." );
        char *pBeg = (char*)hdr->mark + strlen( hdr->mark ) + 1;
        char *pEnd = (char*)(&hdr->signature);
        srand( (unsigned int)time( NULL ) );
        while( pBeg != pEnd ) {
            *pBeg = (char)(rand()%256);
            ++pBeg;
        }
        hdr->signature = LMSIGNATURE;
        hdr->order = N;
        hdr->unkid = unkid;

        /* fill ngram count section
         */
        int *ngtype = (int *)(hdr + 1);
        for (i = 0; i < N; i++)
                ngtype[i] = ngramtype[i];

        /* fill ngram offset section
         */
        offset_t *ngoff = (offset_t *)(ngtype + N);
        ngoff[0] = (char *)ngoff - model + N * sizeof(offset_t);
        for (i = 1; i < N; i++)
                ngoff[i] = ngoff[i - 1] +
                           (ngtype[i - 1] + 1) * sizeof(struct inner_node);
        for (i = 0; i < N - 1; i++)
                inode[i] = (struct inner_node *)(model + ngoff[i]);
        *pfnode = (struct final_node *)(model + ngoff[N - 1]);
}

/* 
 * <va>: inode, fnode, ngoff, N
 */
static void
_fill_model_file(int *ngram, int *count, int last, int is_cutoff, va_list va)
{
        if (is_cutoff)
                return;

        struct inner_node **inode = va_arg(va, struct inner_node **);
        struct final_node *fnode = va_arg(va, struct final_node *);
        int *ngoff = va_arg(va, int *);
        int N = va_arg(va, int);

        ngoff[0] = ngram[0];
        if (last == N - 1) { /* final node */
                fnode[ngoff[last]].wid = ngram[last];
                fnode[ngoff[last]].prob = (float)count[last];
        } else { /* inner node */
                inode[last][ngoff[last]].wid = ngram[last];
                inode[last][ngoff[last]].prob = (float)count[last];
                inode[last][ngoff[last] + 1].link = ngoff[last + 1];
        }
        ngoff[last]++;
}

static void
fill_model_file(const char *cfn, int *cutoff, int N, struct sptags *stags,
                struct inner_node **inode, struct final_node *fnode, int ugcnt)
{
        int *ngoff = NULL, i = 0;

        ngoff = (int *)calloc(N, sizeof(*ngoff));
        if (! ngoff) {
                fprintf(stderr, "[ERROR]: out of memory\n");
                exit(1);
        }
        do_each_ngram(cfn, N, cutoff, stags, "initiate model file",
                        _fill_model_file, inode, fnode, ngoff, N);
        /* fix the link field of unigram
         */
        for (i = 1; i <= ugcnt; i++) {
                inode[0][i].wid = i;
                if (inode[0][i].link == 0)
                        inode[0][i].link = inode[0][i - 1].link;
        }
        free(ngoff);
}

static void
_print_model_file_struct(struct inner_node **inode, struct final_node *fnode,
                         int beg, int end, int level, int N, int *ngrambuf)
{
        int i = beg;

        while (i < end) {
                if (level == 0) {
                        ngrambuf[0] = inode[0][i].wid;
                        _print_model_file_struct(inode, fnode, inode[0][i].link,
                                        inode[0][i + 1].link, 1, N, ngrambuf);
                        for (i++; i < end; i++)
                                if (inode[0][i].link != inode[0][i + 1].link)
                                        break;
                } else if (level == N - 1) {
                        int cnt = (int)fnode[i].prob;
                        ngrambuf[N - 1] = fnode[i].wid;
                        fwrite(ngrambuf, sizeof(*ngrambuf), N, stdout);
                        fwrite(&cnt, sizeof(cnt), 1, stdout);
                        i++;
                } else {
                        ngrambuf[level] = inode[level][i].wid;
                        _print_model_file_struct(inode, fnode, inode[level][i].link,
                                        inode[level][i + 1].link, level + 1, N,
                                        ngrambuf);
                        i++;
                }
        }
}

static void
print_model_file_struct(struct inner_node **inode, struct final_node *fnode,
                        int ugcnt, int N)
{
        int *ngrambuf = (int *)calloc(N, sizeof(*ngrambuf));
        if (! ngrambuf) {
                fprintf(stderr, "[ERROR]: out of memory\n");
                exit(1);
        }
        _print_model_file_struct(inode, fnode, 0, ugcnt, 0, N, ngrambuf);
        free(ngrambuf);
}

static void
estimate_discount_ratio(int **Nr, int *gtmax, float **discr, int N)
{
        int n = 0, c = 0;
        float cmnterm = 0, tmp = 0;
        int maxcnt = 0;

        for (n = 0; n < N; n++) {
                maxcnt = gtmax[n];
                if (Nr[n][1] == 0) {
                        for (c = 1; c <= maxcnt; c++)
                                discr[n][c] = 1.0;
                        continue;
                }
                cmnterm = (float)(1.0 * (maxcnt + 1) * Nr[n][maxcnt + 1] / Nr[n][1]);
                if (cmnterm == 1.0) {
                        for (c = 1; c <= maxcnt; c++)
                                discr[n][c] = 1.0;
                        continue;
                }
                for (c = 1; c <= maxcnt; c++) {
                        if (Nr[n][c] == 0) {
                                discr[n][c] = 1.0;
                                continue;
                        }
                        tmp = (float)(1.0 * (c + 1) / c * Nr[n][c + 1] / Nr[n][c]);
                        if (tmp > 1.0) {
                                discr[n][c] = 1.0;
                                continue;
                        }
                        tmp = (float)(1.0 * (tmp - cmnterm) / (1.0 - cmnterm));
                        if (tmp < 1.0e-6 || tmp < -1.0e8 || tmp > 1.0e8) {
                                discr[n][c] = 1.0;
                                continue;
                        }
                        discr[n][c] = tmp;
                }
        }
}

static float
get_discounted_count(int n, int cnt, float **discr, int *gtmax)
{
        if (cnt == 0)
                return (float)1.0e-9;
        if (cnt > gtmax[n])
                return (float)(cnt - 0.001);
        return cnt * discr[n][cnt];
}

static void
calculate_ngram_prob(struct inner_node **inode, struct final_node *fnode,
                     int *ngramtype, float **discr, int *gtmax, int N,
                     struct sptags *stags)
{
        int n = 0, i = 0, j = 0, n0probs = 0;
        float cnt = 0, dcnt = 0, ttlwordcnt = 0, sum = 0, uniprob = 0;

        //fprintf(stderr, "Calculate probabilities of level ");

        /* calculate probability for 2 to n grams
         */

        for (n = N - 2; n >= 0; n--) {
                //fprintf(stderr, "%d ", n + 2);
                for (i = 0; i < ngramtype[n]; i++) {
                        for (j = inode[n][i].link; j < inode[n][i + 1].link; j++) {
                                if (n == N - 2) {
                                        cnt = fnode[j].prob;
                                        dcnt = get_discounted_count(n + 1, (int)cnt,
                                                        discr, gtmax);
                                        fnode[j].prob =
                                                log10(dcnt / inode[n][i].prob);
                                } else {
                                        cnt = inode[n + 1][j].prob;
                                        dcnt = get_discounted_count(n + 1, (int)cnt,
                                                        discr, gtmax);
                                        inode[n + 1][j].prob = 
                                                log10(dcnt / inode[n][i].prob);
                                }
                        }
                }
        }

        /* calculate probability for unigrams
         */

        //fprintf(stderr, "1 ");

        for (i = 0; i < ngramtype[0]; i++)
                ttlwordcnt += inode[0][i].prob;
        for (i = 0; i < ngramtype[0]; i++) {
                cnt = inode[0][i].prob;
                dcnt = get_discounted_count(0, (int)cnt, discr, gtmax);
                if (dcnt > 1.0e-5) {
                        inode[0][i].prob = log10(dcnt / ttlwordcnt);
                } else {
                        inode[0][i].prob = 0;
                        n0probs++;
                }
                sum += dcnt;
        }
        uniprob = log10((ttlwordcnt - sum) / ttlwordcnt / n0probs);
        for (i = 0; i < ngramtype[0]; i++) {
                if (inode[0][i].prob == 0)
                        inode[0][i].prob = uniprob;
        }
        inode[0][stags->unkid].prob -= 4.5;

        //fprintf(stderr, "... done\n");
}

int
bsrch_cmp_inode(const void *v1, const void *v2)
{
        int *key = (int *)v1;
        struct inner_node *inode = (struct inner_node *)v2;

        return *key - inode->wid;
}

int
bsrch_cmp_fnode(const void *v1, const void *v2)
{
        int *key = (int *)v1;
        struct final_node *fnode = (struct final_node *)v2;

        return *key - fnode->wid;
}

/* 
 * <end> is the index of the one after the last word
 */
static float
_get_ngram_prob(struct inner_node **inode, struct final_node *fnode, 
                int *ngramtype, int N, int *ngram, int beg, int end)
{
        /*
         * deal with unigram
         */

        if (end - beg == 1) {
                if (ngram[beg] < 0 || ngram[beg] >= ngramtype[0])
                        return (float)log10(0.0001);
                return inode[0][ngram[beg]].prob;
        }

        /*
         * deal with 2 to n gram
         */

        int lev = 0, prelev_off = 0, l = 0, h = 0;

        /* check whether the n-1 gram context exists
         */
        if (ngram[beg] < 0 || ngram[beg] >= ngramtype[0]) {
                return _get_ngram_prob(inode, fnode, ngramtype, N, ngram,
                                        beg + 1, end);
        }
        prelev_off = ngram[beg];
        for (lev = 1; lev <= end - beg - 2; lev++) {
                struct inner_node *p = NULL;
                /* find ngram[beg + lev] in inode[lev]
                 */
                l = inode[lev - 1][prelev_off].link;
                h = inode[lev - 1][prelev_off + 1].link;
                p = (struct inner_node *)bsearch(ngram + beg + lev, inode[lev] + l,
                                h - l, sizeof(inode[lev][l]), bsrch_cmp_inode);
                if (! p) {
                        return  _get_ngram_prob(inode, fnode, ngramtype, N, ngram,
                                              beg + 1, end);
                }
                prelev_off = (int)(p - inode[lev]);
        }

        l = inode[lev - 1][prelev_off].link;
        h = inode[lev - 1][prelev_off + 1].link;
        if (lev == N - 1) {
                struct final_node *p = NULL;
                /* check if n-gram exists
                 */
                p = (struct final_node *)bsearch(ngram + beg + lev, fnode + l,
                                h - l, sizeof(fnode[l]), bsrch_cmp_fnode);
                if (! p) {
                        return  inode[lev - 1][prelev_off].backoff +
                               _get_ngram_prob(inode, fnode, ngramtype, N, ngram,
                                              beg + 1, end);
                } else {
                        return p->prob;
                }
        } else {
                struct inner_node *p = NULL;
                /* check if n-gram exists
                 */
                p = (struct inner_node *)bsearch(ngram + beg + lev, inode[lev] + l,
                                h - l, sizeof(inode[lev][l]), bsrch_cmp_inode);
                if (! p) {
                        return   inode[lev - 1][prelev_off].backoff +
                               _get_ngram_prob(inode, fnode, ngramtype, N, ngram,
                                              beg + 1, end);
                } else {
                        return p->prob;
                }
        }
}

static void
_calculate_backoff_weight(struct inner_node **inode, struct final_node *fnode,
                          int n, int lev, int prelev_off, int *ngram,
                          int *ngramtype, int N)
{
        /* iterate through all n-grams(context)
         */
        if (lev < n) {
                if (lev == 0) {
                        int i = 0;
                        for (i = 0; i < ngramtype[0]; i++) {
                                ngram[lev] = inode[0][i].wid;
                                _calculate_backoff_weight(inode, fnode, n, 1, i,
                                                          ngram, ngramtype, N);
                        }
                } else {
                        int off = 0;
                        for (off = inode[lev - 1][prelev_off].link;
                             off < inode[lev - 1][prelev_off + 1].link;
                             off++) {
                                ngram[lev] = inode[lev][off].wid;
                                _calculate_backoff_weight(inode, fnode, n, lev + 1,
                                                        off, ngram, ngramtype, N);
                        }
                }
                return;
        }

        /* calculate backoff weight of level n
         */
        int i = 0;
        float nsum = 0, dsum = 0;

        for (i = inode[lev - 1][prelev_off].link;
             i < inode[lev - 1][prelev_off + 1].link;
             i++) {
                if (lev == N - 1) {
                        ngram[lev] = fnode[i].wid;
                        nsum += (float)pow((double)10, (double)fnode[i].prob);
                } else {
                        ngram[lev] = inode[lev][i].wid;
                        nsum += (float)pow((double)10, (double)inode[lev][i].prob);
                }
                float ngp = _get_ngram_prob(inode, fnode, ngramtype, N, ngram,
                                             1, lev + 1);
                dsum += (float)pow((double)10, (double)ngp);
        }
        if (nsum == 1.0) /* without this if, the result may be -infinity */
                nsum = (float)0.9999;
        if (fabs(1.0 - dsum) < 1.0E-9)
                dsum = (float)0.9999;

        inode[lev - 1][prelev_off].backoff = (float)log10((1.0 - nsum) / (1.0 - dsum));
}

static void
calculate_backoff_weight(struct inner_node **inode, struct final_node *fnode, 
                         int *ngramtype, int N)
{
        int n = 0;
        int *ngram = NULL;

        ngram = (int *)calloc(N, sizeof(*ngram));
        if (! ngram) {
                fprintf(stderr, "[ERROR]: out of memory\n");
                exit(1);
        }

        /* unigram to N - 1 gram
         */
        //fprintf(stderr, "calculate backoff weight of level");
        for (n = 1; n <= N - 1; n++) {
                //fprintf(stderr, " %d", n);
                _calculate_backoff_weight(inode, fnode, n, 0, 0, ngram,
                                          ngramtype, N);
        }
        //fprintf(stderr, " ... done\n");

        free(ngram);
}

static void
mkmodel(const char *vfn, const char *cfn, const char *ofn,
                        int N, int *cutoff, int *gtmax)
{
        int *ngramtype = NULL, **Nr = NULL, memsz = 0, i = 0, *pid = NULL;
        int err = 0;
        float **discr = NULL;
        struct hashtable *htab = NULL;
        struct sptags stags;
        const char *os = "<s>", *cs = "</s>", *unk = "<unk>";
        struct inner_node **inode = NULL;
        struct final_node *fnode = NULL;

        /* initialize <stags>
         */
        ldvocab(vfn, &htab);
        err = htab_search(htab, unk, strlen(unk) + 1, (void **)&pid);
        if (err < 0) {
                fprintf(stderr, "[ERROR]: htab search\n");
                exit(1);
        } else if (err == 1) {
                fprintf(stderr, "[ERROR]: no <unk> in vocabulary\n");
                exit(1);
        }
        stags.unkid = *pid;
        err = htab_search(htab, os, strlen(os) + 1, (void **)&pid);
        if (err == 1) {
                fprintf(stderr, "[ERROR]: no <s> in vocabulary\n");
                exit(1);
        }
        stags.osid = *pid;
        err = htab_search(htab, cs, strlen(cs) + 1, (void **)&pid);
        if (err == 1) {
                fprintf(stderr, "[ERROR]: no </s> in vocabulary\n");
                exit(1);
        }
        stags.csid = *pid;

        /* allocate memory
         */
        memsz = sizeof(*ngramtype) * N +
                sizeof(*Nr) * N +
                sizeof(*discr) * N;
        for (i = 0; i < N; i++)
                memsz += (gtmax[i] + 2) * sizeof(**Nr) +
                         (gtmax[i] + 1) * sizeof(**discr);
        memsz += sizeof(*inode) * N;
        ngramtype = (int *)calloc(1, memsz);
        if (! ngramtype) {
                fprintf(stderr, "[ERROR]: out of memory\n");
                exit(1);
        }
        Nr = (int **)(ngramtype + N);
        Nr[0] = (int *)(Nr + N);
        for (i = 1; i < N; i++)
                Nr[i] = Nr[i - 1] + gtmax[i - 1] + 2;
        discr = (float **)(Nr[N - 1] + gtmax[N - 1] + 2);
        discr[0] = (float *)(discr + N);
        for (i = 1; i < N; i++)
                discr[i] = discr[i - 1] + gtmax[i - 1] + 1;
        inode = (struct inner_node **)(discr[N - 1] + gtmax[N - 1] + 1);

        /* get statistical info: <ngramtype>, <Nr>
         */
        stat_ngram_count(cfn, ngramtype, Nr, gtmax, cutoff, &stags, N);
        ngramtype[0] = htab_size(htab); /* change unigram count */

        /* create model file and set <inode> and <fnode>
         */
        create_model_file(ofn, ngramtype, N, stags.unkid, inode, &fnode);
        memset(inode[0], 0, sizeof(*inode[0]) * (ngramtype[0] + 1));

        /* fill model file with ngrams
         */
        fill_model_file(cfn, cutoff, N, &stags, inode, fnode, ngramtype[0]);

        /* estimate model parameters
         */
        estimate_discount_ratio(Nr, gtmax, discr, N);
        calculate_ngram_prob(inode, fnode, ngramtype, discr, gtmax, N, &stags);
        calculate_backoff_weight(inode, fnode, ngramtype, N);

        free(ngramtype);
}

/* --------------------------------------------------------------------------- */
/*                                 cache                                       */
/* --------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------- */
/* cache strategy:                                                             */
/* use normal hash as cache, pre-allocate all memory necessary                 */
/* --------------------------------------------------------------------------- */

#define CACHE_BUCKET_SZ next_prime(1024 * 10)

/* record in cache
 */
struct record {
        int *ngram;
        int nglen;
        float score;
};

struct cache {
        struct record **buckets;
        int nbuk;
};

static int
cache_hash(int *ngram, int beg, int end, int buksz)
{
        int code = 0, i = 0;

        code = end - beg;
        for (i = beg; i < end; i++)
                code = (code << 4) ^ (code >> 28) ^ ngram[i];
                //code ^= (code << 5) + (code >> 2) + ngram[i];

        return (code & 0x7FFFFFFF) % buksz;
}

static struct cache *
cache_alloc(int nbuk, int N)
{
        int i = 0;
        int rsz = 0; /* record size */
        int csz = 0; /* cache size */
        struct cache *pc = NULL;
        char *base = NULL;

        rsz = sizeof(struct record) + N * sizeof(int);
        csz = sizeof(struct cache) + (rsz + sizeof(struct record *)) * nbuk;

        pc = (struct cache *)calloc(1, csz);
        if (! pc) {
                fprintf(stderr, "[ERROR]: out of memory\n");
                exit(1);
        }
        pc->nbuk = nbuk;
        pc->buckets = (struct record **)(pc + 1);
        base = (char *)(pc->buckets + nbuk);
        for (i = 0; i < nbuk; i++) {
                pc->buckets[i] = (struct record *)(base + i * rsz);
                pc->buckets[i]->ngram = (int *)(pc->buckets[i] + 1);
        }
        
        return pc;
}

/* 
 * return -1 if record not found
 */
static float
cache_search(struct cache *pc, int beg, int end, int *ngram, int *pbukid)
{
        if (! pc)
                return -1;

        int bukid = 0;
        struct record *pr = NULL;

        bukid = cache_hash(ngram, beg, end, pc->nbuk);
        *pbukid = bukid;
        pr = pc->buckets[bukid];
        if (end - beg != pr->nglen) {
                return -1;
        }
        if (! memcmp(pr->ngram, ngram + beg, sizeof(int) * (end - beg)))
                return pr->score;
        return -1;
}

static void
cache_add(struct cache *pc, int beg, int end, int *ngram, float score, int bukid)
{
        if (! pc)
                return;

        struct record *pr = NULL;

        pr = pc->buckets[bukid];
        memcpy(pr->ngram, ngram + beg, sizeof(int) * (end - beg));
        pr->nglen = end - beg;
        pr->score = score;
}

/* --------------------------------------------------------------------------- */
/*                              library function                               */
/* --------------------------------------------------------------------------- */

#ifdef LMTRAIN_LIB

//struct lmodel {
//        struct inner_node **inode;
//        struct final_node *fnode;
//        struct hashtable *vocab;
//        int N, unkid, *ngramtype;
//        struct cache **caches;
//        int ncache;
//        char * mem;
//};

static struct lmodel LM;

/* ------------------------- all exported functions -------------------------- */

/* 
 * This function loads language model and associated vocabulary into memory.
 * <lmfn> specifies the language model file name
 * <vfn> specifies the vocabulary file name
 * <nthrd> specifies the  number of threads accessing this language model,
 * it determines the number of caches needed. -1 or 0 means you don't want to
 * use cache
 * <is_mmap> determines in which way to load language model 0 means to load it
 * by open, 1 means to load it by memory map
 */
extern void
initlm2(struct lmodel *plm, const char *lmfn, const char *vfn,
                                        int nthrd, int is_mmap);
/* 
 * This function returns the n-gram probability.
 * <ngram>, <beg> and <end> determine the n-gram, NOTE that <end> is the index
 * of the one after the last word. For example, if <ngram> = "11 8 7 9 20",
 * <beg> = 1, <end> = 4, then the n-gram to query is "8 7 9".
 * <trdid> denotes the id of the thread accessing this language model, if equals
 * -1, it tells the this function not to access the cache.
 */
extern float
get_ngram_prob2(struct lmodel *plm, int *ngram, int beg, int end, int trdid);

/* 
 * This function generates a word id array (stored in <wids>) from the word
 * string <line>. 
 * *NOTE* that <line> is changed in this function and you need to allocate the
 * <wids> array before calling this function.
 */
extern int
get_wids2(struct lmodel *plm, char *line, int *wids);

/* 
 * This function returns the probability of a string.
 * <wids>, <beg> and <end> determine the string in the same way as those in
 * function <get_ngram_prob2>.
 * <trdid> denotes the id of the thread accessing this language model.
 */
extern float
get_string_prob_wids2(struct lmodel *plm, int *wids, int beg, int end, int trdid);

/* 
 * This function is almost the same as <get_wids> except that it adds tag "<s>"
 * at the beginning and adds tag "</s>" at the end
 * *NOTE* that <line> is changed in this function and you need to allocate the
 * <wids> array before calling this function.
 */
extern void
get_wids_with_tag2(struct lmodel *plm, char *line, int *wids);

/* 
 * The following 5 functions are exactly the same as the previous 5 ones except
 * for the fact that they use a language model statically declared in this file,
 * so if you know in advance that you'll use only one language model in your
 * system, then these functions may be convenient for you.
 */
extern void
initlm(const char *lmfn, const char *vfn, int nthrd, int is_mmap);
extern float
get_ngram_prob(int *ngram, int beg, int end, int trdid);
extern void
get_wids(char *line, int *wids);
extern float
get_string_prob_wids(int *wids, int beg, int end, int trdid);
extern void
get_wids_with_tag(char *line, int *wids);

/* --------------------------------------------------------------------------- */

static char *
open_lmfile(const char *lmfn, fsize_t &memsize)
{
        char *lmf = NULL;
        FILE *lmfp = NULL;
        fsize_t filesz = 0;
        int err = 0;
        memsize = 0;

        lmfp = fopen(lmfn, "rb");
        if (! lmfp) {
                fprintf(stderr, "[ERROR]: fopen %s\n", lmfn);
                exit(1);
        }
#ifndef WIN32
        struct stat statbuf;

        err = stat(lmfn, &statbuf);
        if (err < 0) {
                fprintf(stderr, "[ERROR]: stat %s\n", lmfn);
                exit(1);
        }
        filesz = statbuf.st_size;
#else
        LARGE_INTEGER fsz;
        HANDLE fd = NULL;
        wchar_t *wfn = NULL;
        int wfn_len = 0;

        wfn_len = (int)mbstowcs(NULL, lmfn, strlen(lmfn));
        wfn = (wchar_t *)calloc(wfn_len + 1, sizeof(*wfn));
        if (! wfn) {
                fprintf(stderr, "[ERROR]: out of memory");
                exit(1);
        }
        mbstowcs(wfn, lmfn, strlen(lmfn));

        fd = CreateFileW(wfn, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, 0, NULL);
        if (fd == (HANDLE)(-1)) {
                fprintf(stderr, "[ERROR]: CreateFile");
                exit(1);
        }

        GetFileSizeEx(fd, &fsz);
        filesz = (fsize_t)fsz.QuadPart;
        free(wfn); // added by xiatong 2010/12/27
#endif /* WIN32 */
        lmf = (char *)calloc(1, filesz);
        err = (int)fread(lmf, filesz, 1, lmfp);
        if (err != 1) {
                fprintf(stderr, "[ERROR]: fread %s\n", lmf);
                exit(1);
        }
        fclose(lmfp);
        memsize = filesz;

        return lmf;
}

static void
loadlm2(struct lmodel *plm, const char *lmfn, int is_mmap)
{
        char *lmf = NULL;
        struct model_header *hdr = NULL;
        int i = 0;
        offset_t *off = NULL;

        if (is_mmap)
                lmf = (char *)mmap_file(lmfn, 0, plm->memsize);
        else
                lmf = open_lmfile(lmfn, plm->memsize);

        plm->mem = lmf; // added by xiaotong 2010/12/27

        hdr = (struct model_header *)lmf;
        /* check file validity
         */
        if (strcmp(hdr->mark, "Copyright (C) 2011, NEU-NLPLab (http://www.nlplab.com/). All rights reserved." ) != 0 || hdr->signature != LMSIGNATURE) {
                fprintf(stderr, "[ERROR]: %s is not a language model file\n", lmfn);
                exit(1);
        }
        /* set N, unkid, ngramtype
         */
        plm->N = hdr->order;
        plm->unkid = hdr->unkid;
        plm->ngramtype = (int *)(hdr + 1);

        /* set inode and fnode
         */
        off = (offset_t *)(plm->ngramtype + plm->N);
        plm->inode = (struct inner_node **)calloc(plm->N - 1, sizeof(*plm->inode));
        if (! plm->inode) {
                fprintf(stderr, "[ERROR]: out of memory\n");
                exit(1);
        }
        for (i = 0; i < plm->N - 1; i++)
                plm->inode[i] = (struct inner_node *)(lmf + off[i]);
        plm->fnode = (struct final_node *)(lmf + off[i]);
}

// the below fuction is added by xiaotong 2010/12/27
static void
unloadlm2(struct lmodel *plm, int is_mmap)
{
    if(!is_mmap)
        free((char*)(plm->mem));
    free(plm->inode);
    htab_free(&plm->vocab);
}

static void
loadvocab2(struct lmodel *plm, const char *vfn)
{
        ldvocab(vfn, &(plm->vocab));
}

void
initlm2(struct lmodel *plm, const char *lmfn, const char *vfn,
                                        int nthrd, int is_mmap)
{
        loadlm2(plm, lmfn, is_mmap);
        loadvocab2(plm, vfn);
#ifdef CACHE_ENABLE
        if (nthrd == -1 || nthrd == 0) {
                plm->caches = NULL;
                plm->ncache = 0;
        } else {
                int i = 0;
                plm->caches = (struct cache **)calloc(nthrd, sizeof(*plm->caches));
                if (! plm->caches) {
                        fprintf(stderr, "[ERROR]: out of memory\n");
                        exit(1);
                }
                for (i = 0; i < nthrd; i++)
                        plm->caches[i] = cache_alloc(CACHE_BUCKET_SZ, plm->N);
        }
#endif /* CACHE_ENABLE */
}

float
get_ngram_prob2(struct lmodel *plm, int *ngram, int beg, int end, int trdid)
{
        float prob = 0;

#ifdef CACHE_ENABLE
        struct cache *pc = NULL;
        int bukid = 0;

        if (trdid != -1 && plm->caches) {
                pc = plm->caches[trdid];
                prob = cache_search(pc, beg, end, ngram, &bukid);
                if (prob != -1)
                        return prob;
        }
#endif /* CACHE_ENABLE */

        prob = _get_ngram_prob(plm->inode, plm->fnode, plm->ngramtype, plm->N,
                               ngram, beg, end);
#ifdef CACHE_ENABLE
        /* add to cache
         */
        if (trdid != -1 && plm->caches) {
                pc = plm->caches[trdid];
                cache_add(pc, beg, end, ngram, prob, bukid);
        }
#endif /* CACHE_ENABLE */

        return prob;
}

int
get_wids2(struct lmodel *plm, char *line, int *wids)
{
        int *pwid = NULL, i = 0, err = 0;
        char *word = NULL;

        word = strtok(line, WORDDELIM);
        if (! word)
                return 0;
        do {
                err = htab_search(plm->vocab, word, strlen(word) + 1,
                                  (void **)&pwid);
                if (err < 0) {
                        fprintf(stderr, "[ERROR]: search htab\n");
                        exit(1);
                } else if (err == 1) { /* no such word */
                        wids[i++] = plm->unkid;
                } else {
                        wids[i++] = *pwid;
                }
                word = strtok(NULL, WORDDELIM);
        } while (word);

        return i;
}

void
get_wids_with_tag2(struct lmodel *plm, char *line, int *wids)
{
        int *pwid = NULL, i = 0, err = 0;

        /* add <s>
         */
        err = htab_search(plm->vocab, "<s>", 4, (void **)&pwid);
        if (err == 1) {
                fprintf(stderr, "[ERROR]: no <s>\n");
                exit(1);
        }
        wids[0] = *pwid;
        
        i = get_wids2(plm, line, wids + 1) + 1;

        /* add </s>
         */
        err = htab_search(plm->vocab, "</s>", 5, (void **)&pwid);
        if (err == 1) {
                fprintf(stderr, "[ERROR]: no </s>\n");
                exit(1);
        }
        wids[i] = *pwid;
}

float
get_string_prob_wids2(struct lmodel *plm, int *wids, int beg, int end, int trdid)
{
        int i = 0;
        float prob = 0;

        for (i = beg; i < end; i++) {
                int b = 0, e = 0;
                float p = 0;

                e = i + 1;
                b = e - plm->N > beg ? e - plm->N : beg;
                p = get_ngram_prob2(plm, wids, b, e, trdid);
                prob += p;
        }

        return prob;
}

void
initlm(const char *lmfn, const char *vfn, int nthrd, int is_mmap)
{
        initlm2(&LM, lmfn, vfn, nthrd, is_mmap);
}

float
get_ngram_prob(int *ngram, int beg, int end, int trdid)
{
        return get_ngram_prob2(&LM, ngram, beg, end, trdid);
}

void
get_wids(char *line, int *wids)
{
        get_wids2(&LM, line, wids);
}

float
get_string_prob_wids(int *wids, int beg, int end, int trdid)
{
        return get_string_prob_wids2(&LM, wids, beg, end, trdid);
}

void
get_wids_with_tag(char *line, int *wids)
{
        get_wids_with_tag2(&LM, line, wids);
}

//#else /* LMTRAIN_LIB */

/* --------------------------------------------------------------------------- */
/*                               option handling                               */
/* --------------------------------------------------------------------------- */

#define ERR(s, c) \
        if(opterr){ \
                char errbuf[2]; \
                errbuf[0] = c; errbuf[1] = '\n'; \
                fputs(argv[0], stderr); \
                fputs(s, stderr); \
                fputc(c, stderr); \
        }

int opterr = 1, optind = 1, optopt;
const char *optarg;

/*
 * <from http://note.sonots.com/EngNote/CompLang/cpp/getopt.html>
 */ 
int
my_getopt(int argc, const char **argv, const char *opts)
{
    static int sp = 1;
    int c;
    const char *cp;

    if (sp == 1) {
        if(optind >= argc || argv[optind][0] != '-'
                   || argv[optind][1] == '\0')
            return -1;
        else if (! strcmp(argv[optind], "--")) {
            optind++;
            return -1;
        }
        }
    optopt = c = argv[optind][sp];
    if (c == ':' || ! (cp = strchr(opts, c))) {
        ERR(": illegal option -- ", c);
        if (argv[optind][++sp] == '\0') {
            optind++;
            sp = 1;
        }
        return '?';
    }
    if (*++cp == ':') {
        if (argv[optind][sp+1] != '\0')
            optarg = &argv[optind++][sp+1];
        else if (++optind >= argc) {
            ERR(": option requires an argument -- ", c);
            sp = 1;
            return '?';
        } else
            optarg = argv[optind++];
        sp = 1;
    } else {
        if (argv[optind][++sp] == '\0') {
            sp = 1;
            optind++;
        }
        optarg = NULL;
    }
    return c;
}

static int
decide_action(const char *act)
{
        if (! strcmp(act, "mkvocab")) {
                return MKVOCAB;
        } else if (! strcmp(act, "cntngram")) {
                return CNTNGRRAM;
        } else if (! strcmp(act, "mkmodel")) {
                return MKMODEL;
        } else {
                fprintf(stderr, "[ERROR]: unknown command action %s\n"
                                "Use -h to see the help info\n", act);
                exit(1);
        }
}

static void
usage(const char *cmd)
{
#define HELP(msg) fprintf(stderr, msg)
        int i = 0, cnt = 80;
        for (i = 0; i < cnt; i++) printf("-");
        putchar('\n');
        HELP("[USAGE]:\n");
        fprintf(stderr, "\t%s <action> [OPTIONS]\n", cmd);
        HELP("[ACTION]:\n");
        HELP("\tmkvocab: making the vocabulary, with words occurring less then\n"
             "\t         some given times cut off\n");
        HELP("\tcntngram:counting the ngrams, with ngrams occurring less than\n"
             "\t         some given times cut off\n");
        HELP("\tmkmodel: making the language model\n");
        HELP("[OPTION]:\n");
        HELP("\t-c <num> (default: 1)\n");
        HELP("\t\tthis option specifies the cutoff value, used in action mkvocab\n"
             "\t\tand cntngram\n");
        HELP("\t-t <corpus-file> (default: stdin)\n");
        HELP("\t\tthis option specifies the input corpus file\n");
        HELP("\t-o <output>\n");
        HELP("\t\tfor mkvocab action, this option specifies the output vocabulary\n"
             "\t\t\tdefault: vocab\n"
             "\t\tfor cntngram action, it specifies the output ngram counts\n"
             "\t\t\tdefault: count\n"
             "\t\tfor mkmodel action, it specifies the output language model\n"
             "\t\t\tdefault: lm.bin\n");
        HELP("\t-v <vocab>\n");
        HELP("\t\tload vocabulary from <vocab>\n");
        HELP("\t-n <ngram>\n");
        HELP("\t-m <memory-size> (default 128)\n");
        HELP("\t\tused for cntngram action, specifying the size of memory\n"
             "\t\tavailable in unit of Megabyte\n");
        HELP("\t-u <ngram-count-file>\n");
        HELP("\t\tthis is the file output by cntngram action\n");
        HELP("\t-C <1-gram_cutoff,2-gram_cutoff,...,N-gram_cutoff>\n");
        HELP("\t\t(default: 0,0,0,1,1,2,2,2...)\n");
        HELP("\t\tthe argument of this option is a comma-sperated list of numbers,\n"
             "\t\twith each speifying the cutoff value for 1 to N gram\n");
        HELP("\t-h\n");
        HELP("\t\toutput this help info\n");
        HELP("[EXAMPLE]:\n");
        HELP("\tTo make vocabulary\n");
        fprintf(stderr, "\t   %s mkvocab -n 5 -t corpus -o vocab\n", cmd);
        HELP("\tTo create ngram count file:\n");
        fprintf(stderr, "\t   %s cntngram -n 5 -v vocab -o count -m 2048 -t "
                                                              "corpus\n", cmd);
        HELP("\tTo make language model\n");
        fprintf(stderr, "\t   %s mkmodel -n 5 -v vocab -o lm.bin -u count\n", cmd);
        for (i = 0; i < cnt; i++) printf("-");
        putchar('\n');
#undef HELP
}

int
lmmain(int argc, const char **argv)
{
        int act = 0, opt = 0, cutoff = 1, N = -1, i = 0;
        int *ngram_cutoff = NULL, *gtmax = NULL;
        const char *ifn = NULL, *ofn = NULL, *vfn = NULL, *cfn = NULL;
        const char *def_mdl = "lm.bin", *def_cnt = "count", *def_vcb = "vocab";
        const char *co_del = ",";
        char *co = NULL;
        char *toptarg = NULL;
        size_t memsz = 128;

        if (argc < 3 || ! strcmp(argv[2], "-h")) {
                usage(argv[0]);
                exit(0);
        }

        act = decide_action(argv[2]);

        optind = 2;
        while ((opt = my_getopt(argc - 1, argv + 1, "o:c:t:v:n:m:C:u:")) != -1) {
                switch (opt) {
                case 'o':
                        ofn = optarg;
                        break;
                case 'c':
                        cutoff = atoi(optarg);
                        break;
                case 't':
                        ifn = optarg;
                        break;
                case 'v':
                        vfn = optarg;
                        break;
                case 'n':
                        N = atoi(optarg);
                        break;
                case 'm':
                        memsz = atoi(optarg);
                        break;
                // not debugged
                case 'C':
                        if (N == -1) {
                                fprintf(stderr, "[ERROR]: -n should be specified "
                                                "before -C\n");
                                exit(1);
                        }
                        ngram_cutoff = (int *)calloc(N, sizeof(*ngram_cutoff));
                        if (! ngram_cutoff) {
                                fprintf(stderr, "[ERROR]: out of memory\n");
                                exit(1);
                        }
                        i = 0;
                        toptarg = (char*)malloc(sizeof(char) * (strlen(optarg) + 1));
                        strcpy(toptarg, optarg);
                        co = strtok(toptarg, co_del);
                        do {
                                if (i >= N) {
                                        fprintf(stderr, "[ERROR]: number of cutoff "
                                                        "values not match %d\n", N);
                                        exit(1);
                                }
                                ngram_cutoff[i++] = atoi(co);
                        } while ((co = strtok(NULL, co_del)));
                        if (i < N) {
                                fprintf(stderr, "[ERROR]: number of cutoff "
                                                "values not match %d\n", N);
                                exit(1);
                        }
                        free(toptarg);
                        break;
                case 'u':
                        cfn = optarg;
                        break;
                case '?':
                        fprintf(stderr, "Use -h to see then help info\n");
                        break;
                }
        }

        if (N == -1) {
                fprintf(stderr, "[ERROR]: -n should be specified\n");
                exit(1);
        }

        switch (act) {
        case MKVOCAB:
                if (! ofn)
                        ofn = def_vcb;
                mkvocab(ifn, ofn, cutoff);
                break;
        case CNTNGRRAM:
                if (! vfn) {
                        fprintf(stderr, "[ERROR]: vocabulary needs to be "
                                        "specified using -v option\n");
                        exit(1);
                }
                if (! ofn)
                        ofn = def_cnt;
                cntngram(vfn, ifn, ofn, N, cutoff, (int)memsz);
                break;
        case MKMODEL:
                if (! vfn) {
                        fprintf(stderr, "[ERROR]: vocabulary needs to be "
                                        "specified using -v option\n");
                        exit(1);
                }
                if (! cfn) {
                        fprintf(stderr, "[ERROR]: ngram count file need to be "
                                        "specified using -u option\n");
                        exit(1);
                }
                if (! ofn)
                        ofn = def_mdl;
                if (! ngram_cutoff) {
                        ngram_cutoff = (int *)calloc(N, sizeof(*ngram_cutoff));
                        if (! ngram_cutoff) {
                                fprintf(stderr, "[ERROR]: out of memory\n");
                                exit(1);
                        }
                        for (i = 0; i < 3 && i < N; i++)
                                ngram_cutoff[i] = 0;
                        for (i = 3; i < 5 && i < N; i++)
                                ngram_cutoff[i] = 1;
                        for (i = 5; i < N; i++)
                                ngram_cutoff[i] = 2;
                }
                if (! gtmax) {
                        gtmax = (int *)calloc(N, sizeof(*gtmax));
                        if (! gtmax) {
                                fprintf(stderr, "[ERROR]: out of memory\n");
                                exit(1);
                        }
                        for (i = 0; i < N; i++)
                                gtmax[i] = 5;
                }
                mkmodel(vfn, cfn, ofn, N, ngram_cutoff, gtmax);
                break;
        default:
                fprintf(stderr, "[ERROR]: unknown action\n");
                exit(1);
        }

        return 0;
}

lmodel* g_plm[256] = { NULL };
int g_isMMAP[256] = { 0 };
int g_lmNum = 0;

int g_initlm(const char *lmfn, const char *vfn, int nthrd, int is_mmap)
{
    lmodel* plm = g_plm[g_lmNum];

    g_plm[g_lmNum] = (lmodel*)malloc(sizeof(lmodel));
    memset(g_plm[g_lmNum], 0, sizeof(lmodel));
    initlm2(g_plm[g_lmNum], lmfn, vfn, nthrd, is_mmap);
    g_isMMAP[g_lmNum] = is_mmap;
    ++g_lmNum;

    return g_lmNum - 1;
}

void g_unloadlm(int lmIdx)
{
    unloadlm2(g_plm[lmIdx], g_isMMAP[lmIdx]);
    free(g_plm[lmIdx]);
}

float g_get_ngram_prob2(int lmIdx, int *ngram, int beg, int end, int trdid)
{
    return get_ngram_prob2(g_plm[lmIdx], ngram, beg, end, -1);
}

#endif /* LMTRAIN_LIB */

/* 
 * PS: The structure of model file:

    -------------------------
   |Header:                  |
   |   1. signature          |
   |   2. order              |
   |   3. unkid              |
   |-------------------------|
   |Count: (4 * N bytes)     |
   |   1. 1-gram count       |
   |   2. 2-gram count       |
   |        .                |
   |        .                |
   |   N. N-gram count       |
   |-------------------------|
   |Offset: (8 * N bytes)    |
   |   1. 1-gram offset      |
   |   2. 2-gram offset      |
   |        .                |
   |        .                |
   |   N. N-gram offset      |
   |-------------------------|
   |All 1-grams:             |
   |   ----------------      |
   |   word id               |
   |   probability           |
   |   backoff weight        |
   |   link                  |
   |   ----------------      |
   |          .              |
   |          .              |
   |          .              |
   |-------------------------|
   |All 2-grams:             |
   |          .              |
   |          .              |
   |          .              |
   |-------------------------|
   |          .              |
   |          .              |
   |          .              |
   |-------------------------|
   |All N-grams:             |
   |   ----------------      |
   |   word id               |
   |   probability           |
   |   ----------------      |
    -------------------------
 */

}
