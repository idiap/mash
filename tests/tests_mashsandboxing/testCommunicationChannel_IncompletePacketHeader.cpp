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

        CHECK_EQUAL(ERROR_NONE, slave.receivePacket(&message));
        CHECK_EQUAL(SANDBOX_MESSAGE_PING, message);

        unsigned char v = 0;
        for (unsigned int i = 0; i < 1024 - 12 - 4; ++i)
        {
            CHECK(slave.read(&v));
            CHECK_EQUAL((unsigned char) (i % 256), v);
        }

        CHECK_EQUAL(ERROR_NONE, slave.receivePacket(&message));
        CHECK_EQUAL(SANDBOX_MESSAGE_PING, message);

        unsigned int v2 = 0;
        CHECK(slave.read(&v2));
        CHECK_EQUAL(100, v2);
        
        _exit(0);
    }
    
    slave.close();

    CHECK_EQUAL(ERROR_NONE, master.startPacket(SANDBOX_MESSAGE_PING));
    for (unsigned int i = 0; i < 1024 - 12 - 4; ++i)
        master.add((unsigned char) (i % 256));
    CHECK_EQUAL(ERROR_NONE, master.sendPacket());

    CHECK_EQUAL(ERROR_NONE, master.startPacket(SANDBOX_MESSAGE_PING));
    master.add((unsigned int) 100);
    CHECK_EQUAL(ERROR_NONE, master.sendPacket());

    int exit_status = 0;
    waitpid(pid, &exit_status, 0);
    
    return WEXITSTATUS(exit_status);
}
