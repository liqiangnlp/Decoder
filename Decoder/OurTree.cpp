/*
 * $Id:
 * 0001
 *
 * $File:
 * OurTree.cpp
 *
 * $Proj:
 * Tree Structure
 *
 * $Func:
 * source file of tree function
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

#include "OurTree.h"

namespace decoder_our_tree {

//////////////////////////////////////////////////////////////////////////
/**************************************************************************
 *
 * member functions of class "TimeUtil"
 *
***************************************************************************/
TimeUtil::TimeUtil() {
}


TimeUtil::~TimeUtil() {
}


/*
 @ Function:
 * start counting time
 */
void TimeUtil::StartTimer() {
#if !defined( WIN32 ) && !defined( _WIN32 )
  gettimeofday( &stStartTime, NULL );
#else
  ulStartTime = clock();
#endif
}


/*
 @ Function:
 * end counting time
 */
void TimeUtil::EndTimer() {
#if !defined( WIN32 ) && !defined( _WIN32 )
  gettimeofday( &stEndTime, NULL );
#else
  ulEndTime = clock();
#endif
}


/*
 @ Function:
 * get the time difference between actions of end timer counting and start timer counting.
 *
 @ Return:
 * time difference in unit of "second" (double)
 */
double TimeUtil::GetTimerDiff() {
  double dTimeDiff;
#if !defined( WIN32 ) && !defined( _WIN32 )
  dTimeDiff = (double)( stEndTime.tv_sec - stStartTime.tv_sec ) + \
              (double)( stEndTime.tv_usec - stStartTime.tv_usec ) / 1000000;
#else
  dTimeDiff = difftime( ulEndTime, ulStartTime ) / CLOCKS_PER_SEC;
#endif
  return dTimeDiff;
}


/*
 @ Function:
 * set current system's time
 */
void TimeUtil::SetNowTime( time_t ulNowTime ) {
  this->ulNowTime = ulNowTime;
}


/*
 @ Function:
 * get current system's time in seconds
 *
 @ Return:
 * current time in digit format (time_t or long)
 */
time_t TimeUtil::GetNowTime() {
#if !defined( WIN32 ) && !defined( _WIN32 )
  timeval    stCurTime;
  gettimeofday( &stCurTime, NULL );
  ulNowTime = ( time_t ) stCurTime.tv_sec;
  if( stCurTime.tv_usec > 1000000 * 0.5 )
    ulNowTime += 1L;
#else
  time( &ulNowTime );
#endif
  return ulNowTime;
}


/*
 @ Function:
 * get current system's time
 *
 @ Return:
 * current time in string format (char*)
 */
char* TimeUtil::GetNowTimeString() {
  time_t ulNow = GetNowTime();
  char* szTime = ctime( &ulNow );
  int ullTimeStrLen = (int)strlen(szTime);
  if( szTime[ullTimeStrLen - 1] == '\n' ) {
    szTime[ullTimeStrLen - 1] = '\0';
  }
  return szTime;
}


//////////////////////////////////////////////////////////////////////////
/*
 @ Function:
 * get a new memory block from the OS
 */
MemBlock* MemPool::CreateMemBlock() {
  MemBlock* stMemBlock = new MemBlock();
  stMemBlock->vdMem = new unsigned char[ullMemBlockMaxSize];
  stMemBlock->ullMemSize = ullMemBlockMaxSize;
  stMemBlock->ullMemUsed = 0;
  return stMemBlock;
}


/*
 @ Function:
 * initialize class "MemPool"
 *
 @ Arguments:
 * 1) minimum block size in memory array (unsigned long long)
 * 2) minimum memory array size if argument 3) is true, else is maximum array size (unsigned long long)
 * 3) flag showing whether memory array is expandable or not (bool)
 *
 @ Return:
 * NULL
 *
 @ Basic Algorithm:
 * initialize class "MemPool"'s member variables,
 * and only allocate one memory block for the memory array.
 * more memory blocks will be allocated if needed.
 */
void MemPool::InitMempool( ull_t ullMemBlkSize, ull_t ullMemBlkNum, bool bMemSzExpandable ) {
  if( ullMemBlkSize <= 0 ) {
    fprintf( stderr, "%s:\t%s\n", "[ERROR]:", "Argument(s) Wrong Data Range" );
    exit( 1 );
  }
  /* memory chain */
  if( ullMemBlkNum <= 0 ) {
    ullMemBlkNum = 1;
  }
  this->ullMemBlockNum = ullMemBlkNum;
  this->mpstMemBlocks = new MemBlock*[this->ullMemBlockNum];
  memset( this->mpstMemBlocks, 0, this->ullMemBlockNum * sizeof( MemBlock* ) );
  /* memory status */
  //fprintf(stderr, "%lld %lld", ullMemBlkSize, staticMemBlockMaxSize);
  this->ullMemBlockMaxSize = ( ullMemBlkSize >= staticMemBlockMaxSize ? staticMemBlockMaxSize : ullMemBlkSize );
  this->ullMemTotalSize = this->ullMemBlockMaxSize;
  this->ullMemUsedSize = 0;
  this->bMemSizeExpandable = bMemSzExpandable;
  /* info of current block */
  this->ullMemBlockLeftIndex = 0;
  this->ullMemBlockRightIndex = 1;
  MemBlock* stMemBlock = CreateMemBlock();
  this->mpstMemBlocks[this->ullMemBlockLeftIndex] = stMemBlock;        
}


/*
 @ Function:
 * default constructor of class "MemPool".
 * we defaultly construct a expandable 16 * 4K memory pool.
 */
MemPool::MemPool() {
  /* 4K */
  ull_t ullMemBlkSz = 256;
  ull_t ullMemBlkNum = 16;
  bool bMemSzExpandable = true;
  InitMempool( ullMemBlkSz, ullMemBlkNum, bMemSzExpandable );
}


/*
 @ Function:
 * constructor of class "MemPool" with user specified arguments.
 */
MemPool::MemPool( ull_t ullMemBlkSize, ull_t ullMemBlkNum, bool bMemSzExpandable ) {
  InitMempool( ullMemBlkSize, ullMemBlkNum, bMemSzExpandable );
}


/*
 @ Function:
 * deconstructor of class "MemPool"
 */
MemPool::~MemPool() {
  for( int i=0; i < this->ullMemBlockRightIndex; i++ ) {
    delete[] ( unsigned char* ) ( this->mpstMemBlocks[i]->vdMem );
    delete this->mpstMemBlocks[i];
  }
  delete[] this->mpstMemBlocks;
}


/*
 @ Function:
 * find the memory block which can supplies the specified size memory for users,
 * by scanning over all available memory blocks.
 * if no such block exists, request a new memory block that fullfills the users from the OS.
 *
 @ Arguments:
 * size of wanted memory (unsigned long long)
 *
 @ Return:
 * pointer to the right memory block
 * NULL if failed
 *
 @ Basic Algorithm:
 * scan over available memory blocks (indicated by the two indices in the class) to find the right block
 * or request a new block from the OS
 */
MemBlock* MemPool::FindMemBlock( ull_t ullMemSize ) {
  MemBlock* stMemBlock = NULL;

  /* scan over all available memory blocks */
  for( int i=(int)ullMemBlockLeftIndex; i < ullMemBlockRightIndex; i++ ) {
    stMemBlock = mpstMemBlocks[i];
    /* there exists such memory block */
    if( stMemBlock->ullMemUsed + ullMemSize <= stMemBlock->ullMemSize ) {
      return stMemBlock;
    }
  }
  /* no such memory block */
  if( ullMemBlockRightIndex == ullMemBlockNum ) {
    /* the mempool is full and non-expandable */
    if( !bMemSizeExpandable )
      return NULL;
    else {
      MemBlock** pp_stCreateMemBlocks = new MemBlock*[ullMemBlockNum * 2 + 1];
      memset( pp_stCreateMemBlocks, 0, ( ullMemBlockNum * 2 + 1 ) * sizeof( MemBlock* ) );
      memcpy( pp_stCreateMemBlocks, mpstMemBlocks, ullMemBlockNum * sizeof( MemBlock* ) );
      ullMemBlockNum = ullMemBlockNum * 2 + 1;
      delete[] mpstMemBlocks;
      mpstMemBlocks = pp_stCreateMemBlocks;
    }
  }
  /* allocate a new block from the OS */
  if( ullMemSize > ullMemBlockMaxSize && ullMemSize <= staticMemBlockMaxSize ) {
    ullMemBlockMaxSize = ullMemSize;
  }
  stMemBlock = CreateMemBlock();
  mpstMemBlocks[ullMemBlockRightIndex++] = stMemBlock;
  ullMemTotalSize += ullMemBlockMaxSize;
  return stMemBlock;
}


