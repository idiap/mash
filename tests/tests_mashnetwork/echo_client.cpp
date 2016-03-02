#include <mash-network/client.h>
#include <tests.h>
#include <iostream>

using namespace Mash;
using namespace std;


int testTransmission(Client* pClient, const std::string& strCommand, const ArgumentsList& commandArgs, ArgumentsList* responseArgs)
{
    string strResponse;

    if (!pClient->sendCommand(strCommand, commandArgs))
    {
        cerr << "Failed to send command '" << strCommand << "' to the server" << endl;
        return -1;
    }

    if (!pClient->waitResponse(&strResponse, responseArgs))
    {
        cerr << "Failed to wait for response to command '" << strCommand << "' from the server" << endl;
        return -1;
    }

    CHECK_EQUAL(strCommand, strResponse);
    CHECK_EQUAL(commandArgs.size(), responseArgs->size());

    return 0;
}



int main(int argc, char** argv)
{
    // Declarations
    Client client;
    ArgumentsList commandArgs;
    ArgumentsList responseArgs;
    
    
    if (!client.connect("127.0.0.1", 10000))
    {
        cerr << "Failed to connect to the server" << endl;
        return -1;
    }
    

    commandArgs.clear();
    
    if (testTransmission(&client, "NO_ARGUMENT", commandArgs, &responseArgs) != 0)
        return -1;
    

    commandArgs.clear();
    commandArgs.add("http://something.com");

    if (testTransmission(&client, "ONE_STRING_ARGUMENT", commandArgs, &responseArgs) != 0)
        return -1;

    CHECK_EQUAL(commandArgs.getString(0), responseArgs.getString(0));

    
    commandArgs.clear();
    commandArgs.add(100);

    if (testTransmission(&client, "ONE_INT_ARGUMENT", commandArgs, &responseArgs) != 0)
        return -1;

    CHECK_EQUAL(commandArgs.getInt(0), responseArgs.getInt(0));


    commandArgs.clear();
    commandArgs.add(0.8f);

    if (testTransmission(&client, "ONE_FLOAT_ARGUMENT", commandArgs, &responseArgs) != 0)
        return -1;

    CHECK_EQUAL(commandArgs.getFloat(0), responseArgs.getFloat(0));


    commandArgs.clear();
    commandArgs.add("http://something.com");
    commandArgs.add(100);
    commandArgs.add(0.8f);

    if (testTransmission(&client, "THREE_ARGUMENTS", commandArgs, &responseArgs) != 0)
        return -1;

    CHECK_EQUAL(commandArgs.getString(0), responseArgs.getString(0));
    CHECK_EQUAL(commandArgs.getInt(1), responseArgs.getInt(1));
    CHECK_EQUAL(commandArgs.getFloat(2), responseArgs.getFloat(2));


    commandArgs.clear();
    commandArgs.add("Hello world!");

    if (testTransmission(&client, "ONE_QUOTED_STRING_ARGUMENT", commandArgs, &responseArgs) != 0)
        return -1;

    CHECK_EQUAL(commandArgs.getString(0), responseArgs.getString(0));


    commandArgs.clear();
    commandArgs.add("Escaped 'Hello world!' string");

    if (testTransmission(&client, "ONE_QUOTED_STRING_WITH_ESCAPING_ARGUMENT", commandArgs, &responseArgs) != 0)
        return -1;

    CHECK_EQUAL(commandArgs.getString(0), responseArgs.getString(0));


    commandArgs.clear();
    commandArgs.add("Long string on\nseveral lines");

    if (testTransmission(&client, "ONE_QUOTED_STRING_WITH_SEVERAL_LINES_ARGUMENT", commandArgs, &responseArgs) != 0)
        return -1;

    CHECK_EQUAL(commandArgs.getString(0), responseArgs.getString(0));


    client.close();
    
    return 0;
}
