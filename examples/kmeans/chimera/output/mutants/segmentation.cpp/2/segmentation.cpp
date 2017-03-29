/*
 * segmentation.c
 * 
 * Created on: Sep 9, 2013
 * 			Author: Amir Yazdanbakhsh <a.yazdanbakhsh@gatech.edu>
 */


#include "segmentation.h"
//#include <stdio.h>
//#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "fap.h"

using namespace std;

int count = 0;
#define MAX_COUNT 1200000

void initRgbImage(RgbImage* image) {
  image->w = 0;
  image->h = 0;
  image->pixels = NULL;
  image->meta = NULL;
}

int readCell(FILE *fp, char* w) {
  int c;
  int i;

  w[0] = 0;
  for (c = fgetc(fp), i = 0; c != EOF; c = fgetc(fp)) {
    if (c == ' ' || c == '\t') {
      if (w[0] != '\"')
              continue;
    }

    if (c == ',' || c == '\n') {
      if (w[0] != '\"')
        break;
      else if (c == '\"') {
        w[i] = c;
        i++;
        break;
      }
    }

    w[i] = c;
    i++;
  }
  w[i] = 0;

  return c;
}

int loadRgbImage(const char* fileName, RgbImage* image, float scale) {
  int c;
  int i;
  int j;
  char w[256];
  RgbPixel** pixels;
  FILE *fp;

  //printf("Loading %s ...\n", fileName);

  fp = fopen(fileName, "r");
  if (!fp) {
    printf("Warning: Oops! Cannot open %s!\n", fileName);
    return 0;
  }

  c = readCell(fp, w);
  image->w = atoi(w);
  c = readCell(fp, w);
  image->h = atoi(w);

  //printf("%d x %d\n", image->w, image->h);

  pixels = (RgbPixel**)malloc(image->h * sizeof(RgbPixel*));

  if (pixels == NULL) {
    printf("Warning: Oops! Cannot allocate memory for the pixels!\n");

    fclose(fp);

    return 0;
  }

  c = 0;
  for(i = 0; i < image->h; i++) {
    pixels[i] = (RgbPixel*)malloc(image->w * sizeof(RgbPixel));
    if (pixels[i] == NULL) {
      c = 1;
      break;
    }
  }

  if (c == 1) {
    printf("Warning: Oops! Cannot allocate memory for the pixels!\n");

    for (i--; i >= 0; i--)
      free(pixels[i]);
    free(pixels);

    fclose(fp);

    return 0;
  }

  for(i = 0; i < image->h; i++) {
    for(j = 0; j < image->w; j++) {
      c = readCell(fp, w);
      pixels[i][j].r = atoi(w) / scale;

      c = readCell(fp, w);
      pixels[i][j].g = atoi(w) / scale;

      c = readCell(fp, w);
      pixels[i][j].b = atoi(w) / scale;

      pixels[i][j].cluster = 0;
      pixels[i][j].distance = 0.;
    }
  }
  image->pixels = pixels;

  c = readCell(fp, w);
  image->meta = (char*)malloc(strlen(w) * sizeof(char));
  if(image->meta == NULL) {
    printf("Warning: Oops! Cannot allocate memory for the pixels!\n");

    for (i = 0; i < image->h; i++)
            free(pixels[i]);
    free(pixels);

    fclose(fp);

    return 0;

  }
  strcpy(image->meta, w);

  //printf("%s\n", image->meta);

  //printf("w=%d x h=%d\n", image->w, image->h);

  return 1;
}

int saveRgbImage(RgbImage* image, const char* fileName, float scale) {
  int i;
  int j;
  FILE *fp;

  //printf("Saving %s ...\n", fileName);

  fp = fopen(fileName, "w");
  if (!fp) {
    printf("Warning: Oops! Cannot open %s!\n", fileName);
    return 0;
  }

  fprintf(fp, "%d,%d\n", image->w, image->h);
  //printf("%d,%d\n", image->w, image->h);

  for(i = 0; i < image->h; i++) {
    for(j = 0; j < (image->w - 1); j++) {
      fprintf(fp, "%d,%d,%d,", image->pixels[i][j].r * scale , image->pixels[i][j].g * scale, image->pixels[i][j].b * scale);
    }
    fprintf(fp, "%d,%d,%d\n", image->pixels[i][j].r * scale, image->pixels[i][j].g * scale, image->pixels[i][j].b * scale);
  }

  fprintf(fp, "%s", image->meta);
  //printf("%s\n", image->meta);

  fclose(fp);

  return 1;
}

void freeRgbImage(RgbImage* image) {
  int i;

  if (image->meta != NULL)
          free(image->meta);

  if (image->pixels == NULL)
          return;

  for (i = 0; i < image->h; i++)
          if (image->pixels != NULL)
                  free(image->pixels[i]);

  free(image->pixels);
}

