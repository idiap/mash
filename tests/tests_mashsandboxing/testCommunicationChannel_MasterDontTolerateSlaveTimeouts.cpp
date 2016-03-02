#include <mash-sandboxing/communication_channel.h>
#include <iostream>
#include <string>
#include <signal.h>
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
        
        while (true)
            sleep(1);
        
        _exit(0);
    }
    
    slave.close();

    tSandboxMessage message;
    CHECK_EQUAL(ERROR_CHANNEL_SLAVE_TIMEOUT, master.receivePacket(&message, 1000));

    kill(pid, SIGKILL);

    int exit_status = 0;
    waitpid(pid, &exit_status, 0);
    
    return WEXITSTATUS(exit_status);
}
