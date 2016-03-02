#include <mash-network/client.h>
#include <tests.h>
#include <iostream>
#include "binary_data.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    // Declarations
    Client client;
    ArgumentsList commandArgs;
    string strResponse;
    ArgumentsList responseArgs;
    
    
    if (!client.connect("127.0.0.1", 10000))
    {
        cerr << "Failed to connect to the server" << endl;
        return -1;
    }
    
    if (!client.sendCommand("SEND_DATA", commandArgs))
    {
        cerr << "Failed to send command 'SEND_DATA' to the server" << endl;
        return -1;
    }

    if (!client.waitResponse(&strResponse, &responseArgs))
    {
        cerr << "Failed to wait for response to command 'SEND_DATA' from the server" << endl;
        return -1;
    }

    CHECK_EQUAL("DATA", strResponse);
    CHECK_EQUAL(1, responseArgs.size());

    unsigned int size = responseArgs.getInt(0);
    CHECK_EQUAL(DATA_size, size);

    unsigned char* data = new unsigned char[size];
    
    if (!client.waitData(data, size))
    {
        cerr << "Failed to wait for " << size << " bytes of data from the server" << endl;
        delete[] data;
        return -1;
    }

    
    for (unsigned int i = 0; i < DATA_size; ++i)
        CHECK_EQUAL(DATA[i], data[i]);


    if (!client.sendData(data, size))
    {
        cerr << "Failed to send " << size << " bytes of data to the server" << endl;
        delete[] data;
        return -1;
    }
    
    delete[] data;


    if (!client.waitResponse(&strResponse, &responseArgs))
    {
        cerr << "Failed to wait for message 'TEST_OK' from the server" << endl;
        return -1;
    }

    CHECK_EQUAL("TEST_OK", strResponse);
    CHECK_EQUAL(0, responseArgs.size());

    client.close();
    
    return 0;
}
