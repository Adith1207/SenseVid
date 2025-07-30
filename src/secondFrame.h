#ifndef _SECONDFRAME_H_
#define _SECONDFRAME_H_

#include "sensevid.h"
#include "energy.h"

void encodeSecondFrame(codecParam codecP, simParam simP, double FPS,
		       Mat currentFrame, int frameNb, int mainFrameNb,
		       frameRecordStruct *frameRecord, int* seqNb);

Mat reduceAndPacketizeSecondFrame(codecParam codecP, simParam simP,
				  double FPS, Mat frame, int frameNb,
				  int mainFrameNb, int* compressedSize,
				  int* seqNb, double* energy);

#endif