/*
 @ Function:
 * allocate memory of specified size from memory pool for users
 *
 @ Arguments:
 * size of wanted memory (unsigned long long)
 *
 @ Return:
 * pointer to the allocated memory for users (void*)
 * the memory has been initialized with value "0".
 *
 @ Basic Algorithm:
 * 1) make sure enough memory left in memory pool to satify the users,
 *    by sometimes allocating new memory blocks from the OS.
 * 2) allocate memory by changing used memory offset and updating memory pool's status.
 */
void* MemPool::Alloc( ull_t ullMemSize ) {
  void* p_vdMem = NULL;
  /* wrong input data range, return null pointer */
  if( ullMemSize <= 0 )
    return NULL;

  /* find the block from which the memory will be allocated */
  MemBlock* stMemBlock = FindMemBlock( ullMemSize );
  if( !stMemBlock ) {
    return NULL;
  }

  /* allocate the requested memory from the found block, and update mempool's status */
  p_vdMem = ( void* ) ( ( unsigned char* ) stMemBlock->vdMem + stMemBlock->ullMemUsed );
  memset( p_vdMem, 0, sizeof( unsigned char ) * ullMemSize );
  stMemBlock->ullMemUsed += ullMemSize;
  ullMemUsedSize += ullMemSize;

  /* update maximum range of available memory blocks */
  while( ullMemBlockLeftIndex < ullMemBlockRightIndex ) {
    MemBlock* p_stTmpMemBlk = mpstMemBlocks[ullMemBlockLeftIndex];
    ull_t ullAvailMemSz = p_stTmpMemBlk->ullMemSize - p_stTmpMemBlk->ullMemUsed;
    ull_t ullDynamicThresh = ( ull_t ) ( ullMemBlockMaxSize * 0.01 );
    ull_t ullStaticThresh = 256;
    if( ullAvailMemSz > ullDynamicThresh || ullAvailMemSz > ullStaticThresh ) {
      break;
    }
    ++ullMemBlockLeftIndex;
  }
  return p_vdMem;
}


/*
 @ Function:
 * clear one memory block for reusing
 *
 @ Arguments:
 * pointer to the memory block (MemBlock*)
 *
 @ Return:
 * NULL
 *
 @ Basic Algorithm:
 * clear the memory pointed by the block
 * and reset using status of the block
 */
void MemPool::ClearMemBlock( MemBlock* stMemBlock ) {
  stMemBlock->ullMemUsed = 0;
}


/*
 @ Function:
 * clear the data beyond the specified memory bound of the memory pool
 *
 @ Agruments:
 * preserved size of memory (unsigned long long)
 *
 @ Return:
 * NULL
 *
 @ Basic Algorithm:
 * reset to be "0" within the specified bound of memory
 * free the memory blocks beyond the bound
 */
void MemPool::Clear( ull_t ullPreservedSize ) {
  /* always preserve the first block in memory pool */
  ull_t ullCurSz = mpstMemBlocks[0]->ullMemSize;
  ClearMemBlock( mpstMemBlocks[0] );
  ull_t ullIdx = 1;
  /* continue to clear memory block until the size of clean memory is not less than that requested */
  while( ullIdx < ullMemBlockRightIndex && ullCurSz < ullPreservedSize ) {
    MemBlock* stMemBlock = mpstMemBlocks[ullIdx];
    ullCurSz += stMemBlock->ullMemSize;
    ClearMemBlock( stMemBlock );
    ++ullIdx;
  }

  /* free all the left memory blocks */
  for( int i=(int)ullIdx; i < ullMemBlockRightIndex; i++ ) {
    delete[] (unsigned char*)(mpstMemBlocks[i]->vdMem);
    delete mpstMemBlocks[i];
  }
  /* update using status of the memory pool */
  ullMemBlockNum = ullIdx;
  ullMemTotalSize = ullCurSz;
  ullMemUsedSize = 0;
  ullMemBlockLeftIndex = 0;
  ullMemBlockRightIndex = ullIdx;
}


/*
 @ Function:
 * get the size of total used memory
 *
 @ Return:
 * the used memory size (unsigned long long)
 */
ull_t MemPool::GetUsedMemSize() {
  return ullMemUsedSize;
}


/*
 @ Function:
 * display current running status of the memory pool
 *
 @ Arguments:
 * file pointer to which the program writes information
 *
 @ Return:
 * NULL
 *
 */
void MemPool::ShowMemPoolStatus( FILE* p_stFileHandle ) {
  fprintf( p_stFileHandle, "               Memory Pool Running Status\n" );
  fprintf( p_stFileHandle, "\n" );
  fprintf( p_stFileHandle, "-----------------------------------------------------------\n" );
  fprintf( p_stFileHandle, "\n" );
  fprintf( p_stFileHandle, ">> Total Size:                  %20llu bytes\n", ullMemTotalSize );
  fprintf( p_stFileHandle, ">> Used Size:                   %20llu bytes\n", ullMemUsedSize );
  fprintf( p_stFileHandle, ">> Available Size:              %20llu bytes\n", ullMemTotalSize - ullMemUsedSize );
  fprintf( p_stFileHandle, ">> Expandable:                  %20d\n", bMemSizeExpandable );
  fprintf( p_stFileHandle, ">> Memory Block Number:         %20llu\n", ullMemBlockRightIndex );
  fprintf( p_stFileHandle, "\n" );
  fprintf( p_stFileHandle, "-----------------------------------------------------------\n" );
  fprintf( p_stFileHandle, "\n" );
  fprintf( p_stFileHandle, ">> Memory Blocks\' Status:\n" );
  fprintf( p_stFileHandle, ">> Left Index:                  %20llu\n", ullMemBlockLeftIndex );
  fprintf( p_stFileHandle, "\n" );
  for( ull_t i=0; i < ullMemBlockRightIndex; i++ ) {
    fprintf( p_stFileHandle, ">>>> $ Block ID:                %20llu\n", i );
    fprintf( p_stFileHandle, "     $ Total Size:              %20llu bytes\n", mpstMemBlocks[i]->ullMemSize );
    fprintf( p_stFileHandle, "     $ Used Size:               %20llu bytes\n", mpstMemBlocks[i]->ullMemUsed );
    fprintf( p_stFileHandle, "     $ Available Size:          %20llu bytes\n", mpstMemBlocks[i]->ullMemSize - \
             mpstMemBlocks[i]->ullMemUsed );
    fprintf( p_stFileHandle, "\n" );
  }
  fprintf( p_stFileHandle, "-----------------------------------------------------------\n" );
}


//////////////////////////////////////////////////////////////////////////
/**************************************************************************
*
* assignment (i.e., =) operator overloading for generic inner data types
*
***************************************************************************/
__var_u& var_u::operator =( const __var_u &unData ) {
  this->ullInteger = unData.ullInteger;
  this->ldDecimal = unData.ldDecimal;
  this->mvdObject = unData.mvdObject;
  return *this;
}


//////////////////////////////////////////////////////////////////////////
/********************************************
 *
 * Hash Table's Member Functions
 *
 ********************************************/

/*
 @ Function:
 * compare two ASCII keys
 *
 @ Arguments:
 * 1) the first key (void*)
 * 2) the second key (void*)
 *
 @ Return:
 * 1) "-1" means the first key is smaller than the second key
 * 2) "0" means the first key is equal to the second key
 * 3) "1" means the first key is larger than the second key
 */
int HashTable::ASCKeyCompFunc( const void* vdKey1, const void* vdKey2 ) {
  return strcmp( ( const char* ) vdKey1, ( const char* ) vdKey2 );
}


/*
 @ Function:
 * get number of bytes occupied by an ASCII key
 *
 @ Arguments:
 * pointer to the key (void*)
 *
 @ Return:
 * number of bytes (integer_t)
 */
integer_t HashTable::ASCKeyByteNum( const void* vdStr ) {
  return (integer_t)( strlen( ( const char* ) vdStr ) + 1 );
}


/*
 @ Function:
 * constructor of class "HashTable"
 *
 @ Arguments:
 * 1) size of hash table (unsigned long long)
 * 2) element free function (FreeT)
 * 3) key comparison function (CompareT)
 * 4) key length calculation function (SBNT)
 */
HashTable::HashTable( unsigned long long ullMyHashTabSz, long double ldConflictThreshold, FreeT fnMyFreeFunc, \
  CompareT fnKeyCompFunc, SBNT fnKeyByteNumFunc ) {
  ldThreshold = ldConflictThreshold;
  Create( ullMyHashTabSz, fnMyFreeFunc, fnKeyCompFunc, fnKeyByteNumFunc );
}


/*
 @ Function:
 * create and initialize a new hash table
 *
 @ Arguments:
 * 1) size of hash table (unsigned long long)
 * 2) element free function (FreeT)
 * 3) key comparison function (CompareT)
 * 4) key length calculation function (SBNT)
 */
