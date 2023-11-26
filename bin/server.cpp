#include <ien/io_utils.hpp>
#include <ien/str_utils.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <format>
#include <sys/stat.h>
#include <thread>
#include <unordered_map>

#include "common.h"

constexpr const char* PIPE_PATH = "/tmp/everest_pipe";
constexpr const size_t CMD_MAX_LENGTH = 127;

int main()
{
    const std::optional<std::string> command_text = ien::read_file_text("commands.txt");
    if(!command_text)
    {
        std::cerr << "commands.txt file not found!" << std::endl;
        return -1;
    }
    std::unordered_map<std::string, std::string> commands = get_commands_from_text(*command_text);

    // Clear file creation mask, so that created files don't inherit permissions.
    // Otherwise, pipes created with mkfifo won't be writable by other users.
    umask(0000);

    if(std::filesystem::exists(PIPE_PATH))
    {
        std::filesystem::remove(PIPE_PATH);
    }

    constexpr int PIPE_PERMISSIONS = 0777;
    if(mkfifo(PIPE_PATH, PIPE_PERMISSIONS))
    {
        std::cerr << "failure creating pipe at " << PIPE_PATH << std::endl;
        return -1;
    }
    int fd = open(PIPE_PATH, O_RDONLY | O_NONBLOCK, 0);

    std::string cmd;
    while(true)
    {
        std::array<char, CMD_MAX_LENGTH + 1> buff = {};

        std::memset(buff.data(), 0, CMD_MAX_LENGTH);
        ssize_t bytes_read = read(fd, buff.data(), CMD_MAX_LENGTH);
        if(bytes_read <= 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        std::string partial(buff.data());
        if(partial.ends_with('\n'))
        {
            cmd += partial.substr(0, partial.size() - 1);
            if(!commands.count(cmd))
            {
                std::cerr << "Command '" << cmd << " is not a valid command" << std::endl;
                cmd = {};
                continue;
            }
            const auto& exec_cmd = commands.at(cmd);
            system(exec_cmd.c_str());
            cmd = {};
        }
        else
        {
            cmd += partial; 
            continue;
        }
        continue;
    }

    return 0;
}