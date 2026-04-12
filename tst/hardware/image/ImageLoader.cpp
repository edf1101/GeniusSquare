/**
 * Created by Ed Fillingham on 1/08/2025.
 *
 * The image loader class is responsible for loading images from the SD card.
 * It does this by reading raw binary image files and caching them for reuse so RAM is preserved.
 */
#include "ImageLoader.h"

// static‐member definitions
unordered_map<string, Image *> ImageLoader::imageCache;
unordered_map<string, int>     ImageLoader::imageRefCount;

/**
 * Constructor for the ImageLoader class.
 *
 * @param filePath The path to the image file on the SD card.
 * @param width the width of the image to load
 * @param height the height of the image to load
 */
ImageLoader::ImageLoader(const string &filePath, int width, int height)
        : filePath(filePath), width(width), height(height), imgPtr(nullptr)
{
  // if you ever hash this loader for yourself:
  size_t seed = 0;
  hash_combine(seed, filePath);
  hash_combine(seed, width);
  hash_combine(seed, height);
  uuidHash = seed;

  initImage();
}

/**
 * Initializes the image by checking the cache, reading from the SD card if necessary,
 */
void ImageLoader::initImage() {
  // 1) Already in cache?
  auto cit = imageCache.find(filePath);
  if (cit != imageCache.end()) {
    imgPtr = cit->second;
    imageRefCount[filePath]++;
    return;
  }

  // 2) Open & read raw .bin
  File f = SD.open(filePath.c_str(), FILE_READ);
  if (!f) {
    Serial.print("ERROR: cannot open ");
    Serial.println(filePath.c_str());
    // TODO: replace with project logger
    while (1) delay(1000);
  }

  size_t imgSize = f.size();
  uint8_t *buffer = new uint8_t[imgSize];
  if (!buffer) {
    Serial.println("ERROR: allocation failed");
    // TODO: replace with project logger
    while (1) delay(1000);
  }
  f.read(buffer, imgSize);
  f.close();

  // 3) Hand off to Image (takes ownership of buffer)
  imgPtr = new Image(buffer, imgSize, width, height);

  // 4) Cache it
  imageCache[filePath]   = imgPtr;
  imageRefCount[filePath] = 1;
}

/**
 * Destructor for the ImageLoader class.
 * It decrements the reference count for the image and deletes it if no references remain.
 */
ImageLoader::~ImageLoader() {
  // if we’ve been moved-from, imgPtr==nullptr and filePath is empty → no work
  if (!imgPtr) return;

  auto it = imageRefCount.find(filePath);
  if (it == imageRefCount.end()) return;

  if (--(it->second) <= 0) {
    delete imageCache[filePath];
    imageCache.erase(filePath);
    imageRefCount.erase(it);
  }
}

/**
 * Loads the image from the cache or SD card.
 *
 * @return Pointer to the Image object, or nullptr if the image could not be loaded.
 */
Image* ImageLoader::loadImage() {
  // For debug purposes print all the cached images and ref counts:
//     for (const auto &pair : imageCache)
//        {
//            Serial.print("Cached image: ");
//            Serial.print(pair.first.c_str());
//            Serial.print(", ref count: ");
//            Serial.println(imageRefCount[pair.first]);
//        }
  return imgPtr;
}

/**
 * Returns the hash of the image loader.
 *
 * @return The hash value of the image loader.
 */
ImageLoader::ImageLoader(ImageLoader&& o) noexcept
        : filePath(std::move(o.filePath))
        , width(o.width)
        , height(o.height)
        , imgPtr(o.imgPtr)
        , uuidHash(o.uuidHash)
{
  // Prevent the old loader from deregistering in its dtor:
  o.imgPtr    = nullptr;
  o.filePath.clear();
}

/**
 * Move assignment operator for the ImageLoader class.
 *
 * @param o The ImageLoader object to move from.
 * @return  A reference to this ImageLoader object after the move.
 */
ImageLoader& ImageLoader::operator=(ImageLoader&& o) noexcept {
  if (this != &o) {
    // first, deregister our current image (if any)
    this->~ImageLoader();

    // now steal:
    filePath = std::move(o.filePath);
    width    = o.width;
    height   = o.height;
    uuidHash = o.uuidHash;
    imgPtr   = o.imgPtr;

    o.imgPtr       = nullptr;
    o.filePath.clear();
  }
  return *this;
}