void HashTable::Create( unsigned long long ullMyHashTabSz, FreeT fnMyFreeFunc, CompareT fnKeyCompFunc, SBNT fnKeyByteNumFunc ) {
  this->ullHashTabSz = ( ullMyHashTabSz == 0 ? 1024 : ullMyHashTabSz );
  this->mcsMempool = new MemPool( (ull_t)(ullHashTabSz * ldThreshold * sizeof( HashNode )) );
  this->mstHashTab = ( HashNode** ) mcsMempool->Alloc( ullHashTabSz * sizeof( HashNode* ) );
  this->ullHashNdCnt = 0;
  this->mvdKeyTab = ( void** ) mcsMempool->Alloc( ullHashTabSz * sizeof( void* ) );
  this->ullKeyCnt = 0;
  this->mfnFreeFunc = fnMyFreeFunc;
  this->mfnKeyCompFunc = ( fnKeyCompFunc == NULL ? HashTable::ASCKeyCompFunc : fnKeyCompFunc );
  this->mfnKeyByteNumFunc = ( fnKeyByteNumFunc == NULL ? HashTable::ASCKeyByteNum : fnKeyByteNumFunc );
}


/*
 @ Function:
 * deconstructor of class "HashTable"
 */
HashTable::~HashTable() {
  Destroy();
}


/*
 @ Function:
 * destroy the hash table
 */
void HashTable::Destroy() {
  for( unsigned long long i=0; i < ullHashTabSz; i++ ) {
    HashNode* stCurHashNd = mstHashTab[i];
    while( stCurHashNd != NULL ) {
      HashNode* stTmpHashNd = stCurHashNd;
      stCurHashNd = stCurHashNd->nextNode;
      if( stTmpHashNd->unValue->mvdObject != NULL && mfnFreeFunc != NULL ) {
        mfnFreeFunc( stTmpHashNd->unValue->mvdObject );
      }
    }
  }
  delete mcsMempool;
}


/*
 @ Function:
 * clear the hash table for reuse
 */
void HashTable::Clear() {
  Destroy();
  Create( ullHashTabSz, mfnFreeFunc, mfnKeyCompFunc, mfnKeyByteNumFunc );
}


/*
 @ Function:
 * code a given "string" to an "integer" data
 *
 @ Arguments:
 * pointer to the string ended with character '\0' (void*)
 *
 @ Return:
 * a hash code (unsigned long long)
 */
unsigned long long HashTable::ToHashCode( const void* vdKey ) {
  unsigned long long ullHashCode = 0, g = 0;
  const char* szStr = ( const char* ) vdKey;

  while( *szStr != '\0' ) {
    ullHashCode <<= 4;
    ullHashCode += ( unsigned long long ) *szStr;
    g = ullHashCode & ( ( unsigned long long ) 0xf << ( 64 - 4 ) );
    if( g != 0 ) {
      ullHashCode ^= g >> ( 64 - 8 );
      ullHashCode ^= g;
    }
    ++szStr;
  }
  return ullHashCode;
}


/*
 @ Function:
 * Resize the hash table
 *
 @ Arguments:
 * new size of hash table (unsigned long long)
 */
void HashTable::Resize( unsigned long long ullNewHashTabSz ) {
  if( ullNewHashTabSz <= ullHashTabSz ) {
    return;
  }

  /* backup info of old hash table */
  unsigned long long ullOldHashTabSz = ullHashTabSz;
  MemPool* csOldMempool = mcsMempool;
  HashNode** stOldHashTab = mstHashTab;

  /* update info of new hash table */
  ullHashTabSz = ullNewHashTabSz;
  mcsMempool = new MemPool( (ull_t)(ullHashTabSz * ldThreshold * sizeof( HashNode )) );
  mstHashTab = ( HashNode** ) mcsMempool->Alloc( ullHashTabSz * sizeof( HashNode* ) );
  mvdKeyTab = ( void** ) mcsMempool->Alloc( ullHashTabSz * sizeof( void* ) );
  ullHashNdCnt = 0;
  ullKeyCnt = 0;
  /* transfer hash nodes in old hash table to new hash table */
  for( unsigned long long i=0; i < ullOldHashTabSz; i++ ) {
    HashNode* stHashNd = stOldHashTab[i];
    while( stHashNd != NULL ) {
      HashNode* stHashNdLast = stHashNd;
      stHashNd = stHashNd->nextNode;
      AddHashNode( stHashNdLast->vdKey, stHashNdLast->unValue );
    }
  }

  /* eliminate old hash table */
  delete csOldMempool;
}


/*
 @ Function:
 * get a hash node specified by "key"
 *
 @ Arguments:
 * key (void*)
 *
 @ Return:
 * pointer to the result data of inner data type (var_u*)
 */
var_u* HashTable::GetHashNode( const void* vdKey ) {
  unsigned long long ullHashCode = ToHashCode( vdKey );
  HashNode* stHashNd = mstHashTab[ullHashCode % ullHashTabSz];

  while( stHashNd != NULL ) {
    int r = mfnKeyCompFunc( stHashNd->vdKey, vdKey );
    if( r < 0 ) {
      stHashNd = stHashNd->nextNode;
    }
    else if( r == 0 ) {
      return stHashNd->unValue;
    }
    else {
      break;
    }
  }
  return NULL;
}


/*
 @ Function:
 * create and initialize a new hash node
 *
 @ Arguments:
 * 1) key (void*)
 * 2) value (var_u*)
 *
 @ Return:
 * pointer to the new hash node
 */
HashNode* HashTable::CreateHashNode( const void* vdKey, var_u* unValue ) {
  /* new hash node */
  HashNode* stNewHashNd = ( HashNode* ) mcsMempool->Alloc( sizeof( HashNode ) );
  /* initialize "key" of new hash node */
  integer_t iByteNum = mfnKeyByteNumFunc( vdKey );
  stNewHashNd->vdKey = ( void* ) mcsMempool->Alloc( iByteNum * sizeof( char ) );
  memcpy( stNewHashNd->vdKey, vdKey, iByteNum );
  /* initialize "value" of new hash node */
  stNewHashNd->unValue = ( var_u* ) mcsMempool->Alloc( sizeof( var_u ) );
  *stNewHashNd->unValue = *unValue;
  /* initialize pointer to the next hash node */
  stNewHashNd->nextNode = NULL;
  return stNewHashNd;
}


/*
 @ Function:
 * add a hash node identified by "key" into the hash table
 * if the node of the specified "key" is already exists, update its "value"
 *
 @ Arguments:
 * 1) key (void*)
 * 2) value (var_u*)
 *
 @ Return:
 * pointer to the value (var_u*)
 */
var_u* HashTable::AddHashNode( const void* vdKey, var_u* unValue ) {
  /* if there are too many "key" conflicts */
  long double r = ( ( long double ) ullHashNdCnt / ( long double ) ullHashTabSz );
  if( r >= ldThreshold ) {
    Resize( ullHashTabSz * 2 + 1 );
  }

  /* search the hash node identified by "key" */
  unsigned long long ullHashCode = ToHashCode( vdKey );
  HashNode* stHashNd = mstHashTab[ullHashCode % ullHashTabSz];
  HashNode* stHashNdLast = stHashNd;

  while( stHashNd != NULL ) {
    int r = mfnKeyCompFunc( stHashNd->vdKey, vdKey );
    if( r < 0 ) {
      stHashNdLast = stHashNd;
      stHashNd = stHashNd->nextNode;
    }
    else if( r == 0 ) {
      /* already existed in hash table */
      if( stHashNd->unValue->mvdObject != NULL && mfnFreeFunc != NULL ) {
        mfnFreeFunc( stHashNd->unValue->mvdObject );
      }
      *stHashNd->unValue = *unValue;
      return stHashNd->unValue;
    }
    else {
      break;
    }
  }

  /* no such hash node */
  HashNode* stNewHashNd = CreateHashNode( vdKey, unValue );
  if( stHashNd == mstHashTab[ullHashCode % ullHashTabSz] ) {
    stNewHashNd->nextNode = mstHashTab[ullHashCode % ullHashTabSz];
    mstHashTab[ullHashCode % ullHashTabSz] = stNewHashNd;
  }
  else {
    stNewHashNd->nextNode = stHashNdLast->nextNode;
    stHashNdLast->nextNode = stNewHashNd;
  }
  ++ullHashNdCnt;
  mvdKeyTab[ullKeyCnt++] = stNewHashNd->vdKey;
  return stNewHashNd->unValue;
}


/*
 @ Function:
 * add a new ( key, value<Integer> ) pair into the hash table
 *
 @ Arguments:
 * 1) key (void*)
 * 2) value (unsigned long long)
 */
