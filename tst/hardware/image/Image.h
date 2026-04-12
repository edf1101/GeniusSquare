/**
 * Created by Ed Fillingham on 1/08/2025.
 *
 * The image class represents a 1-bit XBM image, which is a simple bitmap format.
 *
 * It provides methods to access the bitmap data, dimensions, and a unique hash for the image.
 */
#ifndef IMAGE_IMAGE_H
#define IMAGE_IMAGE_H

#include <Arduino.h>
#include <cstddef>
#include "utils/HashHelpers.h"

/**
 * Represents a 1-bit XBM image.
 */
class Image {
    friend class ImageLoader; // allow ImageLoader to create Images
public:
    // Getters
    const uint8_t *getBitmap() const { return bitmap; }

    int getWidth() const { return width; }

    int getHeight() const { return height; }

    std::size_t getUuidHash() const noexcept { return uuidHash; }

private:

    // 1) Templated ctor: compile-time array
    template<size_t N>
    Image(const uint8_t (&bitmapSrc)[N], int w, int h)
            : width(w), height(h), dataLen(N) {
      bitmap = new uint8_t[dataLen];
      memcpy(bitmap, bitmapSrc, dataLen);

      // compute deep-hash
      std::size_t seed = 0;
      hash_combine(seed, width);
      hash_combine(seed, height);
      for (size_t i = 0; i < dataLen; ++i)
        hash_combine(seed, bitmap[i]);
      uuidHash = seed;
    }

    // 2) Runtime ctor: take ownership of a new-allocated buffer
    Image(uint8_t *bitmapSrc, size_t N, int w, int h)
            : width(w), height(h), dataLen(N), bitmap(bitmapSrc) {
      // compute deep-hash
      std::size_t seed = 0;
      hash_combine(seed, width);
      hash_combine(seed, height);
      for (size_t i = 0; i < dataLen; ++i)
        hash_combine(seed, bitmap[i]);
      uuidHash = seed;
    }

    // 3) Big-three
    Image(const Image &other);

    Image &operator=(const Image &other);

    ~Image();

    uint8_t *bitmap;   // owns its own heap buffer
    int width;
    int height;
    size_t dataLen;  // number of bytes in that buffer
    std::size_t uuidHash;
};

// allow Image to be used in hash-based containers
namespace std {
    template<>
    struct hash<Image> {
        std::size_t operator()(Image const &img) const noexcept {
          return img.getUuidHash();
        }
    };
}

#endif // IMAGE_IMAGE_H