void makeGrayscale(RgbImage* image) {
  int i;
  int j;
  float luminance;

  float rC = 0.30;
  float gC = 0.59;
  float bC = 0.11;

  for(i = 0; i < image->h; i++) {
    for(j = 0; j < image->w; j++) {
      luminance =
              rC * image->pixels[i][j].r +
              gC * image->pixels[i][j].g +
              bC * image->pixels[i][j].b;

      image->pixels[i][j].r = (unsigned char)luminance;
      image->pixels[i][j].g = (unsigned char)luminance;
      image->pixels[i][j].b = (unsigned char)luminance;
    }
  }
}

float euclideanDistance(RgbPixel* p, Centroid* c1) {
  float r;

  r = 0;
  double r_tmp;
  
  double dataIn[6];
  dataIn[0] = p->r;
  dataIn[1] = p->g;
  dataIn[2] = p->b;
  dataIn[3] = c1->r;
  dataIn[4] = c1->g;
  dataIn[5] = c1->b;


  r += (p->r - c1->r) * (p->r - c1->r);
  r += (p->g - c1->g) * (p->g - c1->g);
  r += (p->b - c1->b) * (p->b - c1->b);

  r_tmp = sqrt(r);


  return r_tmp;
}

int pickCluster(RgbPixel* p, Centroid* c1) {
  float d1;

  d1 = euclideanDistance(p, c1);

  if (p->distance <= d1)
    return 0;

  return 1;
}

void assignCluster(RgbPixel* p, Clusters* clusters) {
  int i = 0;

  float d;
  d = euclideanDistance(p, &clusters->centroids[i]);
  p->distance = d;

  for(i = 1; i < clusters->k; ++i) {
    d = euclideanDistance(p, &clusters->centroids[i]);
    if (d < p->distance) {
      p->cluster = i;
      p->distance = d;
    }
  }
}

// TUTTO QUELLO SOPRA E' AGGIUNTO
int initClusters(Clusters* clusters, int k, float scale) {
  int i;
  float x;

  clusters->centroids = (Centroid*)malloc(k * sizeof(Centroid));

  if (clusters == NULL) {
    printf("Warning: Oops! Cannot allocate memory for the clusters!\n");

    return 0;
  }

  clusters->k = k;
  for (i = 0; i < clusters->k; i ++) {
    x = (((float)rand())/RAND_MAX) * scale;
    clusters->centroids[i].r = x;

    x = (((float)rand())/RAND_MAX) * scale;
    clusters->centroids[i].g = x;

    x = (((float)rand())/RAND_MAX) * scale;
    clusters->centroids[i].b = x;

    clusters->centroids[i].n = 0;
  }

  return 1;
}

void freeClusters(Clusters* clusters) {
  if (clusters != NULL)
    free(clusters->centroids);
}

int stride9 = 1;
int stride8 = 1;
int stride7 = 1;
int stride6 = 1;
int stride5 = 1;
int stride4 = 1;
int stride3 = 1;
int stride2 = 1;
int stride1 = 1;
void segmentImage(RgbImage* image, Clusters* clusters, int n) {
  int i;
  int x, y;
  int c;

  for (i = 0; i < n; i = i + stride1) {
    for (y = 0; y < image->h; y = y + stride2) {
      for (x = 0; x < image->w; x = x + stride3) {
              assignCluster(&image->pixels[y][x], clusters);
      }
    }

    /** Recenter */
    for (c  = 0; c < clusters->k; c = c + stride4) {
      clusters->centroids[c].r = 0.;
      clusters->centroids[c].g = 0.;
      clusters->centroids[c].b = 0.;
      clusters->centroids[c].n = 0;
    }
    for (y = 0; y < image->h; y = y + stride5) {
      for (x = 0; x < image->w; x = x + stride6) {
        clusters->centroids[image->pixels[y][x].cluster].r += image->pixels[y][x].r;
        clusters->centroids[image->pixels[y][x].cluster].g += image->pixels[y][x].g;
        clusters->centroids[image->pixels[y][x].cluster].b += image->pixels[y][x].b;
        clusters->centroids[image->pixels[y][x].cluster].n += 1;
      }
    }
    for (c  = 0; c < clusters->k; c = c + stride7) {
      if (clusters->centroids[c].n != 0) {
        clusters->centroids[c].r = clusters->centroids[c].r / clusters->centroids[c].n;
        clusters->centroids[c].g = clusters->centroids[c].g / clusters->centroids[c].n;
        clusters->centroids[c].b = clusters->centroids[c].b / clusters->centroids[c].n;
      }
    }
  }

  for (y = 0; y < image->h; y = y + stride8) {
    for (x = 0; x < image->w; x = x + stride9) {
      image->pixels[y][x].r = clusters->centroids[image->pixels[y][x].cluster].r;
      image->pixels[y][x].g = clusters->centroids[image->pixels[y][x].cluster].g;
      image->pixels[y][x].b = clusters->centroids[image->pixels[y][x].cluster].b;
    }
  }
}



// BELLEROPHON FUNCTION

char golden_image_path[] = "/home/ntonjeta/Approximate/examples/kmeans/images/original.rgb";
char noaprx_image_path[] = "/home/ntonjeta/Approximate/examples/kmeans/images/noaprx.rgb";


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


extern "C"  double BELLERO_getError() {
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

  freeRgbImage(&srcImage);
  freeRgbImage(&noaprx);
  return d/(h*w);
}
