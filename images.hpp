#ifndef _TEXTURE_IMAGE
#define _TEXTURE_IMAGE

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "fileio.h"

namespace imgs{
  namespace bmp{
    //common header
    typedef struct header{
      uint16_t type;
      uint32_t size;
      uint16_t reserved1;
      uint16_t reserved2;
      uint32_t ofs;
    } __attribute__((__packed__)) header;

    // read headder size first.

    // if 12 byte
    typedef struct os2coreheader{
      int16_t width;
      int16_t height;
      uint16_t planes;
      uint16_t bitperpixel;
    } __attribute__((__packed__)) os2coreheader;

    // 40 byte
    typedef struct wincoreheader{
      int32_t width;
      int32_t height;
      uint16_t planes;
      uint16_t bitperpixel;
      uint32_t compression;
      uint32_t imgsize;
      uint32_t xppm;//pixel per meter for x
      uint32_t yppm;//pixel per meter for y, ex, 96dpi 3780
      uint32_t clrused;//Color lookup table used count
      uint32_t clrimportant;//important color palatte index
    } __attribute__((__packed__)) wincoreheader;
  }

  // image loader
  class image{
  public:
    uint16_t width;
    uint16_t width_powerof2; //width=height, width is power of 2 and 4*n
    uint16_t *data; //RGB565 format, no transparent for now.

    void newimage(const char *path){
      bmp::header h;
      fil f;
      int coreheadersize;
      
      if(filopen(path,&f)!=0){
	printf("file open err(%s)\n",path);
	fail();
      }
      filread(f,&h,sizeof(bmp::header));
      filread(f,&coreheadersize,4);
      if(coreheadersize==12){
	// bmp os/2 format
	bmp::os2coreheader h;
	filread(f,&h,sizeof(bmp::os2coreheader));
	if(h.width!=h.height){
	  printf("%dx%d, not equal\n",h.width,h.height);
	  fail();
	}
	if(h.bitperpixel!=16){
	  printf("not RGB565,(%dbps)\n",h.bitperpixel);
	  fail();
	}
	width = h.width;
      }else if(coreheadersize==40){
	bmp::wincoreheader h;
	filread(f,&h,sizeof(bmp::wincoreheader));
	if(h.width!=h.height){
	  printf("%dx%d, not equal\n",h.width,h.height);
	  fail();
	}
	if(h.bitperpixel!=16){
	  printf("not RGB565,(%dbps)\n",h.bitperpixel);
	  fail();
	}
	width = h.width;
      }else{
	printf("file format error(%d)\n",coreheadersize);
	fail();
      }
      if((width & (width-1))!=0){
	printf("width=%d is not power of 2\n",width);
	fail();
      }
      width_powerof2=-1;
      for(int i=width;i!=0;i>>=1,width_powerof2++);
      data = (uint16_t*)malloc(width*width*2);
      if(filread(f,data,width*width*2)!=width*width*2){
	printf("file load err\n");
      }
      
      static uint16_t tex_body[65536] =
#include "texture-body"
	;
      data=tex_body;

    }
  };
  
  class images{
  public:
    char **texturenamelist;//texture name buffer;
    image *imagelist;
    int n;

    void add(const char *path,int idx){
      imagelist[idx].newimage(path);
      texturenamelist[idx]=(char*)malloc(strlen(path)+1);
      strcpy(texturenamelist[idx],path);
    }
    images(int max){
      imagelist = (image*)malloc(sizeof(image)*max);
      texturenamelist = (char**)malloc(sizeof(char*)*max);
    }
    
    image get_or_add(const char *path){
      if(n==0){
	add(path,0);
      }else{
	for(int i=0;i<n;i++){
	  if(strcmp(path,texturenamelist[i])==0)
	    return imagelist[i];
	}
	add(path,n);
      }
      
      return imagelist[n++];
    }
  };
}


#endif
