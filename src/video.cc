#include "sensevid.h"
#include "mainFrame.h"
#include "secondFrame.h"
#include <opencv2/opencv.hpp>
#include "trace.h"
#include "energy.h" 
#include <iostream>

using namespace std;

int captureFrames(videoParam videoP, std::string capturedFramesDir){
  
  int captureStep = floor(videoP.origFPS/videoP.FPS);
  cout << "captureFrames ..." << endl;
  
  VideoCapture cap ( videoP.videoFile );
  if( ! cap.isOpened () ) {
    cout << "Problem opening " << videoP.videoFile << endl;
    exit(EXIT_FAILURE);
  }
  videoP.setOrigFramesNb();
  deleteFolderContent(capturedFramesDir);
 
  int frameNb=1;
  for(int i=0; i < cap.get(cv::CAP_PROP_FRAME_COUNT); i=i+captureStep) {
   
    Mat frame, grayFrame;
    cap.set ( cv::CAP_PROP_POS_FRAMES , i );
    cap.read(frame);

    cvtColor(frame, grayFrame, cv::COLOR_RGB2GRAY);
    if (videoP.frameWidth != videoP.origFrameWidth ||
	videoP.frameHeight != videoP.origFrameHeight)
      resize(grayFrame, grayFrame, Size(videoP.frameWidth, videoP.frameHeight));
    
    string imageName = capturedFramesDir + "/frame" + to_string(frameNb) + ".png";
    imwrite(imageName,grayFrame);
    frameNb++;
  }
  return(frameNb - 1);
  
}//end captureFrames()

void encodeVideo(codecParam codecP, videoParam videoP, simParam simP){

  int static seqNb;
  frameRecordStruct frameRecord;
  string imageName;
  int mainFrameNb=1;
  double mse;

  Mat currentFrame, referenceFrame, mainFrame, diffFrame;
  
  if (codecP.codec == "SMPEG"){
    
    for(int i=1; i <= videoP.framesNb; i++) {

      frameRecord.frameNb = i;
      imageName = simP.capturedFramesDir+"/frame" + to_string(i) + ".png";
      currentFrame = imread(imageName, cv::IMREAD_GRAYSCALE);
      if(!currentFrame.data ){
        cout <<  "Could not open or find the image " << imageName << endl;
        exit(EXIT_FAILURE);
      }
  
      if (i > 1){
	Mat currentS, mainS, refMainS;
	currentFrame.convertTo(currentS, CV_16S);
	mainFrame.convertTo(mainS, CV_16S);
	mse = getMSE(currentFrame, mainFrame);
	mse = sqrt(mse);
      } 

      frameRecord.layersSizeVector.clear();
      frameRecord.layersSizeVector.resize(codecP.levelsNb, 0);
	
      if (i == 1 || mse > codecP.gopCoef){ //M encoded frame
	
        mainFrameNb = i;
        mainFrame = currentFrame.clone();
        frameRecord.frameType = "M";
        encodeMainFrame(codecP, videoP, simP, currentFrame, i,
			&frameRecord, &seqNb);

      } else if (mse <= codecP.gopCoef){//S encoded frame
	
	frameRecord.frameType = "S";
	encodeSecondFrame(codecP,  simP, videoP.FPS, currentFrame.clone(), i, mainFrameNb, &frameRecord, &seqNb);
        
      }
    }
  }
  else cout << "ONLY SMPEG iS IMPLEMENTED YET" << endl;

}

/******************************************/
/******* BUILD received video Functions ****/
/******************************************/

