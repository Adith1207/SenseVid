#include "utils.h"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <dirent.h> //For Directories

using namespace std;


void makeDir(string dirName){

  if (mkdir(dirName.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) && errno != EEXIST){
    cerr << "Error while creating directory " << dirName <<endl;
    exit(EXIT_FAILURE);
  }
}

void deleteFolderContent(string dirName){
  DIR *theFolder = opendir(dirName.c_str());
  struct dirent *next_file;
  char filepath[256];

  //cout << "Folder " << dirName.c_str()<< " content will be deleted !" << endl;
  while ((next_file = readdir(theFolder)) != NULL ){
    // build the path for each file in the folder
    snprintf(filepath, sizeof(filepath), "%s/%s", dirName.c_str(), next_file->d_name);
    remove(filepath);
  }
  closedir(theFolder);
}

void createTraceFiles(string tracePath){
  ofstream traceFile;
  string frameTraceName = tracePath + "/st-frame";
  traceFile.open(frameTraceName,ofstream::out);
  traceFile << "#Rank Type Size(Bytes)\trefPSNR\trefSSIM\tbpp\tlayers Size (bits)\tcaptureEnergy(mJ)\tencodingEnergy(mJ)\tbit rate (kbps)" << endl;
  traceFile.close();

  string packetTraceName = tracePath + "/st-packet";
  traceFile.open(packetTraceName,ofstream::out);
  traceFile << "#time seqNb pktSize frameNb frameType layerNb \t blocksList" << endl;
  traceFile.close();
}


void writeFrameRecord(frameRecordStruct frameRecord, string tracePath){
  
  ofstream traceFile;
  string frameTraceName = tracePath + "/st-frame";
  traceFile.open(frameTraceName,ofstream::app);
  traceFile.precision (numeric_limits<double>::digits10 -12);

  traceFile << frameRecord.frameNb <<"\t"
  << frameRecord.frameType <<"\t" 
  <<fixed<< (int)ceil(frameRecord.frameSize/8.0) <<"\t"
  << frameRecord.PSNR << "\t" 
  << frameRecord.SSIM <<"\t"
  << frameRecord.bpp << "\t";
  for (uint ind=0; ind < frameRecord.layersSizeVector.size(); ind++)
    if (frameRecord.frameType=="M")
      traceFile << frameRecord.layersSizeVector[ind]<<" ";
    else traceFile << "- ";
  traceFile << "\t";
  traceFile << frameRecord.captureEnergy << "\t";
  traceFile << frameRecord.encodingEnergy << "\t";
  traceFile << frameRecord.bitRate;
  traceFile << endl;
  traceFile.close();
}



void writePacketRecord(packetRecordStruct packetRecord, string tracePath){
  ofstream traceFile;
  string packetTraceName = tracePath + "/st-packet";
  traceFile.open(packetTraceName, ofstream::app);
  
  traceFile <<packetRecord.sendTime <<"\t"<< packetRecord.seqNb <<"\t"<<ceil(packetRecord.packetSize/8.0)<<"\t"<<packetRecord.frameNb <<"\t"<< packetRecord.frameType <<"\t" << packetRecord.layerNb << "\t";
  
  if(packetRecord.frameType == "S"){
    bool suite = false;
    for (uint ind=0; ind < packetRecord.blockSeqVector.size(); ind++){
      if(ind == 0)
        traceFile << packetRecord.blockSeqVector[ind];
      
      else if(packetRecord.blockSeqVector[ind] == packetRecord.blockSeqVector[ind-1]+1){
        if (ind == packetRecord.blockSeqVector.size()-1)
          traceFile << "-" << packetRecord.blockSeqVector[ind];
        else
          suite = true;      
      }
 else if(suite){
        traceFile << "-" << packetRecord.blockSeqVector[ind-1] << " " << packetRecord.blockSeqVector[ind];
        suite = false;
      }
      else{
        traceFile << " " << packetRecord.blockSeqVector[ind];
      }
    }
    traceFile << endl;
  }else{
    for (uint ind=0; ind < packetRecord.blockSeqVector.size(); ind++)
      traceFile << packetRecord.blockSeqVector[ind]<<" ";
    traceFile << endl;
  }

  
  traceFile.close();
}


void writeDecodedFrameRecord(uint frameNb, frameDecoderStruct frameRecord, string tracePath){

  ofstream traceFile (tracePath+"/rt-frame",  ofstream::out|ofstream::app); 

  if (!traceFile.is_open()){
    cerr << "Problem opening file ..." << tracePath+"/rt-frame" <<endl;
    exit(EXIT_FAILURE);
  }
  
  //cout <<frameRecord.frameNb <<"\t" << frameRecord.frameType <<"\t"<<endl;
  traceFile.precision (numeric_limits<double>::digits10 -12);
  traceFile << frameNb <<"\t"
	    << frameRecord.frameType <<"\t"
	    <<fixed<< frameRecord.PSNR << "\t"
	    << frameRecord.SSIM << endl;
    
    traceFile.close();
    
  
}

