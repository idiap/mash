#include <UnitTest++.h>
#include <mash/imageutils.h>
#include <stdio.h>

using namespace Mash;

SUITE(ImageUtilsSuite)
{
    TEST(PNGImageLoading)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Red_100x50.png");
        
        CHECK(pImage);
        CHECK_EQUAL(100, pImage->width());
        CHECK_EQUAL(50, pImage->height());

        delete pImage;
    }


    TEST(JPEGImageLoading)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Red_100x50.jpg");
        
        CHECK(pImage);
        CHECK_EQUAL(100, pImage->width());
        CHECK_EQUAL(50, pImage->height());

        delete pImage;
    }


    TEST(PNGImageCreationFromBuffer)
    {
        FILE* file = fopen(MASH_DATA_DIR "/unittests/Red_100x50.png", "rb");
        
        CHECK(file);
        
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        unsigned char* pBuffer = new unsigned char[size];
        fread(pBuffer, sizeof(unsigned char), size, file);
        
        fclose(file);
        
        Image* pImage = ImageUtils::createImage("image/png", pBuffer, size);
        
        CHECK(pImage);
        CHECK_EQUAL(100, pImage->width());
        CHECK_EQUAL(50, pImage->height());

        delete[] pBuffer;
        delete pImage;
    }


    TEST(JPEGImageCreationFromBuffer)
    {
        FILE* file = fopen(MASH_DATA_DIR "/unittests/Red_100x50.jpg", "rb");
        
        CHECK(file);
        
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        unsigned char* pBuffer = new unsigned char[size];
        fread(pBuffer, sizeof(unsigned char), size, file);
        
        fclose(file);
        
        Image* pImage = ImageUtils::createImage("image/jpeg", pBuffer, size);
        
        CHECK(pImage);
        CHECK_EQUAL(100, pImage->width());
        CHECK_EQUAL(50, pImage->height());

        delete[] pBuffer;
        delete pImage;
    }


    TEST(MIFImageCreationFromBuffer)
    {
        FILE* file = fopen(MASH_DATA_DIR "/unittests/car.mif", "rb");
        
        CHECK(file);
        
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        unsigned char* pBuffer = new unsigned char[size];
        fread(pBuffer, sizeof(unsigned char), size, file);
        
        fclose(file);
        
        Image* pImage = ImageUtils::createImage("image/mif", pBuffer, size);
        
        CHECK(pImage);
        CHECK_EQUAL(128, pImage->width());
        CHECK_EQUAL(128, pImage->height());

        delete[] pBuffer;
        delete pImage;
    }


    TEST(MissingImageLoading)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/missing.png");

        CHECK(!pImage);
    }


    TEST(LoadedRgbPNGImageContainsRgbPixels)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Red_100x50.png");
        
        CHECK(pImage->hasPixelFormat(Image::PIXELFORMAT_RGB));
        CHECK(pImage->rgbBuffer());
    
        delete pImage;
    }


    TEST(LoadedJPEGImageContainsRgbPixels)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Red_100x50.jpg");
        
        CHECK(pImage->hasPixelFormat(Image::PIXELFORMAT_RGB));
        CHECK(pImage->rgbBuffer());
    
        delete pImage;
    }


    TEST(LoadedGrayscalePNGImageContainsGrayscalePixels)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Gray_50x20.png");
        
        CHECK(pImage->hasPixelFormat(Image::PIXELFORMAT_GRAY));
        CHECK(pImage->grayBuffer());
    
        delete pImage;
    }


    TEST(LoadedRedPNGImageContainsRedPixels)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Red_100x50.png");
        
        RGBPixel_t** pLines = pImage->rgbLines();

        for (unsigned int y = 0; y < pImage->height(); ++y)
        {
            for (unsigned int x = 0; x < pImage->width(); ++x)
            {
                RGBPixel_t pixel = pLines[y][x];
                CHECK_EQUAL(255, pixel.r);
                CHECK_EQUAL(0, pixel.g);
                CHECK_EQUAL(0, pixel.b);
            }
        }

        delete pImage;
    }


    TEST(LoadedGreenPNGImageContainsGreenPixels)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Green_60x30.png");
        
        RGBPixel_t** pLines = pImage->rgbLines();

        for (unsigned int y = 0; y < pImage->height(); ++y)
        {
            for (unsigned int x = 0; x < pImage->width(); ++x)
            {
                RGBPixel_t pixel = pLines[y][x];
                CHECK_EQUAL(0, pixel.r);
                CHECK_EQUAL(255, pixel.g);
                CHECK_EQUAL(0, pixel.b);
            }
        }

        delete pImage;
    }


    TEST(LoadedBluePNGImageContainsBluePixels)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Blue_80x40.png");
        
        RGBPixel_t** pLines = pImage->rgbLines();

        for (unsigned int y = 0; y < pImage->height(); ++y)
        {
            for (unsigned int x = 0; x < pImage->width(); ++x)
            {
                RGBPixel_t pixel = pLines[y][x];
                CHECK_EQUAL(0, pixel.r);
                CHECK_EQUAL(0, pixel.g);
                CHECK_EQUAL(255, pixel.b);
            }
        }

        delete pImage;
    }
    

    TEST(LoadedGrayscalePNGImageContainsCorrectGrayscaleValue)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Gray_50x20.png");
        
        byte_t** pLines = pImage->grayLines();

        for (unsigned int y = 0; y < pImage->height(); ++y)
        {
            for (unsigned int x = 0; x < pImage->width(); ++x)
                CHECK_EQUAL(128, pLines[y][x]);
        }

        delete pImage;
    }


    TEST(ConversionToGrayscale)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Red_100x50.png");
        
        ImageUtils::convertImageToPixelFormats(pImage, Image::PIXELFORMAT_GRAY);
        
        CHECK(pImage->hasPixelFormat(Image::PIXELFORMAT_GRAY));
        CHECK(pImage->grayBuffer());
    
        delete pImage;
    }


    TEST(ConversionToRGB)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Gray_50x20.png");
        
        ImageUtils::convertImageToPixelFormats(pImage, Image::PIXELFORMAT_RGB);
        
        CHECK(pImage->hasPixelFormat(Image::PIXELFORMAT_RGB));
        CHECK(pImage->rgbBuffer());
    
        delete pImage;
    }


    TEST(ImageScalingKeepsAllPixelFormats)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Red_100x50.png");
        
        ImageUtils::convertImageToPixelFormats(pImage, Image::PIXELFORMAT_GRAY);

        RGBPixel_t paddingColor = { 0 };
        Image* pImage2 = ImageUtils::scale(pImage, 50, 25, paddingColor);

        CHECK(pImage2);
        CHECK_EQUAL(50, pImage2->width());
        CHECK_EQUAL(25, pImage2->height());
        CHECK(pImage2->hasPixelFormat(Image::PIXELFORMAT_RGB));
        CHECK(pImage2->rgbBuffer());
        CHECK(pImage2->hasPixelFormat(Image::PIXELFORMAT_GRAY));
        CHECK(pImage2->grayBuffer());
    
        delete pImage;
        delete pImage2;
    }


    TEST(RGBImageScalingWithoutPadding)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Red_100x50.png");
        
        RGBPixel_t paddingColor = { 0 };
        Image* pImage2 = ImageUtils::scale(pImage, 50, 25, paddingColor);

        CHECK(pImage2);
        CHECK_EQUAL(50, pImage2->width());
        CHECK_EQUAL(25, pImage2->height());
        CHECK(pImage2->hasPixelFormat(Image::PIXELFORMAT_RGB));
        CHECK(pImage2->rgbBuffer());
        CHECK(!pImage2->hasPixelFormat(Image::PIXELFORMAT_GRAY));
        CHECK(!pImage2->grayBuffer());
        
        RGBPixel_t** pLines = pImage2->rgbLines();

        for (unsigned int y = 0; y < pImage2->height(); ++y)
        {
            for (unsigned int x = 0; x < pImage2->width(); ++x)
            {
                RGBPixel_t pixel = pLines[y][x];
                CHECK_EQUAL(255, (int) pixel.r);
                CHECK_EQUAL(0, (int) pixel.g);
                CHECK_EQUAL(0, (int) pixel.b);
            }
        }
    
        delete pImage;
        delete pImage2;
    }


    TEST(RGBImageScalingWithVerticalBlackPadding)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Red_100x50.png");
        
        RGBPixel_t paddingColor = { 0 };
        Image* pImage2 = ImageUtils::scale(pImage, 50, 50, paddingColor);

        CHECK(pImage2);
        CHECK_EQUAL(50, pImage2->width());
        CHECK_EQUAL(50, pImage2->height());
        CHECK(pImage2->hasPixelFormat(Image::PIXELFORMAT_RGB));
        CHECK(pImage2->rgbBuffer());
        CHECK(!pImage2->hasPixelFormat(Image::PIXELFORMAT_GRAY));
        CHECK(!pImage2->grayBuffer());
        
        RGBPixel_t** pLines = pImage2->rgbLines();

        for (unsigned int y = 0; y < pImage2->height(); ++y)
        {
            for (unsigned int x = 0; x < pImage2->width(); ++x)
            {
                RGBPixel_t pixel = pLines[y][x];
                
                if ((y < 12) or (y >= 37))
                {
                    CHECK_EQUAL(0, (int) pixel.r);
                    CHECK_EQUAL(0, (int) pixel.g);
                    CHECK_EQUAL(0, (int) pixel.b);
                }
                else
                {
                    CHECK_EQUAL(255, (int) pixel.r);
                    CHECK_EQUAL(0, (int) pixel.g);
                    CHECK_EQUAL(0, (int) pixel.b);
                }
            }
        }
    
        delete pImage;
        delete pImage2;
    }


    TEST(RGBImageScalingWithHorizontalBlackPadding)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Red_100x50.png");
        
        RGBPixel_t paddingColor = { 0 };
        Image* pImage2 = ImageUtils::scale(pImage, 100, 25, paddingColor);

        CHECK(pImage2);
        CHECK_EQUAL(100, pImage2->width());
        CHECK_EQUAL(25, pImage2->height());
        CHECK(pImage2->hasPixelFormat(Image::PIXELFORMAT_RGB));
        CHECK(pImage2->rgbBuffer());
        CHECK(!pImage2->hasPixelFormat(Image::PIXELFORMAT_GRAY));
        CHECK(!pImage2->grayBuffer());
        
        RGBPixel_t** pLines = pImage2->rgbLines();

        for (unsigned int y = 0; y < pImage2->height(); ++y)
        {
            for (unsigned int x = 0; x < pImage2->width(); ++x)
            {
                RGBPixel_t pixel = pLines[y][x];
                
                if ((x < 25) or (x >= 75))
                {
                    CHECK_EQUAL(0, (int) pixel.r);
                    CHECK_EQUAL(0, (int) pixel.g);
                    CHECK_EQUAL(0, (int) pixel.b);
                }
                else
                {
                    CHECK_EQUAL(255, (int) pixel.r);
                    CHECK_EQUAL(0, (int) pixel.g);
                    CHECK_EQUAL(0, (int) pixel.b);
                }
            }
        }
    
        delete pImage;
        delete pImage2;
    }


    TEST(RGBImageScalingWithVerticalWhitePadding)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Red_100x50.png");
        
        RGBPixel_t paddingColor = { 255, 255, 255 };
        Image* pImage2 = ImageUtils::scale(pImage, 50, 50, paddingColor);

        CHECK(pImage2);
        CHECK_EQUAL(50, pImage2->width());
        CHECK_EQUAL(50, pImage2->height());
        CHECK(pImage2->hasPixelFormat(Image::PIXELFORMAT_RGB));
        CHECK(pImage2->rgbBuffer());
        CHECK(!pImage2->hasPixelFormat(Image::PIXELFORMAT_GRAY));
        CHECK(!pImage2->grayBuffer());
        
        RGBPixel_t** pLines = pImage2->rgbLines();

        for (unsigned int y = 0; y < pImage2->height(); ++y)
        {
            for (unsigned int x = 0; x < pImage2->width(); ++x)
            {
                RGBPixel_t pixel = pLines[y][x];
                
                if ((y < 12) or (y >= 37))
                {
                    CHECK_EQUAL(255, (int) pixel.r);
                    CHECK_EQUAL(255, (int) pixel.g);
                    CHECK_EQUAL(255, (int) pixel.b);
                }
                else
                {
                    CHECK_EQUAL(255, (int) pixel.r);
                    CHECK_EQUAL(0, (int) pixel.g);
                    CHECK_EQUAL(0, (int) pixel.b);
                }
            }
        }
    
        delete pImage;
        delete pImage2;
    }


    TEST(GrayscaleImageScalingWithoutPadding)
    {
        Image* pImage = ImageUtils::loadImage(MASH_DATA_DIR "/unittests/Gray_50x20.png");
        
        RGBPixel_t paddingColor = { 0 };
        Image* pImage2 = ImageUtils::scale(pImage, 25, 10, paddingColor);

        CHECK(pImage2);
        CHECK_EQUAL(25, pImage2->width());
        CHECK_EQUAL(10, pImage2->height());
        CHECK(pImage2->hasPixelFormat(Image::PIXELFORMAT_GRAY));
        CHECK(pImage2->grayBuffer());
        
        byte_t** pLines = pImage2->grayLines();

        for (unsigned int y = 0; y < pImage2->height(); ++y)
        {
            for (unsigned int x = 0; x < pImage2->width(); ++x)
            {
                byte_t color = pLines[y][x];
                CHECK_EQUAL(128, (int) color);
            }
        }
    
        delete pImage;
        delete pImage2;
    }
}
