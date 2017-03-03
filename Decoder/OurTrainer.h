// a trainer for SMT system
// created by xiaotong (xiaotong@mail.neu.edu.cn)

#ifndef _OURTRAINER_H_
#define _OURTRAINER_H_

#include <string.h>
#include "configuration.h"

using namespace decoder_configuration;

namespace decoder_our_trainer {

////////////////////////////////////////
// Global variables
class GlobalVar
{
public:
    static bool     normalizeText;
    static bool     allLowerCase;

public:
    static void Init();
    static void SetNormalizeText(bool label);
};


////////////////////////////////////////
// Utility classes

class Util
{
public:
    static void TrimRight(char * line);
    static int Split(char * line, char * separator, char ** &terms);
    static int SplitWithSpace(char * line, char ** &terms);
    static char * NormalizeText(char * line);
    static void ToLowerCase(char * line);
};

#define ITEM_RATE_IN_HASH  0.2            // to contral the conflicting rate in hash tables

// index node of hash table 
typedef struct HashIndex
{
    char *          key;
    int             value;
    HashIndex *     next;
}* pHashIndex;

// hash table
class HashTable
{
public:
    HashIndex **   table;
    unsigned       tableSize;
    int            itemCount;
    char **        keyTable;
    int            keyCount;

public:
    HashTable( unsigned myTableSize );
    ~HashTable();
    void Resize(unsigned myNewTableSize);
    HashIndex * Add( const char * myKey, int myValue, HashIndex * newNode );
    int GetValue( const char * myKey );
    void Clear();

protected:
    int ToHashCode( const char * myKey );
    HashIndex * NewHashIndexNode( const char * myKey, int myValue );
    HashIndex * GetAndNewIndexHead( const char * myKey, int myValue, HashIndex * newNode );

public:
    HashIndex * GetIndexHead( const char * myKey );
};

// types of BLEU
// 0: NIST version BLEU (using the shortest reference length) - default
// 1: IBM version BLEU (using the closest reference length)
// 2: BLEU-SBP (Chiang et al., EMNLP2008)
enum BLEU_TYPE {NIST_BLEU, IBM_BLEU, BLEU_SBP};
extern char BLEU_TYPE_NAME[3][20];

// evaluation metrics (e.g. BLEU used in defining error function)
class Evaluator
{
public:
    static float CalculateBLEU(float matchedNGram[], float statNGram[], float bestRefLen, 
                               float shortestRefLen, float SBPRefLen, int ngram, BLEU_TYPE type = NIST_BLEU);
    static float CalculateBLEUSmoothed(float matchedNGram[], float statNGram[], float bestRefLen,
                                       float shortestRefLen, float SBPRefLen, int ngram, BLEU_TYPE type = NIST_BLEU);
};

////////////////////////////////////////
// Basic structure used in training

class Sentence
{
public:
    char *          str;
    char **         words;
    int             wordCount;

public:
    Sentence();
    Sentence(char * line);
    ~Sentence();
    void Init();

public:
    void Create(char * line);    // create a candidate from a char string
};

typedef struct FeatureNode{
    int             id;
    float          value;
}* pFeatureNode;

// translation candidate
class TransCand
{
public:
    Sentence *      source;      // source sentence
    Sentence *      translation; // target sentence
    FeatureNode *   feats;       // accosiated features
    int             featCount;   // number of features
    float *        nGramMatch;

    // model-score(trans-cand) = slope * w + b
    float           slope;
    float           b;

