#include <mash-sandboxing/communication_channel.h>
#include <iostream>
#include <string>
#include <memory.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tests.h"

using namespace Mash;
using namespace std;


const unsigned int NB_VALUES    = 50000;
const unsigned int BUFFER_SIZE  = NB_VALUES * sizeof(unsigned int);


int main(int argc, char** argv)
{
    char* DATA[BUFFER_SIZE];
    
    for (unsigned int i = 0; i < NB_VALUES; ++i)
        ((unsigned int*) DATA)[i] = i;

    CommunicationChannel master, slave;
    CommunicationChannel::create(&master, &slave);

    pid_t pid = fork();
    if (pid == 0)
    {
        master.close();

        tSandboxMessage message;
        CHECK_EQUAL(ERROR_NONE, slave.receivePacket(&message));
        CHECK_EQUAL(SANDBOX_MESSAGE_PING, message);
        
        unsigned int size = 0;
        CHECK(slave.read(&size));
        CHECK_EQUAL(NB_VALUES, size);
        
        unsigned int buffer[NB_VALUES];
        memset(buffer, 0, sizeof(buffer));
        CHECK(slave.read((char*) buffer, BUFFER_SIZE));
        
        for (unsigned int i = 0; i < NB_VALUES; ++i)
            CHECK_EQUAL(((unsigned int*) DATA)[i], buffer[i]);
        
        _exit(0);
    }
    
    slave.close();

    CHECK_EQUAL(ERROR_NONE, master.startPacket(SANDBOX_MESSAGE_PING));
    master.add(NB_VALUES);
    master.add((char*) DATA, BUFFER_SIZE);
    CHECK_EQUAL(ERROR_NONE, master.sendPacket());
    
    int exit_status = 0;
    waitpid(pid, &exit_status, 0);
    
    return WEXITSTATUS(exit_status);
}
