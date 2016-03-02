#include <UnitTest++.h>
#include <mash-utils/data_writer.h>
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


const double    d1 = 1.23456789;
const double    d2 = 12.3456789;
const double    d3 = 123.456789;
const long      l1 = 16;
const long      l2 = 256;
const long      l3 = 1024;
const long      l4 = 4096;
const long      l5 = 65536;


void DisplayDefault(DataWriter &writer)
{
   writer << "d1 = " << d1 << endl;
   writer << "d2 = " << d2 << endl;
   writer << "d3 = " << d3 << endl;
}


void DisplayWidth(DataWriter &writer, unsigned int width)
{
   writer << "d1 = " << setw(width) << d1 << endl;
   writer << "d2 = " << setw(width) << d2 << endl;
   writer << "d3 = " << setw(width) << d3 << endl;
}


void DisplayLongs(DataWriter &writer, int base)
{
   writer << setbase(base);
   writer << "l1 = " << l1 << endl;
   writer << "l2 = " << l2 << endl;
   writer << "l3 = " << l3 << endl;
   writer << "l4 = " << l4 << endl;
   writer << "l5 = " << l5 << endl;
}


SUITE(DataWriterSuite)
{
    TEST_FIXTURE(Environment, FileCreation)
    {
        DataWriter writer;
        
        writer.open(DATAFILE);

        int result = access(DATAFILE, R_OK);
        CHECK_EQUAL(0, result);
    }


    TEST_FIXTURE(Environment, FileDeletion)
    {
        DataWriter writer;
        
        writer.open(DATAFILE);
        
        writer.deleteFile();

        int result = access(DATAFILE, R_OK);
        CHECK_EQUAL(-1, result);
    }


    TEST_FIXTURE(Environment, Dump)
    {
        DataWriter writer;
        
        writer.open(DATAFILE);

        const char* TEXT = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus viverra rutrum egestas.";

        writer << TEXT;
        
        string content = writer.dump();
        
        CHECK_EQUAL(TEXT, content);
    }


    TEST_FIXTURE(Environment, CopyUsingConstructor)
    {
        DataWriter writer;
        
        writer.open(DATAFILE);

        DataWriter writer2 = writer;

        const char* TEXT1 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
        const char* TEXT2 = "Phasellus viverra rutrum egestas.";

        const char* TEXT_COMPLETE = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus viverra rutrum egestas.\n";

        writer << TEXT1;
        writer2 << " " << TEXT2 << endl;
        
        string content = writer.dump();
        CHECK_EQUAL(TEXT_COMPLETE, content);

        content = writer2.dump();
        CHECK_EQUAL(TEXT_COMPLETE, content);
    }


    TEST_FIXTURE(Environment, CopyUsingAssignementOperator)
    {
        DataWriter writer, writer2;
        
        writer.open(DATAFILE);

        writer2 = writer;

        const char* TEXT1 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
        const char* TEXT2 = "Phasellus viverra rutrum egestas.";

        const char* TEXT_COMPLETE = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus viverra rutrum egestas.\n";

        writer << TEXT1;
        writer2 << " " << TEXT2 << endl;
        
        string content = writer.dump();
        CHECK_EQUAL(TEXT_COMPLETE, content);

        content = writer2.dump();
        CHECK_EQUAL(TEXT_COMPLETE, content);
    }


    TEST_FIXTURE(Environment, CopyStillOpenAfterOriginalClose)
    {
        DataWriter writer;
        
        writer.open(DATAFILE);

        DataWriter writer2 = writer;

        writer.close();

        const char* TEXT = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus viverra rutrum egestas.";

        writer2 << TEXT;
        
        string content = writer2.dump();
        
        CHECK_EQUAL(TEXT, content);
    }


    TEST_FIXTURE(Environment, SizeLimit)
    {
        DataWriter writer;
        
        writer.open(DATAFILE, 5);

        writer << 1;
        writer << 2;
        writer << 3;
        writer << 4;
        writer << 5;
        writer << 6;
        
        string content = writer.dump();
        
        CHECK_EQUAL("12345", content);
    }


    TEST_FIXTURE(Environment, SizeLimitCutStrings)
    {
        DataWriter writer;
        
        writer.open(DATAFILE, 26);

        string TEXT = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus viverra rutrum egestas.";
        string TEXT2 = "Lorem ipsum dolor sit amet";

        writer << TEXT;
        
        string content = writer.dump();
        
        CHECK_EQUAL(TEXT2, content);
    }


    TEST_FIXTURE(Environment, Modificator_SetPrecision)
    {
        DataWriter writer;
        
        writer.open(DATAFILE);

        writer << setprecision(3);

        DisplayDefault(writer);

        string content = writer.dump();

        const char* TEXT = "d1 = 1.23\n" \
                           "d2 = 12.3\n" \
                           "d3 = 123\n";
        
        CHECK_EQUAL(TEXT, content);
    }


    TEST_FIXTURE(Environment, Modificator_SetFill)
    {
        DataWriter writer;
        
        writer.open(DATAFILE);

        writer << setprecision(9);

        writer << setfill(' ');
        DisplayWidth(writer, 15);

        writer << setfill('S');
        DisplayWidth(writer, 15);

        string content = writer.dump();

        const char* TEXT = "d1 =      1.23456789\n" \
                           "d2 =      12.3456789\n" \
                           "d3 =      123.456789\n" \
                           "d1 = SSSSS1.23456789\n" \
                           "d2 = SSSSS12.3456789\n" \
                           "d3 = SSSSS123.456789\n";                           
        
        CHECK_EQUAL(TEXT, content);
    }


    TEST_FIXTURE(Environment, Modificator_SetBase)
    {
        DataWriter writer;
        
        writer.open(DATAFILE);

        DisplayLongs(writer, 10);
        DisplayLongs(writer, 8);
        DisplayLongs(writer, 16);


        string content = writer.dump();

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

        CHECK_EQUAL(TEXT, content);
    }
}
