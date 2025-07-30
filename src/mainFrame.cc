#include "mainFrame.h"
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;

void encodeMainFrame(codecParam codecP, videoParam videoP, simParam simP, Mat frameToEncode, int frameNb, frameRecordStruct *frameRecord, int* seqNb){
  
  Mat quantifFrame = getQuantifFrame(codecP, frameToEncode);
  Mat referenceFrame = getReferenceFrame(codecP, quantifFrame);
  
  //entropy coding and packetization
  
  packetRecordStruct packetRecordArray [codecP.levelsNb];//one per frame
  uint frameCompressedSize=0;
  int entropyC = 0;
  
  for (int i = 0; i < frameToEncode.rows; i += 8){
    for (int j = 0; j < frameToEncode.cols; j += 8){

      Mat quantifBlock, zigzagBlock;
      
      if (codecP.DCT.at(0)=='s')
	quantifBlock =  quantifFrame(Range(i,i+codecP.zoneSize),Range(j,j+codecP.zoneSize));
      else 
	quantifBlock =  quantifFrame(Range(i,i+8),Range(j,j+8));
      
      zigzagBlock = zigzagScan(quantifBlock);
      vector<layerInfoStruct> layersVector; // one per block
      layersVector = makeLayers(codecP, frameRecord , zigzagBlock);
      //compute entropy cycles
      for (uint ind=0; ind < layersVector.size(); ind++)
	entropyC += layersVector[ind].entropyCycles;
      
      frameCompressedSize += getBlockCompressedSize(layersVector);
      int blockNb = i*quantifFrame.cols/64 + j/8;
      packetizeMainFrameBlock(videoP, simP, frameNb, blockNb, layersVector, packetRecordArray, seqNb);
   }
 }
  //write frame record
  frameRecord->PSNR = getPSNR(referenceFrame, frameToEncode);
  frameRecord->SSIM = getSSIM(referenceFrame, frameToEncode);
  frameRecord->frameSize = frameCompressedSize;
  frameRecord->bpp = frameCompressedSize *
    1.0/(frameToEncode.cols * frameToEncode.rows);
  frameRecord->bitRate = frameCompressedSize * videoP.FPS /1000;
  int frameBlocksNb = (frameToEncode.cols * frameToEncode.rows)/64;
  frameRecord->captureEnergy = CAPTURE_E_PER_BLOCK*frameBlocksNb/1000;//mJ 
  frameRecord->encodingEnergy = getEncodingEnergyM(codecP, frameBlocksNb)
    + entropyC * POWER / PROC_CLOCK/1e6; 
 
  writeFrameRecord(*frameRecord, simP.tracePath);

  //write the reference frame
  string imageName;
  imageName = simP.capturedFramesDir+"/frame" +
    to_string(frameRecord->frameNb) + ".png";
  Mat origFrame = imread(imageName, cv::IMREAD_GRAYSCALE);
  
  imageName = simP.referenceFramesDir+"/frame"
    + to_string(frameRecord->frameNb) + ".png";
  imwrite(imageName,referenceFrame);
  
}//end encodeMainFrame


