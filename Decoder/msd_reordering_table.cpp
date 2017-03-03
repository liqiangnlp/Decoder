/*
* $Id:
* 0020
*
* $File:
* me_reordering_table.h
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
* 2012-12-12,14:22
*/

#include "msd_reordering_table.h"


namespace msd_reordering_table {

bool MsdReorderingTable::init(Configuration &config) {
  ifstream inFile(config.msdtabFile.c_str());
  if (!inFile) {
    cerr<<"ERROR: Can not open file \""<<config.msdtabFile<<"\".\n"<<flush;
    exit(1);
  } else {
    cerr<<"Loading MSD-Reordering-Model\n"<<"  >> From File: "<<config.msdtabFile<<"\n"<<flush;
  }
  clock_t start, finish;
  start = clock();
  string lineOfInFile;
  size_t lineNo = 0  ;
  while (getline(inFile, lineOfInFile)) {
    ++lineNo;
    ClearIllegalChar( lineOfInFile );
    RmEndSpace(       lineOfInFile );
    RmStartSpace(     lineOfInFile );
    vector< string > msdReorderTableContents;
    SplitWithStr( lineOfInFile, " ||| ", msdReorderTableContents );
    vector< string > scoresOfMsdTable;
    Split( msdReorderTableContents.at( 2 ), ' ', scoresOfMsdTable);
    double prevMono = atof(scoresOfMsdTable.at( 0 ).c_str());                   // MSD reordering model: Previous  & Monotonic
    double prevSwap = atof(scoresOfMsdTable.at( 1 ).c_str());                   // MSD reordering model: Previous  & Swap
    double prevDisc = atof(scoresOfMsdTable.at( 2 ).c_str());                   // MSD reordering model: Previous  & Discontinuous
    double follMono = atof(scoresOfMsdTable.at( 3 ).c_str());                   // MSD reordering model: Following & Monotonic
    double follSwap = atof(scoresOfMsdTable.at( 4 ).c_str());                   // MSD reordering model: Following & Swap
    double follDisc = atof(scoresOfMsdTable.at( 5 ).c_str());                   // MSD reordering model: Following & Discontinuous
    ScoresOfMsdReorderOption scoresOfMsdReorderOption(prevMono, prevSwap, prevDisc, follMono, follSwap, follDisc);
    MsdReorderOption msdReorderOption( msdReorderTableContents.at( 1 ), scoresOfMsdReorderOption );
    msdReordersOptions[ msdReorderTableContents.at( 0 ) ].msdReorderOption.push_back( msdReorderOption );
    finish = clock();
    if (lineNo % 10000 == 0) {
      cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
  }
  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<"  Done!\n\n"<<flush;
  inFile.close();
  return true;
}

}

