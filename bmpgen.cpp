/*
 * bmpgen.cpp
 * Main function of bmpgen
 *
 * bmpgen messes around with bmp files, particularly in the topic of
 * proceedurally generated images.

 * Josh Chandler
*/

#ifndef BMPGEN_CPP
#define BMPGEN_CPP

#include <iostream>
#include <string>
#include <cmath>
#include <chrono>

#include "EasyBMP_1.06/EasyBMP.h"

using namespace std::chrono;
using namespace std;

enum dir { N = 0, NE = 1, E = 2, SE = 3,
            S = 4, SW = 5, W = 6, NW = 7};

int main(int argc, char* argv[])
{
   // error checking
   if (argc != 4)
   {
      cout << "usage: <source>.bmp <output>.bmp <output size>\n";
      exit(1);
   }

   // set random seed
   srand(system_clock::now().time_since_epoch() / milliseconds(1));

   int i,j,k;
   int outsize = atoi(argv[3]);

   RGBApixel blackpxl;
   blackpxl.Red = 0; blackpxl.Green = 0; blackpxl.Blue = 0;

   RGBApixel centerpxl;
   RGBApixel lookpxl;

   BMP inimg;
   inimg.ReadFromFile(argv[1]);

   int bprob[8] = { }; // probabiltiy that a pixel near a center black pixel will be black.
   int wprob[8] = { }; // probabiltiy that a pixel near a center white pixel will be black.
   int imap[8] = { 0,  1,  1,  1,  0, -1, -1, -1}; // i and j offset maps for checking surrounding pixels
   int jmap[8] = {-1, -1,  0,  1,  1,  1,  0, -1}; // order is N, NE, E, SE, S, SW, W, NW

   int totalblack = 0;
   int totalwhite = 0;

   // read source image and store probabiltiy data in table
   // read only the inner pixels for simplicity
   for (i=1; i<inimg.TellWidth()-1; i++)
   {
      for (j=1; j<inimg.TellHeight()-1; j++)
      {
         // get the center pixel
         centerpxl = inimg.GetPixel(i,j);

         if (centerpxl.Red < 64) // center pixel is "black"
         {
            totalblack++;
         }
         else
         {
            totalwhite++;
         }

         for (k=0; k<8; k++)
         {
            // get the pixel to look at, starting with north
            lookpxl = inimg.GetPixel(i+imap[k],j+jmap[k]);
            // analyze
            if (lookpxl.Red < 64) // look pixel is "black";
            {
               if (centerpxl.Red < 64) // center pixel is "black"
               {
                  bprob[k]++;
               }
               else
               {
                  wprob[k]++;
               }
            }
         }
      }
   }

   // convert gathered data into a number between 0 and 100
   for (i=0; i<8; i++)
   {
      if (totalblack != 0)
      {
         bprob[i] = bprob[i]*10000/totalblack;
      }
      else
      {
         bprob[i] = 0;
      }

      if (totalwhite != 0)
      {
         wprob[i] = wprob[i]*10000/totalwhite;
      }
      else
      {
         wprob[i] = 0;
      }
   }

   // debug
   // display numbers
   for (i=0; i<8; i++)
   {
      cout << bprob[i] << " " << wprob[i] << endl;
   }

   // generate output image(s)
   BMP outimg;
   outimg.SetSize(outsize,outsize);
   outimg.SetBitDepth(1);

   // start with random noise
   for (i=0; i<outimg.TellWidth() * outimg.TellHeight(); i++)
   {
      if (rand()%100 < 2) outimg.SetPixel(i%outimg.TellWidth(), i/outimg.TellWidth(), blackpxl);
   }

   int x;
   int y;
   bool *pxlvisited = new bool[outimg.TellWidth() * outimg.TellHeight()];
   for (i=0; i<outimg.TellWidth() * outimg.TellHeight(); i++) pxlvisited[i] = false;
   int *table;

   int *pxlstack = new int[outimg.TellWidth() * outimg.TellHeight()]; // could probably be smaller
   int stacktop = 0;

   int dir = 0;

   // pick random starting point
   x = rand()%outimg.TellWidth();
   y = rand()%outimg.TellHeight();

   cout << "xy: " << x << " " << y << endl;

   // add starting point to stack
   pxlstack[stacktop] = y*outimg.TellWidth()+x;

   // mark starting point as visited
   pxlvisited[stacktop] = true;

   // while there are pixels on the stack to expand...
   while (stacktop > -1 && stacktop < outimg.TellWidth() * outimg.TellHeight())
   {
      // get coords of pixel on stack top
      x = pxlstack[stacktop]%outimg.TellWidth();
      y = pxlstack[stacktop]/outimg.TellWidth();
      stacktop--;

      // check what color the center pixel is
      if (outimg.GetPixel(x,y).Red < 64) // pixel is "black"
      {
         table = &bprob[0];
      }
      else
      {
         table = &wprob[0];
      }

      dir = rand()%8;

      // expand pixel
      for (i=0; i<8; i++)
      {
         // check if the pixel is actually on the image and hasn't been visited
         if (x+imap[(i+dir)%8] < outimg.TellWidth()  &&
             x+imap[(i+dir)%8] > -1                  &&
             y+jmap[(i+dir)%8] < outimg.TellHeight() &&
             y+jmap[(i+dir)%8] > -1                    )
         {
            // expand if not visited
            if (rand()%10000 < table[(i+dir)%8]) // new pixel will be black
            {
               outimg.SetPixel(x+imap[(i+dir)%8], y+jmap[(i+dir)%8], blackpxl);
            }

            // add pixel to stack if it's not there already
            if (pxlvisited[(y+jmap[(i+dir)%8])*outimg.TellWidth()+x+imap[(i+dir)%8]] == false)
            {
               pxlstack[++stacktop] = (y+jmap[(i+dir)%8])*outimg.TellWidth()+x+imap[(i+dir)%8];
               // add pixel to visited array
               pxlvisited[(y+jmap[(i+dir)%8])*outimg.TellWidth()+x+imap[(i+dir)%8]] = true;
            }
         }
      }
   }

   outimg.WriteToFile(argv[2]);

   return 0;
}

#endif