    float           bestRefLen;     // effective length for IBM-BLEU
    float           shortestRefLen; // effective length for NIST-BLEU
    float           SBPRefLen;         // effective length for BLEU-SBP

public:
    TransCand();
    TransCand(char * line);
    ~TransCand();
    void Init();

public:
    void Clear();
    void Create(char * line);    // create a candidate from a char string
    void Deliver(TransCand * target);
    int GetFeatIndex(int id);
};

// reference translation
class RefSents
{    
public:
    Sentence *       refs;       // reference translations
    int              refCount;   // number of reference translations
    HashTable *      ngramMatch; // ngram hash
    int              shortestRefLen;  // shortest reference length

public:
    RefSents();
    RefSents(char ** refSent, int refCount, int ngram);
    ~RefSents();
    void Init();
    
public:
    float * GetNgramMatchStat(Sentence * sent, int ngram);
};

////////////////////////////////////////
// Trainer

// training sample
class TrainingSample
{
public:
    Sentence *       srcSent;     // source sentence
    TransCand *      Cands;          // list of candidates
    int              CandCount;
    RefSents *       ref;

public:
    TrainingSample();
    ~TrainingSample();
    void Init();

public:
    void Deliver(TrainingSample * target);
    void Clear();
    void ClearCands();
};

// training set
class TrainingSet
{
public:
    TrainingSample * samples;
    int              sampleCount;
    int              ngram;

public:
    TrainingSet();
    TrainingSet(char * refFileName, int refNum, int ngram);
    ~TrainingSet();
    void Init();

public:
    void Clear();
    void LoadRefData( const char * refFileName, int refNum, int ngram);
    void LoadTrainingData(const char * transFileName, bool accumlative);
};

class ParaInfo
{
public:
    float            weight;
    float            minValue;
    float            maxValue;
    bool             isFixed;

public:
    ParaInfo();
};

class ParaBase
{
public:
    /*ParaInfo **     paraList;
    int *           paraLen;
    float *         bleuList;*/
    ParaInfo *      paraList[10];
    int             paraLen[10];
    float           bleuList[10];
    int             paraListLen;

public:
    ParaBase(int maxNum);
    ~ParaBase();
    void Add(ParaInfo * para, int paraCount, float bleu); // add a para-vector
    static char * ToStr(ParaInfo * para, int paraCount);

};

enum METHOD{MERT,PERCEPTRON, MIRA, RANKPAIRWISE};
extern char METHOD_NAME[4][20];

// intersection (for MERT)
class Intersection
{
public:
    float           coord;
    float *         deltaMatch;
    float *         deltaNGram;
    float           deltaBestRefLen;
    float           deltaShortestRefLen;
    float           deltaSBPRefLen;

public:
    Intersection();
    ~Intersection();
    void Create(int ngram);
};

// trainer
class OurTrainer
{
public:
    ParaInfo *       para;
    ParaInfo *       finalPara;
    int              paraCount;
    int              round;
    bool             isPAveraged; // for perceptron-based training
    float            pScale;      // update scale

public:
    OurTrainer();
    ~OurTrainer();
    void Init();

public:
    void Clear();
    void CreatePara(int weightNum);
    void LoadPara(const char * configFileName);
    void SetParaInfo(char * paraLine);
    void OptimizeWeights(Configuration &config, TrainingSet * ts, ParaInfo * para, int paraCount, int ngram, BLEU_TYPE BLEUType, METHOD method);
    void OptimzieWeightsWithMERT(Configuration &config, TrainingSet * ts, int ngram, BLEU_TYPE BLEUType);

protected:
    float GetModelScore(ParaInfo * para, int paraCount, TransCand * cand);
    TransCand * GetTop1(ParaInfo * para, int paraCount, TransCand * cands, int candCount);
    float GetBLEU(TransCand ** transList, int transCount, TrainingSet * ts, 
                  int ngram, BLEU_TYPE type = NIST_BLEU);
    void DumpPara(const char * fileName, ParaInfo * para, int paraCount, char * msg);
    //int SortIntersectionWithW( const void * arg1, const void * arg2 );

// minimum error rate training
public:
    int ComputeIntersections(TrainingSample * sample, int dim, Intersection * intersections, 
                             int &num, int ngram);
    int MERTWithLineSearch(TrainingSet * ts, ParaInfo * para, ParaInfo * finalPara, int paraCount, 
                           int nRoundConverged, int ngram, BLEU_TYPE BLEUType);

// perceptron
public:
    int PeceptronBasedTraining(TrainingSet * ts, ParaInfo * para, ParaInfo * finalPara, int paraCount, 
                           int nRoundConverged, int ngram, BLEU_TYPE BLEUType);
};

}

#endif

