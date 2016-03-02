#include <UnitTest++.h>
#include <mash-utils/data_writer.h>
#include <mash-utils/data_reader.h>
#include <string>
#include <unistd.h>

using namespace Mash;
using namespace std;


const char* WDATAFILE = "tmp_data.txt";


struct Environment
{
    Environment()
    {
        remove(WDATAFILE);
    }
    
    ~Environment()
    {
        remove(WDATAFILE);
    }
};


SUITE(DataWriterSuite)
{
    TEST_FIXTURE(Environment, FileCreation)
    {
        DataWriter w;
    
        CHECK(w.open(WDATAFILE));
        CHECK(w.isOpen());
        CHECK_EQUAL(WDATAFILE, w.getFileName());

        int result = access(WDATAFILE, R_OK);
        CHECK_EQUAL(0, result);

        w.close();
    }


    TEST_FIXTURE(Environment, FileDeletion)
    {
        DataWriter w;
    
        w.open(WDATAFILE);
        w.isOpen();

        w.deleteFile();

        int result = access(WDATAFILE, R_OK);
        CHECK_EQUAL(-1, result);
    }

    
    TEST_FIXTURE(Environment, CopyUsingConstructor)
    {
        DataWriter w;
        DataReader r;
    
        string s1 = "Hello world!";
        string s2 = "What's up doc'?";
    
        w.open(WDATAFILE);
        
        DataWriter w2(w);
        CHECK(w2.isOpen());
        CHECK_EQUAL(WDATAFILE, w2.getFileName());

        w << s1 << endl;
        w2 << s2;
        
        w.close();
        w2.close();
    
        r.open(WDATAFILE);
    
        char buffer[100];
    
        int64_t nb = r.readline(buffer, 100);
        CHECK_EQUAL(s1.size(), nb - 1);     // -1 due to the discarded endl
        CHECK_EQUAL(s1, string(buffer));
    
        nb = r.readline(buffer, 100);
        CHECK_EQUAL(s2.size(), nb);
        CHECK_EQUAL(s2, string(buffer));
    
        r.close();
    }


    TEST_FIXTURE(Environment, CopyUsingAssignementOperator)
    {
        DataWriter w, w2;
        DataReader r;
    
        string s1 = "Hello world!";
        string s2 = "What's up doc'?";
    
        w.open(WDATAFILE);
        
        w2 = w;
        CHECK(w2.isOpen());
        CHECK_EQUAL(WDATAFILE, w2.getFileName());

        w << s1 << endl;
        w2 << s2;
        
        w.close();
        w2.close();
    
        r.open(WDATAFILE);
    
        char buffer[100];
    
        int64_t nb = r.readline(buffer, 100);
        CHECK_EQUAL(s1.size(), nb - 1);     // -1 due to the discarded endl
        CHECK_EQUAL(s1, string(buffer));
    
        nb = r.readline(buffer, 100);
        CHECK_EQUAL(s2.size(), nb);
        CHECK_EQUAL(s2, string(buffer));
    
        r.close();
    }
    
    
    TEST_FIXTURE(Environment, CopyStillOpenAfterOriginalClose)
    {
        DataWriter w;
        DataReader r;
    
        string s1 = "Hello world!";
        string s2 = "What's up doc'?";
    
        w.open(WDATAFILE);
        
        DataWriter w2 = w;

        w.close();

        w2 << s1 << endl << s2;
        
        w2.close();
    
        r.open(WDATAFILE);
    
        char buffer[100];
    
        int64_t nb = r.readline(buffer, 100);
        CHECK_EQUAL(s1.size(), nb - 1);     // -1 due to the discarded endl
        CHECK_EQUAL(s1, string(buffer));
    
        nb = r.readline(buffer, 100);
        CHECK_EQUAL(s2.size(), nb);
        CHECK_EQUAL(s2, string(buffer));
    
        r.close();
    }
}