void HashTable::AddInt( const void* vdKey, integer_t ullValue ) {
  var_u unTmpVal = {0};
  unTmpVal.ullInteger = ullValue;
  AddHashNode( vdKey, &unTmpVal );
}


/*
 @ Function:
 * get an "Integer" value from the hash table
 *
 @ Arguments:
 * key (void*)
 *
 @ Return:
 * value (unsigned long long)
 */
integer_t HashTable::GetInt( const void* vdKey ) {
  var_u* unVal = GetHashNode( vdKey );
  if( unVal != NULL ) {
    return unVal->ullInteger;
  }
  else {
    return ( integer_t ) -1;
  }
}


/*
 @ Function:
 * add a new ( key, value<Decimal> ) pair into the hash table
 *
 @ Arguments:
 * 1) key (void*)
 * 2) value (long double)
 */
void HashTable::AddFloat( const void* vdKey, decimal_t ldValue ) {
  var_u unTmpVal = {0};
  unTmpVal.ldDecimal = ldValue;
  AddHashNode( vdKey, &unTmpVal );
}


/*
 @ Function:
 * get a "Decimal" value from the hash table
 *
 @ Arguments:
 * key (void*)
 *
 @ Return:
 * value (long double)
 */
decimal_t HashTable::GetFloat( const void* vdKey ) {
  var_u* unVal = GetHashNode( vdKey );
  if( unVal != NULL ) {
    return unVal->ldDecimal;
  }
  else {
    return -1.0;
  }
}


/*
 @ Function:
 * add a new ( key, value<user-defined complex objects> ) pair into the hash table
 *
 @ Arguments:
 * 1) key (void*)
 * 2) value (void*)
 */
void HashTable::AddObject( const void* vdKey, void* vdValue ) {
  var_u unTmpVal = {0};
  unTmpVal.mvdObject = vdValue;
  AddHashNode( vdKey, &unTmpVal );
}


/*
 @ Function:
 * get a complex "user-defined" object from the hash table
 *
 @ Arguments:
 * key (void*)
 *
 @ Return:
 * pointer to the value (void*)
 */
void* HashTable::GetObject( const void* vdKey ) {
  var_u* unVal = GetHashNode( vdKey );
  if( unVal != NULL ) {
    return unVal->mvdObject;
  }
  else {
    return NULL;
  }
}


/*
 @ Function:
 * get the number of different keys
 *
 @ Return:
 * different key number (unsigned long long)
 */
unsigned long long HashTable::GetKeyCnt() {
  return ullKeyCnt;
}


/*
 @ Function:
 * get a key by its index which indicates the timestamp
 * the key was added
 *
 @ Arguments:
 * index of the key (unsigned long long)
 *
 @ Return:
 * pointer to the key (void*)
 */
void* HashTable::GetKey( unsigned long long ullKeyIdx ) {
  if( ullKeyIdx < 0 || ullKeyIdx >= ullKeyCnt ) {
    return NULL;
  }
  return mvdKeyTab[ullKeyIdx];
}


//////////////////////////////////////////////////////////////////////////
/**************************************************************************
 *
 * "string processing" functions, which are of static scopes
 *
***************************************************************************/
/*
 @ Function:
 * eliminate all '\r' and '\n' characters at the end of line
 *
 @ Arguments:
 * pointer to string
 *
 @ Return:
 * NULL
 */
void StringUtil::TrimRight( char* IsEnPuncszLine ) {
  long long llStrLen = strlen( IsEnPuncszLine );
  long long llIdx = llStrLen - 1;
  while( llIdx >= 0 ) {
    if( IsEnPuncszLine[llIdx] != '\r' && IsEnPuncszLine[llIdx] != '\n' ) {
      break;
    }
    IsEnPuncszLine[llIdx] = '\0';
    --llIdx;
  }
}


/*
 @ Function:
 * read one line from the given input stream
 *
 @ Arguments:
 * file pointer as input stream (FILE*)
 *
 @ Return:
 * pointer to the read line
 *
 @ Basic Algorithm:
 * incrementally read in data from input stream
 * until encounter the character '\n' or reach the end of file
 */
char* StringUtil::ReadLine( FILE* IsEnPuncstFileHnd ) {
  ull_t ullLineLen = 128;
  char* IsEnPuncszLine = new char[ullLineLen];
  if( !fgets( IsEnPuncszLine, (int)ullLineLen, IsEnPuncstFileHnd ) ) {
    if( feof( IsEnPuncstFileHnd ) ) {
    }
    else if( ferror( IsEnPuncstFileHnd ) ) {
      fprintf( stderr, "[ERROR]: reading file failed!\n" );
    }
    delete[] IsEnPuncszLine;
    return NULL;
  }
  /* read the rest part of line into string buffer */
  while( IsEnPuncszLine[strlen( IsEnPuncszLine ) - 1] != '\n' ) {
    char* IsEnPuncszTmpPtr = NULL;
    ull_t ullTmpLen = ullLineLen + 1;
    char* IsEnPuncszOldLine = IsEnPuncszLine;
    ullLineLen *= 2;
    IsEnPuncszLine = new char[ullLineLen];
    strcpy( IsEnPuncszLine, IsEnPuncszOldLine );
    delete[] IsEnPuncszOldLine;
    IsEnPuncszTmpPtr = IsEnPuncszLine + strlen( IsEnPuncszLine );
    if( !fgets( IsEnPuncszTmpPtr, (int)ullTmpLen, IsEnPuncstFileHnd ) ) {
      if( feof( IsEnPuncstFileHnd ) ) {
        break;
      }
      else {
        fprintf( stderr, "[ERROR]: reading file failed!\n" );
        delete[] IsEnPuncszLine;
        return NULL;
      }
    }
  }
  /* eliminate carriage return and line feed characters at the end of line */
  TrimRight( IsEnPuncszLine );
  return IsEnPuncszLine;
}


/*
 @ Function:
 * split source string into several tokens which are originally separated by specified delimiter
 * the function does not break the source string
 *
 @ Arguments:
 * 1) pointer to source string (const char*)
 * 2) pointer to delimiter (const char*)
 * 3) reference to token arrays as split results (char**&)
 *
 @ Return:
 * number of tokens generated (unsigned long long)
 *
 @ Basic Algorithm:
 * 1) recognize and record all tokens between one or more delimiters
 * 2) construct array of tokens
 */
ull_t StringUtil::Split( const char* szSrcStr, const char* szDelim, char**& szTerms ) {
  ull_t ullSrcStrLen = strlen( szSrcStr );
  ull_t ullDelimLen = strlen( szDelim );
  /* empty source string or the source string is exactly the same with the delimiter string */
  if( ullSrcStrLen == 0 || !strcmp( szSrcStr, szDelim ) ) {
    szTerms = NULL;
    return 0;
  }
  /* an empty delimiter string or delimiter string is not shorter than the source string */
  if( ullDelimLen == 0 || ullDelimLen >= ullSrcStrLen  ) {
    szTerms = new char*[1];
    szTerms[0] = new char[ullSrcStrLen + 1];
    strcpy( szTerms[0], szSrcStr );
    return 1;
  }
  /* now for general case, that is, the delimiter string is shorter than the source string :) */
  szTerms = new char*[ullSrcStrLen + 1];
  ull_t ullTermCnt = 0;
  bool matched = false;
  /* scan the source string from left to right */
  for( int i=0; i < ullSrcStrLen; ) {
    /* match delimiter */
    if( !strncmp( &szSrcStr[i], szDelim, ullDelimLen ) ) {
      i += (int)ullDelimLen;
      if(!matched || i == ullSrcStrLen){
        szTerms[ullTermCnt] = new char[1];
        szTerms[ullTermCnt][0] = '\0';
        ++ullTermCnt;
      }
      matched = false;
    }
    /* match token */
    else {
      /* find end boundary of the matched token */
      ull_t j = i + 1;
      while( j < ullSrcStrLen && strncmp( &szSrcStr[j], szDelim, ullDelimLen ) ) {
        ++j;
      }
      /* record the matched token */
      ull_t ullTokenLen = j - i;
      szTerms[ullTermCnt] = new char[ullTokenLen + 1];
      strncpy( szTerms[ullTermCnt], &szSrcStr[i], ullTokenLen );
      szTerms[ullTermCnt][ullTokenLen] = '\0';
      ++ullTermCnt;
      i = (int)j;
      matched = true;
    }
  }
  return ullTermCnt;
}


/*
 @ Function:
 * free a string array
 *
 @ Arguments:
 * 1) pointer to the string array
 * 2) size of the string array
 */
void StringUtil::FreeStringArray( char** szStrArr, ull_t ullStrArrSz ) {
  for( ull_t i=0; i < ullStrArrSz; i++ ) {
    delete[] szStrArr[i];
  }
  delete[] szStrArr;
}


