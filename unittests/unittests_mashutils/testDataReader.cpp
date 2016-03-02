#include <UnitTest++.h>
#include <mash-utils/data_writer.h>
#include <mash-utils/data_reader.h>
#include <string>
#include <unistd.h>

using namespace Mash;
using namespace std;


const char* DATAFILE = "tmp_data.txt";


struct Environment
{
    Environment()
    {
        remove(DATAFILE);
    }
    
    ~Environment()
    {
        remove(DATAFILE);
    }
};


SUITE(DataReaderSuite)
{
    TEST_FIXTURE(Environment, FileOpening)
    {
        DataWriter w;
        DataReader r;
    
        CHECK(w.open(DATAFILE));
        w.close();
    
        CHECK(r.open(DATAFILE));
        CHECK(r.isOpen());

        int result = access(DATAFILE, R_OK);
        CHECK_EQUAL(0, result);

        r.close();
    }
    
    
    TEST_FIXTURE(Environment, FileDeletion)
    {
        DataWriter w;
        DataReader r;
    
        w.open(DATAFILE);
        w.close();

        r.open(DATAFILE);
        r.deleteFile();

        int result = access(DATAFILE, R_OK);
        CHECK_EQUAL(-1, result);
    }


    TEST_FIXTURE(Environment, CopyUsingConstructor)
    {
        DataWriter w;
        DataReader r;
    
        string s1 = "Hello world!";
        string s2 = "What's up doc'?";
    
        w.open(DATAFILE);
        w << s1 << endl << s2;
        w.close();
    
        r.open(DATAFILE);
        DataReader r2(r);
    
        CHECK(r2.isOpen());
        
        char buffer[100];
    
        int64_t nb = r.readline(buffer, 100);
        CHECK_EQUAL(s1.size(), nb - 1);     // -1 due to the discarded endl
        CHECK_EQUAL(s1, string(buffer));
    
        nb = r.readline(buffer, 100);
        CHECK_EQUAL(s2.size(), nb);
        CHECK_EQUAL(s2, string(buffer));

        nb = r2.readline(buffer, 100);
        CHECK_EQUAL(s1.size(), nb - 1);     // -1 due to the discarded endl
        CHECK_EQUAL(s1, string(buffer));
    
        nb = r2.readline(buffer, 100);
        CHECK_EQUAL(s2.size(), nb);
        CHECK_EQUAL(s2, string(buffer));
    
        r.close();
        r2.close();
    }


    TEST_FIXTURE(Environment, CopyUsingAssignementOperator)
    {
        DataWriter w;
        DataReader r, r2;
    
        string s1 = "Hello world!";
        string s2 = "What's up doc'?";
    
        w.open(DATAFILE);
        w << s1 << endl << s2;
        w.close();
    
        r.open(DATAFILE);
        r2 = r;

        CHECK(r2.isOpen());
    
        char buffer[100];
    
        int64_t nb = r.readline(buffer, 100);
        CHECK_EQUAL(s1.size(), nb - 1);     // -1 due to the discarded endl
        CHECK_EQUAL(s1, string(buffer));
    
        nb = r.readline(buffer, 100);
        CHECK_EQUAL(s2.size(), nb);
        CHECK_EQUAL(s2, string(buffer));

        nb = r2.readline(buffer, 100);
        CHECK_EQUAL(s1.size(), nb - 1);     // -1 due to the discarded endl
        CHECK_EQUAL(s1, string(buffer));
    
        nb = r2.readline(buffer, 100);
        CHECK_EQUAL(s2.size(), nb);
        CHECK_EQUAL(s2, string(buffer));
    
        r.close();
        r2.close();
    }


    TEST_FIXTURE(Environment, CopyStillOpenAfterOriginalClose)
    {
        DataWriter w;
        DataReader r;
    
        string s1 = "Hello world!";
        string s2 = "What's up doc'?";
    
        w.open(DATAFILE);
        w << s1 << endl << s2;
        w.close();
    
        r.open(DATAFILE);
        DataReader r2 = r;

        r.close();
    
        char buffer[100];
    
        int64_t nb = r2.readline(buffer, 100);
        CHECK_EQUAL(s1.size(), nb - 1);     // -1 due to the discarded endl
        CHECK_EQUAL(s1, string(buffer));
    
        nb = r2.readline(buffer, 100);
        CHECK_EQUAL(s2.size(), nb);
        CHECK_EQUAL(s2, string(buffer));
    
        r2.close();
    }
    
    
    TEST_FIXTURE(Environment, ReadBinaryData)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t buffer[100];
        
        for (unsigned int i = 0; i < 100; ++i)
        {
            data[i] = i - 50;
            buffer[i] = 0;
        }
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE);
    
        int64_t nb = r.read(buffer, 100);
        CHECK_EQUAL(100, nb);
    
        for (unsigned int i = 0; i < 100; ++i)
            CHECK_EQUAL(data[i], buffer[i]);

        CHECK(r.eof());
    
        r.close();
    }
    
    
    TEST_FIXTURE(Environment, ReadLines)
    {
        DataWriter w;
        DataReader r;
    
        string s1 = "Hello world!";
        string s2 = "What's up doc'?";
    
        w.open(DATAFILE);
        w << s1 << endl << s2;
        w.close();
    
        r.open(DATAFILE);
    
        char buffer[100];
    
        int64_t nb = r.readline(buffer, 100);
        CHECK_EQUAL(s1.size(), nb - 1);     // -1 due to the discarded endl
        CHECK_EQUAL(s1, string(buffer));
    
        nb = r.readline(buffer, 100);
        CHECK_EQUAL(s2.size(), nb);
        CHECK_EQUAL(s2, string(buffer));
    
        r.close();
    }


    TEST_FIXTURE(Environment, Peek)
    {
        DataWriter w;
        DataReader r;
    
        string s1 = "Hello world!";
        string s2 = "What's up doc'?";
    
        w.open(DATAFILE);
        w << s1 << endl << s2;
        w.close();
    
        r.open(DATAFILE);
    
        char buffer[100];
    
        uint8_t c = r.peek();
        CHECK_EQUAL('H', (char) c);
         
        int64_t nb = r.readline(buffer, 100);
        CHECK_EQUAL(s1.size(), nb - 1);     // -1 due to the discarded endl
        CHECK_EQUAL(s1, string(buffer));

        c = r.peek();
        CHECK_EQUAL('W', (char) c);
    
        nb = r.readline(buffer, 100);
        CHECK_EQUAL(s2.size(), nb);
        CHECK_EQUAL(s2, string(buffer));
    
        r.close();
    }
    

    TEST_FIXTURE(Environment, Read8bitsTypes)
    {
        DataWriter w;
        DataReader r;

        w.open(DATAFILE);
        w << (int8_t) -127 << endl;
        w << (uint8_t) 255 << endl;
        w.close();

        r.open(DATAFILE);

        int8_t v1;
        uint8_t v2;
        
        r >> v1 >> v2;
        
        CHECK_EQUAL(-127, (int16_t) v1);
        CHECK_EQUAL(255, (uint16_t) v2);

        r.close();
    }


    TEST_FIXTURE(Environment, Read16bitsTypes)
    {
        DataWriter w;
        DataReader r;
    
        w.open(DATAFILE);
        w << (int16_t) -32767 << endl;
        w << (uint16_t) 65535 << endl;
        w.close();
    
        r.open(DATAFILE);
    
        int16_t v1;
        uint16_t v2;
        
        r >> v1 >> v2;
        
        CHECK_EQUAL(-32767, v1);
        CHECK_EQUAL(65535, v2);
    
        r.close();
    }


    TEST_FIXTURE(Environment, Read32bitsTypes)
    {
        DataWriter w;
        DataReader r;
    
        w.open(DATAFILE);
        w << (int32_t) -2147483647 << endl;
        w << (uint32_t) 4294967295 << endl;
        w.close();
    
        r.open(DATAFILE);
    
        int32_t v1;
        uint32_t v2;
        
        r >> v1 >> v2;
        
        CHECK_EQUAL(-2147483647, v1);
        CHECK_EQUAL(4294967295, v2);
    
        r.close();
    }


    TEST_FIXTURE(Environment, Read64bitsTypes)
    {
        DataWriter w;
        DataReader r;
    
        w.open(DATAFILE);
        w << (int64_t) -9223372036854775807LL << endl;
        w << (uint64_t) 18446744073709551615ULL << endl;
        w.close();
    
        r.open(DATAFILE);
    
        int64_t v1;
        uint64_t v2;
        
        r >> v1 >> v2;
        
        CHECK_EQUAL(-9223372036854775807LL, v1);
        CHECK_EQUAL(18446744073709551615ULL, v2);
    
        r.close();
    }


    TEST_FIXTURE(Environment, ReadFloatingPointTypes)
    {
        DataWriter w;
        DataReader r;
    
        w.open(DATAFILE);
        w << (float) 5.2f << endl;
        w << (double) 5.2 << endl;
        w << (long double) 5.2 << endl;
        w.close();
    
        r.open(DATAFILE);
    
        float v1;
        double v2;
        long double v3;
        
        r >> v1 >> v2 >> v3;
        
        CHECK_CLOSE(5.2f, v1, 1e-6f);
        CHECK_CLOSE(5.2, v2, 1e-6);
        CHECK_CLOSE(5.2, v3, 1e-6);
    
        r.close();
    }


    TEST_FIXTURE(Environment, Tell)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t buffer[100];
        
        for (unsigned int i = 0; i < 100; ++i)
        {
            data[i] = i - 50;
            buffer[i] = 0;
        }
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE);

        CHECK_EQUAL(0, r.tell());
    
        int64_t nb = r.read(buffer, 50);

        CHECK_EQUAL(50, r.tell());
    
        r.close();
    }


    TEST_FIXTURE(Environment, SeekFromBeginning)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t v;
        
        for (unsigned int i = 0; i < 100; ++i)
            data[i] = i;
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE);

        r.seek(0, DataReader::BEGIN);
        CHECK_EQUAL(0, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(0, v);


        r.seek(50, DataReader::BEGIN);
        CHECK_EQUAL(50, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(50, v);


        r.seek(75, DataReader::BEGIN);
        CHECK_EQUAL(75, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(75, v);


        r.seek(0, DataReader::BEGIN);
        CHECK_EQUAL(0, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(0, v);
    
        r.close();
    }


    TEST_FIXTURE(Environment, SeekFromCurrent)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t v;
        
        for (unsigned int i = 0; i < 100; ++i)
            data[i] = i;
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE);

        r.seek(0, DataReader::CURRENT);
        CHECK_EQUAL(0, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(0, v);


        r.seek(49, DataReader::CURRENT);
        CHECK_EQUAL(50, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(50, v);


        r.seek(24, DataReader::CURRENT);
        CHECK_EQUAL(75, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(75, v);


        r.seek(-51, DataReader::CURRENT);
        CHECK_EQUAL(25, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(25, v);


        r.seek(0, DataReader::CURRENT);
        CHECK_EQUAL(26, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(26, v);
    
        r.close();
    }


    TEST_FIXTURE(Environment, SeekFromEnd)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t v;
        
        for (unsigned int i = 0; i < 100; ++i)
            data[i] = i;
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE);

        r.seek(0, DataReader::END);
        CHECK_EQUAL(100, r.tell());

        CHECK(r.eof());

        r.seek(-50, DataReader::END);
        CHECK_EQUAL(50, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(50, v);


        r.seek(-25, DataReader::END);
        CHECK_EQUAL(75, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(75, v);


        r.seek(0, DataReader::END);
        CHECK_EQUAL(100, r.tell());

        CHECK(r.eof());
    
        r.close();
    }


    TEST_FIXTURE(Environment, SeekFromBeginningWithOffset)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t v;
        
        for (unsigned int i = 0; i < 100; ++i)
            data[i] = i;
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE, 10);

        r.seek(0, DataReader::BEGIN);
        CHECK_EQUAL(0, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(10, v);


        r.seek(50, DataReader::BEGIN);
        CHECK_EQUAL(50, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(60, v);


        r.seek(75, DataReader::BEGIN);
        CHECK_EQUAL(75, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(85, v);


        r.seek(0, DataReader::BEGIN);
        CHECK_EQUAL(0, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(10, v);
    
        r.close();
    }


    TEST_FIXTURE(Environment, SeekFromCurrentWithOffset)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t v;
        
        for (unsigned int i = 0; i < 100; ++i)
            data[i] = i;
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE, 10);

        r.seek(0, DataReader::CURRENT);
        CHECK_EQUAL(0, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(10, v);


        r.seek(49, DataReader::CURRENT);
        CHECK_EQUAL(50, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(60, v);


        r.seek(24, DataReader::CURRENT);
        CHECK_EQUAL(75, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(85, v);


        r.seek(-51, DataReader::CURRENT);
        CHECK_EQUAL(25, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(35, v);


        r.seek(0, DataReader::CURRENT);
        CHECK_EQUAL(26, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(36, v);
    
        r.close();
    }


    TEST_FIXTURE(Environment, SeekFromEndWithOffset)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t v;
        
        for (unsigned int i = 0; i < 100; ++i)
            data[i] = i;
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE, 10);

        r.seek(0, DataReader::END);
        CHECK_EQUAL(90, r.tell());

        CHECK(r.eof());

        r.seek(-50, DataReader::END);
        CHECK_EQUAL(40, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(50, v);


        r.seek(-25, DataReader::END);
        CHECK_EQUAL(65, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(75, v);


        r.seek(0, DataReader::END);
        CHECK_EQUAL(90, r.tell());

        CHECK(r.eof());
    
        r.close();
    }


    TEST_FIXTURE(Environment, SeekFromBeginningLimits)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t v;
        
        for (unsigned int i = 0; i < 100; ++i)
            data[i] = i;
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE);

        r.seek(-100, DataReader::BEGIN);
        CHECK_EQUAL(0, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(0, v);


        r.seek(100, DataReader::BEGIN);
        CHECK_EQUAL(100, r.tell());

        CHECK(r.eof());


        r.seek(200, DataReader::BEGIN);
        CHECK_EQUAL(100, r.tell());

        CHECK(r.eof());
    
        r.close();
    }


    TEST_FIXTURE(Environment, SeekFromCurrentLimits)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t v;
        
        for (unsigned int i = 0; i < 100; ++i)
            data[i] = i;
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE);

        r.seek(200, DataReader::CURRENT);
        CHECK_EQUAL(100, r.tell());

        CHECK(r.eof());


        r.seek(-200, DataReader::CURRENT);
        CHECK_EQUAL(0, r.tell());
 
        r.read(&v, 1);
        CHECK_EQUAL(0, v);
    
        r.close();
    }


    TEST_FIXTURE(Environment, SeekFromEndLimits)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t v;
        
        for (unsigned int i = 0; i < 100; ++i)
            data[i] = i;
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE);

        r.seek(100, DataReader::END);
        CHECK_EQUAL(100, r.tell());

        CHECK(r.eof());

        r.seek(-100, DataReader::END);
        CHECK_EQUAL(0, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(0, v);


        r.seek(-200, DataReader::END);
        CHECK_EQUAL(0, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(0, v);
    
        r.close();
    }


    TEST_FIXTURE(Environment, SeekFromBeginningWithOffsetLimits)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t v;
        
        for (unsigned int i = 0; i < 100; ++i)
            data[i] = i;
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE, 10);

        r.seek(-100, DataReader::BEGIN);
        CHECK_EQUAL(0, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(10, v);


        r.seek(100, DataReader::BEGIN);
        CHECK_EQUAL(90, r.tell());

        CHECK(r.eof());


        r.seek(200, DataReader::BEGIN);
        CHECK_EQUAL(90, r.tell());

        CHECK(r.eof());
    
        r.close();
    }


    TEST_FIXTURE(Environment, SeekFromCurrentWithOffsetLimits)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t v;
        
        for (unsigned int i = 0; i < 100; ++i)
            data[i] = i;
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE, 10);

        r.seek(200, DataReader::CURRENT);
        CHECK_EQUAL(90, r.tell());

        CHECK(r.eof());


        r.seek(-200, DataReader::CURRENT);
        CHECK_EQUAL(0, r.tell());
 
        r.read(&v, 1);
        CHECK_EQUAL(10, v);
    
        r.close();
    }


    TEST_FIXTURE(Environment, SeekFromEndWithOffsetLimits)
    {
        DataWriter w;
        DataReader r;
    
        int8_t data[100];
        int8_t v;
        
        for (unsigned int i = 0; i < 100; ++i)
            data[i] = i;
    
        w.open(DATAFILE);
        w.write(data, 100);
        w.close();
    
        r.open(DATAFILE, 10);

        r.seek(100, DataReader::END);
        CHECK_EQUAL(90, r.tell());

        CHECK(r.eof());

        r.seek(-100, DataReader::END);
        CHECK_EQUAL(0, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(10, v);


        r.seek(-200, DataReader::END);
        CHECK_EQUAL(0, r.tell());

        r.read(&v, 1);
        CHECK_EQUAL(10, v);
    
        r.close();
    }
}
