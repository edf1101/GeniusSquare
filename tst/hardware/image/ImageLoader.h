/**
 * Created by Ed Fillingham on 1/08/2025.
 *
 * The image loader class is responsible for loading images from the SD card.
 * It does this by reading raw binary image files and caching them for reuse so RAM is preserved.
 *
 * Note: this class MUST be instantiated for each single object and then moved if wanted NOT copied.
 */
#ifndef IMAGELOADER_IMAGELOADER_H
#define IMAGELOADER_IMAGELOADER_H

#include "Image.h"
#include <SPI.h>
#include <SD.h>
#include <unordered_map>
#include <string>
#include "utils/HashHelpers.h"

using namespace std;

class ImageLoader {
public:
    // filePath = full path (including filename) on the SD card, e.g. "/images/returnBig.bin"
    ImageLoader(const string &filePath, int width, int height);
    ImageLoader(const ImageLoader&)            = delete;
    ImageLoader& operator=(const ImageLoader&) = delete;

    ImageLoader(ImageLoader&& other) noexcept;
    ImageLoader& operator=(ImageLoader&& other) noexcept;
    ~ImageLoader();

    // Returns the loaded (or cached) Image*.  Never nullptr once ctor succeeds.
    Image *loadImage();

    size_t getUuidHash() const noexcept { return uuidHash; }

private:
    // shared among all loaders
    static unordered_map<string, Image *> imageCache;
    static unordered_map<string, int>     imageRefCount;

    string filePath;  // full SD path to this one image
    int    width;     // pixel width
    int    height;    // pixel height
    Image *imgPtr;    // pointer to our cached Image
    size_t uuidHash;  // hash of this loader (if you ever need it)

    // helper to actually open/read/create on first use
    void initImage();
};

namespace std {
    template<>
    struct hash<ImageLoader> {
        size_t operator()(ImageLoader const &L) const noexcept {
          return L.getUuidHash();
        }
    };
}

#endif // IMAGELOADER_IMAGELOADER_H
