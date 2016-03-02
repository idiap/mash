#include <UnitTest++.h>
#include <mash/images_cache.h>

using namespace Mash;


class MyCacheListener: public ImagesCache::IListener
{
public:
    MyCacheListener()
    : last_removed_index(0)
    {
    }

    virtual ~MyCacheListener() {}
    
    virtual void onImageRemoved(unsigned int index)
    {
        last_removed_index = index;
    }
    
    unsigned int last_removed_index;
};



SUITE(ImagesCacheSuite)
{
    TEST(NewCacheIsEmpty)
    {
        ImagesCache cache(100);
        
        CHECK_EQUAL(0, cache.nbImages());
    }


    TEST(NewCacheHasTheCorrectSize)
    {
        ImagesCache cache(100);
        
        CHECK_EQUAL(100, cache.size());
    }


    TEST(CacheWithOneImageReportsTheCorrectNumberOfImages)
    {
        ImagesCache cache(100);
        
        cache.addImage(1, new Image(128, 128));
        
        CHECK_EQUAL(1, cache.nbImages());
    }


    TEST(CacheWithSeveralImagesReportsTheCorrectNumberOfImages)
    {
        ImagesCache cache(100);
        
        cache.addImage(1, new Image(128, 128));
        cache.addImage(2, new Image(128, 128));
        cache.addImage(3, new Image(128, 128));
        cache.addImage(4, new Image(128, 128));
        
        CHECK_EQUAL(4, cache.nbImages());
    }


    TEST(RetrieveCachedImage)
    {
        ImagesCache cache(100);
        
        Image* pImage = new Image(128, 128);
        
        cache.addImage(1, pImage);
        
        CHECK_EQUAL(pImage, cache.getImage(1));
    }


    TEST(RetrieveNotCachedImageFail)
    {
        ImagesCache cache(100);
        
        cache.addImage(1, new Image(128, 128));
        
        CHECK(!cache.getImage(2));
    }


    TEST(AutomaticRemovalOfOlderImages)
    {
        ImagesCache cache(5);
        
        cache.addImage(1, new Image(128, 128));
        cache.addImage(2, new Image(128, 128));
        cache.addImage(3, new Image(128, 128));
        cache.addImage(4, new Image(128, 128));
        cache.addImage(5, new Image(128, 128));
        
        CHECK_EQUAL(5, cache.nbImages());

        cache.addImage(6, new Image(128, 128));
        
        CHECK_EQUAL(5, cache.nbImages());

        CHECK(!cache.getImage(1));
        CHECK(cache.getImage(2));
        CHECK(cache.getImage(3));
        CHECK(cache.getImage(4));
        CHECK(cache.getImage(5));
        CHECK(cache.getImage(6));
    }


    TEST(RetrievingImageModifiesTheOrderOfAutomaticRemovalOfOlderImages)
    {
        ImagesCache cache(5);
        
        cache.addImage(1, new Image(128, 128));
        cache.addImage(2, new Image(128, 128));
        cache.addImage(3, new Image(128, 128));
        cache.addImage(4, new Image(128, 128));
        cache.addImage(5, new Image(128, 128));
        
        CHECK_EQUAL(5, cache.nbImages());

        cache.getImage(1);

        cache.addImage(6, new Image(128, 128));
        
        CHECK_EQUAL(5, cache.nbImages());

        CHECK(cache.getImage(1));
        CHECK(!cache.getImage(2));
        CHECK(cache.getImage(3));
        CHECK(cache.getImage(4));
        CHECK(cache.getImage(5));
        CHECK(cache.getImage(6));
    }


    TEST(ListenerIsNotifiedAboutRemovedImages)
    {
        ImagesCache cache(5);
        MyCacheListener listener;
        
        cache.setListener(&listener);
        CHECK_EQUAL(0, listener.last_removed_index);
                
        cache.addImage(1, new Image(128, 128));
        cache.addImage(2, new Image(128, 128));
        cache.addImage(3, new Image(128, 128));
        cache.addImage(4, new Image(128, 128));
        cache.addImage(5, new Image(128, 128));

        CHECK_EQUAL(0, listener.last_removed_index);

        cache.addImage(6, new Image(128, 128));

        CHECK_EQUAL(1, listener.last_removed_index);

        cache.setListener(0);
    }


    TEST(ListenerIsNotifiedAboutRemovedImagesOnClear)
    {
        ImagesCache cache(5);
        MyCacheListener listener;
        
        cache.setListener(&listener);
        CHECK_EQUAL(0, listener.last_removed_index);
                
        cache.addImage(1, new Image(128, 128));
        cache.addImage(2, new Image(128, 128));
        cache.addImage(3, new Image(128, 128));
        cache.addImage(4, new Image(128, 128));
        cache.addImage(5, new Image(128, 128));

        CHECK_EQUAL(0, listener.last_removed_index);

        cache.clear();

        CHECK_EQUAL(1, listener.last_removed_index);

        cache.setListener(0);
    }
}
