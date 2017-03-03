/*
 * $Id:
 * 0001
 *
 * $File:
 * OurTree.h
 *
 * $Proj:
 * Tree Structure
 *
 * $Func:
 * header file of tree function
 *
 * $Version:
 * 0.0.1
 *
 * $Created by:
 * Qiang Li
 *
 * $Email
 * liqiangneu@gmail.com
 *
 * $Last Modified by:
 * 2014-10-29,12:09
 */

#ifndef DECODER_OURTREE_H_
#define DECODER_OURTREE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if !defined( WIN32 ) && !defined( _WIN32 )
#include "sys/time.h"
#include "time.h"
#include "iconv.h"
#else
#include "time.h"
#endif


namespace decoder_our_tree {

#define MTEVAL_NORMALIZATION

typedef unsigned long long ull_t;

#if !defined( WIN32 ) && !defined( _WIN32 )
  #define __MACHINE_64_DATATYPE__
#endif

#ifdef __MACHINE_64_DATATYPE__
  typedef unsigned long long      integer_t;      /* 8 bytes */
  typedef long double             decimal_t;      /* 8 bytes */
#else
  typedef unsigned int            integer_t;      /* 4 bytes */
  typedef float                   decimal_t;      /* 4 bytes */
#endif


//////////////////////////////////////////////////////////////////////////
typedef int       (*CompareT)( const void * vdVar1, const void * vdVar2 );
typedef void      (*FreeT)( const void * vdVar );
typedef integer_t (*SBNT)( const void * vdStr );


//////////////////////////////////////////////////////////////////////////
/*
 *
 * Time
 *
 */
class TimeUtil {
 public:
  TimeUtil();
  ~TimeUtil();
  /* set or get system's current time */
  void SetNowTime( time_t ulNowTime );
  time_t GetNowTime();
  char* GetNowTimeString();
  /* for program timing */
  void StartTimer();
  void EndTimer();
  double GetTimerDiff();

 private:
#if !defined( WIN32 ) && !defined( _WIN32 )
  timeval            stStartTime;
  timeval            stEndTime;
#else
  clock_t            ulStartTime;
  clock_t            ulEndTime;
#endif
  time_t             ulNowTime;
};


//////////////////////////////////////////////////////////////////////////
struct MemBlock {
 public:
  void *       vdMem;
  ull_t        ullMemSize;
  ull_t        ullMemUsed;
};


class MemPool {
 public:
  MemPool();
  MemPool( ull_t ullMemBlkSize, ull_t ullMemBlkNum = 256, bool bMemSzExpandable = true );
  ~MemPool();
  void * Alloc( ull_t ullMemSize );
  void Clear( ull_t ullPreservedSize = 0 );
  ull_t GetUsedMemSize();
  void ShowMemPoolStatus( FILE* p_stFileHandle = stderr );

 protected:
  MemBlock * CreateMemBlock();
  void InitMempool( ull_t ullMemBlkSize, ull_t ullMemBlkNum, bool bMemSzExpandable = true );
  MemBlock * FindMemBlock( ull_t ullMemSize );
  void ClearMemBlock( MemBlock * stMemBlock );

 private:
  /* chain of memory blocks */
  MemBlock**               mpstMemBlocks;
  ull_t                    ullMemBlockNum;
  /* running status of memory pool */
  ull_t                    ullMemTotalSize;
  ull_t                    ullMemUsedSize;
  ull_t                    ullMemBlockMaxSize;
  bool                     bMemSizeExpandable;
  /* maximum range of available memory blocks */
  ull_t                    ullMemBlockLeftIndex;
  ull_t                    ullMemBlockRightIndex;

