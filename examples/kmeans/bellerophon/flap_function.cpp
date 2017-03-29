#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../src/segmentation.h"
#include "csv.h"
//paths to the input image and output of approximate version

// BELLEROPHON FUNCTION

char golden_image_path[] = "/home/ntonjeta/Approximate/examples/kmeans/images/original.rgb";
char noaprx_image_path[] = "/home/ntonjeta/Approximate/examples/kmeans/images/noaprx.rgb";

char report_path[] = "/home/ntonjeta/Approximate/examples/kmeans/chimera/output/mutants/segmentation.cpp/1/flap_float_report.csv";

double eDistance(RgbPixel p, RgbPixel q) {
  float r;

  r = 0;
  double r_tmp;
  
  r += (p.r - q.r) * (p.r - q.r);
  r += (p.g - q.g) * (p.g - q.g);
  r += (p.b - q.b) * (p.b - q.b);

  r_tmp = sqrt(r);

  return r_tmp;
}


extern "C" double BELLERO_getError() {
   double error = 0;
  RgbImage noaprx;
  RgbImage srcImage;
  Clusters clusters;
  // Read no approximated image
  initRgbImage(&noaprx);
  loadRgbImage(noaprx_image_path, &noaprx,256);
  // Read Golden image 
  initRgbImage(&srcImage);
  loadRgbImage(golden_image_path, &srcImage,256);
  // Apply kmeans with aprx code
  initClusters(&clusters, 6, 1);
  segmentImage(&srcImage, &clusters, 1);
  
  // Calculate MSE 
  int i,j;
  double d = 0;

  int w = srcImage.w;
  int h = srcImage.h;

  for(i = 0; i<w; i++) {
    for( j = 0; j<h; j++){
      d += eDistance(srcImage.pixels[i][j],noaprx.pixels[i][j]);
    }
  }
 
  return d/(h*w);
}


extern "C" double BELLERO_Reward()
{
  double rew = 0;
  
//  MantType mant;
  int grade[6];
  
  grade[0] = 23 - OP_0.mant_size;
  grade[1] = 23 - OP_1.mant_size;
  grade[2] = 23 - OP_2.mant_size;
  grade[3] = 23 - OP_3.mant_size;
  grade[4] = 23 - OP_4.mant_size;
  grade[5] = 23 - OP_5.mant_size;
 
  rew = [(2*grade[0]) + (2*grade[1]) + (2*grade[1])] + [ (2* grade[3] * grade[3] ) + (2* grade[4] * grade[4] ) + (2* grade[5] * grade[5])];
//  FloatPrecTy prec = OP_1.getPrec();  
 
//  cout << prec.mant_size; 

  return rew;
}
