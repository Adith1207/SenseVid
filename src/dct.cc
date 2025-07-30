#include "sensevid.h"
#include <iostream>

using namespace std;

Mat DCT(Mat block, string DCT, int zoneSize){
  Mat dctBlock;

  if (DCT=="CLA")
    dct(block, dctBlock);
  
  if (DCT=="tLLM")
    dctBlock = tllm2DCT(block, zoneSize);
  
  if (DCT=="tBIN")
    dctBlock = tbin2DCT(block, zoneSize);

  if (DCT=="sLLM")
    dctBlock = sllm2DCT(block, zoneSize);
  
   if (DCT=="sBIN")
     dctBlock = sbin2DCT(block, zoneSize);
  
  return dctBlock;
}

Mat iDCT(Mat  dctBlock, string DCT, int zoneSize){

  Mat  block;
 
   if (DCT=="CLA")
      idct(dctBlock, block); 
      
    if (DCT.substr(1)=="LLM")
      block = llm2iDCT(dctBlock);
      
    if (DCT.substr(1)=="BIN")
      block = bin2iDCT(dctBlock);

    return block;
}

/******** Triangular BIN and LLM DCTs ************/

Mat_<double> tllm2DCT(Mat_<double> block, uchar zoneSize){

  Mat_<double> llmBlock = llm2DCT(block);
   
  Mat_<double> triangBlock =  Mat::zeros(8,8,CV_32F);
  for (int i = 0; i < zoneSize; i++)
    for (int j = 0; j < zoneSize - i; j++)
      triangBlock.at<double>(i,j) = llmBlock.at<double>(i,j);

  return triangBlock;
}

Mat_<short> tbin2DCT(Mat_<short> block, uchar zoneSize){

  Mat_<short> llmBlock = bin2DCT(block);
  
  Mat_<short> triangBlock =  Mat::zeros(8,8,CV_32F);
  for (int i = 0; i < zoneSize; i++)
    for (int j = 0; j < zoneSize - i; j++)
      triangBlock.at<short>(i,j) = llmBlock.at<short>(i,j);

  return triangBlock;
}

/******** Square BIN and LLM DCTs ************/

Mat_<double> sllm2DCT(Mat_<double> block, uchar zoneSize){
  
  Mat_<double> llmBlock = llm2DCT(block);

  Mat_<double> sqBlock =  Mat::zeros(8,8,CV_32F);
  for (int i = 0; i < zoneSize; i++)
    for (int j = 0; j < zoneSize; j++)
      sqBlock.at<double>(i,j) = llmBlock.at<double>(i,j);
  
  return sqBlock;
}

Mat_<short> sbin2DCT(Mat_<short> block, uchar zoneSize){
 
  Mat_<short> binBlock = bin2DCT(block);
  
  Mat_<short> sqBlock =  Mat::zeros(8,8,CV_32F);
  for (int i = 0; i < zoneSize; i++)
    for (int j = 0; j < zoneSize; j++)
      sqBlock.at<short>(i,j) = binBlock.at<short>(i,j);
  
  return sqBlock;
}




//******** 2DCT ******

Mat llm2DCT(Mat block){

  Mat_<double>  tempRow = Mat::zeros(1,8,CV_32F);
  Mat_<double> tempBlock = Mat::zeros(8,8,CV_32F);
  Mat_<double>  temp2Block = Mat::zeros(8,8,CV_32F);
  
  for (int j = 0; j < block.cols; j++){
    tempRow = llm1DCT( block.row(j) );
    tempRow.copyTo(tempBlock.row(j));
  }
 
  transpose(tempBlock,tempBlock);

  for (int j = 0; j < tempBlock.rows; j++){
    tempRow = llm1DCT( tempBlock.row(j) );
    tempRow.copyTo(temp2Block.row(j));
  }
  transpose(temp2Block,temp2Block);
  return temp2Block;
}

