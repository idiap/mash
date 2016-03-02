#include <UnitTest++.h>
#include <mash/dynlibs_manager.h>

using namespace Mash;


SUITE(DynlibsManagerSuite)
{
    TEST(CorrectDynamicLibraryPath)
    {
#if MASH_PLATFORM == MASH_PLATFORM_WIN32
#   if MASH_IDE_USED
        CHECK_EQUAL("examples\\" + string(CMAKE_INTDIR) + "\\identity.dll", DynlibsManager::getDynamicLibraryPath("examples/identity"));
#   else
        CHECK_EQUAL("examples\\identity.dll", DynlibsManager::getDynamicLibraryPath("examples/identity"));
#   endif
#else
#   if MASH_IDE_USED
        CHECK_EQUAL("examples/" + string(CMAKE_INTDIR) + "/libidentity.so", DynlibsManager::getDynamicLibraryPath("examples/identity"));
#   else
        CHECK_EQUAL("examples/libidentity.so", DynlibsManager::getDynamicLibraryPath("examples/identity"));
#   endif
#endif
    }


    TEST(CorrectDynamicLibraryPathWithVersion)
    {
#if MASH_PLATFORM == MASH_PLATFORM_WIN32
#   if MASH_IDE_USED
        CHECK_EQUAL("examples\\" + string(CMAKE_INTDIR) + "\\identity_v2.dll", DynlibsManager::getDynamicLibraryPath("examples/identity/2"));
#   else
        CHECK_EQUAL("examples\\identity_v2.dll", DynlibsManager::getDynamicLibraryPath("examples/identity/2"));
#   endif
#else
#   if MASH_IDE_USED
        CHECK_EQUAL("examples/" + string(CMAKE_INTDIR) + "/libidentity_v2.so", DynlibsManager::getDynamicLibraryPath("examples/identity/2"));
#   else
        CHECK_EQUAL("examples/libidentity_v2.so", DynlibsManager::getDynamicLibraryPath("examples/identity/2"));
#   endif
#endif
    }


    TEST(CorrectSourceFilePath)
    {
#if MASH_PLATFORM == MASH_PLATFORM_WIN32
        CHECK_EQUAL("examples\\identity.cpp", DynlibsManager::getSourceFilePath("examples/identity"));
#else
        CHECK_EQUAL("examples/identity.cpp", DynlibsManager::getSourceFilePath("examples/identity"));
#endif
    }


    TEST(CorrectSourceFilePathWithVersion)
    {
#if MASH_PLATFORM == MASH_PLATFORM_WIN32
        CHECK_EQUAL("examples\\identity_v2.cpp", DynlibsManager::getSourceFilePath("examples/identity/2"));
#else
        CHECK_EQUAL("examples/identity_v2.cpp", DynlibsManager::getSourceFilePath("examples/identity/2"));
#endif
    }
}
