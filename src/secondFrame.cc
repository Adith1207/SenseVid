#include "secondFrame.h"
#include <iostream>

using namespace std;

void encodeSecondFrame(codecParam codecP, simParam simP, double FPS,
		       Mat currentFrame, int frameNb, int mainFrameNb,
		       frameRecordStruct *frameRecord, int* seqNb){
 

  Mat refMainFrame = getImage(simP.referenceFramesDir + "/frame" +
			      to_string(mainFrameNb) + ".png");
  Mat mainFrame = getImage(simP.capturedFramesDir + "/frame" +
			   to_string(mainFrameNb) + ".png");
  
  Mat currentS, mainS, refMainS;
  currentFrame.convertTo(currentS, CV_16S);
  mainFrame.convertTo(mainS, CV_16S);
  refMainFrame.convertTo(refMainS, CV_16S);

  Mat redFrame = reduceAndPacketizeSecondFrame(codecP, simP, FPS, currentS - mainS, frameNb, mainFrameNb, &frameRecord->frameSize, seqNb, &frameRecord->encodingEnergy); //reduce and packetize the S frame
    
  Mat refCurrentS = refMainS + redFrame.clone();
  Mat diffReferenceFrame; 
  refCurrentS.convertTo(diffReferenceFrame, CV_8U);
  
  string imageName = simP.referenceFramesDir + "/frame" + to_string(frameNb) + ".png";
  imwrite(imageName, diffReferenceFrame);
  
  frameRecord->bpp = frameRecord->frameSize * 1.0 /(currentFrame.cols * currentFrame.rows);
  frameRecord->bitRate = frameRecord->frameSize * FPS /1000;
  frameRecord->PSNR = getPSNR(currentFrame.clone(), diffReferenceFrame.clone());
  frameRecord->SSIM = getSSIM(currentFrame.clone(), diffReferenceFrame.clone());
  int frameBlocksNb = (currentFrame.cols * currentFrame.rows)/64;
  frameRecord->captureEnergy = CAPTURE_E_PER_BLOCK*frameBlocksNb/1000;//mJ 
   
  writeFrameRecord(*frameRecord, simP.tracePath);
  
}//SecondFrame encoding

//////////////////////////////


Mat reduceAndPacketizeSecondFrame(codecParam codecP, simParam simP, double FPS,
				  Mat frame, int frameNb, int mainFrameNb,
				  int* compressedSize, int* seqNb,
				  double* energy){
  
  int msv[4]={650, 205, 51, 13};
  Mat block, linearBlock;
  double ms;
  int blockNb, blockPriority;
  Mat retFrame  = Mat::zeros(frame.rows, frame.cols, CV_16S);
  *compressedSize = 0;
  
  vector<packetRecordStruct> packetRecordVector;//one per frame
  packetRecordStruct packetRecordArray [MAX_S_LAYERS];
  for (int k=0; k < MAX_S_LAYERS; k++){
    packetRecordArray[k].sendTime = (frameNb-1)*1.0/FPS;
    packetRecordArray[k].packetSize = 0;
    packetRecordArray[k].frameNb = frameNb;
    packetRecordArray[k].frameType = "S" + to_string(mainFrameNb);
    packetRecordArray[k].layerNb = k;
    packetRecordArray[k].blockSeqVector.clear();
  }
  
  int cycleNb = frame.rows*frame.cols * (CYCLES_PER_ADD+CYCLES_PER_MUL);
  //count for cycles in red1
   
  for (int i = 0; i < frame.rows; i += 8){
    for (int j = 0; j < frame.cols; j += 8){
      
      blockNb = i * frame.cols / 64 + j/8;
      block = frame (Range(i,i + 8), Range(j, j + 8));
      ms = getMS(block.clone());

      if (ms > 0){
	if (ms >= msv[0]) blockPriority = 0;
	else if (ms >= msv[1]) blockPriority = 1;
	else if (ms >= msv[2]) blockPriority = 2;
	else if (ms >= msv[3]) blockPriority = 3;
	else blockPriority = 4;
	
	cycleNb += 192; //3*64 cycles (thresholding)
	//Thresholding to zero all pixels > codecP.threshold, thresh type=3
	//255 is max pixel value : opencv fct
	threshold(block, block, codecP.threshold, 255, 3);
	if (countNonZero(block)==0 || blockPriority > codecP.maxLevelS)
	  continue;
	  
	block.copyTo(retFrame(Range(i,i + 8),Range(j, j + 8)));
	//encode Block and put it in a packet !
	if ( ! block.isContinuous()) block = block.clone();
	linearBlock = block.reshape(1,1);
	int prevBlockNb = 0;
	if (packetRecordArray[blockPriority].blockSeqVector.size()!=0)
	  prevBlockNb = packetRecordArray[blockPriority].blockSeqVector[packetRecordArray[blockPriority].blockSeqVector.size()-1];

	int compressedLayerSize = blockEntropyCoder(codecP.entropyCoding, linearBlock, blockNb, prevBlockNb);
	cycleNb += getEntropyCycles(linearBlock, codecP.entropyCoding);
	
	if (compressedLayerSize > simP.pktPayloadSize*8){
	  cout << "S Very small payload size !! Block "<< blockNb <<" requires " << (int)compressedLayerSize/8+1 <<" bytes." <<endl;
	  exit(EXIT_FAILURE);
	}
		
	if (packetRecordArray[blockPriority].packetSize + compressedLayerSize < simP.pktPayloadSize*8){//there is room
	  packetRecordArray[blockPriority].packetSize += compressedLayerSize;
	  packetRecordArray[blockPriority].layerNb = blockPriority;
	  packetRecordArray[blockPriority].blockSeqVector.push_back(blockNb);
	} else {//packet record to write
	  
	  packetRecordArray[blockPriority].seqNb=++*seqNb;
	  writePacketRecord(packetRecordArray[blockPriority], simP.tracePath);
	  
	  *compressedSize+=packetRecordArray[blockPriority].packetSize;//bits
	  packetRecordArray[blockPriority].blockSeqVector.clear();
	  packetRecordArray[blockPriority].blockSeqVector.push_back(blockNb);
	  packetRecordArray[blockPriority].packetSize =
	    blockEntropyCoder(codecP.entropyCoding, linearBlock, blockNb, 0);
	  cycleNb += getEntropyCycles(linearBlock, codecP.entropyCoding);
	}
	
      }//ms>0
    }
  }//end scan of frame

  *energy = cycleNb * POWER / PROC_CLOCK /1000/1000;
    
  for (int k=0; k<MAX_S_LAYERS; k++){//write the remaining packets
    if (packetRecordArray[k].packetSize > 0){
      packetRecordArray[k].seqNb=++*seqNb;
      writePacketRecord(packetRecordArray[k], simP.tracePath);
      *compressedSize+=packetRecordArray[k].packetSize;
    }
   }
    
   return retFrame;
}//reduce and packetize S




