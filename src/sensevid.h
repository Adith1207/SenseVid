#ifndef _SENSEVID_H_
#define _SENSEVID_H_

#include "utils.h"
#include "trace.h"
#include <getopt.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sys/stat.h>
#include <fstream>
#include <dirent.h>

#define MAX_S_LAYERS 5
#define MAX_M_LAYERS 13

using namespace std;
using namespace cv;

//Order of parameters in array inputParamKeywords
#define LEVELS_IND 3
#define THRESH_IND 6
#define RESOL_IND 10
#define CAP_FRAME_IND 13

static string inputParamKeywords[] = {
  "CODEC", "GOP coef.", "Quality coef.",//0..2
  "M levels nb", "DCT", "Zone size", //3..5
  "Threshold", "Max. considered S level", "Entropy coding", //6..8
  "Original resolution", "Target resolution",//9,10
  "Original FPS", "Target FPS", "Captured Frames",//11,13
  "Simulation Id", "Packet payload size", "Output directory"//14-16
};


struct codecParam{
  string codec;
  int threshold;
  double gopCoef;
  uint qualityCoef;
  int levelsNb;
  string DCT;
  int zoneSize;
  string entropyCoding;
  int maxLevelS;

  codecParam();
      
}; //codecParam


struct videoParam{
  string videoFile;
  string videoName;
  int framesNb; int origFramesNb;
  int frameHeight;  int origFrameHeight;
  int frameWidth;  int origFrameWidth;
  int blocksPerFrame; //number of blocks in the frame
  double FPS;  double origFPS;
  
videoParam() : videoFile(""), origFramesNb(0), frameHeight(0),
    origFrameHeight(0), frameWidth(0), origFrameWidth(0),
    FPS(0), origFPS(0) {}
  
  void setOrigFrameHeight();
  void setOrigFrameWidth();
  void setOrigFPS();
  void setVideoName();
  void setOrigFramesNb();
  void setBlocksPerFrame(){blocksPerFrame = frameHeight * frameWidth /64;}
};//videoParam

  //****Network simulation parameters

struct simParam{
  string simId;
  int pktPayloadSize;
  string rtFile;
  string outputDir;

  string tracePath;
  string capturedFramesDir;
  string referenceFramesDir;
  string decodedFramesDir;
  
simParam() : simId("sim"), pktPayloadSize(96), rtFile(""), outputDir("./") {}

  void setTracePath(string videoName);
  void setCapturedFramesDir(string videoName);
  void setReferenceFramesDir(string videoName);
  void setDecodedFramesDir(string videoName);
  
}; //simParam

struct layerInfoStruct{
  int layerNb; //dataPriority 0..12 for M frames;
  int layerSize; //size of compressed data sizeof(layerData)
  string layerData; //compressed binary data
  Mat_<short> layerRawData; //raw data
  int entropyCycles; //Nb of cycles required by the entropy coder
};

/******************************************************/
/****************** Other functionss ******************/
/******************************************************/
//InputParameters
void createInputParametersFile(codecParam codecP,
			       videoParam videoP,
			       simParam simP);
void getInputParam(string inputFileName,
		   int * levelsNb, int * threshold,
		   int * framesNb, int* frameWidth, int * frameHeight);

//Video functions
int captureFrames(videoParam videoP, string capturedFramesDir);
void encodeVideo(codecParam codecP, videoParam videoP, simParam simP);
void buildReceivedVideo(codecParam codecP, videoParam videoP, simParam simP);
int getLastReceivedFrame(int frameNb,
			 vector<frameDecoderStruct> decodedFrameVector);
void fillInMainFrame(Mat & decodedFrame, vector<int> packetBlocks,
		     int layerNb, int layersNb);
void decodeSecondFrame(Mat & decodedFrame, vector<int> packetBlocks,
		       int layerNb);
void fillLayer(Mat & block, int layerNb, int layersNb);

//get functions

double getMS(Mat block);
double msToPsnr (double mse);
double getMSE (Mat block1, Mat block2);
double getPSNR(Mat block1, Mat block2);
double getSSIM(Mat block1, Mat block2);
Mat getImage(string imageName);

//DCT functions
Mat DCT(Mat block, string DCT, int zoneSize);
Mat iDCT(Mat  dctBlock, string DCT, int zoneSize);

Mat_<double> tllm2DCT(Mat_<double> block, uchar zoneSize);
Mat_<double> sllm2DCT(Mat_<double> block, uchar zoneSize);
Mat llm1DCT(Mat  block);
Mat llm1iDCT(Mat dct1D);
Mat llm2DCT(Mat block);
Mat_<double> llm2iDCT(Mat_<double> block);

Mat_<short> tbin2DCT(Mat_<short> block, uchar zoneSize);
Mat_<short> sbin2DCT(Mat_<short> block, uchar zoneSize);
Mat_<short> bin1DCT(Mat_<short> block);
Mat_<short> bin1iDCT(Mat_<short> block);
Mat_<short> bin2DCT(Mat_<short> block);
Mat_<short> bin2iDCT(Mat_<short> block);


//Entropy functions
Mat_<short> truncateLinearBlock(Mat_<short> block);
string dec2bin(short dec,int bits);
string entropyCoder(Mat_<short> layer, int layerNb, string entropy);
int blockEntropyCoder(string entropy, Mat linearBlock,
		      int blockNb, int prevBlockNb);

string encodeEG(Mat block);
int getEntropyCycles(Mat block, string entropyEncoder);
string expGolomEncodeValue(short value);
short getGroupID(short ne);
short getAmplitudeSize(short amplitude);
short biasEncoder(short amplitude);

string encodeRLE_EG(Mat block);
vector<short> encodeRLE(Mat level); 
string encodeHUFFMAN(Mat block);

#endif