Mat_<short> bin2DCT(Mat_<short> block){
 
  Mat_<short> tempBlock = Mat::zeros(8,8,CV_16S);
  Mat_<short>  temp2Block = Mat::zeros(8,8,CV_16S);
  Mat_<short>  tempRow = Mat::zeros(1,8,CV_16S);
  
  for (int j = 0; j < block.cols; j++){
    tempRow = bin1DCT( block.row(j) );
    tempRow.copyTo(tempBlock.row(j));
  }
  
  transpose(tempBlock,tempBlock);

  for (int j = 0; j < tempBlock.rows; j++){
    tempRow = bin1DCT( tempBlock.row(j) );
    tempRow.copyTo(temp2Block.row(j));
  }
  transpose(temp2Block,temp2Block);
 
  return temp2Block;
}


Mat_<double> llm2iDCT(Mat_<double> block){
 
  Mat_<double> tempBlock = Mat::zeros(8,8,CV_32F);
  Mat_<double>  temp2Block =Mat::zeros(8,8,CV_32F);
  Mat_<double>  tempRow =Mat::zeros(1,8,CV_32F);
  
  for (int j = 0; j < block.cols; j++){
    tempRow = llm1iDCT( block.row(j) );
    tempRow.copyTo(tempBlock.row(j));
  }
 transpose(tempBlock,tempBlock);

  for (int j = 0; j < tempBlock.rows; j++){
    tempRow = llm1iDCT( tempBlock.row(j) );
    tempRow.copyTo(temp2Block.row(j));
  }
  transpose(temp2Block,temp2Block);

  return temp2Block/64;
}


Mat_<short> bin2iDCT(Mat_<short> block){
  
  Mat_<short> tempBlock = Mat::zeros(8,8,CV_16S);
  Mat_<short>  temp2Block =Mat::zeros(8,8,CV_16S);
  Mat_<short>  tempRow =Mat::zeros(1,8,CV_16S);
  
  for (int j = 0; j < block.cols; j++){
    tempRow = bin1iDCT( block.row(j) );
    tempRow.copyTo(tempBlock.row(j));
  }
 
  transpose(tempBlock,tempBlock);

  for (int j = 0; j < tempBlock.rows; j++){
    tempRow = bin1iDCT( tempBlock.row(j) );
    tempRow.copyTo(temp2Block.row(j));
  }
  transpose(temp2Block,temp2Block);
  return temp2Block/16;
}



//******************************************
//***** 1-D BIN and LLM iDCTs and DCTs *****
//******************************************


Mat_<short> bin1DCT(Mat_<short> block){

  Mat_<short> dct1D = Mat::zeros(1,8,CV_16S);
  short tmp,tmp0,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7,tmp8,tmp9,tmp10,
    tmp11,tmp12,tmp13,tmp14,tmp15,tmp16,tmp17,tmp18,tmp19,tmp20,
    tmp21,tmp22,tmp23,tmp24;
  
  tmp0 = block.at<short>(0,0) + block.at<short>(0,7);
  tmp1 = block.at<short>(0,0) - block.at<short>(0,7);
  tmp2 = block.at<short>(0,1) + block.at<short>(0,6);
  tmp3 = block.at<short>(0,1) - block.at<short>(0,6);
  tmp4 = block.at<short>(0,2) + block.at<short>(0,5);
  tmp5 = block.at<short>(0,2) - block.at<short>(0,5);
  tmp6 = block.at<short>(0,3) + block.at<short>(0,4);
  tmp7 = block.at<short>(0,3) - block.at<short>(0,4);

  tmp = ( tmp5 << 2) - tmp5; tmp8 = tmp3 + ( tmp >> 3);
  tmp = ( tmp8 << 2) + tmp8; tmp9 = ( tmp >> 3) - tmp5;

  tmp10 = tmp0 + tmp6;
  tmp11 = tmp0 - tmp6;
  tmp12 = tmp7 + tmp9;
  tmp13 = tmp7 - tmp9;
  tmp14 = tmp1 - tmp8;
  tmp15 = tmp1 + tmp8;
  tmp16 = tmp2 + tmp4;
  tmp17 = tmp2 - tmp4;
  
  tmp = ( tmp15 >> 3); tmp18 = tmp12 - tmp; tmp19 = tmp10 + tmp16;
  tmp = ( tmp19 >> 1);  tmp20 = tmp - tmp16;
  tmp = ( tmp11 << 2) - tmp11; tmp21 = tmp17 - ( tmp >> 3);
  tmp = ( tmp21 << 2) - tmp21; tmp22 = tmp11 + ( tmp >> 3);
  tmp = ( tmp14 << 3) - tmp14; tmp23 = tmp13 + ( tmp >> 3);
  tmp = ( tmp23 >> 1);  tmp24 = tmp14 - tmp;
  
  dct1D.at<short>(0,0) = tmp0 + tmp6 + tmp16;
  dct1D.at<short>(0,1) = tmp1 + tmp8;
  dct1D.at<short>(0,2) = tmp22;
  dct1D.at<short>(0,3) = tmp24;
  dct1D.at<short>(0,4) = tmp20;
  dct1D.at<short>(0,5) = tmp23;
  dct1D.at<short>(0,6) = tmp21;
  dct1D.at<short>(0,7) = tmp18;

  return (dct1D);
}