/*
 @ Function:
 * change a source string to a string in lowercase
 *
 @ Arguments:
 * the source string (char*)
 *
 @ Return:
 * pointer to the changed string
 *
 @ Basic Algorithm:
 * scan over the source string and change any character which is in uppercase
 */
char* StringUtil::ToLowercase( char* szStr ) {
  char* szChPtr = szStr;
  while( *szChPtr != '\0' ) {
    if( *szChPtr >= 'A' && *szChPtr <= 'Z' ) {
      *szChPtr += 'a' - 'A';
    }
    ++szChPtr;
  }
  return szStr;
}


/*
 @ Function:
 * judge whether a token is an English (in ASCII code) punctuation
 *
 @ Arguments:
 * pointer to the token (const char*)
 *
 @ Return:
 * true if it is an English punctuation
 * flase if it is not (bool)
 *
 @ Basic Algorithm:
 * judge whether the token has only one character and is a member of pre-specified punctuations
 */
bool StringUtil::IsEnPunc( const char* szToken ) {
  if( strlen( szToken ) == 1 && strstr( szEnPuncs, szToken ) )
    return true;
  return false;
}


bool StringUtil::IsLiteral(const char * word) {
  int len = (int)strlen(word);

  for(int i = 0; i < len; i++){
    if(word[i] < 0 || word[i] > 127)
      return false;
  }
  return true;
}


HashTable * StringUtil::punctDict = NULL;


void StringUtil::LoadPunctDict(const char * dictFileName) {
  int maxWordLen = 1024;
  int punctCount = 0;
  char * punct = new char[maxWordLen];

  punctDict = new HashTable(1000);

  FILE * f = fopen(dictFileName, "rb");
  if(f == NULL){
    fprintf(stderr, "ERROR: cannot open file \"%s\"!", dictFileName);
    return;
  }

  TimeUtil timer;
  timer.StartTimer();
  fprintf( stderr, "Loading Punctuations\n");
  fprintf( stderr, "  >> From File: %s ...\n", dictFileName );
  fprintf( stderr, "  >> " );

  while(fgets(punct, maxWordLen - 1, f)){
    for(int i = (int)strlen(punct) - 1; i >= 0; i--){
      if(punct[i] == '\r' || punct[i] == '\n')
        punct[i] = '\0';
      else
        break;
    }

    punctDict->AddInt(punct, ++punctCount);
    if( punctCount % 5 == 0 ) {
      fprintf( stderr, "#" );
    }
  }
  timer.EndTimer();
  double time = timer.GetTimerDiff();
  fprintf( stderr, "\nDone [%d entries, %.3f sec(s)]\n", punctCount, time );
  delete[] punct;
}


void StringUtil::UnloadPunctDict() {
  delete punctDict;
}


bool StringUtil::IsPunc(const char * word) {
  if( strlen( word ) == 1 && strstr( szEnPuncs, word ) )
    return true;
  if( punctDict != NULL ){
    int pid = punctDict->GetInt(word);
    if(pid != -1){
      //fprintf(stderr, "punct: %d %s\n", punctDict->GetInt(word), word);
      return true;
    }
  }
  return false;
}


char * StringUtil::GeneateString(char * src, MemPool * mem) {
  char * s = (char *)mem->Alloc(sizeof(char) * ((int)strlen(src) + 1));
  strcpy(s, src);
  return s;
}


char * StringUtil::Copy(char * string) {
  int length = (int)strlen(string);
  char * s = new char[length + 1];
  strcpy(s, string);
  return s;
}


char * StringUtil::Copy(char * string, MemPool * mem) {
  int length = (int)strlen(string);
  char * s = (char*)mem->Alloc(length + 1);
  strcpy(s, string);
  return s;
}


void StringUtil::RemoveRightSpaces(char * line) {
  long long llStrLen = strlen(line);
  long long llIdx = llStrLen - 1;
  while( llIdx >= 0 ) {
    if( line[llIdx] != ' ' && line[llIdx] != '\t' ) {
      break;
    }
    line[llIdx] = '\0';
    --llIdx;
  }
}


char * StringUtil::NormalizeText(char * line) {
  int length = (int)strlen(line);
  char * newText = new char[length * 3 + 1];
  int i = 0, j = 0;

  newText[0] = '\0';

  if(length == 0)
    return newText;

  for(; i < length; i++)
    if( line[i] != ' ' && line[i] != '\t' )
      break;

    for(; i < length; i++){
      int op = 0;

    if( line[i] == '\n' || line[i] == '\r' )
      op = 1;
#ifdef MTEVAL_NORMALIZATION
    else if(line[i] != ' '){
      if( line[i] >= '{' && line[i] <= '~' ) // 0x7b-0x7e
        op = 2;
      else if( line[i] >= '[' && line[i] <= '`') // 0x5b-0x60
        op = 2;
      else if( line[i] >= ' ' && line[i] <= '&') // 0x20-0x26
        op = 2;
      else if( line[i] >= '(' && line[i] <= '+') // 0x28-0x2b
        op = 2;
      else if( line[i] >= ':' && line[i] <= '@') // 0x3a-0x40
        op = 2;
      else if( line[i] == '/') // 0x2f
        op = 2;
    }

    if(i > 0){
      if((line[i-1] < '0' || line[i-1] > '9') && (line[i] == '.' || line[i] == ',')) // ([^0-9])([\.,])"
        op = 3;
      else if( (line[i-1] >= '0' && line[i-1] <= '9') && line[i] == '-') // "([0-9])(-)"
        op = 3;
      else if( (line[i-1] == '.' || line[i-1] == ',') && (line[i] < '0' || line[i] > '9')) // "([\.,])([^0-9])"
        op = 4;
    }
#else
    else if(line[i] == '%')
      op = 1;
    else if(i > 0 && i < length - 2 && line[i-1] == ' ' && line[i] == '\'' &&
            line[i+1] == 's' && line[i+2] == ' ')
      op = 5;
    else if(i > 1 && i < length - 1 && line[i-2] == 's' && line[i-1] == ' ' &&
            line[i] == '\'' && line[i+1] == ' ')
      op = 5;
    else if(i > 2 && ((line[i-3] >= 'a' && line[i-3] <= 'z') || (line[i-3] >= 'A' && line[i-3] <= 'Z')) &&
            line[i-2] == ' ' && line[i-1] == '-' && line[i] == ' ')
      op = 6;
#endif

    if( op == 0 ){              // "$1" => "$1" where "$1" = line[i]
      newText[j++] = line[i];
    }
    else if( op == 1){          // "$1" => " $1"  where "$1" = line[i]
      newText[j++] = ' ';
      newText[j++] = line[i];
    }
    else if( op == 2 ){         // "$1" => " $1 "  where "$1" = line[i]
      newText[j++] = ' ';
      newText[j++] = line[i];
      newText[j++] = ' ';
    }
    else if(op == 3){           // "$1$2" => "$1 $2 "  where "$1" = line[i-1] and "$2" = line[i]
      newText[j++] = ' ';
      newText[j++] = line[i];
      newText[j++] = ' ';
    }
    else if(op == 4){           // "$1$2" => " $1 $2"  where "$1" = line[i-1] and "$2" = line[i]
      if( newText[j-1] == line[i-1] ){
        newText[j] = newText[j-1];
        newText[j-1] = ' ';
        j++;
        newText[j++] = ' ';
        newText[j++] = line[i];
      }
      else
        newText[j++] = line[i];
    }
    else if(op == 5){          // " $1" => "$1" where where "$1" = line[i] 
      newText[j-1] = line[i];
    }
    else if(op == 6){          // " $1 " => "$1" where where "$1" = line[i-1]
      newText[j-2] = newText[j-1];
      j--;
    }
  }

  // remove sequencial spaces
  int p = 1, q = 1;
  for(; q < j;  q++){
    if( newText[p-1] != ' ' || newText[q] != ' ' )
      newText[p++] = newText[q];
  }

  // remove head spaces
  q = 0;
  while( q < p && newText[q] == ' ' ) ++q;
  if( q == p ) {
    newText[0] = '\0';
    return newText;
  }
  else {
    strncpy( newText, &newText[q], p-q );
    newText[p-q] = '\0';
    p = p - q;
  }

  // remove trailing spaces
  while( p > 0 && newText[p-1] == ' ' ) --p;

  if( length > 1)
    newText[p] = '\0';
  return newText;
}


//////////////////////////////////////////////////////////////////////////
// list
List::List() {
  mem    = NULL;
  maxNum = 0;
  count  = 0;
  items  = NULL;
}


List::List(int myMaxNum) {
  mem    = NULL;
  maxNum = myMaxNum;
  count  = 0;
  items  = new void*[myMaxNum];
}


List::List(int myMaxNum, MemPool * myMem) {
  mem    = myMem;
  maxNum = myMaxNum;
  count  = 0;
  items  = (void**)mem->Alloc(sizeof(void*) * maxNum);
}


