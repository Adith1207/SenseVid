#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include "sensevid.h"
#include "energy.h"

void encodeMainFrame(codecParam codecP, videoParam videoP, simParam simP,
		     Mat frameToEncode, int frameNb,
		     frameRecordStruct *frameRecord, int* seqNb);

Mat getQuantisationMatrix(int QC);
Mat getQuantifFrame(codecParam codecP, Mat frameToEncode);
Mat getReferenceFrame(codecParam codecP, Mat quantifFrame);
  
Mat_<short> zigzagScan(Mat_<short> block);


vector<layerInfoStruct> makeLayers(codecParam codecP,
				   frameRecordStruct *frameRecord,
				   Mat_<short> zigzagBlock);

void packetizeMainFrameBlock(videoParam videoP, simParam simP, int frameNb,
			     int blockNb,vector<layerInfoStruct> layersVector,
			     packetRecordStruct *packetRecordArray, int* seqNb);

ushort getBlockCompressedSize(vector<layerInfoStruct> blockLayers);
double getEncodingEnergyM(codecParam codecP, int frameBlocksNb);

#endif
