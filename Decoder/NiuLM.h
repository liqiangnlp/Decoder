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
 * language model wrapper; OurLM.h
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); June 19th, 2011;
 * export functions "lmmain()", "g_initlm()", "g_unloadlm()", "g_get_ngram_prob2()"
 * for both Windows and Linux platforms
 *
 */


#ifndef _OURLM_H_
#define _OURLM_H_


namespace niu_lm {
/* --------------------------------------------------------------------------- */
/*                              language model                                 */
/* --------------------------------------------------------------------------- */

typedef unsigned long long fsize_t;

struct lmodel {
        struct inner_node **inode;
        struct final_node *fnode;
        struct hashtable *vocab;
        int N, unkid, *ngramtype;
        struct cache **caches;
        int ncache;
        char * mem;
        fsize_t memsize;
};

#ifdef WIN32

#ifndef USING_DLL
#define EXPORT_OR_IMPORT __declspec(dllexport)
#else
#define EXPORT_OR_IMPORT __declspec(dllimport)
#endif

#else
#define EXPORT_OR_IMPORT

#endif

extern "C" EXPORT_OR_IMPORT int lmmain(int argc, const char **argv);
extern "C" EXPORT_OR_IMPORT int g_initlm(const char *lmfn, const char *vfn, int nthrd, int is_mmap);
extern "C" EXPORT_OR_IMPORT void g_unloadlm(int lmIdx);
extern "C" EXPORT_OR_IMPORT float g_get_ngram_prob2(int lmIdx, int *ngram, int beg, int end, int trdid);

}

#endif