Mat_<short> bin1iDCT(Mat_<short> dct1D){
  Mat_<short> block = Mat::zeros(1,8,CV_16S);//8->dct1D.cols
  short tmp,tmp0,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7,tmp8,tmp9,tmp10,
    tmp11,tmp12,tmp13,tmp14,tmp15,tmp16,tmp17,tmp18,tmp19,tmp20,
    tmp21,tmp22,tmp23,tmp24;

  tmp0 = (dct1D.at<short>(0,0) >> 1) - dct1D.at<short>(0,4);
  tmp1 = dct1D.at<short>(0,0) - tmp0;
  tmp = (dct1D.at<short>(0,6) << 2) - dct1D.at<short>(0,6);
  tmp2 = dct1D.at<short>(0,2) - ( tmp >> 3);
  tmp = ( tmp2 << 2) - tmp2; tmp3 = dct1D.at<short>(0,6) + ( tmp >> 3);
  tmp = dct1D.at<short>(0,5) >> 1; tmp4 = dct1D.at<short>(0,3) + tmp;
  tmp = ( tmp4 << 3) - tmp4; tmp5 = dct1D.at<short>(0,5) - ( tmp >> 3);
  tmp = dct1D.at<short>(0,1) >> 3; tmp6 = dct1D.at<short>(0,7) + tmp;
  tmp7 = tmp0 + tmp3;
  tmp8 = tmp0 - tmp3;
  tmp9 = tmp1 + tmp2;
  tmp10 = tmp1 - tmp2;
  tmp11 = tmp5 + tmp6;
  tmp12 = tmp6 - tmp5;
  tmp13 = dct1D.at<short>(0,1) - tmp4;
  tmp14 = dct1D.at<short>(0,1) + tmp4;
  tmp = ( tmp13 << 2) + tmp13; tmp15 = ( tmp >> 3) - tmp12;
  tmp = ( tmp15 << 2) - tmp15; tmp16 = tmp13 - ( tmp >> 3);
  tmp17 = tmp10 + tmp11;
  tmp18 = tmp10 - tmp11;
  tmp19 = tmp8 + tmp15;
  tmp20 = tmp8 - tmp15;
  tmp21 = tmp7 + tmp16;
  tmp22 = tmp7 - tmp16;
  tmp23 = tmp9 + tmp14;
  tmp24 = tmp9 - tmp14;
  
  block.at<short>(0,0) = tmp23;
  block.at<short>(0,1) = tmp21;
  block.at<short>(0,2) = tmp19;
  block.at<short>(0,3) = tmp17;
  block.at<short>(0,4) = tmp18;
  block.at<short>(0,5) = tmp20;
  block.at<short>(0,6) = tmp22;
  block.at<short>(0,7) = tmp24;
  
  return (block);
}//bin1iDCT


