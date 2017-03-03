/*
* $Id:
* 0018
*
* $File:
* me_reordering_table.cpp
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* decoder
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
* 2012-12-28,19:55
* 2012-12-12,14:20
*/

#include "me_reordering_table.h"


namespace decoder_me_reordering_table {

bool MeReorderingTable::init(Configuration &config) {
  initMeReorderingTable( config );
  return true;
}


bool MeReorderingTable::initMeReorderingTable(Configuration &config) {
  ifstream inFile(config.metabFile.c_str());
  if (!inFile) {
    cerr<<"ERROR: Can not open file \""<<config.metabFile<<"\".\n"<<flush;
    exit(1);
  } else {
    cerr<<"Loading ME-Reordering-Table\n"<<"  >> From File: "<<config.metabFile<<"\n"<<flush;
  }

  clock_t start, finish;
  start = clock();

  string lineOfInFile;
  size_t lineNo = 0;
  while (getline(inFile, lineOfInFile)) {
    ++lineNo;
    ClearIllegalChar(lineOfInFile);
    RmEndSpace(lineOfInFile);
    RmStartSpace(lineOfInFile);

    vector<string> tmpMeTab;
    Split(lineOfInFile, '\t', tmpMeTab);

    if (tmpMeTab.size() != 2) {
      cerr<<"WARNING: Format error in line "<<lineNo<<"\n"<<flush;
      continue;
    }

    if (!loadFeature(tmpMeTab, lineNo)) {
      continue;
    }

    finish = clock();
    if (lineNo % 50000 == 0) {
      cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
  }
  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<"  Done!\n\n"<<flush;
  inFile.close();
  return true;
}


bool MeReorderingTable::loadFeature(vector<string> &meTab, size_t &lineNo) {
  if (meTab.at( 0 ).size() < 7) {
    cerr<<"  WARNING: Format error in line "<<lineNo<<"\n"<<flush;
    return false;
  }

  float  weight             = ( float )atof( meTab.at( 1 ).c_str() )            ;
  bool   straightOrInverted = meTab.at( 0 ).substr( 0, 1 ) == "0" ? true : false;
  bool   srcOrTgt           = meTab.at( 0 ).substr( 2, 1 ) == "S" ? true : false;
  string position           = meTab.at( 0 ).substr( 3, 2 )                      ;
  string srcOrTgtWord       = meTab.at( 0 ).substr( 6 )                         ;

  int basePos = 0;
  if (!straightOrInverted) {
    basePos += 4;
  }
  if (position == "LR") {
    basePos += 1;
  } else if (position == "RL") {
    basePos += 2;
  } else if (position == "RR") {
    basePos += 3;
  }

  if (srcOrTgt) {
    if (meTableSrc.find( srcOrTgtWord ) == meTableSrc.end()) {
      vector< float > tmpWeights( 8, 0.0 );
      tmpWeights.at( basePos ) = weight;
      meTableSrc.insert( make_pair( srcOrTgtWord, tmpWeights ) );
    } else {
      meTableSrc[ srcOrTgtWord ].at( basePos ) = weight;
    }
  } else {
    if (meTableTgt.find( srcOrTgtWord ) == meTableTgt.end()) {
      vector< float > tmpWeights( 8, 0.0 );
      tmpWeights.at( basePos ) = weight;
      meTableTgt.insert( make_pair( srcOrTgtWord, tmpWeights ) );
    } else {
      meTableTgt[ srcOrTgtWord ].at( basePos ) = weight;
    }
  }
  return true;
}

}


