#include <mash-sandboxing/communication_channel.h>
#include <iostream>
#include <string>
#include <memory.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tests.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    const char* TEXT = "You've got a mail!";

    CommunicationChannel master, slave;
    CommunicationChannel::create(&master, &slave);

    pid_t pid = fork();
    if (pid == 0)
    {
        master.close();

        tSandboxMessage message;
        CHECK_EQUAL(ERROR_NONE, slave.receivePacket(&message));
        CHECK_EQUAL(SANDBOX_MESSAGE_PING, message);

        unsigned int v1 = 0;
        CHECK(slave.read(&v1));
        CHECK_EQUAL(100, v1);

        int v2 = 0;
        CHECK(slave.read(&v2));
        CHECK_EQUAL(20, v2);

        float v3 = 0.0f;
        CHECK(slave.read(&v3));
        CHECK(v3 >= 10.0f - 1e-6f);
        CHECK(v3 <= 10.0f + 1e-6f);

        double v4 = 0.0;
        CHECK(slave.read(&v4));
        CHECK(v4 >= 15.0 - 1e-6);
        CHECK(v4 <= 15.0 + 1e-6);

        bool v5 = false;
        CHECK(slave.read(&v5));
        CHECK_EQUAL(true, v5);

        string v6;
        CHECK(slave.read(&v6));
        CHECK_EQUAL(string("Hello world"), v6);

        string v7;
        CHECK(slave.read(&v7));
        CHECK_EQUAL(string("What's up doc?"), v7);

        char v8[20];
        memset(v8, 0, sizeof(v8));
        CHECK(slave.read(v8, strlen(TEXT)));
        CHECK_EQUAL(string(TEXT), v8);

        _exit(0);
    }
    
    slave.close();

    CHECK_EQUAL(ERROR_NONE, master.startPacket(SANDBOX_MESSAGE_PING));
    master.add((unsigned int) 100);
    master.add((int) 20);
    master.add(10.0f);
    master.add(15.0);
    master.add(true);
    master.add(string("Hello world"));
    master.add("What's up doc?");
    master.add((char*) TEXT, strlen(TEXT));
    CHECK_EQUAL(ERROR_NONE, master.sendPacket());

    int exit_status = 0;
    waitpid(pid, &exit_status, 0);
    
    return WEXITSTATUS(exit_status);
}
