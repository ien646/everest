#include <ien/io_utils.hpp>

#include <fcntl.h>
#include <span>

#include "common.h"

constexpr const char* PIPE_PATH = "/tmp/everest_pipe";

int main(int argc, char** argv)
{
    if(argc <= 1)
    {
        std::cerr << "No arguments for command. Aborting..." << std::endl;
        return 0;
    }
    std::span<char*> sargv = {argv, static_cast<size_t>(argc)};

    std::vector<std::string> args;
    for(int i = 1; i < argc; ++i)
    {
        args.push_back(sargv[i]);
    }
    std::string cmd = join_strings(args, 0, ' ');
    cmd += "\n";

    int fd = open(PIPE_PATH, O_WRONLY | O_NONBLOCK, 0);
    write(fd, cmd.data(), cmd.size());
}