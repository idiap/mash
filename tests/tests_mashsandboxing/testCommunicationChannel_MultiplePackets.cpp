#include <mash-sandboxing/communication_channel.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include "tests.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    CommunicationChannel master, slave;
    CommunicationChannel::create(&master, &slave);

    pid_t pid = fork();
    if (pid == 0)
    {
        master.close();

        sleep(1);

        tSandboxMessage message;

        for (unsigned int i = 0; i < 10; ++i)
        {
            CHECK_EQUAL(ERROR_NONE, slave.receivePacket(&message));
            CHECK_EQUAL(SANDBOX_MESSAGE_PING, message);

            unsigned int id = 0;
            CHECK(slave.read(&id));
            CHECK_EQUAL(i, id);
        }
        
        _exit(0);
    }
    
    slave.close();

    for (unsigned int i = 0; i < 10; ++i)
    {
        CHECK_EQUAL(ERROR_NONE, master.startPacket(SANDBOX_MESSAGE_PING));
        master.add(i);
        CHECK_EQUAL(ERROR_NONE, master.sendPacket());
    }

    int exit_status = 0;
    waitpid(pid, &exit_status, 0);
    
    return WEXITSTATUS(exit_status);
}
