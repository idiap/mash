#include <sys/stat.h>

int main(int argc, char** argv)
{
    if (argc != 2)
        return -1;
    
    return chmod(argv[1], S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
}
