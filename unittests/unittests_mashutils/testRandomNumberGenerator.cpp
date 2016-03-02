#include <UnitTest++.h>
#include <mash-utils/random_number_generator.h>

using namespace Mash;

SUITE(RandomNumberGeneratorSuite)
{
    TEST(RandomizeUnsignedIntWithMaxEqualsTo0)
    {
        RandomNumberGenerator generator;
        CHECK_EQUAL(0, generator.randomize((unsigned int) 0));
    }

    TEST(RandomizeUnsignedIntBetween0And0)
    {
        RandomNumberGenerator generator;
        CHECK_EQUAL(0, generator.randomize((unsigned int) 0, (unsigned int) 0));
    }

    TEST(RandomizeUnsignedIntBetween100And100)
    {
        RandomNumberGenerator generator;
        CHECK_EQUAL(100, generator.randomize((unsigned int) 100, (unsigned int) 100));
    }

    TEST(RandomizeIntWithMaxEqualsTo0)
    {
        RandomNumberGenerator generator;
        CHECK_EQUAL(0, generator.randomize((int) 0));
    }

    TEST(RandomizeIntBetween0And0)
    {
        RandomNumberGenerator generator;
        CHECK_EQUAL(0, generator.randomize((int) 0, (int) 0));
    }

    TEST(RandomizeIntBetween100And100)
    {
        RandomNumberGenerator generator;
        CHECK_EQUAL(100, generator.randomize((int) 100, (int) 100));
    }

    TEST(RandomizeFloatWithMaxEqualsTo0)
    {
        RandomNumberGenerator generator;
        CHECK_CLOSE(0.0f, generator.randomize(0.0f), 1e-6f);
    }

    TEST(RandomizeFloatBetween0And0)
    {
        RandomNumberGenerator generator;
        CHECK_CLOSE(0.0f, generator.randomize(0.0f, 0.0f), 1e-6f);
    }

    TEST(RandomizeFloatBetween100And100)
    {
        RandomNumberGenerator generator;
        CHECK_CLOSE(100.0f, generator.randomize(100.0f, 100.0f), 1e-6f);
    }
}
