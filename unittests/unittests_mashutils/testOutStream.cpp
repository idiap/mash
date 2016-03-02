#include <UnitTest++.h>
#include <mash-utils/outstream.h>
#include <mash-utils/stringutils.h>
#include <string>
#include <unistd.h>

using namespace Mash;
using namespace std;


const char* LOGFILE = "tmp_outstream.txt";


struct Environment
{
    Environment()
    {
        remove(LOGFILE);
    }
    
    ~Environment()
    {
        remove(LOGFILE);
    }
};


string addHeader(const string& content, bool bAddLFAtEnd = false)
{
    return "********************************************************************************\n" \
           "*\n" \
           "*                   Test\n" \
           "*\n" \
           "********************************************************************************\n" \
           "\n" +
           content + (bAddLFAtEnd ? "\n" : "");
}


const double    d1 = 1.23456789;
const double    d2 = 12.3456789;
const double    d3 = 123.456789;
const long      l1 = 16;
const long      l2 = 256;
const long      l3 = 1024;
const long      l4 = 4096;
const long      l5 = 65536;


void DisplayDefault(OutStream &stream)
{
   stream << "d1 = " << d1 << endl;
   stream << "d2 = " << d2 << endl;
   stream << "d3 = " << d3 << endl;
}


void DisplayWidth(OutStream &stream, unsigned int width)
{
   stream << "d1 = " << setw(width) << d1 << endl;
   stream << "d2 = " << setw(width) << d2 << endl;
   stream << "d3 = " << setw(width) << d3 << endl;
}


void DisplayLongs(OutStream &stream, int base)
{
   stream << setbase(base);
   stream << "l1 = " << l1 << endl;
   stream << "l2 = " << l2 << endl;
   stream << "l3 = " << l3 << endl;
   stream << "l4 = " << l4 << endl;
   stream << "l5 = " << l5 << endl;
}


string getFileContent(const std::string& strFileName)
{
    ifstream inFile;
    inFile.open(strFileName.c_str());
    if (!inFile.is_open())
        return "";

    string strResult;
    char buffer[1025];

    while (!inFile.eof())
    {
        inFile.read(buffer, 1024);
        unsigned int nb = inFile.gcount();

        if (nb > 0)
        {
            buffer[nb] = 0;
            strResult += buffer;
        }
    }

    inFile.close();

    return strResult;
}