Mat llm1DCT(Mat  block){
  
  Mat_<double> dct1D = Mat::zeros(1,8,CV_32F);
  double t0,t1,t2,t3,t4,t5,t6,t7; double c0,c1,c2,c3; double r[8];
  r[0]=1.414214f;   r[1]=1.387040f;   r[2]=1.306563f; r[3]=1.175876f;
  r[4]=1.000000f;   r[5]=0.785695f;   r[6]=0.541196f;  r[7]=0.275899f;
  const double invsqrt2= 0.707107f;//(float)(1.0f / M_SQRT2);
  
  c1 = block.at<double>(0,0); c2 = block.at<double>(0,7);
  t0 = c1 + c2; t7 = c1 - c2;
  c1 = block.at<double>(0,1); c2 = block.at<double>(0,6);
  t1 = c1 + c2; t6 = c1 - c2;
  c1 = block.at<double>(0,2); c2 = block.at<double>(0,5);
  t2 = c1 + c2; t5 = c1 - c2;
  c1 = block.at<double>(0,3); c2 = block.at<double>(0,4);
  t3 = c1 + c2; t4 = c1 - c2;
  
  c0 = t0 + t3; c3 = t0 - t3;
  c1 = t1 + t2; c2 = t1 - t2;
  
  dct1D.at<double>(0,0) = c0 + c1;
  dct1D.at<double>(0,4) = c0 - c1;
  dct1D.at<double>(0,2) = c2 * r[6] + c3 * r[2];
  dct1D.at<double>(0,6) = c3 * r[6] - c2 * r[2];
  
  c3 = t4 * r[3] + t7 * r[5];
  c0 = t7 * r[3] - t4 * r[5];
  c2 = t5 * r[1] + t6 * r[7];
  c1 = t6 * r[1] - t5 * r[7];
  
  dct1D.at<double>(0,5) = c3 - c1; dct1D.at<double>(0,3) = c0 - c2;
  c0 = (c0 + c2) * invsqrt2;
  c3 = (c3 + c1) * invsqrt2;
  dct1D.at<double>(0,1) = c0 + c3; dct1D.at<double>(0,7) = c0 - c3;

    return (dct1D);
}

Mat llm1iDCT(Mat dct1D){
  Mat_<double> block = Mat::zeros(1,8,CV_32F);

  double a0,a1,a2,a3,b0,b1,b2,b3; double z0,z1,z2,z3,z4;  double r[8];
  r[0]=1.414214f;   r[1]=1.387040f;   r[2]=1.306563f; r[3]=1.175876f;
  r[4]=1.000000f;   r[5]=0.785695f;   r[6]=0.541196f;  r[7]=0.275899f;

  z0 = dct1D.at<double>(0,1) + dct1D.at<double>(0,7);
  z1 = dct1D.at<double>(0,3) + dct1D.at<double>(0,5);
  z2 = dct1D.at<double>(0,3) + dct1D.at<double>(0,7);
  z3 = dct1D.at<double>(0,1) + dct1D.at<double>(0,5);
  z4 = (z0 + z1) * r[3];

  z0 = z0 * (-r[3] + r[7]);
  z1 = z1 * (-r[3] - r[1]);
  z2 = z2 * (-r[3] - r[5]) + z4;
  z3 = z3 * (-r[3] + r[5]) + z4;

  b3 = dct1D.at<double>(0,7) * (-r[1] + r[3] + r[5] - r[7]) + z0 + z2;
  b2 = dct1D.at<double>(0,5) * ( r[1] + r[3] - r[5] + r[7]) + z1 + z3;
  b1 = dct1D.at<double>(0,3) * ( r[1] + r[3] + r[5] - r[7]) + z1 + z2;
  b0 = dct1D.at<double>(0,1) * ( r[1] + r[3] - r[5] - r[7]) + z0 + z3;

  z4 = (dct1D.at<double>(0,2) + dct1D.at<double>(0,6)) * r[6];
  z0 = dct1D.at<double>(0,0) + dct1D.at<double>(0,4);
  z1 = dct1D.at<double>(0,0) - dct1D.at<double>(0,4);
  z2 = z4 - dct1D.at<double>(0,6) * (r[2] + r[6]);
  z3 = z4 + dct1D.at<double>(0,2) * (r[2] - r[6]);
  a0 = z0 + z3; a3 = z0 - z3;
  a1 = z1 + z2; a2 = z1 - z2;
  
  block.at<double>(0,0) = a0 + b0;
  block.at<double>(0,7) = a0 - b0;
  block.at<double>(0,1) = a1 + b1;
  block.at<double>(0,6) = a1 - b1;
  block.at<double>(0,2) = a2 + b2;
  block.at<double>(0,5) = a2 - b2;
  block.at<double>(0,3) = a3 + b3;
  block.at<double>(0,4) = a3 - b3;

  return (block);

}

