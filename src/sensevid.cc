#include "sensevid.h"
#include <iostream>

using namespace std;

int main(int argc,char *argv[]){

  codecParam codecP;
  videoParam videoP;
  simParam simP;
  bool rtFileSpecified = false;

  static struct option long_options[] = {
    
    //encoder options : all of them have default values
    {"codec",     required_argument,       0, 'c'},
    {"thresh",  required_argument,       0, 't'},
    {"gopCoef",  required_argument,       0, 'g'},
    {"qualityCoef",  required_argument,       0, 'q'},
    {"levelsNb",  required_argument,       0, 'l'},
    {"dct",  required_argument,       0, 'd'},
    {"zoneSize",  required_argument,       0, 'z'},
    {"entropy",  required_argument,       0, 'e'},
    {"maxLevelS",  required_argument,       0, 'm'},
    //max layer nb to consider in S frames : 0..4 range
   
    //video options : default ones can be deduced from the video
    {"height",  required_argument,       0, 'h'},
    {"width",  required_argument,       0, 'w'},
    {"fps",  required_argument,       0, 'f'},
    
    //simulation/network options
    {"sim",  required_argument, 0, 's'},
    {"pps",  required_argument, 0, 'p'},
    {"rt",  required_argument, 0, 'r'},
    {"output",    required_argument, 0, 'o'},
    {0, 0, 0, 0}
  };
  
  int c;
  int option_index = 0;
  
  while ( (c = getopt_long (argc, argv, "c:d:e:f:g:h:l:m:o:p:q:r:s:t:w:z:", long_options, &option_index)) != - 1){
    
    switch (c) {
    case 0:
      if (long_options[option_index].flag != 0)
	break;
      printf ("option %s", long_options[option_index].name);
      if (optarg)
	printf (" with arg %s", optarg);
      printf ("\n");
      break;
	
    case 'c':
      codecP.codec= optarg;
      if (codecP.codec!="SMPEG") {
	cerr << "NOT IMPLEMENTED CODEC" << codecP.DCT <<endl;
	exit(EXIT_FAILURE);
      }	
      break;
      
    case 't':
      codecP.threshold = atoi(optarg);
      if (codecP.threshold > 255) {
	cerr << "Threshold has to be in 0..255 integer range" << endl;
	exit(EXIT_FAILURE);
      }
      break;

    case 'g':
      codecP.gopCoef = atof(optarg);
      if (codecP.gopCoef > 255) {
	cerr << "GOP coefficient has to be in 0..255 real range" << endl;
	exit(EXIT_FAILURE);
      }
      break;

    case 'q':
      codecP.qualityCoef = atoi(optarg);
      if (codecP.qualityCoef < 0 || codecP.qualityCoef > 100){
	cerr << "QF must be in 1..100 integer range" <<endl;
	exit(EXIT_FAILURE);
      }	
      
      break;

    case 'l':
      codecP.levelsNb = atoi(optarg);
      
      if (codecP.levelsNb > MAX_M_LAYERS && codecP.levelsNb <= 0) {
	cerr << "Priority levels (M) must be in 1..13 integer range" << endl;
	exit(EXIT_FAILURE);
      }	
      break;
      
    case 'd':
      codecP.DCT = optarg;
      if (codecP.DCT!="CLA" && codecP.DCT!="sBIN" && codecP.DCT!="sLLM" && codecP.DCT!="tBIN" && codecP.DCT!="tLLM") {
	cerr << "NOT RECOGNIZED DCT" << codecP.DCT <<endl;
	exit(EXIT_FAILURE);
      }	
      break;

    case 'z':
      codecP.zoneSize = atoi(optarg);
      if (codecP.zoneSize > 8 && codecP.zoneSize <= 0) {
	cerr << "Zone size (M) must be in 1..8 integer range" << endl;
	exit(EXIT_FAILURE);
      }	
      break;

      
    case 'e':
      codecP.entropyCoding = optarg;
      // not recognised Entropy is dealt with in entropy.cc
      break;
      
    case 'm':
      codecP.maxLevelS = atoi(optarg);
      if (codecP.maxLevelS > MAX_S_LAYERS) {
	cerr << "max S levelS must be < 5" << endl;
	exit(EXIT_FAILURE);
      }
      break;

    case 'h':
      videoP.frameHeight = atoi(optarg);
      if (videoP.frameHeight % 8 != 0)  {//if 0, dealt with below
	cerr << "frameh height must be non zero and multiple of 8 !!!" << endl;
	exit(EXIT_FAILURE);
      }
      break;

    case 'w':
      videoP.frameWidth = atoi(optarg);
      if (videoP.frameWidth % 8 != 0) {
	cerr << "frameh width must be non zero and multiple of 8 !!!" << endl;
	exit(EXIT_FAILURE);
      }
      break;

    case 'f':
      videoP.FPS = atof(optarg);
      break;

    case 's':
      simP.simId = optarg;
      break;

    case 'p':
      simP.pktPayloadSize = atoi(optarg);
      break;

    case 'r':
      simP.rtFile = optarg;
      rtFileSpecified = true;
      break;

    case 'o':
      simP.outputDir = optarg;
      break;
      
      
    case '?':
      break;
	
    default:
      abort ();
      
    }
  }
  
   
  /* Deal with remaining (non option) command line arguments */
  if (optind < argc){
    //VideoFile parameters 
    videoP.videoFile = argv[optind];
    videoP.setVideoName();
    
    if (videoP.FPS == videoP.origFPS)
      videoP.framesNb = videoP.origFramesNb;
    
    videoP.setOrigFrameHeight();
    if (videoP.frameHeight==0)
      videoP.frameHeight = videoP.origFrameHeight;

    videoP.setOrigFrameWidth();
    if (videoP.frameWidth==0)
      videoP.frameWidth = videoP.origFrameWidth;
    
    videoP.setOrigFPS();
    if (videoP.FPS==0)
      videoP.FPS = videoP.origFPS;

    videoP.setBlocksPerFrame();
  }

  //Is videoFile is provided ?
  if(videoP.videoFile ==""){
    cerr << "No video file provided !!" << endl;
    exit(EXIT_FAILURE);
  }

  //Set other simulation parameters 
  simP.setTracePath(videoP.videoName);
  simP.setCapturedFramesDir(videoP.videoName);
  simP.setReferenceFramesDir(videoP.videoName);
  
  if (!rtFileSpecified){
    
    makeDir(simP.outputDir);
    makeDir(simP.tracePath);
    makeDir(simP.capturedFramesDir);
    deleteFolderContent(simP.capturedFramesDir);
    makeDir(simP.referenceFramesDir);
    deleteFolderContent(simP.referenceFramesDir);
    createTraceFiles(simP.tracePath);
    
    //Video capture and encoding
    videoP.framesNb = captureFrames(videoP, simP.capturedFramesDir);
    createInputParametersFile(codecP, videoP, simP);
    encodeVideo(codecP, videoP, simP);

  } else {//rtFile provided
    //Required options : output directory and simId
    //if not provided can take default values i.e. "./" and "sim" resp.
    //Others : can be deduced from the inputParameters file
    //located at outputDir/videoName-simId/
    
    simP.setDecodedFramesDir(videoP.videoName);
    makeDir(simP.decodedFramesDir);
    deleteFolderContent(simP.decodedFramesDir);
    //create rt-frame 
    ofstream traceFile;
    string frameTraceName = simP.tracePath + "/rt-frame";
    traceFile.open(frameTraceName,ofstream::out);
    traceFile << "#Rank\tframeType\tPSNR\tSSIM" << endl;

    //Get required parameters to decode 
    getInputParam(simP.tracePath + "/inputParameters",
		  &codecP.levelsNb, &codecP.threshold,
		  &videoP.framesNb, &videoP.frameWidth, &videoP.frameHeight);
    
    //rebuild the received video
    buildReceivedVideo(codecP, videoP, simP);
  }
  return 0;
}


