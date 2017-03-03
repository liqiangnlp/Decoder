#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include "OurTrainer.h"

#ifndef    WIN32
#include <sys/time.h>
#endif

namespace decoder_our_trainer {

#define INVALID_ID           -1
#define NGRAM_MIN_NUM        1000
#define MAX_LINE_LENGTH      1024 * 16
#define MAX_WORD_LENGTH      1024
#define MAX_SENT_LENGTH      MAX_LINE_LENGTH
#define MAX_CAND_NUM         1000000
#define MAX_SAMPLE_NUM       1000000
#define MAX_REF_NUM          100
#define MIN_float_VALUE      -1000000

#ifndef FLT_MIN
#define FLT_MIN              -3.4e38F
#endif

#define MTEVAL_NORMALIZATION 1

////////////////////////////////////////
// Global variables

char METHOD_NAME[4][20] = {"MERT", "PERCEPTRON", "MIRA", "RANKPAIRWISE"};
char BLEU_TYPE_NAME[3][20] = {"NIST_BLEU", "IBM_BLEU", "BLEU_SBP"};

bool GlobalVar::normalizeText = true;
bool GlobalVar::allLowerCase  = true;

void GlobalVar::Init()
{
}

void GlobalVar::SetNormalizeText(bool label)
{
    normalizeText = label;
}

////////////////////////////////////////
// Utility

void Util::TrimRight(char * line)
{
    int len = (int)strlen(line);

    for( int i = len - 1; i >= 0; i-- ){
        if( line[i] != '\r' && line[i] != '\n')
            break;
        line[i] = '\0';
    }
}

int Util::Split(char * line, char * separator, char ** &terms)
{
    int termCount  = 0;
    int length     = (int)strlen(line);
    int sepLength  = (int)strlen(separator);
    char * word    = new char[length + 1];
    char * tLine   = new char[length + 1];
    char ** tTerms = new char*[length + 1];
    char * lastW   = NULL;
    int        i;
    
    strcpy(tLine, line);
    Util::TrimRight(tLine);
    
    for( i = 0; i < length; i++ ){

        // naive implememtatoin (can be replaced by the KMP algorithm if needed)
        bool match = true;
        for( int j = 0; j < sepLength; j++ ){
            if( tLine[i + j] != separator[j] ){
                match = false;
                break;
            }
        }

        if( match ){
            tLine[i] = '\0';    // segmentation
            if( lastW != NULL ){
                char * tTerm = new char[strlen(lastW) + 1];
                strcpy( tTerm, lastW );
                tTerms[termCount++] = tTerm;
            }
            else{
                char * tTerm = new char[1];
                tTerm[0] = '\0';
                tTerms[termCount++] = tTerm;
            }
            i += sepLength - 1;
            lastW = NULL;
        }
        else if(lastW == NULL)
            lastW = tLine + i;

    }

    if( lastW != NULL ){
        char * tTerm = new char[strlen(lastW) + 1];
        strcpy( tTerm, lastW );
        tTerms[termCount++] = tTerm;
    }

    delete[] word;
    delete[] tLine;
    
    terms = new char*[termCount +1];
    for( i = 0; i < termCount; i++ )
        terms[i] = tTerms[i];
    terms[termCount] = NULL;
    delete[] tTerms;

    return termCount;
}

int Util::SplitWithSpace(char * line, char ** &terms)
{
    int termCount  = 0;
    int length     = (int)strlen(line);
    char * word    = new char[length + 1];
    char * tLine   = new char[length + 1];
    char ** tTerms = new char*[length + 1];
    char * lastW   = NULL;
    int        i;
    
    strcpy(tLine, line);
    Util::TrimRight(tLine);
    
    for( i = 0; i < length; i++ ){
        if( tLine[i] == ' ' ){
            tLine[i] = '\0';    // segmentation
            if( lastW != NULL ){
                char * tTerm = new char[strlen(lastW) + 1];
                strcpy( tTerm, lastW );
                tTerms[termCount++] = tTerm;
            }
            lastW = NULL;
        }
        else if(lastW == NULL)
            lastW = tLine + i;

    }

    if( lastW != NULL ){
        char * tTerm = new char[strlen(lastW) + 1];
        strcpy( tTerm, lastW );
        tTerms[termCount++] = tTerm;
    }

    delete[] word;
    delete[] tLine;
    
    terms = new char*[termCount +1];
    for(  i = 0; i < termCount; i++ )
        terms[i] = tTerms[i];
    terms[termCount] = NULL;
    delete[] tTerms;

    return termCount;
}

// we reimplemente the normalization fuction in "mteval-v11b.pl" (see below script in perl)
// sub NormalizeText {
//     my ($norm_text) = @_; 
// # language-independent part:
//     several lines ...
//
// # language-dependent part (assuming Western languages):
//     $norm_text = " $norm_text ";
//     $norm_text =~ tr/[A-Z]/[a-z]/ unless $preserve_case;
//     $norm_text =~ s/([\{-\~\[-\` -\&\(-\+\:-\@\/])/ $1 /g;   # tokenize punctuation
//     $norm_text =~ s/([^0-9])([\.,])/$1 $2 /g; # tokenize period and comma unless preceded by a digit
//     $norm_text =~ s/([\.,])([^0-9])/ $1 $2/g; # tokenize period and comma unless followed by a digit
//     $norm_text =~ s/([0-9])(-)/$1 $2 /g; # tokenize dash when preceded by a digit
//     $norm_text =~ s/\s+/ /g; # one space only between words
//     $norm_text =~ s/^\s+//;  # no leading space
//     $norm_text =~ s/\s+$//;  # no trailing space
// 
//    return $norm_text;
// }
char * Util::NormalizeText(char * line)
{
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
            if( line[i] >= '{' && line[i] <= '~' )
                op = 2;
            else if( line[i] >= '[' && line[i] <= '`')
                op = 2;
            else if( line[i] >= ' ' && line[i] <= '&')
                op = 2;
            else if( line[i] >= '(' && line[i] <= '+')
                op = 2;
            else if( line[i] >= ':' && line[i] <= '@')
                op = 2;
            else if( line[i] == '/')
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

    // add in 201300311
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
    // add in 201300311

    if( length > 1)
        newText[p] = '\0';

    return newText;
}

void Util::ToLowerCase(char * line)
{
    char * c;
    for( c = line; *c != '\0'; c++ )
        if( *c >= 'A' && *c <= 'Z' )
            *c -= 'A' - 'a';
}

HashTable::HashTable( unsigned myTableSize )
{
    itemCount = 0;
    keyCount  = 0;
    tableSize = myTableSize;
    table = new HashIndex*[tableSize];    
    assert(table);
    memset( table, 0, sizeof(HashIndex*) * tableSize );
    keyTable = new char*[tableSize];
    assert(keyTable);
    memset( keyTable, 0, sizeof(char*) * tableSize );
}

HashTable::~HashTable()
{
    for( unsigned i = 0; i < tableSize; i++ ){
        HashIndex * curNode = table[i];
        while( curNode != NULL ){
            HashIndex * tmpNode = curNode;
            curNode = curNode->next;
            delete[] tmpNode->key;
            delete tmpNode;
        }
    }

    delete[] table;
    delete[] keyTable;
}

void HashTable::Resize(unsigned myNewTableSize)
{
    if( myNewTableSize < tableSize )
        return;

    HashTable * newTable = new HashTable(myNewTableSize);

    // duplicate the old data
    for( unsigned i = 0; i < tableSize; i++ ){
        HashIndex * curNode = table[i];
        while( curNode != NULL ){
            HashIndex * tmpNode = curNode;
            curNode = curNode->next;
            tmpNode->next = NULL;

            // add new node
            if( tmpNode != newTable->Add(tmpNode->key, tmpNode->value, tmpNode) ){
                delete[] tmpNode->key;
                delete tmpNode;
            }    
        }
    }

    // copy
    delete[] table;
    delete[] keyTable;
    *this = *newTable;
    
    memset(newTable, 0, sizeof(HashTable));
    delete newTable;
}

int HashTable::ToHashCode( const char * myKey )
{
    unsigned hashValue = 0;

    for ( const char * p = myKey; *p; p++ )
    {
        hashValue ^= (hashValue<<5) + (hashValue>>2) + *p;
    }
    return hashValue & (tableSize - 1);
}

HashIndex * HashTable::NewHashIndexNode( const char * myKey, int myValue )
{
    HashIndex * newIndexNode = new HashIndex;
    assert(newIndexNode);
    newIndexNode->key   = new char[strlen(myKey)+1];
    strcpy( newIndexNode->key, myKey);
    newIndexNode->value    = myValue;
    newIndexNode->next    = NULL;

    return newIndexNode;
}

HashIndex * HashTable::GetAndNewIndexHead( const char * myKey, int myValue, HashIndex * newNode )
{
    if( itemCount >= ITEM_RATE_IN_HASH * tableSize )
        Resize(tableSize * 2 + 1);

    int hashCode = ToHashCode( myKey );
    assert( hashCode <= (int)tableSize && hashCode >= 0);
    HashIndex * tempP   = table[hashCode];
    HashIndex * lastP   = tempP;

    while( tempP ){
        int cmp = strcmp( tempP->key, myKey );
        if( cmp == 0 ){
            return tempP;
        }
        else if( cmp < 0 ){
            lastP = tempP;
            tempP = tempP->next;
        }
        else
            break;
    }

    itemCount++;

    if( newNode == NULL )
        newNode = NewHashIndexNode( myKey, myValue );

    keyTable[keyCount++] = newNode->key;

    if( tempP == table[hashCode] ){
        newNode->next = table[hashCode];
        table[hashCode] = newNode;
        return table[hashCode];
    }
    else{
        tempP = lastP->next;
        lastP->next = newNode;
        newNode->next = tempP;
        return newNode;
    }
}

HashIndex * HashTable::GetIndexHead( const char * myKey )
{    
    int hashCode = ToHashCode( myKey );
    assert( hashCode <= (int)tableSize && hashCode >= 0);
    HashIndex * tempP = table[hashCode];
    HashIndex * lastP = tempP;
    
    while( tempP ){
        int cmp = strcmp( tempP->key, myKey );
        if( cmp == 0 )
            return tempP;
        else if( cmp < 0 ){
            lastP = tempP;
            tempP = tempP->next;
        }
        else
            break;
    }
    
    return NULL;
}

HashIndex * HashTable::Add( const char * myKey, int myValue, HashIndex * newNode )
{
    return GetAndNewIndexHead( myKey, myValue, newNode );
}


int HashTable::GetValue( const char * myKey )
{
    HashIndex * indexNode = GetIndexHead( myKey );
    if( indexNode != NULL )
        return indexNode->value;
    else
        return INVALID_ID;
}

void HashTable::Clear()
{
    unsigned  i;

    for(  i = 0; i < tableSize; i++ ){
        HashIndex * curNode = table[i];
        while( curNode != NULL ){
            HashIndex * tmpNode = curNode;
            curNode = curNode->next;
            delete[] tmpNode->key;
            delete tmpNode;
        }
        table[i] = NULL;
    }

    for(  i = 0; i < keyCount; i++ )
        keyTable[i] = NULL;
    keyCount  = 0;
    itemCount = 0;
}

////////////////////////////////////////
// evaluation metrics

float Evaluator::CalculateBLEU(float matchedNGram[], float statNGram[], float bestRefLen, 
                               float shortestRefLen, float SBPRefLen, int ngram, BLEU_TYPE type)
{
    float BP    = 0;
    float score = 0;

    if( type == NIST_BLEU )
        BP = 1 - shortestRefLen / statNGram[0];
    else if( type == IBM_BLEU )
        BP = 1 - bestRefLen / statNGram[0];
    else if( type == BLEU_SBP )
        BP = 1 - shortestRefLen / SBPRefLen;
    if (BP > 0)
        BP = 0;

    for ( int i = 0; i < ngram; i++ ){
        if (statNGram[i] == 0)
            continue;
        if (matchedNGram[i] != 0)
            score += log(matchedNGram[i] / statNGram[i]);
        else
            score += log(0.5 / statNGram[i]);
    }

    return exp(score / ngram + BP);
}

float Evaluator::CalculateBLEUSmoothed(float matchedNGram[], float statNGram[], float bestRefLen, 
                                       float shortestRefLen, float SBPRefLen, int ngram, BLEU_TYPE type)
{
    int    n;
    float bleu = 0;
    float factor = 2;
    
    for( n = 1; n <= ngram; n++ )
        factor *= 2;
    for( n = 1; n <= ngram; n++ ){
        factor /= 2;
        bleu += CalculateBLEU(matchedNGram, statNGram, bestRefLen, shortestRefLen, SBPRefLen, ngram, type);
    }

    return bleu;
}


////////////////////////////////////////
// Basic structure used in training

Sentence::Sentence()
{
    Init();
}

Sentence::Sentence(char * line)
{
    Init();
    Create(line);
}

Sentence::~Sentence()
{
    delete[] str;
    for( int i = 0; i < wordCount; i++ )
        delete[] words[i];
    delete[] words;
}

void Sentence::Init()
{
    str         = NULL;
    words       = NULL;
    wordCount   = 0;
}

void Sentence::Create(char * line)
{
    int len = (int)strlen(line);
    str = new char[len + 1];
    strcpy(str, line);
    Util::TrimRight(str);

    wordCount = Util::SplitWithSpace(str, words);
}

TransCand::TransCand()
{
    Init();
}

TransCand::TransCand(char * line)
{
    Init();
    Create(line);
}

TransCand::~TransCand()
{
    delete source;
    delete translation;
    delete[] feats;
    delete[] nGramMatch;
}

void TransCand::Init()
{
    source         = NULL;
    translation    = NULL;
    feats          = NULL;
    featCount      = 0;
    nGramMatch     = NULL;
    slope          = 0;
    b              = 0;
    bestRefLen     = MAX_SENT_LENGTH;
    shortestRefLen = MAX_SENT_LENGTH;
    SBPRefLen         = MAX_SENT_LENGTH;
}

// format of line:
// source ||| feat1 feat2 feat3
void TransCand::Create(char * line)
{
    int termCount;
    char ** terms;

    termCount = Util::Split(line, " |||| ", terms); // terms[0] : source-sentence; terms[1]: feature values

    if( termCount > 1 ){

        if( GlobalVar::allLowerCase )
            Util::ToLowerCase(terms[0]);

        // load translation
        if( GlobalVar::normalizeText ){
            char * s = Util::NormalizeText(terms[0]);
            translation = new Sentence(s);
            delete[] s;
        }
        else
            translation = new Sentence(terms[0]);

        // load feature id and feature value
        char ** featTerms;
        featCount = Util::Split(terms[1], " ", featTerms);
        
        featCount++;
        feats = new FeatureNode[featCount+1];

        for( int i = 0; i < featCount - 1; i++ ){
            sscanf(featTerms[i], "%d:%f", &feats[i+1].id, &feats[i+1].value );
            delete[] featTerms[i];
        }

        delete[] featTerms;
    }

    for( int i = 0; i < termCount; i++ )
        delete[] terms[i];
    delete[] terms;
}

void TransCand::Deliver(TransCand * target)
{
    *target     = *this;
    Init();
}

int TransCand::GetFeatIndex(int id)
{
    if( id < featCount && id == feats[id].id )
        return id;
    else{
        for( int i = 1; i < featCount; i++ )
            if( feats[i].id == id )
                return i;
    }

    return -1;
}

RefSents::RefSents()
{
    Init();
}

RefSents::RefSents(char ** myRefSent, int myRefCount, int myNGram)
{
    char nGramStr[MAX_WORD_LENGTH];
    int        i, j;

    Init();    
    refs = new Sentence[myRefCount];

    for( i = 0; i < myRefCount; i++ ){
        refs[i].Create(myRefSent[i]);

        // reference length
        if( refs[i].wordCount < shortestRefLen )
            shortestRefLen = refs[i].wordCount;

        // count n-gram
        HashTable * hashSentNGram = new HashTable(NGRAM_MIN_NUM);

        for( j = 0; j < refs[i].wordCount; j++ ){

            nGramStr[0] = '\0';

            for( int k = 0; k < myNGram && k + j < refs[i].wordCount; k++){
                if( k > 0 )
                    strcat(nGramStr, " ");
                strcat(nGramStr, refs[i].words[k+j]);

                HashIndex * index = hashSentNGram->GetIndexHead(nGramStr);

                if( index == NULL )
                    hashSentNGram->Add(nGramStr, 1, NULL);
                else
                    index->value++;
            }
        }

        for(  j = 0; j < hashSentNGram->keyCount; j++ ){
            char * key = hashSentNGram->keyTable[j];
            HashIndex * indexSent  = hashSentNGram->GetIndexHead(key);
            HashIndex * indexMatch = ngramMatch->GetIndexHead(key);

            if( indexMatch == NULL )
                ngramMatch->Add(key, indexSent->value, NULL);
            else if( indexSent->value > indexMatch->value )
                indexMatch->value = indexSent->value;
        }

        delete hashSentNGram;

        refCount++;
    }
}

RefSents::~RefSents()
{
    delete[] refs;
    delete ngramMatch;
}

void RefSents::Init()
{
    refs        = NULL;
    refCount    = 0;
    ngramMatch  = new HashTable(NGRAM_MIN_NUM);
    shortestRefLen  = MAX_SENT_LENGTH;
}

float * RefSents::GetNgramMatchStat(Sentence * sent, int ngram)
{
    char nGramStr[MAX_WORD_LENGTH];
    float * nGramStat = new float[ngram];
    memset(nGramStat, 0, sizeof(float) * ngram);
    HashTable * hashSentNGram = new HashTable(NGRAM_MIN_NUM);
    
    for( int n = 0; n < ngram; n++ ){
        hashSentNGram->Clear();
        for( int i = 0; i < sent->wordCount - n; i++ ){
            nGramStr[0] = '\0';
            for( int j = 0; j <= n && i + j < sent->wordCount; j++ ){
                if( j > 0 )
                    strcat(nGramStr, " ");
                strcat(nGramStr, sent->words[i+j]);
            }

            HashIndex * index = hashSentNGram->GetIndexHead(nGramStr);
            if( index == NULL )
                hashSentNGram->Add(nGramStr, 1, NULL);
            else
                index->value++;
        }

        for( int j = 0; j < hashSentNGram->keyCount; j++ ){
            char * key = hashSentNGram->keyTable[j];
            HashIndex * indexSent  = hashSentNGram->GetIndexHead(key);
            HashIndex * indexMatch = ngramMatch->GetIndexHead(key);

            if( indexMatch == NULL )
                continue;

            if( indexSent->value < indexMatch->value )
                nGramStat[n] += indexSent->value;
            else
                nGramStat[n] += indexMatch->value;
        }
    }

    delete hashSentNGram;

    return nGramStat;
}

TrainingSample::TrainingSample()
{
    Init();
}

TrainingSample::~TrainingSample()
{
    Clear();
}

void TrainingSample::Init()
{
    srcSent        = NULL;
    Cands          = NULL;
    CandCount      = 0;
    ref            = NULL;
    
}

void TrainingSample::Clear()
{
    delete   srcSent;
    srcSent        = NULL;
    delete[] Cands;
    Cands          = NULL;
    CandCount      = 0;
    delete   ref;
    ref            = NULL;
}

void TrainingSample::ClearCands()
{
    delete[] Cands;
    Cands     = NULL;
    CandCount = 0;
}

void TrainingSample::Deliver(TrainingSample * target)
{
    *target      = *this;
    Init();
}

TrainingSet::TrainingSet()
{
    Init();
}

TrainingSet::TrainingSet(char * refFileName, int refNum, int ngram)
{
    Init();
    LoadRefData(refFileName, refNum, ngram);
}

TrainingSet::~TrainingSet()
{
    Clear();
}

void TrainingSet::Init()
{
    samples     = NULL;
    sampleCount = 0;
    ngram       = 0;
}

void TrainingSet::Clear()
{
    delete[] samples;
    samples     = NULL;
    sampleCount = 0;
    ngram       = 0;
}

void TrainingSet::LoadRefData( const char * refFileName, int refNum, int ngram)
{
    char **           refs;
    char *            line = new char[MAX_LINE_LENGTH];
    TrainingSample ** tSamples = new TrainingSample*[MAX_SAMPLE_NUM];  // training samples
    int               tSampleCount = 0;
    bool              breakFlag = false;
    int                    i;

    this->ngram = ngram;

    fprintf( stderr, "\rLoading reference translatons");

    refs = new char*[MAX_REF_NUM];
    for( i = 0; i < MAX_REF_NUM; i++ )
        refs[i] = new char[MAX_SENT_LENGTH];

    FILE * file = fopen(refFileName, "r");

    if( file == NULL ){
        fprintf( stderr, "cannot open file \"%s\"!\n", refFileName );
        return;
    }

    while(fgets(line, MAX_LINE_LENGTH, file )){
        TrainingSample * newSample = new TrainingSample();

        char ** terms;
        int termCount = Util::Split(line, " |||| ", terms);

        if( GlobalVar::allLowerCase )
            Util::ToLowerCase(terms[0]);

        // load source sentence
        if( GlobalVar::normalizeText ){
            char * s = Util::NormalizeText(terms[0]);
            newSample->srcSent = new Sentence(s);
            delete[] s;
        }
        else
            newSample->srcSent = new Sentence(terms[0]);

        for( i = 0; i < termCount; i++ )
            delete[] terms[i];
        delete[] terms;

        // space line
        if( !fgets(line, MAX_LINE_LENGTH, file ) ){
            delete newSample;
            break;
        }

        int refCount = 0;
        for(  i = 0; i < refNum; i++ ){
            if( !fgets(line, MAX_LINE_LENGTH, file ) ){
                breakFlag = true;
                break;
            }

            Util::TrimRight(line);

            if( GlobalVar::allLowerCase )
                Util::ToLowerCase(line);

            if( GlobalVar::normalizeText ){
                char * s = Util::NormalizeText(line);
                strcpy(refs[i], s);
                delete[] s;
            }
            else
                strcpy(refs[i], line);
            refCount++;
        }

        newSample->ref = new RefSents(refs, refCount, ngram);

        if( breakFlag ){
            delete newSample;
            break;
        }

        tSamples[tSampleCount++] = newSample;

        fprintf( stderr, "\rLoaded %d sentence(s) in ref-file", tSampleCount );
    }

    fclose(file);
    delete line;
    fprintf( stderr, "  Done!\n\n" );

    sampleCount = tSampleCount;
    samples = new TrainingSample[sampleCount];
    for(  i = 0; i < tSampleCount; i++ ){
        tSamples[i]->Deliver(samples + i);
        delete tSamples[i];
    }
    delete[] tSamples;

    for(  i = 0; i < MAX_REF_NUM; i++ )
        delete[] refs[i];
    delete[] refs;
    
    
}

void TrainingSet::LoadTrainingData(const char * transFileName, bool accumlative)
{
    char *            line = new char[MAX_LINE_LENGTH];
    TransCand **      tCands = new TransCand*[MAX_CAND_NUM];      // translation candidates for each source sentence
    int               tCandCount = 0;
    int               sampleId = 0;

    FILE * file = fopen(transFileName, "r");

    if( file == NULL ){
        fprintf( stderr, "cannot open file \"%s\"!\n", transFileName );
        return;
    }

    fprintf( stderr, "\rLoading training samples ...");

    while(fgets(line, MAX_LINE_LENGTH, file ) && sampleId < sampleCount){
        TrainingSample * sample = samples + sampleId;

        if( strlen(line) >= 3 && line[0] == '=' && line[1] == '=' && line[2] == '=' ){ // segmented by "==="
            if(sample->CandCount > 0 && accumlative){
                TransCand * candsCopy = sample->Cands;
                sample->Cands = new TransCand[tCandCount + sample->CandCount];    // resize
                for( int i = 0; i < sample->CandCount; i++ )
                    candsCopy[i].Deliver(sample->Cands + i);
                //    candsCopy[i].Deliver(sample->Cands + i + tCandCount);
                delete[] candsCopy;
            }
            else{
                sample->ClearCands();
                sample->Cands = new TransCand[tCandCount];
            }

            
            for( int i = 0; i < tCandCount; i++ ){
                tCands[i]->Deliver(sample->Cands + sample->CandCount);
                sample->CandCount++;
                delete tCands[i];
            }
            /*
            for( int i = 0; i < tCandCount; i++ ){
                tCands[i]->Deliver(sample->Cands + i);
                delete tCands[i];
            }*/

            sampleId++;
            tCandCount = 0;
            fprintf( stderr, "\rLoaded %d sentence(s) in training-samples-file", sampleId );
            continue;
        }

        RefSents *  ref  = sample->ref;
        TransCand * cand = new TransCand(line);

        cand->nGramMatch = ref->GetNgramMatchStat(cand->translation, ngram);

        cand->shortestRefLen = (float)sample->ref->shortestRefLen;           // shortest length (NIST-BLEU)
        for(int i = 0; i < sample->ref->refCount; i++){                      // "best" length (IBM-BLEU)
            int refLen = sample->ref->refs[i].wordCount; // length of the i-th reference translation
            if( abs((int)(refLen - cand->translation->wordCount)) < abs((int)(cand->bestRefLen - cand->translation->wordCount)) )
                cand->bestRefLen = (float)refLen;
        }
        cand->SBPRefLen = cand->translation->wordCount < cand->shortestRefLen ? // "best" length (BLEU-SBP)
            cand->translation->wordCount :
            cand->shortestRefLen;

        tCands[tCandCount++] = cand;
    }

    fclose(file);
    fprintf( stderr, "  Done!\n" );

    delete[] line;
    delete[] tCands;
}

ParaInfo::ParaInfo()
{
    weight   = 0;
    minValue = -1;
    maxValue = 1;
    isFixed  = false;
}

ParaBase::ParaBase(int maxNum)
{
    /*paraList    = new ParaInfo*[maxNum];
    paraLen     = new int[maxNum];
    bleuList    = new float[maxNum];
    paraListLen = 0;
    memset(paraList, 0, sizeof(ParaInfo*) * maxNum);
    memset(paraLen, 0, sizeof(int) * maxNum );
    memset(bleuList, 0, sizeof(float) * maxNum );*/
    paraListLen = 0;
}

ParaBase::~ParaBase()
{
    /*for( int i = 0; i < paraListLen; i++ )
        delete[] paraList[i];
    delete[] paraList;
    delete[] paraLen;
    delete[] bleuList;*/
}

void ParaBase::Add(ParaInfo * para, int paraCount, float bleu)
{
    bleuList[paraListLen] = bleu;
    paraLen[paraListLen] = paraCount;
    paraList[paraListLen] = new ParaInfo[paraCount];
    for( int i = 0; i < paraCount; i++)
        paraList[paraListLen][i] = para[i];
    paraListLen++;
}

char * ParaBase::ToStr(ParaInfo * para, int paraCount)
{
    char * wStr = new char[paraCount * 20];
    wStr[0] = '\0';
    for( int i = 1; i < paraCount; i++ ){
        if( i >= 1 )
            sprintf(wStr, "%s ", wStr);
        sprintf(wStr, "%s%.8f", wStr, para[i].weight);
    }
    return wStr;
}

Intersection::Intersection()
{
    coord               = 0;
    deltaMatch          = NULL;
    deltaNGram          = NULL;
    deltaBestRefLen     = 0;
    deltaShortestRefLen = 0;
    deltaSBPRefLen      = 0;
}

Intersection::~Intersection()
{
    delete[] deltaMatch;
    delete[] deltaNGram;
}

void Intersection::Create(int ngram)
{
    coord               = 0;
    deltaBestRefLen     = 0;
    deltaShortestRefLen = 0;
    deltaSBPRefLen         = 0;
    delete[] deltaMatch;
    delete[] deltaNGram;
    deltaMatch = new float[ngram];
    deltaNGram = new float[ngram];
    memset(deltaMatch, 0, sizeof(float) * ngram);
    memset(deltaNGram, 0, sizeof(float) * ngram);
}

////////////////////////////////////////
// Trainer

OurTrainer::OurTrainer()
{
    Init();
}

OurTrainer::~OurTrainer()
{
    Clear();
}

void OurTrainer::Init()
{
    para       = NULL;
    finalPara  = NULL;
    paraCount  = 0;
    round      = 0;
    isPAveraged = 1;
}

void OurTrainer::Clear()
{
    delete[] para;
    delete[] finalPara;
    Init();
}

void OurTrainer::CreatePara(int weightNum)
{
    delete[] para;
    delete[] finalPara;
    paraCount = 0;
    para = new ParaInfo[weightNum + 1];    // index from 1
    finalPara = new ParaInfo[weightNum + 1];
    paraCount = weightNum + 1;
}

void OurTrainer::LoadPara(const char * configFileName)
{
    char * line = new char[MAX_LINE_LENGTH];
    FILE * file = fopen(configFileName, "r");
    

    if( !fgets(line, MAX_LINE_LENGTH - 1, file) )
        return;

    sscanf(line, "%d", &paraCount);
    paraCount++;
    delete[] para;
    delete[] finalPara;
    para = new ParaInfo[paraCount];    // index from 1
    finalPara = new ParaInfo[paraCount];

    while( fgets(line, MAX_LINE_LENGTH - 1, file) )
        SetParaInfo(line);

    fclose(file);
    delete[] line;
}

void OurTrainer::SetParaInfo(char * paraLine)
{
    int id, isFixed;
    float w, min, max;

    if( sscanf(paraLine, "%d %f %f %f %d", &id, &w, &min, &max, &isFixed) < 5 ){
        fprintf( stderr, "invalid para-line \"%s\"\n", paraLine );
        return;
    }

    if( id >= paraCount || id <= 0){
        fprintf( stderr, "invalid para-id %d is not in [1,%d]\n", id, paraCount );
        return;
    }

    para[id].weight   = w;
    para[id].minValue = min;
    para[id].maxValue = max;
    para[id].isFixed  = isFixed == 1 ? true : false;
}

void OurTrainer::OptimizeWeights(Configuration &config, TrainingSet * ts, ParaInfo * para, int paraCount, int ngram, BLEU_TYPE BLEUType, METHOD method)
{
    char * wMsg1;
    char * wMsg2;
    char msg[1024];
    float oldBLEU, newBLEU;
    int      i;

    TransCand ** trans = new TransCand*[ts->sampleCount];

//    fprintf(stderr, "Preparing ...");

    // BLEU before conducting optimization
    //for( int i = 0; i < ts->sampleCount; i++ )
    //    trans[i] = GetTop1(para, paraCount, ts->samples[i].Cands, ts->samples[i].CandCount);
    for( i = 0; i < ts->sampleCount; i++ )
        trans[i] = ts->samples[i].Cands;
    oldBLEU = GetBLEU(trans, ts->sampleCount, ts, ts->ngram, BLEUType);

    fprintf(stderr, "MERT : method = \"%s + %s\", BLEU = %.4f \n", 
            METHOD_NAME[(int)method], BLEU_TYPE_NAME[(int)BLEUType], oldBLEU);

    ++round;

    sprintf(msg, "ROUND %2d BLEU = %.4f", round, oldBLEU);
    DumpPara(config.mert_log_file_.c_str(), para, paraCount, msg);

    if( method == MERT )
        MERTWithLineSearch(ts, para, finalPara, paraCount, 5, ngram, BLEUType);

    // BLEU after conducting optimization
    for(  i = 0; i < ts->sampleCount; i++ )
        trans[i] = GetTop1(para, paraCount, ts->samples[i].Cands, ts->samples[i].CandCount);
    newBLEU = GetBLEU(trans, ts->sampleCount, ts, ts->ngram, BLEUType);

    msg[0] = '\0';

    wMsg1 = ParaBase::ToStr(finalPara, paraCount);
    wMsg2 = ParaBase::ToStr(para, paraCount);
    sprintf(msg,"Before optimization - BLEU: %.4f", oldBLEU);
    sprintf(msg,"%s\nAfter optimization - BLEU: %.4f", msg, newBLEU);
    sprintf(msg, "%s\nOptimized Weights : %s", msg, wMsg1);
    sprintf(msg, "%s\nFinal Weights : %s\n", msg, wMsg2);

    FILE * resultF = fopen("niutransserver.mert.result.txt", "w");
    fprintf( resultF, "%s\n", msg );
    fclose(resultF);

    delete[] trans;
    delete[] wMsg1;
    delete[] wMsg2;
}

void OurTrainer::OptimzieWeightsWithMERT(Configuration &config, TrainingSet * ts, int ngram, BLEU_TYPE BLEUType)
{
    OptimizeWeights(config, ts, para, paraCount, ngram, BLEUType, MERT);
}

float OurTrainer::GetModelScore(ParaInfo * para, int paraCount, TransCand * cand)
{
    float score = 0;

    if(cand == NULL)
        return FLT_MIN;
    
    for( int i = 1; i < cand->featCount; i++ ){
        int id = cand->feats[i].id;
        score += cand->feats[i].value * para[id].weight;
    }

    return score;
}

TransCand * OurTrainer::GetTop1(ParaInfo * para, int paraCount, TransCand * cands, int candCount)
{
    float maxScore = FLT_MIN;
    int   maxCand = -1;

    for( int i = 0; i< candCount; i++ ){
        TransCand * cand = cands + i;
        float curScore = GetModelScore(para, paraCount, cand);
        if( curScore > maxScore ){
            maxScore = curScore;
            maxCand = i;
        }
    }

    if( maxCand == -1 )
        return NULL;
    else
        return cands + maxCand;
}

float OurTrainer::GetBLEU(TransCand ** transList, int transCount, TrainingSet * ts, int ngram, BLEU_TYPE type)
{

    float bestRefLen     = 0;
    float shortestRefLen = 0;
    float SBPRefLen      = 0;
    float * matchedNGram = new float[ngram];
    float * statNGram = new float[ngram];

    for( int n = 0; n < ngram; n++ ){
        matchedNGram[n] = 0;
        statNGram[n] = 0;
    }

    for( int i = 0; i < transCount; i++ ){
        TransCand * cand = transList[i];
        if( cand == NULL || ts->samples[i].CandCount == 0){
            bestRefLen += ts->samples[i].ref->shortestRefLen;
            shortestRefLen += ts->samples[i].ref->shortestRefLen;
            SBPRefLen += ts->samples[i].ref->shortestRefLen;
            continue;
        }

        int wordCount = cand->translation->wordCount;
        for( int n = 0; n < ngram && n < wordCount; n++ ){
            matchedNGram[n] += cand->nGramMatch[n];
            statNGram[n] += cand->translation->wordCount - n;
        }
        bestRefLen += cand->bestRefLen;
        shortestRefLen += cand->shortestRefLen;
        SBPRefLen += cand->SBPRefLen;

    }

    float bleu = Evaluator::CalculateBLEU(matchedNGram, statNGram, 
                                          bestRefLen, shortestRefLen, SBPRefLen, ngram, type);

    delete[] matchedNGram;
    delete[] statNGram;

    return bleu;
}

void OurTrainer::DumpPara(const char * fileName, ParaInfo * para, int paraCount, char * msg)
{
    FILE * file = fopen(fileName, "a");

    fprintf(file, "%s\n", msg);
    fprintf(file, "<");
    for(int i = 1; i < paraCount; i++ ){
        if( i > 1 )
            fprintf(file, " ");
        fprintf(file, "%0.4f", para[i].weight);
    }

    fprintf(file, ">\n");

    fclose(file);
}

int OurTrainer::ComputeIntersections(TrainingSample * sample, int dim, Intersection * intersections, int &num, int ngram)
{
    TransCand * cands = sample->Cands;

    int * fId = new int[sample->CandCount];
    float y1 = FLT_MIN, y2 = FLT_MIN;
    float a1 = FLT_MIN, a2 = FLT_MIN;
    int n1 = 0, n2 = 0;
    int count = 0;

    for( int n = 0; n < sample->CandCount; n++ ){
        TransCand * cand = cands + n;

        cand->b = 0;
        for(int i = 1; i < cand->featCount; i++){
            int fId = cand->feats[i].id;
            if( fId != dim )
                cand->b += para[fId].weight * cand->feats[i].value;
            else
                cand->slope = cand->feats[fId].value;
        }

        if(cand->slope * para[dim].minValue + cand->b > y1 ){
            y1 = cand->slope * para[dim].minValue + cand->b;
            a1 = cand->slope;
            n1 = n;
        }

        if(cand->slope * para[dim].maxValue + cand->b > y2 ){
            y2 = cand->slope * para[dim].maxValue + cand->b;
            a2 = cand->slope;
            n2 = n;
        }
    }

    Intersection * isct = intersections + num;
    isct->Create(ngram);

    isct->coord = para[dim].minValue - (float)0.0001;  // min bound
    for( int i = 0; i < ngram; i++ ){    // for BLEU computation
        isct->deltaMatch[i] = cands[n1].nGramMatch[i];
        isct->deltaNGram[i] = (float)(cands[n1].translation->wordCount - i);
    }
    isct->deltaBestRefLen = cands[n1].bestRefLen;
    isct->deltaShortestRefLen = cands[n1].shortestRefLen;
    isct->deltaSBPRefLen = cands[n1].SBPRefLen;
    num++;

    int lastN = n1;

    while(true){
        bool breakFlag = true;
        int curN = -1;
        float min = para[dim].maxValue;

        for( int n = 0; n < sample->CandCount; n++ ){
            TransCand * curCand = cands + n;

            if (curCand->slope <= cands[lastN].slope || curCand->slope > a2)
                continue;

            float x = (curCand->b - cands[lastN].b) / (cands[lastN].slope - curCand->slope); // intersection of the two lines
            if( x < min || (x == min && curCand->slope > cands[lastN].slope) ){
                min  = x;
                curN = n;
                breakFlag = false;
            }
        }

        if( breakFlag )
            break;

        isct = intersections + num;
        isct->Create(ngram);
        isct->coord = min;
        for( int i = 0; i < ngram; i++ ){  // for BLEU computation
            isct->deltaMatch[i] = cands[curN].nGramMatch[i] - cands[lastN].nGramMatch[i];
            isct->deltaNGram[i] = (float)(cands[curN].translation->wordCount - cands[lastN].translation->wordCount);
        }
        isct->deltaBestRefLen = cands[curN].bestRefLen - cands[lastN].bestRefLen;
        isct->deltaShortestRefLen = cands[curN].shortestRefLen - cands[lastN].shortestRefLen;
        isct->deltaSBPRefLen = cands[curN].SBPRefLen - cands[lastN].SBPRefLen;

        lastN = curN;
        num++;
        count++;
    }

    delete[] fId;

    return count;
}

int SortIntersectionWithW( const void * arg1, const void * arg2 )
 {
     Intersection * p1 = (Intersection *)arg1;
     Intersection * p2 = (Intersection *)arg2;

    if ( p1->coord > p2->coord )
         return 1;
     if ( p1->coord < p2->coord )
         return -1;
     return 0;
 };

// Minimum error rate training with line search (Och, ACL 2003)
int OurTrainer::MERTWithLineSearch(TrainingSet * ts, ParaInfo * para, 
                                   ParaInfo * finalPara, 
                                   int paraCount, int nRoundConverged, 
                                   int ngram, BLEU_TYPE BLEUType)
{
    int ncoverged = 0;
    float lastBLEU = 0; 
    float maxBLEU, maxW;
    int maxDim, maxRound;
    int maxItersectionNum = 0;
    int iNum = 0;
    int n = 0, count = 0;
    int    i, j;

    for( i = 0; i < ts->sampleCount; i++ ){
        int sNum = ts->samples[i].CandCount;
        maxItersectionNum += sNum * sNum + 2;
    }
    if( maxItersectionNum > 1000000 )
        maxItersectionNum = 1000000;
    Intersection * intersections = new Intersection[maxItersectionNum];

    ParaBase * pb = new ParaBase(nRoundConverged + 1);

    while(count++ < 100){
        maxBLEU = 0;
        maxDim  = 0;
        maxW    = 0;

        for( int d = 1; d < paraCount; d++ ){
            if( para[d].isFixed )
                continue;

            float * matchedNGram = new float[ngram];
            float * statNGram = new float[ngram];
            float bestRefLen, shortestRefLen, SBPRefLen;
            float wRecord = para[d].weight; // record the i-th weight
            para[d].weight = 0;             // remove it
            iNum = 0;                       // clear "intersections" list

            for( int s = 0; s < ts->sampleCount; s++ )
                ComputeIntersections(ts->samples + s, d, intersections, iNum, ngram);

            // sort by the d-th weight
            qsort(intersections, iNum, sizeof(Intersection), SortIntersectionWithW );

            // max bound
            intersections[iNum].Create(ngram);
            intersections[iNum++].coord = para[d].maxValue;

            memset(matchedNGram, 0, sizeof(float) * ngram);
            memset(statNGram, 0, sizeof(float) * ngram);
            bestRefLen = 0;
            shortestRefLen = 0;
            SBPRefLen = 0;

            for( i = 0; i < iNum - 1; i++ ){
                for( j = 0; j < ngram; j++){
                    matchedNGram[j] += intersections[i].deltaMatch[j];
                    statNGram[j] += intersections[i].deltaNGram[j];
                }
                bestRefLen += intersections[i].deltaBestRefLen;
                shortestRefLen += intersections[i].deltaShortestRefLen;
                SBPRefLen += intersections[i].deltaSBPRefLen;

                //if( intersections[i].coord < para[d].minValue || intersections[i].coord == intersections[i + 1].coord )
                //    continue;
                if( intersections[i].coord - para[d].minValue < 0)
                    continue;

                float bleu = Evaluator::CalculateBLEU(matchedNGram, statNGram, bestRefLen, 
                                                      shortestRefLen, SBPRefLen, ngram, BLEUType);

                if(bleu > maxBLEU){
                    maxDim = d;
                    maxBLEU = bleu;
                    maxW = (intersections[i].coord + intersections[i + 1].coord) / 2;
                }
            }

            para[d].weight = wRecord;
            
            delete[] matchedNGram;
            delete[] statNGram;
        }

        para[maxDim].weight = maxW;
        fprintf( stderr, "\rFeature %d at %.4f with BLEU = %.4f", maxDim, maxW, maxBLEU );

        if( fabs(lastBLEU - maxBLEU) < 1.0E-5 ){
            fprintf( stderr, "\r  Convergence %d with BLEU = %.4f            \n", n + 1, maxBLEU );
            pb->Add(para, paraCount, maxBLEU);    // record the weights
            n++;

            for( int d = 1; d < paraCount; d++ ){
    #ifndef    WIN32
                timeval time;
                gettimeofday (&time, NULL);
                srand(time.tv_sec);
    #else
                srand(clock());
    #endif
                int per = rand() % 21;
                //if( abs(para[d].weight) > 1 )
                //    para[d].weight *= (float)(1.0 + ((float)per - 10) / 100); // shirt +/- 10%
                //else
                    para[d].weight += ((float)per - 10) / 100; // shirt +/- 0.1

                if( para[d].weight <= para[d].minValue )
                    para[d].weight = para[d].minValue;
                if( para[d].weight >= para[d].maxValue )
                    para[d].weight = para[d].maxValue;

                maxDim = 0;
                maxBLEU = 0;
            }
        
            if(n >= nRoundConverged)
                break;
        }

        lastBLEU = maxBLEU;
    }

    maxBLEU = 0;
    maxRound = -1;
    for(  i = 0; i < pb->paraListLen; i++ ){
        if( maxBLEU < pb->bleuList[i] ){
            maxBLEU = pb->bleuList[i];
            maxRound = i;
        }
    }

    if( maxRound >= 0){
        for( int i = 1; i < pb->paraLen[maxRound]; i++ )
            finalPara[i].weight = pb->paraList[maxRound][i].weight;    // optimized weight
    }
    else
        fprintf( stderr, "Invalid BLEU is generated during MER training\n" );

    delete[] intersections;
    delete pb;

    return 0;
}

// Perceptron-based training
int OurTrainer::PeceptronBasedTraining(TrainingSet * ts, ParaInfo * para, ParaInfo * finalPara, 
                                       int paraCount, int nRoundConverged, int ngram, 
                                       BLEU_TYPE BLEUType)
{
    return 0;
}

}

