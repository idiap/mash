#include <mash-network/server.h>
#include <mash-network/server_listener.h>
#include <mash-utils/stringutils.h>
#include "binary_data.h"

using namespace Mash;
using namespace std;


class BinaryListener: public ServerListener
{
public:
    BinaryListener(int socket)
    : ServerListener(socket)
    {
        char buffer1[50];
        char buffer2[50];

        sprintf(buffer1, "BinaryListener #%d", socket);
        sprintf(buffer2, "logs/tests/BinaryListener_%d_$TIMESTAMP.log", socket);
        
        _outStream.open(buffer1, buffer2);
    }

    virtual ~BinaryListener()
    {
    }

    virtual tAction handleCommand(const std::string& strCommand,
                                  const ArgumentsList& arguments)
    {
        if (strCommand != "SEND_DATA")
        {
            if (!sendResponse("UNKNOWN_COMMAND", ArgumentsList()))
                return ACTION_CLOSE_CONNECTION;

            return ACTION_NONE;
        }

        if (!sendResponse("DATA", ArgumentsList(DATA_size)))
            return ACTION_CLOSE_CONNECTION;

        if (!sendData(DATA, DATA_size))
            return ACTION_CLOSE_CONNECTION;

        unsigned char* data = new unsigned char[DATA_size];

        if (!waitData(data, DATA_size))
            return ACTION_CLOSE_CONNECTION;

        for (unsigned int i = 0; i < DATA_size; ++i)
        {
            if (DATA[i] != data[i])
                return ACTION_CLOSE_CONNECTION;
        }

        if (!sendResponse("TEST_OK", ArgumentsList()))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
};


ServerListener* createListener(int socket)
{
    return new BinaryListener(socket);
}



int main(int argc, char** argv)
{
    unsigned int nbMaxClients = 0;
    
    if (argc == 2)
        nbMaxClients = StringUtils::parseUnsignedInt(argv[1]);
    
    Server server(nbMaxClients);
    
    server.listen("127.0.0.1", 10000, createListener);
    
    return 0;
}
