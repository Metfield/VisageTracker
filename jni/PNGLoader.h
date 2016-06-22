/*
 * PNGLoader.h
 *
 *  Created on: Jun 21, 2016
 *      Author: taich
 */

#ifndef PNGLOADER_H_
#define PNGLOADER_H_

#include <png.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <string>

#include <Logging.h>

using namespace std;



struct ImageData
{
	int img_width, img_height;
	uint8_t* pixels;
};

inline bool IsPowerOfTwo(int n)
{
	// http://www.exploringbinary.com/ten-ways-to-check-if-an-integer-is-a-power-of-two-in-c/
	return (n & (n - 1)) == 0;
}

inline int NextPowerOfTwo(int n)
{
	// Explanation : http://stackoverflow.com/questions/1322510/given-an-integer-how-do-i-find-the-next-largest-power-of-two-using-bit-twiddlin
	if (n < 0)
	{
		return 0;
	}

	n--;
	n |= n >> 1;   // Divide by 2^k for consecutive doublings of k up to 32,
	n |= n >> 2;   // and then or the results.
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;           // The result is a number of 1 bits equal to the number
	// of bits in the original number, plus 1. That's the
	// next highest power of 2.
	return n;
}

struct PNGImageData : public ImageData
{
	PNGImageData(png_byte* pixels, int width, int height)
	{
		this->img_width = width;
		this->img_height = height;
		this->pixels = (uint8_t*)pixels;
	}
};

static AAsset* asset;

void png_asset_read(png_structp png_ptr, png_bytep data, png_size_t length)
{
  AAsset_read(asset, data, length);
}

// Adapted from http://en.wikibooks.org/wiki/OpenGL_Programming/Intermediate/Textures
ImageData* FromAssetPNGFile(AAssetManager* mgr, const string& fname) {
  asset = AAssetManager_open(mgr, fname.c_str(), AASSET_MODE_STREAMING);
  if (!asset) {
    //LOG(ERROR) << "Error opening " <<  fname;
	  LOGI("error opening file! %s", fname.c_str());
    return NULL;
  }

  //header for testing if it is a png
  png_byte header[8];

  //read the header
  AAsset_read(asset, header, 8);

  //test if png
  int is_png = !png_sig_cmp(header, 0, 8);
  if (!is_png) {
    AAsset_close(asset);
    LOGI("Not a png file %s", fname.c_str());
    return NULL;
  }

  //create png struct
  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
      NULL, NULL);
  if (!png_ptr) {
    AAsset_close(asset);
    LOGI("Unable to create png struct %s", fname.c_str());
    return NULL;
  }

  //create png info struct
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
    LOGI( "Unable to create png info %s ", fname.c_str());
    AAsset_close(asset);
    return NULL;
  }

  //create png end info struct
  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
    LOGI("Unable to create png end info %s", fname.c_str());
    AAsset_close(asset);
    return NULL;
  }

  //png error stuff, not sure libpng man suggests this.
  if (setjmp(png_jmpbuf(png_ptr))) {
    AAsset_close(asset);
    LOGI("Error during setjmp %s", fname.c_str());
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    return NULL;
  }

  //init png reading
  //png_init_io(png_ptr, fp);
  png_set_read_fn(png_ptr, NULL, png_asset_read);

  //let libpng know you already read the first 8 bytes
  png_set_sig_bytes(png_ptr, 8);

  // read all the info up to the image data
  png_read_info(png_ptr, info_ptr);

  //variables to pass to get info
  int bit_depth, color_type;
  png_uint_32 width, height;

  // get info about png
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
      NULL, NULL, NULL);

  if (!IsPowerOfTwo(width) || !IsPowerOfTwo(height)) {
      LOGI("Non-power of two! %s", fname.c_str());
      png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
      AAsset_close(asset);
      return NULL;
  }

  // Update the png info struct.
  png_read_update_info(png_ptr, info_ptr);

  // Row size in bytes.
  int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  // Allocate the image_data as a big block
  png_byte *image_data = new png_byte[rowbytes * height];
  if (!image_data) {
    //clean up memory and close stuff
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    LOGI("Unable to allocate image_data while loading %s", fname.c_str());
    AAsset_close(asset);
    return NULL;
  }

  //row_pointers is for pointing to image_data for reading the png with libpng
  png_bytep *row_pointers = new png_bytep[height];
  if (!row_pointers) {
    //clean up memory and close stuff
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    delete[] image_data;
    LOGI("Unable to allocate row_pointer while loading %s", fname.c_str());
    AAsset_close(asset);
    return NULL;
  }
  // set the individual row_pointers to point at the correct offsets of image_data
  for (int i = 0; i < height; ++i) {
    //row_pointers[height - 1 - i] = image_data + i * rowbytes;
    row_pointers[i] = image_data + i * rowbytes;
  }

  //read the png into image_data through row_pointers
  png_read_image(png_ptr, row_pointers);

  ImageData* img = new PNGImageData(image_data, width, height);

  //clean up memory and close stuff
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  delete[] row_pointers;

  AAsset_close(asset);

  return img;
}


#endif /* PNGLOADER_H_ */