vector<layerInfoStruct> makeLayers(codecParam codecP, frameRecordStruct *frameRecord, Mat_<short> zigzagBlock){
  //retruns a vector of layers for the zigzag bloc
  
  uchar layerStart[13]={0,3,6,10,15,21,28,36,43,49,54,58,61};
  int levelStart = 0;
  layerInfoStruct layerInfo;
  vector<layerInfoStruct> layersVector;
  uchar endBlock;

  if (codecP.DCT=="CLA" || codecP.DCT.at(0)=='s') 
    endBlock = zigzagBlock.cols;

  if (codecP.DCT=="sLLM" || codecP.DCT=="sBIN"){
    uchar maxLevelsNb = 2*(codecP.zoneSize -1);
    
    if (codecP.levelsNb > maxLevelsNb)
      codecP.levelsNb = maxLevelsNb;
    //update layerStart for square DCT
    int inc = codecP.zoneSize -1;
    for(int i=codecP.zoneSize; i < maxLevelsNb; i++){
      layerStart[i] = layerStart[i-1] + inc--;
    }
  }//if square DCT
  
  if (codecP.DCT=="tLLM" || codecP.DCT=="tBIN"){
    if (codecP.levelsNb > codecP.zoneSize -1)
      codecP.levelsNb = codecP.zoneSize -1;
    endBlock = layerStart[codecP.zoneSize-1];
  }

  for (int i = 0; i < codecP.levelsNb; i++) {
   
    if (i < codecP.levelsNb-1) 
      layerInfo.layerRawData = zigzagBlock(Range::all(),Range(layerStart[i],layerStart[i+1])); 
    else layerInfo.layerRawData = zigzagBlock(Range::all(),Range(levelStart,endBlock));
      
    layerInfo.layerNb = i;
    levelStart = levelStart + layerInfo.layerRawData.cols;

    layerInfo.layerData = entropyCoder(layerInfo.layerRawData,
				       layerInfo.layerNb,
				       codecP.entropyCoding);
    layerInfo.entropyCycles = getEntropyCycles(layerInfo.layerRawData,
					       codecP.entropyCoding);
    if (layerInfo.layerData=="00") layerInfo.layerData="";
    layerInfo.layerSize = layerInfo.layerData.size();

    frameRecord->layersSizeVector[i] += layerInfo.layerSize;
    
    layersVector.push_back(layerInfo);
    
  }
  return layersVector;
}

Mat getQuantisationMatrix(int QC){

  Mat quantificationMatrix = (Mat_<int>(8,8) << 16,11,10,16,24,40,51,61,12,12,14,19,26,58,60,55,14,13,16,24,40,57,69,56,14,17,22,29,51,87,80,62,18,22,37,56,68,109,103,77,24,35,55,64,81,104,113,92,49,64,78,87,103,121,120,101,72,92,95,98,112,100,103,99);
  
  int S;
  if (QC < 50)   S = 5000/QC;
  else  S = 200 - 2* QC;
 
  Mat quantifM;
  Mat temp = S * quantificationMatrix + 50;
  for (int i = 0; i < temp.rows; i++)
   for (int j = 0; j < temp.cols; j++)
     temp.at<int>(i,j) /= 100;
  temp.setTo(1, temp == 0);
  temp.convertTo(quantifM, CV_32F);
  return (quantifM);
}

Mat getQuantifFrame(codecParam codecP, Mat frameToEncode){

  Mat block, dctBlock, floatBlock, dctFloatBlock, quantifBlock;
  Mat quantifFrame = Mat::zeros(frameToEncode.rows, frameToEncode.cols, CV_16S);     
  for (int i = 0; i < frameToEncode.rows; i += 8){
   for (int j = 0; j < frameToEncode.cols; j += 8){
     
     block = frameToEncode(Range(i,i + 8),Range(j, j + 8));
     block.convertTo(floatBlock, CV_32F);
     subtract (floatBlock, 128, floatBlock);
     dctBlock = DCT(floatBlock, codecP.DCT, codecP.zoneSize);
     dctBlock.convertTo(dctFloatBlock, CV_32F);
     divide(dctBlock, getQuantisationMatrix(codecP.qualityCoef), quantifBlock, 1, CV_16S);
     quantifBlock.copyTo(quantifFrame(Range(i,i + 8),Range(j, j + 8)));
     
   }//end for j
  }//end 2nd for i
  return quantifFrame;
}