void buildReceivedVideo(codecParam codecP, videoParam videoP, simParam simP){
  string line, frameType, blocksList;
  int frameNb, layerNb;
  vector<frameDecoderStruct> decodedFrameVector (videoP.framesNb);
 
  Mat zeroFrame = Mat::zeros(videoP.frameHeight, videoP.frameWidth, CV_8U);

  string imageName;
  vector<int> packetBlocks;
  
  ifstream traceFile (simP.rtFile);
  
  if (!traceFile.is_open()){//isopen
    cout << "ERROR when accessing " << simP.rtFile <<endl;
    exit(EXIT_FAILURE);
  }
  
  //Initialize the decodedFrameVector
  for (uint ind=0; ind < decodedFrameVector.size(); ind++){
    decodedFrameVector[ind].decodedFrame = zeroFrame.clone();
    decodedFrameVector[ind].frameType = "N";
  }

  //Parse RT file and fill in the decodedFrameVector
  while ( getline (traceFile,line) ){
    if (line[0] != '#' && line[0] != ' ' && !(line.empty())){
       parseRcvFile(line, &frameNb,  &frameType, &layerNb, packetBlocks);

       if (frameType=="M"){
	 if (decodedFrameVector[frameNb-1].frameType == "N"){
	   //1st pkt for this frame : fill all blocks at the empty layers
	   vector<int> emptyLayers = getEmptyLayers(frameNb, simP.tracePath);
	   vector<int> allBlocks;
	   allBlocks.push_back(0);
	   allBlocks.push_back(videoP.frameWidth * videoP.frameHeight/64 -1);
	   for (uint li=0; li < emptyLayers.size(); li++)
	     fillInMainFrame(decodedFrameVector[frameNb-1].decodedFrame, allBlocks, emptyLayers[li], codecP.levelsNb);
	 }

	 //Then fill M with the received blocks in this packet 
	 fillInMainFrame(decodedFrameVector[frameNb-1].decodedFrame, packetBlocks, layerNb, codecP.levelsNb);
       }//M-frame decoding
       
       if (frameType.at(0)=='S'){
	 decodeSecondFrame(decodedFrameVector[frameNb-1].decodedFrame, packetBlocks, layerNb);
	 packetBlocks.clear();
       }
       decodedFrameVector[frameNb-1].frameType = frameType;
           
    }
  }
  
  /*******************************************************/
  /***** Process every frame from decodedFrameVector *****/
  /*******************************************************/

  if (decodedFrameVector.size() == 0){
    cout <<  "No frame received !! " << imageName << endl;
    exit(EXIT_FAILURE);
  }
 
  for (uint ind=0; ind < decodedFrameVector.size(); ind++){

    Mat referenceFrame = getImage(simP.referenceFramesDir +
				  "/frame" + to_string(ind+1) + ".png");
   
    if (decodedFrameVector[ind].frameType=="N"){
      //copy previous received frame
      int lastReceived = getLastReceivedFrame(ind, decodedFrameVector);
      if (lastReceived >= 0)
	decodedFrameVector[ind].decodedFrame = decodedFrameVector[lastReceived].decodedFrame;
    }
    
    if (decodedFrameVector[ind].frameType=="M"){
       Mat capturedFrame = getImage(simP.capturedFramesDir +
				    "/frame" + to_string(ind+1) + ".png");
       
       Mat quantifFrame = getQuantifFrame(codecP, capturedFrame);
       Mat decodedFrameS;
       decodedFrameVector[ind].decodedFrame.convertTo(decodedFrameS, CV_16S);
       decodedFrameS = decodedFrameS.mul(quantifFrame);
       decodedFrameVector[ind].decodedFrame
	 = getReferenceFrame(codecP, decodedFrameS);
       
       //enhance M-frame
       for (int i = 0; i < referenceFrame.rows; i += 8)
	 for (int j = 0; j < referenceFrame.cols; j += 8){
	   Mat tBlock = decodedFrameVector[ind].decodedFrame(Range(i,i + 8),Range(j, j + 8));
	   if (countNonZero(tBlock)==0)
	     blur( tBlock, tBlock, Size( 8, 36 ), Point(-1,-1) );
	 }
    }

    if (decodedFrameVector[ind].frameType.at(0)=='S'){
      decodedFrameVector[ind].decodedFrame = decodedFrameVector[ind].decodedFrame.mul(referenceFrame);
      //add to received main frame if block is not received
      int mainFrameNb = stoi(decodedFrameVector[ind].frameType.substr(1));
    
      if (decodedFrameVector[mainFrameNb-1].frameType=="N" ||  mainFrameNb==-1){
	//corresponding M-frame not received
      	decodedFrameVector[ind].frameType="N";
	int lastReceived = getLastReceivedFrame(ind, decodedFrameVector);
	if (lastReceived >= 0)
	  decodedFrameVector[ind].decodedFrame = decodedFrameVector[lastReceived].decodedFrame;
	continue;
      }

      decodedFrameVector[ind].frameType = "S";
      imageName = simP.decodedFramesDir + "/frame" + to_string(mainFrameNb) + ".png";
      Mat decodedMainFrame = imread(imageName,cv::IMREAD_GRAYSCALE);
      
      for (int i = 0; i < decodedMainFrame.rows; i += 8)
	for (int j = 0; j < decodedMainFrame.cols; j += 8){
	  Mat tempBlock = decodedFrameVector[ind].decodedFrame(Range(i,i + 8),Range(j, j + 8));
	  if (countNonZero(tempBlock)==0)
	    tempBlock +=  decodedMainFrame(Range(i,i + 8),Range(j, j + 8));
	}
    }

    
    imageName = simP.capturedFramesDir + "/frame" + to_string(ind+1) + ".png";
    Mat origFrame = imread(imageName,cv::IMREAD_GRAYSCALE);

    decodedFrameVector[ind].PSNR = getPSNR(decodedFrameVector[ind].decodedFrame, origFrame);
    decodedFrameVector[ind].SSIM = getSSIM(decodedFrameVector[ind].decodedFrame, origFrame);
    writeDecodedFrameRecord(ind+1, decodedFrameVector[ind], simP.tracePath);
    //Populating decodedFrames directory
    imageName = simP.decodedFramesDir + "/frame" + to_string(ind+1) + ".png";
    imwrite(imageName,decodedFrameVector[ind].decodedFrame);
  }

  traceFile.close();
  
}
/******************************************/


int getLastReceivedFrame(int ind,
			 vector<frameDecoderStruct> decodedFrameVector){
  
  if (decodedFrameVector.size() > 0 && ind > 0)
    for(uint i=ind-1; i>=0; i--)
      return i;
  return -1;
}

