#ifndef _TRACE_H_
#define _TRACE_H_

#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;

struct frameRecordStruct{
  int frameNb;
  string frameType;
  int frameSize; //Bytes
  double PSNR;
  double SSIM;
  double bpp;
  double bitRate; //Kbps
  vector<int> layersSizeVector; //list of layers size of a frame (bits)
  double captureEnergy; //mJ
  double encodingEnergy;
};

struct frameDecoderStruct{
  cv::Mat decodedFrame;
  string frameType;
  double PSNR;
  double SSIM;
};


struct packetRecordStruct{
  double sendTime;
  int seqNb;
  int packetSize;
  int frameNb;
  string frameType;//M or Sx where x is the M to which is related the S frame
  int layerNb;
  vector<int> blockSeqVector;
};

void writeFrameRecord(frameRecordStruct frameRecord, string tracePath);
void writePacketRecord(packetRecordStruct packetRecord, string tracePath);


#endif