List::~List() {
  if(mem == NULL)
    delete[] items;
}


void List::Create(int myMaxNum, MemPool * myMem) {
  mem    = myMem;
  maxNum = myMaxNum;
  count  = 0;
  items  = (void**)mem->Alloc(sizeof(void*) * maxNum);
}


void List::Add(void * item) {
  if( count == maxNum ){
    void ** newItems;
    if( mem == NULL )
      newItems = new void*[maxNum * 2 + 1];
    else
      newItems = (void**)mem->Alloc(sizeof(void*) * (maxNum * 2 + 1));
    memcpy(newItems, items, sizeof(void*) * maxNum);
    if( mem == NULL )
      delete[] items;
    items = newItems;
    maxNum = maxNum * 2 + 1;
  }
  items[count++] = item;
}


void List::Add(void ** inputItems, int inputItemCount) {
  if( count + inputItemCount >= maxNum ){
    int newMaxNum = (count + inputItemCount) * 2 + 1;
    void ** newItems;
    if( mem == NULL )
      newItems = new void*[newMaxNum];
    else
      newItems = (void**)mem->Alloc(sizeof(void*) * newMaxNum);
    memcpy(newItems, items, sizeof(void*) * maxNum);
    if( mem == NULL )
      delete[] items;
    items = newItems;
    maxNum = newMaxNum;
  }
  memcpy(items + count, inputItems, sizeof(void*) * inputItemCount);
  count += inputItemCount;
}


void List::Insert(int pos, void * item) {
  if( count == maxNum ){
    void ** newItems;
  if( mem == NULL )
    newItems = new void*[maxNum * 2 + 1];
  else
    newItems = (void**)mem->Alloc(sizeof(void*) * (maxNum * 2 + 1));
  memcpy(newItems, items, sizeof(void*) * maxNum);
  if( mem == NULL )
    delete[] items;
    items = newItems;
    maxNum = maxNum * 2 + 1;
  }

  for(int i = count - 1; i >= pos; i--)
    items[i + 1] = items[i];
  items[pos] = item;
  count++;
}


void * List::GetItem(int i) {
  if( i >= 0 && i < count )
    return items[i];
  else
    return NULL;
}


void List::SetItem(int i, void * item) {
  if( i >= 0 && i < count )
    items[i] = item;
}


void List::Clear() {
  count = 0;
}


void List::Sort(int itemSize, ListCompare comp) {
  qsort(items, count, itemSize, comp);
}


void List::Reverse() {
  int half = count/2;
  for(int i = 0; i < half; i++){
    void * tmp = items[i];
    items[i] = items[count - i - 1];
    items[count - i - 1] = tmp;
  }
}


List * List::Copy(MemPool * myMem) {
  List * newList = new List(maxNum, myMem);
  for(int i = 0; i < count; i++){
    newList->Add(GetItem(i));
  }
  return newList;
}


//////////////////////////////////////////////////////////////////////////
// tree fragment
TreeFrag::TreeFrag(TreeNode * root) {
  memset(this, 0, sizeof(TreeFrag));
  this->root = root;
  frontierSequence = NULL;
  frontierNTs = new List(1);
}


TreeFrag::TreeFrag() {
  memset(this, 0, sizeof(TreeFrag));
}


TreeFrag::~TreeFrag() {
  delete[] frontierSequence;
  delete frontierNTs;
}


void TreeFrag::AddNT(TreeNode * NT, bool fromLeft, bool useNTSymbol) {
  int baseLength = frontierSequence == NULL ? 0 : ((int)strlen(frontierSequence) + 1);
  int keyLength = (int)strlen(NT->label) + baseLength + 2;

  char * key = new char[keyLength];
  memset(key, 0, sizeof(char) * keyLength);

  // sequence of word and variables
  if(frontierSequence == NULL){
    if(useNTSymbol)
      sprintf(key, "#%s", NT->label);
    else
      sprintf(key, "%s", NT->label);
    frontierSequence = key;
  } else {
    if(useNTSymbol){
      if(fromLeft)
        sprintf(key, "#%s %s", NT->label, frontierSequence);
      else
        sprintf(key, "%s #%s", frontierSequence, NT->label);
    } else{
      if(fromLeft)
        sprintf(key, "%s %s", NT->label, frontierSequence);
      else
        sprintf(key, "%s %s", frontierSequence, NT->label);
    }

    delete[] frontierSequence;
    frontierSequence = key;
  }

  // variable list
  if(fromLeft)
    frontierNTs->Insert(0, (void*)NT);
  else
    frontierNTs->Add((void*)NT);
  NTNum++;
}


void TreeFrag::AddWord(TreeNode * NT, bool fromLeft) {
  int baseLength = frontierSequence == NULL ? 0 : ((int)strlen(frontierSequence) + 1);
  int keyLength = (int)strlen(NT->word) + baseLength + 1;
  char * key = new char[keyLength];
  memset(key, 0, sizeof(char) * keyLength);
  // sequence of words and variables
  if(frontierSequence == NULL){
    sprintf(key, "%s", NT->word);
    frontierSequence = key;
  } else {
    if(fromLeft)
      sprintf(key, "%s %s", NT->word, frontierSequence);
    else
      sprintf(key, "%s %s", frontierSequence, NT->word);
    delete[] frontierSequence;
    frontierSequence = key;
  }
  wordNum++;
}


// to generate tree structure
void TreeFrag::AttachLabel(TreeNode * NT) {
  if(frontierSequence == NULL)
    return;

  int baseLength =  (int)strlen(frontierSequence) + 1;
  int keyLength = (int)strlen(NT->label) + baseLength + 3;

  char * key = new char[keyLength];
  memset(key, 0, sizeof(char) * keyLength);
  //if(NT->isLeaf)
  //    sprintf(key, "(%s %s)", NT->label, frontierSequence); // sample format: (NN car)
  //else
  //    sprintf(key, "%s (%s)", NT->label, frontierSequence); // sample format: NP(NP VP)
  sprintf(key, "(%s %s)", NT->label, frontierSequence); // sample format: NN (car)
  delete[] frontierSequence;
  frontierSequence = key;
}


void TreeFrag::AttachLabelOnly(TreeNode * NT) {
  if(frontierSequence == NULL)
    return;

  int baseLength =  (int)strlen(frontierSequence) + 1;
  int keyLength = (int)strlen(NT->label) + baseLength + 1;
  char * key = new char[keyLength];
  memset(key, 0, sizeof(char) * keyLength);
  sprintf(key, "%s %s", NT->label, frontierSequence); // sample format: NP (NN)
  delete[] frontierSequence;
  frontierSequence = key;
}


void TreeFrag::AttachBrackets() {
  if(frontierSequence == NULL)
    return;

  int baseLength =  (int)strlen(frontierSequence) + 1;
  int keyLength = baseLength + 3;

  char * key = new char[keyLength];
  memset(key, 0, sizeof(char) * keyLength);

  sprintf(key, "(%s)", frontierSequence); // sample format: (NP)

  delete[] frontierSequence;
  frontierSequence = key;
}


TreeFrag * TreeFrag::Copy() {
  TreeFrag * newFrag = new TreeFrag();
  memcpy(newFrag, this, sizeof(TreeFrag));
  newFrag->frontierSequence = NULL;
  if(frontierSequence != NULL)
    newFrag->frontierSequence = StringUtil::Copy(frontierSequence);

  newFrag->frontierNTs = new List(frontierNTs->count);
  newFrag->frontierNTs->Add(frontierNTs->items, frontierNTs->count);
  return newFrag;
}


TreeFrag * TreeFrag::Join(TreeFrag * frag1, TreeFrag * frag2) {
  TreeFrag * newFrag = new TreeFrag();
  newFrag->frontierSequence = new char[(int)strlen(frag1->frontierSequence) + 
                              (int)strlen(frag2->frontierSequence) + 2];

  sprintf(newFrag->frontierSequence, "%s %s", frag1->frontierSequence, frag2->frontierSequence);
  newFrag->frontierNTs = new List(frag1->frontierNTs->count + frag2->frontierNTs->count);
  newFrag->frontierNTs->Add(frag1->frontierNTs->items, frag1->frontierNTs->count);
  newFrag->frontierNTs->Add(frag2->frontierNTs->items, frag2->frontierNTs->count);
  newFrag->wordNum = frag1->wordNum + frag2->wordNum;
  newFrag->NTNum   = frag1->NTNum + frag2->NTNum;
  newFrag->depth   = frag1->depth > frag2->depth ? frag1->depth : frag2->depth;
  return newFrag;
}


//////////////////////////////////////////////////////////////////////////
Tree::Tree() {
  Init();
}


Tree::~Tree() {
  Clear();
  delete nodeBase;
  delete edgeBase;
  delete leaves;
}


