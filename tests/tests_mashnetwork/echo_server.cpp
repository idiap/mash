#include <mash-network/server.h>
#include <mash-network/server_listener.h>
#include <mash-utils/stringutils.h>
#include <string.h>

using namespace Mash;
using namespace std;


class EchoListener: public ServerListener
{
public:
    EchoListener(int socket)
    : ServerListener(socket)
    {
        char buffer1[50];
        char buffer2[50];

        sprintf(buffer1, "EchoListener #%d", socket);
        sprintf(buffer2, "logs/tests/EchoListener_%d_$TIMESTAMP.log", socket);
        
        _outStream.open(buffer1, buffer2);
    }

    virtual ~EchoListener()
    {
    }

    virtual tAction handleCommand(const std::string& strCommand,
                                  const ArgumentsList& arguments)
    {
        if (!sendResponse(strCommand, arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
};


ServerListener* createListener(int socket)
{
    return new EchoListener(socket);
}



int main(int argc, char** argv)
{
    unsigned int nbMaxClients = 0;
    
    if (argc == 3)
    {
        OutStream::verbosityLevel = (strcmp(argv[1], "--verbose") == 0 ? 1 : 0);
        nbMaxClients = StringUtils::parseUnsignedInt(argv[2]);
    }
    else if (argc == 2)
    {
        nbMaxClients = StringUtils::parseUnsignedInt(argv[1]);
    }
    
    Server server(nbMaxClients);
    
    server.listen("127.0.0.1", 10000, createListener);
    
    return 0;
}
