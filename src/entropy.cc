#include "sensevid.h"
#include "huffman.h"
#include <iostream>

using namespace std;

string entropyCoder(Mat_<short> layer, int layerNb, string entropyEncoder){
  //*******
  // EG : all values are EG encoded
  //******************************
  // RLE_EG
  // AC : RLE_EG
  // DC : VLI (size, DCValue) = (symb1, symb2)
  //************************
  // HUFFMAN
  // DC : VLI (size, DCValue) = (symb1, symb2)
  // AC : symb1, symb2
  //  where symb1=(RL_zeros,size_amplitude) symb2=amplitude
  //  Huffman is applied on symb1 both DC and AC
  //************************
  
  string eg="";
  
  if (entropyEncoder=="EG") {
    eg += encodeEG(layer);
  }

  else if (entropyEncoder=="RLE_EG"){
    if (layerNb==0){
      string symb1, symb2;//DC VLI coded
      int amplitude = getAmplitudeSize(layer.at<short>(0));
      symb1 = dec2bin(amplitude,4);
      symb2 = dec2bin(biasEncoder(layer.at<short>(0)), amplitude); 
      eg = symb1 + symb2;
      eg += encodeRLE_EG(layer(Range::all(), Range(1,layer.cols)));
			 
    } else {
      eg += encodeRLE_EG(layer);
    }
    
  }//RLE_EG

  else if (entropyEncoder == "HUFFMAN"){
    Mat truncBlock = layer.clone();
    Mat shortBlock;
    truncBlock.convertTo(shortBlock, CV_16S);
    
    if (layerNb==0){
      string symb1, symb2;//DC VLI coded
      int DC = layer.at<short>(0);
      symb1 = dc_huffman[getAmplitudeSize(DC)];
      symb2 = dec2bin(biasEncoder(DC), getAmplitudeSize(DC)); 
      eg = symb1 + symb2;

      Mat tempBlock = shortBlock(Range::all(), Range(1,layer.cols));
      Mat tBlock = truncateLinearBlock(tempBlock.clone());
      
      eg += encodeHUFFMAN(tBlock);
      
    } else {
      truncBlock = truncateLinearBlock(shortBlock.clone());
      eg += encodeHUFFMAN(truncBlock);
    }

  }//HUFFMAN
  
  else {
    cerr << "Not recognized entropy coder " << entropyEncoder << endl;
    exit(EXIT_FAILURE);
  }
  
  return (eg);
}// entropyCoder


/*******************************
 * S block entropy coder *******
 ******************************/ 
int blockEntropyCoder(string entropy, Mat linearBlock,
		      int blockNb, int prevBlockNb){
  
  int valueToEncode = blockNb - prevBlockNb;
  //valueToEncode is coded using VLI :
  //on 4 bits : required number (n) of bits to encode valueToEncode
  //on n bits : bias encoding of valueToEncode
  
  string blockNbCode = dec2bin(getAmplitudeSize(valueToEncode),4) + dec2bin(biasEncoder(valueToEncode), getAmplitudeSize(valueToEncode));
  
  string layerData = blockNbCode + entropyCoder(linearBlock, 5, entropy);
  //layerNb !=1
  
  return layerData.size();
} 


/**************************************
 ******       Encoders     *************
 **************************************/

string encodeEG(Mat block){//All values are EG encoded
  
  string eg="";
  int firstZeroInd=-1;
  for (int i = 0; i < block.cols; i++){
   
    string egCode=expGolomEncodeValue(block.at<short>(i));
    
    if (egCode=="0" && firstZeroInd==-1) firstZeroInd=eg.length();
    if (egCode!="0") firstZeroInd=-1;
    eg+= egCode;
  }
  eg = eg.substr(0,firstZeroInd); //retrieve all zeros at the end
  
  eg+="00";//EOB EG encoded
  return eg; 
}

string encodeRLE_EG(Mat block){
  //RLE is applied on zero values : {NonZeroValue, 0-occurrence}* 
  //EG is applied on the resulted RLE

  string eg="";
  vector<short> rle =  encodeRLE(block);
 
  for (vector<short>::const_iterator i=rle.begin(); i != rle.end(); ++i){
    
    eg+=expGolomEncodeValue(*i);
    
  }
 
  return eg; 
}




