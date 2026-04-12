/**
 * Created by Ed Fillingham on 1/08/2025.
 *
 * The image class represents a 1-bit XBM image, which is a simple bitmap format.
 *
 * It provides methods to access the bitmap data, dimensions, and a unique hash for the image.
 */
#include "Image.h"
#include <cstring>  // for memcpy

/**
 * Copy Constructor for the Image class.
 *
 * @param other The Image object to copy from.
 */
Image::Image(const Image &other)
        : width(other.width)
        , height(other.height)
        , dataLen(other.dataLen)
{
  bitmap   = new uint8_t[dataLen];
  memcpy(bitmap, other.bitmap, dataLen);
  uuidHash = other.uuidHash;
}

/**
 * Assignment operator for the Image class.
 *
 * @param other The Image object to copy from.
 * @return This Image object after assignment.
 */
Image& Image::operator=(const Image &other) {
  if (this == &other) return *this;
  delete[] bitmap;

  width    = other.width;
  height   = other.height;
  dataLen  = other.dataLen;
  bitmap   = new uint8_t[dataLen];
  memcpy(bitmap, other.bitmap, dataLen);
  uuidHash = other.uuidHash;
  return *this;
}

/**
 * Constructor for the Image class.
 *
 * @param _width The width of the image.
 * @param _height The height of the image.
 * @param _bitmap Pointer to the bitmap data.
 */
Image::~Image() {
  delete[] bitmap;
}