Mat getReferenceFrame(codecParam codecP, Mat quantifFrame){

  Mat refBlock, refFloatBlock;
  Mat quantifBlock, dequantifBlock;
 
  Mat refFrame = Mat::zeros(quantifFrame.rows, quantifFrame.cols, CV_8U);  
  
  
  for (int i = 0; i < quantifFrame.rows; i += 8){
   for (int j = 0; j < quantifFrame.cols; j += 8){
     
     quantifBlock = quantifFrame(Range(i,i + 8),Range(j, j + 8));
     multiply(quantifBlock, getQuantisationMatrix(codecP.qualityCoef), dequantifBlock, 1, CV_32F);
     
     refFloatBlock = iDCT(dequantifBlock, codecP.DCT, codecP.zoneSize);
     refFloatBlock.convertTo(refBlock, CV_16S);
     refBlock=refBlock+128;
     refBlock.convertTo(refBlock,CV_8U);
     refBlock.copyTo(refFrame(Range(i,i + 8),Range(j, j + 8)));
   }
  }
  return refFrame;
}


/**************************************************/
void packetizeMainFrameBlock(videoParam videoP, simParam simP, int frameNb,
			     int blockNb, vector<layerInfoStruct> layersVector,
			     packetRecordStruct *packetRecordArray, int* seqNb){

  for (uint k=0; k<layersVector.size(); k++){

    if (layersVector[k].layerSize > simP.pktPayloadSize*8){
      cout << "M Very small payload size !! "
	   << layersVector[k].layerSize << endl;
      exit(EXIT_FAILURE);
    }
    
    if (blockNb == 0) {//first block
      packetRecordArray[k].frameType = "M";
      packetRecordArray[k].packetSize=0;
      packetRecordArray[k].blockSeqVector.clear();
      packetRecordArray[k].blockSeqVector.push_back(blockNb);
      packetRecordArray[k].layerNb = k;
      packetRecordArray[k].frameNb = frameNb;
      packetRecordArray[k].sendTime = (frameNb-1)*1.0/videoP.FPS;
    }
    
    if (packetRecordArray[k].packetSize + layersVector[k].layerSize
	< simP.pktPayloadSize*8){
      
      packetRecordArray[k].packetSize += layersVector[k].layerSize;
      packetRecordArray[k].layerNb = k;
     
      if (blockNb == videoP.blocksPerFrame -1 && packetRecordArray[k].packetSize > 0){//lastblock
        
        packetRecordArray[k].seqNb = ++*seqNb;
	packetRecordArray[k].blockSeqVector.push_back(blockNb);
	writePacketRecord(packetRecordArray[k], simP.tracePath);

	packetRecordArray[k].blockSeqVector.clear();
        packetRecordArray[k].blockSeqVector.push_back(blockNb+1);
      }
      
    } else if (packetRecordArray[k].packetSize > 0){
      
      packetRecordArray[k].seqNb=++*seqNb;
      packetRecordArray[k].blockSeqVector.push_back(blockNb);
      writePacketRecord(packetRecordArray[k], simP.tracePath);
            
      packetRecordArray[k].packetSize=0;
      packetRecordArray[k].blockSeqVector.clear();
      packetRecordArray[k].blockSeqVector.push_back(blockNb+1);
    }
  }

}


/*********************************************************/
Mat_<short> zigzagScan(Mat_<short> block){
  Mat_<short> blockZigZag;
  int i = 0, j = 0;
  int jmax = block.rows - 1, imax = block.cols - 1;

  while ((j <= jmax) && (i <= imax)){
    if (((i + j) % 2) == 0) {
      if (j == 0){
        blockZigZag.push_back(block.at<short>(j,i));
        if (i == imax) j++;
        else i++;
      } else if ((i == imax) && (j < jmax)){
        blockZigZag.push_back(block.at<short>(j,i));
        j++;
      } else if ((j > 0) && (i < imax)){
        blockZigZag.push_back(block.at<short>(j,i));
        j--; i++;
      }
    } else {
      if ((j == jmax) && (i <= imax)){
        blockZigZag.push_back(block.at<short>(j,i));
        i++;
      } else if (i == 0){
        blockZigZag.push_back(block.at<short>(j,i));
        if (j == jmax)  i++;
        else  j++;
        
      } else if ((j < jmax) && (i > 0)){
        blockZigZag.push_back(block.at<short>(j,i));
        j++;
        i--;
      }
    }
      if ((j == jmax) && (i == imax)){
      blockZigZag.push_back(block.at<short>(j,i));
      break;
    }
  } 
  return blockZigZag.t();
}//end zigzagScan()