vector<int> getEmptyLayers(int frameNb, string tracePath){
  
  string frameTraceName = tracePath + "/st-frame";
  ifstream traceFile (frameTraceName);
  string line; int layerRank=0;
  vector<int> emptyBlocks;
  
  if (traceFile.is_open()){
    while (getline(traceFile,line,'\n')){
      if (line[0] != '#' && line[0] != ' ' && !(line.empty())){
	char* line2 = const_cast<char*>(line.c_str());
	//cout << line2 << endl;
	if(frameNb == stoi(strtok(line2,"\t"))){
	  strtok(0,"\t");//bypass frameType,size,PSNR,SSIM and bbp
	  strtok(0,"\t");strtok(0,"\t");strtok(0,"\t");strtok(0,"\t");
	  string blocks = strtok(0,"\t");
	  //cout <<"Blocks " <<  blocks <<endl;
	  char *blocks2 = const_cast<char*>(blocks.c_str());
	  char *pch;
	  pch = strtok (blocks2," ");
	  while (pch != NULL ){
	    if (stoi(pch) == 0)
	      emptyBlocks.push_back(layerRank);
	    pch = strtok (NULL, " ");
	    layerRank++;
	  }
	}
      }
    }

    traceFile.close();
    
  }else{//traceFile.is_open()
    cout << "ERROR when accessing " << frameTraceName <<endl;
    exit(EXIT_FAILURE);
  }

  //cout <<" emptyBlocks " <<endl;
  return emptyBlocks;
}


void getRowCol(int blockNb, int frameWidth, int *row, int *col){

  assert (frameWidth > 0 && frameWidth%8==0);
  int blocksPerRow = frameWidth/8;
  *row = 8*(blockNb/blocksPerRow);
  *col = 8*(blockNb%blocksPerRow);
}

/**********************************
 ** Parse Files **
 ***********************************/

void getInputParam(string inputFileName, int * levelsNb, int * threshold,
	      int * framesNb, int* frameWidth, int * frameHeight){

  ifstream paramFile (inputFileName);
  if (!paramFile.is_open()){
    cout << "ERROR when accessing " << inputFileName  << endl;
    exit(EXIT_FAILURE);
  }
    
  string line;
  while ( getline (paramFile, line) ){
    char* line2 = const_cast<char*>(line.c_str());
    char* token = strtok(line2,"\t");
    
    while (token != NULL){
      string stringToken = string(token); //keyword
      token = strtok(0,"\t"); //value
      
      if (stringToken ==  inputParamKeywords[LEVELS_IND])
	*levelsNb = atoi(token);

      else if (stringToken == inputParamKeywords[THRESH_IND])
	*threshold = atoi(token);

      else if (stringToken == inputParamKeywords[CAP_FRAME_IND])
	*framesNb = atoi(token);
      else if (stringToken == inputParamKeywords[RESOL_IND]){
	stringToken = string(token);
 	*frameWidth = stoi(stringToken.substr(0, stringToken.find_last_of('x')));
	*frameHeight= stoi(stringToken.substr(stringToken.find_last_of('x') + 1,((int) stringToken.length())));
      }
      
      token = strtok(0,"\t");
    }    
  }
}




void parseRcvFile(string line, int * frameNb, string  *frameType, int *layerNb, vector<int> & packetBlocks){

  //  cout << line <<endl;
  //printVector(packetBlocks);
  packetBlocks.clear();
  
  char* line2 = const_cast<char*>(line.c_str());
  strtok(line2,"\t");
  strtok(0,"\t");
  strtok(0,"\t");
  *frameNb = stoi(strtok(0,"\t"));
  *frameType = strtok(0,"\t");
  *layerNb = stoi(strtok(0,"\t"));
  string blocks = strtok(0,"\t");
  
  char *blocks2 = const_cast<char*>(blocks.c_str());
  char *pch;
  pch = strtok (blocks2," ");
  while (pch != NULL){
    //cout << "got " << pch << endl;
    size_t found = string(pch).find_first_of("-");
    if (found!=string::npos){
      int beginBlock = stoi( string(pch).substr(0,found));
      int endBlock = stoi(string(pch).substr(found+1, string(pch).npos));
      for (int elt=beginBlock; elt<=endBlock; elt++)
	packetBlocks.push_back(elt);
    } else 
      packetBlocks.push_back(stoi(pch));
    pch = strtok (NULL, " ");
  }
  
  //cout << "In parse ...";
  //printVector(packetBlocks);
}

void printVector(vector<int> vect){
  vector<int>::iterator i;
  for (i=vect.begin(); i != vect.end(); ++i)
    cout << *i << ' ';
  
  cout << endl;
  
}