void Tree::Init() {
  nodeBase = new List(MAX_TREE_NODE_NUM);
  edgeBase = new List(MAX_TREE_EDGE_NUM);
  root     = NULL;
  leaves   = new List(MAX_WORD_NUM_IN_TREE);
}


void Tree::Clear() {
  for (int i = 0; i < nodeBase->count; i++) {
    TreeNode * node = (TreeNode *)nodeBase->GetItem(i);
    delete[] node->label;
    delete[] node->word;
    delete node->edges;

    if (node->treefrags != NULL) {
      List * frags = node->treefrags;
      for (int j = 0; j < frags->count; j++) {
        TreeFrag * frag = (TreeFrag *)frags->GetItem(j);
        delete frag;
      }
      delete node->treefrags;
      node->treefrags;
    }
    delete node;
  }
  nodeBase->Clear();

  for (int i = 0; i < edgeBase->count; i++) {
    TreeEdge * edge = (TreeEdge *)edgeBase->GetItem(i);
    delete edge->children;
    delete edge;
  }
  edgeBase->Clear();

  leaves->Clear();
  root = NULL;
}


char leftBracket[2] = "(";
char rightBracket[2] = ")";


List * Tree::TokenizeString(const char * string) {
  int length    = strlen(string);
  int pos       = 0;
  List * tokens = new List(length);
  TreeStringToken * token           = NULL;
  char tokenString[MAX_WORD_LENGTH] = "";
  while (pos < length) {
    char curChar = string[pos];
    if (curChar == '('){
    // specail case: "(abc)" is recongized as one word instead of three words "(", "abc", ")"
    bool spectalFlag = false;
    int tmpPos = pos + 1;
    while (tmpPos < length) {
      if (string[tmpPos] == ')') {
        spectalFlag = true;
        break;
      } else if (string[tmpPos] == ' ')
        break;
        tmpPos++;
      }
      if (spectalFlag && tmpPos > pos + 1) {
        strncpy(tokenString, string + pos, tmpPos - pos + 1);
        tokenString[tmpPos - pos + 1] = '\0';
        token = GenerateToken(tokenString, tokens->count);
        tokens->Add((void*)token);
        tokenString[0] = '\0';
        pos = tmpPos;
      } else {
        if (strcmp(tokenString, "")) {
          token = GenerateToken(tokenString, tokens->count);
          tokens->Add((void*)token);
        }
        tokenString[0] = '\0';
        token = GenerateToken(leftBracket, tokens->count);
        tokens->Add((void*)token);
      }
    } else if (curChar == ')') {
      if (strcmp(tokenString, "")) {
        token = GenerateToken(tokenString, tokens->count);
        tokens->Add((void*)token);
      }
      tokenString[0] = '\0';
      token = GenerateToken(rightBracket, tokens->count);
      tokens->Add((void*)token);
    } else if (curChar == ' ') {
      if (strcmp(tokenString, "")) {
        token = GenerateToken(tokenString, tokens->count);
        tokens->Add((void*)token);
      }
      tokenString[0] = '\0';
    } else {
      int tLength = (int)strlen(tokenString);
      tokenString[tLength] = curChar;
      tokenString[tLength + 1] = '\0';
    }
    pos++;
  }
  return tokens;
}


bool Tree::CreateTree(const char * string) {
  List * tokens = TokenizeString(string);
  int pos = 1;

  /*for(int i = 0; i < tokens->count; i++){
      TreeStringToken * token = (TreeStringToken *)tokens->GetItem(i);
      fprintf(stderr, "%d %s\n", i, token->token);
  }*/

  if (tokens->count == 0){
    DestroyTokenList(tokens);
    return false;
  }

  TreeStringToken * firstToken = (TreeStringToken *)tokens->GetItem(0);
  TreeStringToken * lastToken  = (TreeStringToken *)tokens->GetItem(tokens->count - 1);
  if(strcmp(firstToken->token, "(") || strcmp(lastToken->token, ")")){
    DestroyTokenList(tokens);
    return false;
  }

  leaves->Clear();
  leaves->Add(NULL);

  root = BuildSubTree(tokens, pos, NULL);

  if (pos != tokens->count - 1){
    root = NULL;
    DestroyTokenList(tokens);
    return false;
  }

  DestroyTokenList(tokens);
  return true;
}


TreeNode * Tree::BuildSubTree(List * tokens, int &pos, TreeNode * parentNode) {
  TreeStringToken * token = (TreeStringToken *)tokens->GetItem(pos);

  if (pos >= tokens->count)
    return NULL;
  if (strcmp(token->token, "("))
    return NULL;

  pos++;     // skip '('
  token = (TreeStringToken *)tokens->GetItem(pos);

  TreeNode * curTreeNode = CreateTreeNode(token->token, NULL, nodeBase->count);

  pos++;
  token = (TreeStringToken *)tokens->GetItem(pos);

  bool isLeafNode = false;

  if (pos < tokens->count && strcmp(token->token, "("))
    isLeafNode = true;
  if (pos < tokens->count - 1){
    TreeStringToken * nextToken = (TreeStringToken *)tokens->GetItem(pos + 1);
    if(!strcmp(nextToken->token, ")"))
      isLeafNode = true;
  }

  if (isLeafNode){     // leaf node
    token = (TreeStringToken *)tokens->GetItem(pos);
    //StringUtil::ToLowercase(token->token);
    curTreeNode->word = StringUtil::Copy(token->token);
    curTreeNode->isLeaf = true;
    curTreeNode->beg = leaves->count;
    curTreeNode->end = curTreeNode->beg;

    leaves->Add((void*)curTreeNode);

    pos++;
  }
  else{
    int beg = MAX_WORD_NUM_IN_TREE, end = -1;
    TreeEdge * edge = CreateTreeEdge(curTreeNode, 3);
    while (pos < tokens->count){
      token = (TreeStringToken *)tokens->GetItem(pos);
      if(!strcmp(token->token, ")"))
        break;

      TreeNode * childNode = NULL;
      if(!strcmp(token->token, "("))
        childNode = BuildSubTree(tokens, pos, curTreeNode);

      if (childNode == NULL)
        return NULL;

      if(beg > childNode->beg)
        beg = childNode->beg;
      if(end < childNode->end)
        end = childNode->end;

      edge->children->Add(childNode);
    }

    curTreeNode->isLeaf = false;
    curTreeNode->edges = new List(1);
    curTreeNode->edges->Add((void*)edge);
    curTreeNode->beg = beg;
    curTreeNode->end = end;
  }

  if (pos >= tokens->count)
    return NULL;
  pos++; // skip ')';
  return curTreeNode;
}


bool Tree::CreateForest(const char * string) {
  const char * blockSeg = strstr(string, " ||| "); // block1: hyper-nodes, block2: hyper-edges
  const char * beg = string;
  const char * end = NULL;
  char buf[MAX_WORD_NUM_IN_TREE * 10];
  int  ibuf[MAX_WORD_NUM_IN_TREE];

  if(blockSeg == NULL){
    fprintf(stderr, "invalid format in forest \"%s\"", string);
    return false;
  }

  // build nodes first
  while(1){
    end = strstr(beg, " || ");
    if(end != NULL){
      if(end < blockSeg){
        strncpy(buf, beg, end - beg);
        buf[end - beg] = '\0';
      }
      else{
        strncpy(buf, beg, blockSeg - beg);
        buf[blockSeg - beg] = '\0';
      }
    }
    else
      strcpy(buf, beg);

    int length = (int)strlen(buf);
    TreeNode * node = new TreeNode();
    node->isLeaf = false;
    node->label  = new char[length + 1];
    node->word   = new char[length + 1];

    // EXAMPLE: "6 2 2 NN report" means
    // node 6 is a (pre-)terminal node with lable "NN" and terminal "report". The corresponding span is [2, 2].
    int tn = sscanf(buf, "%d %d %d %s %s", &node->id, &node->beg, &node->end, node->label, node->word);

    if(tn == 5){
      node->isLeaf = true;
    }
    else if(tn == 4){
      delete[] node->word;
      node->word = NULL;
      node->edges = new List(1);
    }
    else{
      fprintf(stderr, "invalid format in hyper-node \"%s\"", buf);
      delete node;
      continue;
    }

    node->beg++;
    node->end++;

    nodeBase->Add(node);

    //fprintf(stderr, "node%d: %s\n", nodeBase->count, buf);

    if(end >= blockSeg || end == NULL)
      break;
    else
      beg = end + 4;
  }

  beg = blockSeg + 5;
  end = NULL;

  // then edges
  while(1){
    end = strstr(beg, " || ");
    if(end != NULL){
      strncpy(buf, beg, end - beg);
      buf[end - beg] = '\0';
    }
    else
      strcpy(buf, beg);

    int length = (int)strlen(buf);
    const char * ibeg = beg;
    int i = 0;

    while(ibeg != '\0'){
      while(*ibeg == ' ')
        ibeg++;

      if(*ibeg < '0' || *ibeg > '9')
        break;

      sscanf(ibeg, "%d", &ibuf[i++]);
      while(*ibeg != ' ' && *ibeg != '\0')
        ibeg++;
    }

    if(i < 2){
      fprintf(stderr, "invalid format in hyper-edge \"%s\"", buf);
      continue;
    }

    TreeEdge * edge = new TreeEdge();
    edge->parent = (TreeNode *)nodeBase->GetItem(ibuf[0]); // head of hyper-edge
    edge->children = new List(i - 1);
    for(int k = 1; k < i; k++){
      TreeNode * childNode = (TreeNode *)nodeBase->GetItem(ibuf[k]); // tail of hyper-edge
      edge->children->Add(childNode);
    }

    edge->parent->edges->Add(edge);
    edgeBase->Add(edge);

    //fprintf(stderr, "edge%d: %s\n", edgeBase->count, buf);

    if(end == NULL)
      break;
    else
      beg = end + 4;
  }

  nodeBase->Reverse();
  for(int k = 0; k < nodeBase->count; k++){
    TreeNode * node = (TreeNode *)nodeBase->GetItem(k);
    node->id = k;
  }
  root = (TreeNode *)nodeBase->GetItem(0);
  return true;
}


