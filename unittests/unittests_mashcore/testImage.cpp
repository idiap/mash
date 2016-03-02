#include <UnitTest++.h>
#include <mash/image.h>

using namespace Mash;


SUITE(ImageSuite)
{
    TEST(NewImageSize)
    {
        Image image(100, 50);
        
        CHECK_EQUAL(100, image.width());
        CHECK_EQUAL(50, image.height());
    }


    TEST(NewImageHasNoRGBRepresentation)
    {
        Image image(100, 50);
        
        CHECK(!image.hasPixelFormat(Image::PIXELFORMAT_RGB));
        CHECK(!image.rgbBuffer());
        CHECK(!image.rgbLines());
    }


    TEST(NewImageHasNoGrayRepresentation)
    {
        Image image(100, 50);
        
        CHECK(!image.hasPixelFormat(Image::PIXELFORMAT_GRAY));
        CHECK(!image.grayBuffer());
        CHECK(!image.grayLines());
    }


    TEST(RGBRepresentationCreation)
    {
        Image image(100, 50);
        
        image.addPixelFormats(Image::PIXELFORMAT_RGB);
        
        CHECK(image.hasPixelFormat(Image::PIXELFORMAT_RGB));
        CHECK(image.rgbBuffer());
        CHECK(image.rgbLines());
    }


    TEST(GrayRepresentationCreation)
    {
        Image image(100, 50);
        
        image.addPixelFormats(Image::PIXELFORMAT_GRAY);
        
        CHECK(image.hasPixelFormat(Image::PIXELFORMAT_GRAY));
        CHECK(image.grayBuffer());
        CHECK(image.grayLines());
    }


    TEST(CreationOfTwoRepresentationAtTheSameTime)
    {
        Image image(100, 50);
        
        image.addPixelFormats(Image::PIXELFORMAT_RGB |
                              Image::PIXELFORMAT_GRAY);
        
        CHECK(image.hasPixelFormat(Image::PIXELFORMAT_RGB));
        CHECK(image.rgbBuffer());
        CHECK(image.rgbLines());

        CHECK(image.hasPixelFormat(Image::PIXELFORMAT_GRAY));
        CHECK(image.grayBuffer());
        CHECK(image.grayLines());
    }


    TEST(OnlyTheFirstRepresentationCreationDoesSomething)
    {
        Image image(100, 50);
        
        image.addPixelFormats(Image::PIXELFORMAT_GRAY);
        
        byte_t* pFirst = image.grayBuffer();
        
        image.addPixelFormats(Image::PIXELFORMAT_GRAY);

        CHECK_EQUAL(pFirst, image.grayBuffer());
    }
}