void fillInMainFrame(Mat & decodedFrame, vector<int> packetBlocks,
		     int layerNb, int layersNb){
  
  assert (packetBlocks.size() >= 2);
  for (int ind = packetBlocks[0]; ind<= packetBlocks[1]; ind++){
    int i,j;
    getRowCol(ind, decodedFrame.cols , &i, &j);
    
    Mat block = decodedFrame(Range(i,i + 8),Range(j, j + 8));
    fillLayer(block, layerNb, layersNb);
  }
}

void decodeSecondFrame(Mat & decodedFrame,
		       vector<int> packetBlocks, int layerNb){
  assert (packetBlocks.size() >= 1);
  for (uint ind=0; ind<packetBlocks.size(); ind++){
    int i,j;
    getRowCol(packetBlocks[ind], decodedFrame.cols, &i, &j);
    Mat block = decodedFrame(Range(i,i + 8),Range(j, j + 8));
    block = Mat::ones(8, 8, CV_8U);
  }
}


void fillLayer(Mat & block, int layerNb, int layersNb){

  if (layerNb == 0) block.at<uchar>(0,0) = 1;
  if (layerNb == 12) block.at<uchar>(7,7) = 1;
  
  int x;
  int y; 
  if (layerNb <=  6){
    x = 0; y = layerNb+1;
    while (y >=0)
      block.at<uchar>(x++,y--) = 1;
    
  } else {
    x = 7; y= layerNb - 6;
    while (y <=7)
      block.at<uchar>(x--,y++) = 1;
  }

  if (layerNb == layersNb-1) {//fill all  layers > layerNb
    for (int i=0; i < block.rows; i++)
      for (int j=0; j < block.cols; j++)
	if (i+j >= layersNb)
	  block.at<uchar>(i,j) = 1;
  }
  
}


//***********************************/
/******* EVAL FCTs****/
/******************************************/

double getMS(Mat block){
  Mat largeBlock;
  block.convertTo(largeBlock, CV_16U);
  largeBlock= block.mul(block);
  Scalar error2= sum(largeBlock);
  return error2.val[0] / (block.rows * block.cols);
}

double msToPsnr (double mse){
  if (mse >= 1e-10) return (10.0 * log10((255*255) / mse));
  else     return(600);
  
}

double getMSE (Mat block1, Mat block2){
  Mat diffBlock, diffB;
 
  absdiff(block1, block2, diffBlock);
  diffBlock.convertTo(diffB, CV_16U);
  diffB = diffB.mul(diffB);
  Scalar error2= sum(diffB);
  return error2.val[0] / (block1.rows * block1.cols);
}

double getPSNR (Mat block1, Mat block2){
  
  double mse = getMSE(block1, block2);
  if (mse >= 1e-10) return (10.0 * log10(255*255/mse));
  else     return(600);
}

double getSSIM(Mat capturedFrame, Mat decodedFrame){
  
  Mat I1,I2,I1_2,I2_2,I1_I2,mu1,mu2,mu1_2,mu2_2,mu1_mu2,
    sigma1_2,sigma2_2,sigma12,t1,t2,t3,ssim_map;
  Scalar mssim;
  double ssim,C1 = 6.5025,C2 = 58.5225;

  capturedFrame.convertTo(I1,CV_64F);
  decodedFrame.convertTo(I2,CV_64F);
  
  I1_2 = I1.mul(I1);
  I2_2 = I2.mul(I2);
  I1_I2 = I1.mul(I2);
  
  GaussianBlur(I1,mu1,Size(11,11),1.5);
  GaussianBlur(I2,mu2,Size(11,11),1.5);
  
  mu1_2 = mu1.mul(mu1);
  mu2_2 = mu2.mul(mu2);
  mu1_mu2 = mu1.mul(mu2);
  
  GaussianBlur(I1_2,sigma1_2,Size(11, 11),1.5);
  sigma1_2 -= mu1_2;
  
  GaussianBlur(I2_2,sigma2_2,Size(11, 11),1.5);
  sigma2_2 -= mu2_2;
  
  GaussianBlur(I1_I2,sigma12,Size(11, 11),1.5);
  sigma12 -= mu1_mu2;
  
  t1 = 2 * mu1_mu2 + C1;
  t2 = 2 * sigma12 + C2;
  t3 = t1.mul(t2);
  
  t1 = mu1_2 + mu2_2 + C1;
  t2 = sigma1_2 + sigma2_2 + C2;
  t1 = t1.mul(t2);
  
  divide(t3,t1,ssim_map);
  
  mssim = mean(ssim_map);
  
  ssim = mssim(0);
  
  return ssim;
}

/********************************/
/** GET FCTs for images on disk */
/********************************/

Mat getImage(string imageName){
  Mat image = imread(imageName,cv::IMREAD_GRAYSCALE);
   if(!image.data ){
     cout <<  "Could not open or find the image " << imageName << endl;
     exit(EXIT_FAILURE);
   }
   return image;
}