SUITE(OutStreamSuite)
{
    TEST_FIXTURE(Environment, FileCreation)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE);

        int result = access(LOGFILE, R_OK);
        CHECK_EQUAL(0, result);
    }


    TEST_FIXTURE(Environment, FileDeletion)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE);
        
        stream.deleteFile();

        int result = access(LOGFILE, R_OK);
        CHECK_EQUAL(-1, result);
    }


    TEST_FIXTURE(Environment, Dump)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE);
    
        const char* TEXT = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus viverra rutrum egestas.";
    
        stream << TEXT << endl;
        
        unsigned char* buffer;
        int size = stream.dump(&buffer);
        
        string ref = addHeader(TEXT, true);
        CHECK_EQUAL(ref.size(), size);
        CHECK_EQUAL(ref, string((const char*) buffer));
        
        delete[] buffer;
    }


    TEST_FIXTURE(Environment, DumpWithLimit)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE);
    
        const char* TEXT = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus viverra rutrum egestas. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus viverra rutrum egestas.";
    
        stream << TEXT << endl;
        
        unsigned char* buffer;
        int size = stream.dump(&buffer, 100);
        
        string ref = addHeader(TEXT, true);
        CHECK_EQUAL(100, size);
        CHECK_EQUAL("...\n" + ref.substr(ref.size() - 96), string((const char*) buffer));
        
        delete[] buffer;
    }
    
    
    TEST_FIXTURE(Environment, StreamCopyUsingConstructor)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE);
    
        OutStream stream2 = stream;
    
        const char* TEXT1 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
        const char* TEXT2 = "Phasellus viverra rutrum egestas.";
    
        const char* TEXT_COMPLETE = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus viverra rutrum egestas.";
    
        stream << TEXT1;
        stream2 << " " << TEXT2 << endl;
        
        unsigned char* buffer;
        int size = stream.dump(&buffer);
        
        string ref = addHeader(TEXT_COMPLETE, true);
        CHECK_EQUAL(ref.size(), size);
        CHECK_EQUAL(ref, string((const char*) buffer));
    
        delete[] buffer;
    
        size = stream2.dump(&buffer);
    
        CHECK_EQUAL(ref.size(), size);
        CHECK_EQUAL(ref, string((const char*) buffer));
    
        delete[] buffer;
    }
    
    
    TEST_FIXTURE(Environment, StreamCopyUsingAssignementOperator)
    {
        OutStream stream, stream2;
        
        stream.open("Test", LOGFILE);
    
        stream2 = stream;
    
        const char* TEXT1 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
        const char* TEXT2 = "Phasellus viverra rutrum egestas.";
    
        const char* TEXT_COMPLETE = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus viverra rutrum egestas.";
    
        stream << TEXT1;
        stream2 << " " << TEXT2 << endl;
        
        unsigned char* buffer;
        int size = stream.dump(&buffer);
        
        string ref = addHeader(TEXT_COMPLETE, true);
        CHECK_EQUAL(ref.size(), size);
        CHECK_EQUAL(ref, string((const char*) buffer));
    
        delete[] buffer;
    
        size = stream2.dump(&buffer);
    
        CHECK_EQUAL(ref.size(), size);
        CHECK_EQUAL(ref, string((const char*) buffer));
    
        delete[] buffer;
    }
    
    
    TEST_FIXTURE(Environment, StreamCopyStillOpenAfterOriginalClose)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE);
    
        OutStream stream2 = stream;
    
        stream.close();
    
        const char* TEXT = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus viverra rutrum egestas.";
    
        stream2 << TEXT << endl;
        
        unsigned char* buffer;
        int size = stream2.dump(&buffer);
        
        string ref = addHeader(TEXT, true);
        CHECK_EQUAL(ref.size(), size);
        CHECK_EQUAL(ref, string((const char*) buffer));
        
        delete[] buffer;
    }
    
    
    TEST_FIXTURE(Environment, SoftLimitForCString)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE, 100);
    
        const char* TEXT = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor " \
                           "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud " \
                           "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute " \
                           "irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla " \
                           "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia " \
                           "deserunt mollit anim id est laborum.";
    
        stream << TEXT << endl;

        string content = getFileContent(LOGFILE);
        
        string orig = string(TEXT);
        string ref = orig.substr(orig.size() - 100) + "\n";
        CHECK_EQUAL(ref.size(), content.size());
        CHECK_EQUAL(ref, content);

        stream.close();
    }


    TEST_FIXTURE(Environment, SoftLimitForStdString)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE, 100);
    
        string TEXT = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor " \
                      "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud " \
                      "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute " \
                      "irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla " \
                      "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia " \
                      "deserunt mollit anim id est laborum.";
    
        stream << TEXT << endl;

        string content = getFileContent(LOGFILE);
        
        string orig = string(TEXT);
        string ref = orig.substr(orig.size() - 100) + "\n";
        CHECK_EQUAL(ref.size(), content.size());
        CHECK_EQUAL(ref, content);

        stream.close();
    }


    TEST_FIXTURE(Environment, HardLimitForCString)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE, 300, false);
    
        const char* TEXT = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor " \
                           "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud " \
                           "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute " \
                           "irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla " \
                           "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia " \
                           "deserunt mollit anim id est laborum.";
    
        stream << TEXT << endl;

        string content = getFileContent(LOGFILE);
        
        string ref = addHeader("-------- Log file size limit reached -----------", true);
        CHECK_EQUAL(ref.size(), content.size());
        CHECK_EQUAL(ref, content);

        stream.close();
    }


    TEST_FIXTURE(Environment, HardLimitForStdString)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE, 300, false);
    
        string TEXT = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor " \
                      "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud " \
                      "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute " \
                      "irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla " \
                      "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia " \
                      "deserunt mollit anim id est laborum.";
    
        stream << TEXT << endl;

        string content = getFileContent(LOGFILE);
        
        string ref = addHeader("-------- Log file size limit reached -----------", true);
        CHECK_EQUAL(ref.size(), content.size());
        CHECK_EQUAL(ref, content);

        stream.close();
    }


    TEST_FIXTURE(Environment, HardLimitForIntegers)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE, 200, false);

        for (unsigned int i = 0; i < 300; ++i)
            stream << (i % 10);
    
        string content = getFileContent(LOGFILE);
        
        CHECK(StringUtils::endsWith(content, "-------- Log file size limit reached -----------\n", false));

        stream.close();
    }
    
    
    TEST_FIXTURE(Environment, Modificator_SetPrecision)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE);
    
        stream << setprecision(3);
    
        DisplayDefault(stream);
    
        unsigned char* buffer;
        int size = stream.dump(&buffer);
        
        const char* TEXT = "d1 = 1.23\n" \
                           "d2 = 12.3\n" \
                           "d3 = 123\n";
    
        string ref = addHeader(TEXT);
    
        CHECK_EQUAL(ref.size(), size);
        CHECK_EQUAL(ref, string((const char*) buffer));
        
        delete[] buffer;
    }
    
    
    TEST_FIXTURE(Environment, Modificator_SetFill)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE);
    
        stream << setprecision(9);
    
        stream << setfill(' ');
        DisplayWidth(stream, 15);
    
        stream << setfill('S');
        DisplayWidth(stream, 15);
    
        unsigned char* buffer;
        int size = stream.dump(&buffer);
        
        const char* TEXT = "d1 =      1.23456789\n" \
                           "d2 =      12.3456789\n" \
                           "d3 =      123.456789\n" \
                           "d1 = SSSSS1.23456789\n" \
                           "d2 = SSSSS12.3456789\n" \
                           "d3 = SSSSS123.456789\n";                           
    
        string ref = addHeader(TEXT);
    
        CHECK_EQUAL(ref.size(), size);
        CHECK_EQUAL(ref, string((const char*) buffer));
        
        delete[] buffer;
    }
    
    
    TEST_FIXTURE(Environment, Modificator_SetBase)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE);
    
        DisplayLongs(stream, 10);
        DisplayLongs(stream, 8);
        DisplayLongs(stream, 16);
    
        unsigned char* buffer;
        int size = stream.dump(&buffer);
        
        const char* TEXT = "l1 = 16\n" \
                           "l2 = 256\n" \
                           "l3 = 1024\n" \
                           "l4 = 4096\n" \
                           "l5 = 65536\n" \
                           "l1 = 20\n" \
                           "l2 = 400\n" \
                           "l3 = 2000\n" \
                           "l4 = 10000\n" \
                           "l5 = 200000\n" \
                           "l1 = 10\n" \
                           "l2 = 100\n" \
                           "l3 = 400\n" \
                           "l4 = 1000\n" \
                           "l5 = 10000\n";                           
    
        string ref = addHeader(TEXT);
    
        CHECK_EQUAL(ref.size(), size);
        CHECK_EQUAL(ref, string((const char*) buffer));
        
        delete[] buffer;
    }


    TEST_FIXTURE(Environment, LogDataTypes)
    {
        OutStream stream;
        
        stream.open("Test", LOGFILE);

        stream << int8_t(-127) << endl;
        stream << uint8_t(255) << endl;
        stream << int16_t(-32767) << endl;
        stream << uint16_t(65535) << endl;
        stream << int32_t(-2147483647) << endl;
        stream << uint32_t(4294967295) << endl;
        stream << int64_t(-9223372036854775807LL) << endl;
        stream << uint64_t(18446744073709551615ULL) << endl;
        stream << size_t(500) << endl;
        stream << off_t(500) << endl;
        stream << (char) -5 << endl;
        stream << (signed char) -5 << endl;
        stream << (unsigned char) 5 << endl;
        stream << (short) -50 << endl;
        stream << (signed short) -50 << endl;
        stream << (unsigned short) 50 << endl;
        stream << (int) -500 << endl;
        stream << (signed int) -500 << endl;
        stream << (unsigned int) 500 << endl;
        stream << (long) -5000 << endl;
        stream << (signed long) -5000 << endl;
        stream << (unsigned long) 5000 << endl;
        stream << (long long) -50000 << endl;
        stream << (signed long long) -50000 << endl;
        stream << (unsigned long long) 50000 << endl;
        stream << (float) 5.2f << endl;
        stream << (double) 5.2 << endl;
        stream << (long double) 5.2 << endl;
        stream << true << endl;
        stream << false << endl;

        unsigned char* buffer;
        int size = stream.dump(&buffer);
        
        const char* TEXT = "-127\n" \
                           "255\n" \
                           "-32767\n" \
                           "65535\n" \
                           "-2147483647\n" \
                           "4294967295\n" \
                           "-9223372036854775807\n" \
                           "18446744073709551615\n" \
                           "500\n" \
                           "500\n" \
                           "-5\n" \
                           "-5\n" \
                           "5\n" \
                           "-50\n" \
                           "-50\n" \
                           "50\n" \
                           "-500\n" \
                           "-500\n" \
                           "500\n" \
                           "-5000\n" \
                           "-5000\n" \
                           "5000\n" \
                           "-50000\n" \
                           "-50000\n" \
                           "50000\n" \
                           "5.2\n" \
                           "5.2\n" \
                           "5.2\n" \
                           "1\n" \
                           "0\n";
    
        string ref = addHeader(TEXT);
    
        CHECK_EQUAL(ref.size(), size);
        CHECK_EQUAL(ref, string((const char*) buffer));
        
        delete[] buffer;
    }
}