 private:
  static const ull_t        staticMemBlockMaxSize = (ull_t)1024 * 1024 * 1024 * 16; //10737418240; //1024 * 1024 * 1024 * 16;
};


//////////////////////////////////////////////////////////////////////////
/*
 *
 * data structure to simulate basic C data types, including
 * signed or unsigned char, short, int, long (Integer),
 * float, double (Decimal),
 * and void* (Object).
 *
 */
typedef union __var_u {
 public:
  __var_u& operator=( const __var_u& unData );

 public:
  integer_t           ullInteger;
  decimal_t           ldDecimal;
  void *              mvdObject;
}var_u;



//////////////////////////////////////////////////////////////////////////
struct HashNode {
  void *        vdKey;
  var_u *       unValue;
  HashNode *    nextNode;
};

class HashTable {
 public:
  /* constructors and deconstructor for "new" operator */
  HashTable(unsigned long long ullMyHashTabSz = 1024, long double ldConflictThreshold = 0.75, FreeT fnMyFreeFunc = NULL, \
            CompareT fnKeyCompFunc = NULL, SBNT fnKeyByteNumFunc = NULL);
  ~HashTable();

 public:
  /* constructors and deconstructor for "calloc, malloc, memory pool" operations */
  void Create(unsigned long long ullMyHashTabSz = 1024, FreeT fnMyFreeFunc = NULL, \
              CompareT fnKeyCompFunc = NULL, SBNT fnKeyByteNumFunc = NULL);
  void Destroy();

 public:
  void Clear();
  /* interfaces for "Integer" data types */
  void AddInt( const void* vdKey, integer_t ullValue );
  integer_t GetInt( const void* vdKey );
  /* interfaces for "Decimal" data types */
  void AddFloat( const void* vdKey, decimal_t ldValue );
  decimal_t GetFloat( const void* vdKey );
  /* interfaces for "user-defined" data types */
  void AddObject( const void* vdKey, void* vdValue );
  void* GetObject( const void* vdKey );

 public:
  void* GetKey( unsigned long long ullKeyIdx );
  unsigned long long GetKeyCnt();

 protected:
  unsigned long long ToHashCode( const void* vdKey );
  void Resize( unsigned long long ullNewHashTabSz );
  var_u* GetHashNode( const void* vdKey );
  HashNode* CreateHashNode( const void* vdKey, var_u* unValue );
  var_u* AddHashNode( const void* vdKey, var_u* unValue );

 private:
  static int ASCKeyCompFunc( const void* vdKey1, const void* vdKey2 );
  static integer_t ASCKeyByteNum( const void* vdStr );

 private:
  /* table of hashnode(s) */
  HashNode**                  mstHashTab;
  unsigned long long          ullHashTabSz;
  unsigned long long          ullHashNdCnt;
  /* table of key(s) */
  void**                      mvdKeyTab;
  unsigned long long          ullKeyCnt;
  /* threshold of average "key" conflicts */
  long double                 ldThreshold;

 private:
  MemPool*                  mcsMempool;
  FreeT                       mfnFreeFunc;
  CompareT                    mfnKeyCompFunc;
  SBNT                        mfnKeyByteNumFunc;
};


//////////////////////////////////////////////////////////////////////////
/*
 * common English punctuations
 */
static const char szEnPuncs[] = ".,:;\'\"?!()[]{}/";


class StringUtil {
 protected:
  static HashTable * punctDict;

 public:
  static void TrimRight( char* szLine );
  static char* ReadLine( FILE* stFileHnd );
  static ull_t Split( const char* szSrcStr, const char* szDelim, char**& szTerms );
  static void FreeStringArray( char** szStrArr, ull_t ullStrArrSz );
  static char* ToLowercase( char* szStr );
  static bool IsEnPunc( const char* szToken );
  static bool IsLiteral(const char * word);
  static void ms_mvStr( char* szStr, long lMvOff, bool bToLeft );
  static void LoadPunctDict(const char * dictFileName);
  static void UnloadPunctDict();
  static bool IsPunc(const char * word);
  static char * GeneateString(char * src, MemPool * mem);
  static char * Copy(char * string);
  static char * Copy(char * string, MemPool * mem);
  static void RemoveRightSpaces(char * line);
  static char * NormalizeText(char * line);
  static char * InternationalTokenization(const char * line);
};


//////////////////////////////////////////////////////////////////////////
typedef    int (* ListCompare)(const void * item1, const void * item2);

class List {
 public:
  void **         items;
  int             count;
  int             maxNum;
  MemPool *       mem;