void Tree::DestroyTokenList(List * tokens) {
  for(int i = 0; i < tokens->count; i++){
    TreeStringToken * token = (TreeStringToken *)tokens->GetItem(i);
    delete[] token->token;
    delete token;
  }
  delete tokens;
}


TreeStringToken * Tree::GenerateToken(char * string, int id) {
  TreeStringToken * token = new TreeStringToken();
  token->token = new char[(int)strlen(string) + 1];
  strcpy(token->token, string);
  token->id = id;
  return token;
}


TreeNode * Tree::CreateTreeNode(char * label, char * word, int id) {
  TreeNode * node = new TreeNode();
  memset(node, 0, sizeof(TreeNode));
  node->isLeaf = false;
  node->beg = MAX_WORD_NUM_IN_TREE;
  node->end = -1;

  if(label != NULL)
    node->label = StringUtil::Copy(label);
  if(word != NULL)
    node->word  = StringUtil::Copy(word);

  node->id = id;
  nodeBase->Add((void*)node);
  return node;
}


TreeEdge * Tree::CreateTreeEdge(TreeNode * parent, int childrenNum) {
  TreeEdge * edge = new TreeEdge();
  edge->parent = parent;

  if(childrenNum >= 0)
    edge->children = new List(childrenNum);
  else
    edge->children = new List(3);

  edgeBase->Add((void*)edge);
  return edge;
}


void Tree::GenerateTreeFragments(bool generateTreeStructure) {
  for(int i = nodeBase->count - 1; i >= 0; i--){
    TreeNode * node = (TreeNode *)nodeBase->GetItem(i);
    GenerateTreeFragments(node, generateTreeStructure);
  }
}


void Tree::GenerateTreeFragments(TreeNode * root, bool generateTreeStructure) {
  int keyLength = 0;

  //fprintf(stderr, "%d: %s\n", root->id, root->label);
  root->treefrags = new List(2);

  // root
  TreeFrag * baseFrag = new TreeFrag(root);
  baseFrag->AddNT(root, true, !generateTreeStructure);
  if(generateTreeStructure)
    baseFrag->AttachBrackets();
  root->treefrags->Add((void*)baseFrag);

  if(root->isLeaf){ // word
    TreeFrag * leafFrag = new TreeFrag(root);
    leafFrag->AddWord(root, true);
    if(generateTreeStructure){
      leafFrag->AttachBrackets();
      leafFrag->AttachLabel(root);
    }
    root->treefrags->Add((void*)leafFrag);
  }
  else{ // build it recursively
    for(int e = 0; e < root->edges->count; e++){ // loop for each edge
      TreeEdge * edge = (TreeEdge *)root->edges->GetItem(e);
      List * fragList = NULL;
      int maxFragNum  = maxFragNumPerNode / root->edges->count + 1;

      for(int i = 0; i < edge->children->count; i++){ // loop for each child node (tail of edge)
        TreeNode * child = (TreeNode *)edge->children->GetItem(i);

        //if(child->treefrags == NULL)
        //    GenerateTreeFragments(child);

        List * childFrags = child->treefrags;

        if(i == 0){
          fragList = new List(childFrags->count);
          for(int j = 0; j < childFrags->count; j++){
            TreeFrag * frag = (TreeFrag *)childFrags->GetItem(j);
            TreeFrag * newFrag = frag->Copy();
            newFrag->root = root;
            fragList->Add((void*)newFrag);
          }
        }
        else{
          List * newFrags = new List(fragList->count);
          for(int j = 0; j < childFrags->count; j++){
            TreeFrag * childFrag = (TreeFrag *)childFrags->GetItem(j);
            for(int k = 0; k < fragList->count; k++){
              TreeFrag * curFrag = (TreeFrag *)fragList->GetItem(k);
              if(!IsValidTreeJoin(curFrag, childFrag))
                continue;

              TreeFrag * newFrag = TreeFrag::Join(curFrag, childFrag);
              newFrag->root = root;
              newFrags->Add(newFrag);
            }
            if(newFrags->count >= maxFragNum) // too many fragments. so ...
              break;
          }

          if(fragList != NULL){
            for(int k = 0; k < fragList->count; k++){
              TreeFrag * oldFrag = (TreeFrag *)fragList->GetItem(k);
              delete oldFrag;
            }
            delete fragList;
          }
          fragList = newFrags;
        }
      }

      if(generateTreeStructure){
        bool unaryProduction = edge->children->count == 1 ? true : false;
        for(int j = 0; j < fragList->count; j++){
          TreeFrag * frag = (TreeFrag *)fragList->GetItem(j);
          //if(unaryProduction)
          //    frag->AttachLabelOnly(root);
          //else
          frag->AttachLabel(root);
        }
      }
      root->treefrags->Add(fragList->items, fragList->count);
      delete fragList;
    }
  }
}


bool Tree::IsValidTreeFrag(TreeFrag * frag) {
  if(frag->wordNum > maxWordNum)
    return false;
  if(frag->NTNum > maxNTNum)
    return false;
  if(frag->depth > maxDepth)
    return false;
  return true;
}


bool Tree::IsValidTreeJoin(TreeFrag * frag1, TreeFrag * frag2) {
  if(frag1->wordNum + frag2->wordNum > maxWordNum)
    return false;
  if(frag1->NTNum + frag2->NTNum > maxNTNum)
    return false;
  if(frag1->depth > maxDepth || frag2->depth > maxDepth)
    return false;
  return true;
}


char * Tree::ToString(bool label, int * validWordIndicator) {
  int bufLength = 0;
  char * buf = new char[MAX_LINE_LENGTH];
  char * treeString = NULL;
  if(root == NULL){
    treeString = new char[1];
    treeString[0] = '\0';
  }
  else{
    ToString(buf, bufLength, root, label, validWordIndicator);
    treeString = new char[(int)strlen(buf) + 3];
    sprintf(treeString, "(%s)", buf);
  }
  delete[] buf;
  return treeString;
}


void Tree::ToString(char * buf, int &bufLength, TreeNode * rootNode, bool label, int * validWordIndicator) {
  bool nonSkeleton = true;
  for(int i = rootNode->beg; i <= rootNode->end; i++){
    if(validWordIndicator[i]){ // if the span contains a skeleton word
      nonSkeleton = false;
      break;
    }
  }

  if(nonSkeleton){
    sprintf(buf + bufLength, " %s", label ? rootNode->label : SKELETON_SLOT_SYMBOL);
    bufLength += 1 + label ? (int)strlen(rootNode->label) : (int)strlen(SKELETON_SLOT_SYMBOL);
  }
  else{
    if(rootNode->isLeaf){
      sprintf(buf + bufLength, " (%s %s)", rootNode->label, rootNode->word);
      bufLength += 4 + (int)strlen(rootNode->label) + (int)strlen(rootNode->word);
    }
    else{
      sprintf(buf + bufLength, " (%s", rootNode->label);
      bufLength += 2 + (int)strlen(rootNode->label);
      for(int e = 0; e < rootNode->edges->count; e++){
        TreeEdge * edge = (TreeEdge*)rootNode->edges->GetItem(e);
        for(int c = 0; c < edge->children->count; c++){
          ToString(buf, bufLength, (TreeNode*)edge->children->GetItem(c), label, validWordIndicator);
        }
        break; // TODO: forest
      }
      strcat(buf, ")");
      bufLength++;
    }
  }
}

}


