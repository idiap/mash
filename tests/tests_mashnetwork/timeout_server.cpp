#include <mash-network/server.h>
#include <mash-network/server_listener.h>
#include <mash-utils/stringutils.h>
#include <string.h>

using namespace Mash;
using namespace std;


class TimeoutListener: public ServerListener
{
public:
    TimeoutListener(int socket)
    : ServerListener(socket), _bTimeoutCalled(false)
    {
        char buffer1[50];
        char buffer2[50];

        sprintf(buffer1, "EchoListener #%d", socket);
        sprintf(buffer2, "logs/tests/EchoListener_%d_$TIMESTAMP.log", socket);
        
        _outStream.open(buffer1, buffer2);
        
        _timeout.tv_sec = 1;
    }

    virtual ~TimeoutListener()
    {
    }

    virtual tAction handleCommand(const std::string& strCommand,
                                  const ArgumentsList& arguments)
    {
        if (!sendResponse((_bTimeoutCalled ? "OK" : "KO"), ArgumentsList()))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    virtual void onTimeout()
    {
        _bTimeoutCalled = true;
    }

    bool _bTimeoutCalled;
};


ServerListener* createListener(int socket)
{
    return new TimeoutListener(socket);
}



int main(int argc, char** argv)
{
    if (argc == 2)
        OutStream::verbosityLevel = (strcmp(argv[1], "--verbose") == 0 ? 1 : 0);
    
    Server server(1);
    
    server.listen("127.0.0.1", 10000, createListener);
    
    return 0;
}
