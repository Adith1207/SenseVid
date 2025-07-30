#include "sensevid.h"
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;

codecParam::codecParam(){
  codec ="SMPEG";
  threshold = 1; //0 means lossless-encoding for S
  gopCoef = 0; // 0 means all frames are M-encoded
  qualityCoef = 10;
  levelsNb = 1;
  DCT = "tBIN";
  zoneSize = 8;
  entropyCoding = "EG";
  maxLevelS = 0; //only the highest priority are considered.
}

void videoParam::setOrigFrameHeight(){
  VideoCapture cap ( videoFile ); 
  if( ! cap.isOpened () ) { 
    cout << "Problem opening video file " << videoFile << endl;
    exit(EXIT_FAILURE);
  }
  origFrameHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
}

void videoParam::setOrigFrameWidth(){
  VideoCapture cap ( videoFile ); 
  if( ! cap.isOpened () ) { 
    cout << "Problem opening " << videoFile << endl;
    exit(EXIT_FAILURE);
  }
  origFrameWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
}

void videoParam::setOrigFPS(){
  VideoCapture cap ( videoFile ); 
  if( ! cap.isOpened () ) { 
    cout << "Problem opening " << videoFile << endl;
    exit(EXIT_FAILURE);
  }
  origFPS  = cap.get( cv::CAP_PROP_FPS );
}

void videoParam::setVideoName(){
  videoName = videoFile.substr(videoFile.find_last_of('/') + 1,
			       videoFile.find_last_of('.')
			       - videoFile.find_last_of('/') - 1);
}

void videoParam::setOrigFramesNb(){
  VideoCapture cap ( videoFile ); 
  if( ! cap.isOpened () ) { 
    cout << "Problem opening " << videoFile << endl;
    exit(EXIT_FAILURE);
  }
  origFramesNb  = cap.get( cv::CAP_PROP_FRAME_COUNT );
}


void simParam::setTracePath(string videoName){
  tracePath = outputDir + '/' + videoName + '-' + simId;
}

void simParam::setCapturedFramesDir(string videoName){
  capturedFramesDir = tracePath + "/capturedFrames";
}

void simParam::setReferenceFramesDir(string videoName){
  referenceFramesDir = tracePath + "/referenceFrames";
}

void simParam::setDecodedFramesDir(string videoName){
  decodedFramesDir = tracePath + "/decodedFrames";
}

void createInputParametersFile(codecParam codecP, videoParam videoP, simParam simP){

  ofstream file;
  string paramFileName = simP.tracePath + "/inputParameters";
  file.open(paramFileName, ofstream::out);
  int i=0;
  file << inputParamKeywords[i++] << "\t" << codecP.codec <<endl;
  file << inputParamKeywords[i++] <<"\t" << codecP.gopCoef << endl;
  file << inputParamKeywords[i++] <<"\t" << codecP.qualityCoef << endl;
  file << inputParamKeywords[i++] <<"\t" << codecP.levelsNb << endl;
  file << inputParamKeywords[i++] <<"\t" << codecP.DCT << endl;
  file << inputParamKeywords[i++] <<"\t" << codecP.zoneSize << endl;
  file << inputParamKeywords[i++] << "\t" << codecP.threshold << endl;
  file << inputParamKeywords[i++] << "\t" << codecP.maxLevelS << endl;
  file << inputParamKeywords[i++] <<"\t"  << codecP.entropyCoding << endl;

  file << inputParamKeywords[i++] <<"\t"  << videoP.origFrameWidth <<"x"<<videoP.origFrameHeight << endl;
  file << inputParamKeywords[i++] <<"\t"  << videoP.frameWidth<<"x"<<videoP.frameHeight  << endl;
  file << inputParamKeywords[i++] <<"\t"  << videoP.origFPS << endl;
  file << inputParamKeywords[i++] <<"\t"  << videoP.FPS << endl;
  file << inputParamKeywords[i++] <<"\t"  << videoP.framesNb << endl;
  file << inputParamKeywords[i++] <<"\t"  << simP.simId << endl;
  file << inputParamKeywords[i++] <<"\t"  << simP.pktPayloadSize << endl;
  file << inputParamKeywords[i++] <<"\t"  << simP.outputDir << endl;
   
  file.close();
}

