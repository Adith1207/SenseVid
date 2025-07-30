#ifndef _UTILS_H_
#define _UTILS_H_

#include "trace.h"
#include "sensevid.h"
#include <string>

using namespace std;

void makeDir(string dirName);
void deleteFolderContent(string dirName);
void createTraceFiles(string tracePath);
void writeDecodedFrameRecord(uint frameNb, frameDecoderStruct frameRecord, string tracePath);
void parseRcvFile(string line, int* frameNb, string* frameType, int* layerNb, vector<int> & packetBlocks);
  
vector<int> getEmptyLayers(int frameNb, string tracePath);
void getRowCol(int blockNb, int frameWidth, int *row, int *col);
void printVector(vector<int> vect);

#endif