ushort getBlockCompressedSize(vector<layerInfoStruct> blockLayers){
  ushort size=0;
  vector<layerInfoStruct>::iterator i = blockLayers.begin();
  for (i = blockLayers.begin(); i != blockLayers.end(); ++i)
    size += (*i).layerSize;
  
  return (size);  
}


double getEncodingEnergyM(codecParam codecP, int frameBlocksNb){

  //1D DCT number of ADD, MUL or SHT ...
  int llm_add[] = {0,6,20,23,24,35,26,28,29};
  int acc_llm_add[] = {0,6,26,49,73,98,124,152,181};

  int llm_mul[] = {0,0,6,8,9,9,10,11,11};
  int acc_llm_mul[] = {0,0,6,14,23,32,42,53,64};
  
  int bin_add[] = {0,7,13,19,27,28,28,28,30};
  int acc_bin_add[] = {0,7,20,39,66,94,122,150,180};

  int bin_shift[] = {0,0,2,6,11,12,12,12,13};
  int acc_bin_shift[] = {0,0,2,8,19,31,43,55,68};
  //my LLM implementaion 26 add and 14 mul
  
  int dctCycles, quantifCycles;
  int addLLM, mulLLM, addBIN, shiftBIN;
  double energy;
  
  if (codecP.DCT == "CLA"){
    dctCycles = CLA_ADD_NB * CYCLES_PER_FADD + CLA_MUL_NB * CYCLES_PER_FMUL;
    quantifCycles = 64*CYCLES_PER_FMUL;
  }

  if (codecP.DCT.at(0) == 't'){
    quantifCycles = codecP.zoneSize * (codecP.zoneSize + 1)/2;
    addLLM = llm_add[codecP.zoneSize] * codecP.zoneSize
      + acc_llm_add[codecP.zoneSize];
    mulLLM = llm_mul[codecP.zoneSize] * codecP.zoneSize
      + acc_llm_mul[codecP.zoneSize];
    addBIN = bin_add[codecP.zoneSize] * codecP.zoneSize
      + acc_bin_add[codecP.zoneSize];
    shiftBIN =bin_shift[codecP.zoneSize] * codecP.zoneSize
      + acc_bin_shift[codecP.zoneSize];
  }

  if (codecP.DCT.at(0) == 's'){
    quantifCycles = codecP.zoneSize * codecP.zoneSize;
    addLLM = llm_add[codecP.zoneSize] * (codecP.zoneSize + 8);
    mulLLM = llm_mul[codecP.zoneSize] * (codecP.zoneSize + 8);
    addBIN = bin_add[codecP.zoneSize] * (codecP.zoneSize + 8);
    shiftBIN = bin_shift[codecP.zoneSize] * (codecP.zoneSize + 8);
  }
  
  if (codecP.DCT.substr(1) == "LLM"){
    quantifCycles *= CYCLES_PER_FMUL;
    dctCycles = CYCLES_PER_FADD *  addLLM + CYCLES_PER_FMUL * mulLLM;
  }
    
  if (codecP.DCT.substr(1) == "BIN"){
    quantifCycles *= CYCLES_PER_MUL;
    dctCycles = CYCLES_PER_ADD * addBIN + CYCLES_PER_SHIFT * shiftBIN;
  }  
 
  
  energy = (quantifCycles + dctCycles)* frameBlocksNb/1000
    * POWER / PROC_CLOCK;
  
  return energy/1000.0; //mJ
}