string encodeHUFFMAN(Mat block){

  //symb1 = RL(nb of zeros), size_amplitude
  //symb2 = amplitude
  // Huffman is applied on symb1
  // symb1 = size for the DC
  //       = (RL, size) for the AC

  
  string eg="";
  short occurrence = 0;
  
  for (int i=0; i < block.cols; i++){
    int value = block.at<short>(i);
    if (value != 0){
      if (getAmplitudeSize(value) > 10){
	cerr << "Amplitude size > 10 !!" << endl;
	exit(EXIT_FAILURE);
      }
      eg += ac_huffman[occurrence][getAmplitudeSize(value)];
      eg += dec2bin(biasEncoder(value), getAmplitudeSize(value));//amplitude
      
    } else if (i < block.cols - 1){
      occurrence++;
      if (block.at<short>(i + 1)!=0){
	while (occurrence >= 15){
	  eg+= ac_huffman[15][0];
	 
	  occurrence-=15;
	}

	//encode next non zero : size then amplitude
	  int nextValue = block.at<short>(i+1);
	  if (getAmplitudeSize(nextValue) > 10){
	    cerr << "Amplitude size > 10 !!" << endl;
	    exit(EXIT_FAILURE);
	  }
	  eg += ac_huffman[occurrence][getAmplitudeSize(nextValue)];
	  eg+=dec2bin(biasEncoder(nextValue), getAmplitudeSize(nextValue));
	  occurrence = 0;
	  i++;
	}
      }
  } 
  eg+=ac_huffman[0][0];//EOB
  return eg; 
}//VLI_HUFF


//**RLE
vector<short> encodeRLE(Mat level){
  vector<short> RLE;
  double occurrence = 0;
 
  if (countNonZero(level)==0){
    RLE.push_back(0); RLE.push_back(0);
    return RLE;
  }
 
  short* p; int j;
  p = level.ptr<short>(0);
  for (j = 0; j < level.cols; ++j){
    if (p[j] != 0 || j==level.cols-1){
      RLE.push_back(p[j]);
     
    } else {
      occurrence++;
      if (p[j] != p[j+1]){
	RLE.push_back(0);
	RLE.push_back(occurrence);
	occurrence = 0;
      }
    }
  }

  //drop all zeros at the end
  int ind = RLE.size()-1;
  while (ind >= 0)
    if (RLE[ind--]==0) 
      RLE.pop_back(); 
    else ind=-1;
  //add EOB
  RLE.push_back(0); RLE.push_back(0);
  return RLE;
}//encodeRLE



//*********EG

int getEntropyCycles(Mat block, string entropyEncoder){
  int cycles=0;

  if (entropyEncoder=="EG") {
    for (int i = 0; i < block.cols; i++){
      int value = block.at<short>(i);
      if (value >= 0) value = 2 * value;
      else  value = -2 * value - 1;
      if (value == 0) cycles += 1;
      else      cycles += getGroupID(value)*3+7;
    }
  }
  return cycles;
}


string  expGolomEncodeValue(short value){
  string bin;
  short groupID, index;
  
  if (value >= 0) value = 2 * value;
  else  value = -2 * value - 1;
  
  if (value == 0)  bin = '0';
  else {
    groupID = getGroupID(value);
    index = value - ((1 << (groupID)) - 1);
    bin = dec2bin(pow(2,groupID)-1, groupID) +'0'+ dec2bin(index,groupID);
  }
  return bin;
}

Mat_<short> truncateLinearBlock(Mat_<short> block){
  
  Mat tempBlock = block.clone();
  
  int i = tempBlock.cols;
  while (i>=0){
    
    if (tempBlock.at<short>(0,i) != 0) 
      break;
    else i--;
  }
  
  if (i==tempBlock.cols)
    return tempBlock(Range::all(), Range::all());
  
  
  return  tempBlock(Range::all(), Range(0, i+1) );
}



short getGroupID(short ne){// floor(log2(n+1))
    double groupID = 2;
    while (ne > ((1 << ((int) groupID)) - 2))
      groupID++;
    return groupID - 1;
}

string dec2bin(short dec, int bits){
  // returns bits-string that represents dec
    string bin;
    int r;
    
    bin.assign(bits,'0');
    
    for (int i = bits - 1;i >= 0;i--){
        r = floor(fabs(dec) / 2.0);
	bin.replace(i,1,to_string(((int) fabs(dec)) - (2 * r)));
	dec = r;
    }
    return bin;
}

short getAmplitudeSize(short amplitude){
  //gives the number of required number of bits to repredent amplitude
  if (amplitude==0) return 0;
  
  if (amplitude < 0)
    amplitude = -amplitude;
  
  return floor( log10(amplitude)/log10(2) + 1 );
}

short biasEncoder(short amplitude) {
      
    if (amplitude < 0){
      short  size = getAmplitudeSize(amplitude);
      return (amplitude + (pow(2,size) - 1));
    }else  return (amplitude);
    
}	