 public:
  List();
  List(int myMaxNum);
  List(int myMaxNum, MemPool * myMem);
  ~List();
  void Create(int myMaxNum, MemPool * myMem);
  void Add(void * item);
  void Add(void ** inputItems, int inputItemCount);
  void Insert(int pos, void * item);
  void * GetItem(int i);
  void SetItem(int i, void * item);
  void Clear();
  void Sort(int itemSize, ListCompare comp);
  void Reverse();
  List * Copy(MemPool * myMem);
};


//////////////////////////////////////////////////////////////////////////
// hyper-node
typedef struct TreeNode {
    char *          label;
    char *          word;
    bool            isLeaf;
    List *          edges;
    int             id;             // node id
    int             beg;            // beginning of corresponding span
    int             end;            // end of corresponding span
    List *          treefrags;      // tree fragments rooting at the node
}* pTreeNode;


//////////////////////////////////////////////////////////////////////////
#define MAX_TREE_NODE_NUM    100000
#define MAX_TREE_EDGE_NUM    100000
#define MAX_WORD_NUM_IN_TREE 256
#define MAX_WORD_LENGTH      1024
#define MAX_LINE_LENGTH      1024 * 100
#define SKELETON_SLOT_SYMBOL "SSX"


typedef struct TreeEdge {
    TreeNode *      parent;      // parent node (head)
    List *          children;    // child nodes (tails)
}* pTreeEdge;


typedef struct TreeStringToken {
  char *          token;
  int             id;
}* pTreeStringToken;


class TreeFrag {
 public:
  TreeNode *   root;
  char *       frontierSequence; // sequence of terminals and frontier non-terminals
  List *       frontierNTs;      // frontier non-terminals

  short        wordNum;
  short        NTNum;
  short        depth;

 public:
  TreeFrag(TreeNode * root);
  TreeFrag();
  ~TreeFrag();
  void AddNT(TreeNode * NT, bool fromLeft, bool useNTSymbol);
  void AddWord(TreeNode * NT, bool fromLeft);
  void AttachLabel(TreeNode * NT);
  void AttachLabelOnly(TreeNode * NT);
  void AttachBrackets();
  TreeFrag * Copy();
  static TreeFrag * Join(TreeFrag * frag1, TreeFrag * frag2);
};


class Tree {
public:
    List *          nodeBase;
    List *          edgeBase;

public:
    TreeNode *      root;
    List *          leaves;

    short           maxWordNum;
    short           maxNTNum;
    short           maxDepth;
    int             maxEdgeNumPerNode;
    int             maxFragNumPerNode;

public:
    Tree();
    ~Tree();
    void Init();
    void Clear();
    List * TokenizeString(const char * string);
    bool CreateTree(const char * string);
    TreeNode * BuildSubTree(List * tokens, int &pos, TreeNode * parentNode);
    bool CreateForest(const char * string);
    void DestroyTokenList(List * tokens);
    TreeStringToken * GenerateToken(char * string, int id);
    TreeNode * CreateTreeNode(char * label, char * word, int id);
    TreeEdge * CreateTreeEdge(TreeNode * parent, int childrenNum);
    void GenerateTreeFragments(bool generateTreeStructure);
    void GenerateTreeFragments(TreeNode * root, bool generateTreeStructure);
    bool IsValidTreeFrag(TreeFrag * frag);
    bool IsValidTreeJoin(TreeFrag * frag1, TreeFrag * frag2);
	char * ToString(bool label = false, int * validWordIndicator = NULL);
	void ToString(char * buf, int &bufLength, TreeNode * rootNode, bool label = false, int * validWordIndicator = NULL);
};


}

#endif
