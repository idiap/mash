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

        CommunicationChannel clone(slave);

        tSandboxMessage message;
        CHECK_EQUAL(ERROR_NONE, clone.receivePacket(&message));
        CHECK_EQUAL(SANDBOX_MESSAGE_PING, message);

        unsigned int id = 0;
        CHECK(clone.read(&id));
        CHECK_EQUAL(100, id);
        
        _exit(0);
    }
    
    slave.close();

    CommunicationChannel clone;
    clone = master;
    master.close();
    
    CHECK_EQUAL(ERROR_NONE, clone.startPacket(SANDBOX_MESSAGE_PING));
    clone.add((unsigned int) 100);
    CHECK_EQUAL(ERROR_NONE, clone.sendPacket());

    int exit_status = 0;
    waitpid(pid, &exit_status, 0);
    
    return WEXITSTATUS(exit_status);
}
