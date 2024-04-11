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
#include <unistd.h>
#include <unordered_map>

#include "common.h"

constexpr const char* PIPE_PATH = "/tmp/everest_pipe";
constexpr const size_t CMD_MAX_LENGTH = 127;
constexpr const size_t PIPE_POLL_PERIOD_MS = 200;

int main(int argc, char** argv)
{
    bool no_root = false;
    if(argc > 1)
    {
        const std::span<char*> sargv(argv, argc);
        if(std::string{sargv[1]} == "--no-root")
        {
            no_root = true;
        }
        else
        {
            std::cout << "Unknown argument: " << sargv[1] << std::endl;
        }
    }

    if(!no_root && geteuid() != 0)
    {
        std::cerr << "Everest server should be run as root, unless --no-root is supplied" << std::endl;
        return -1;
    }

    const std::optional<std::string> command_text = ien::read_file_text("/etc/everest/commands.txt");
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
        try
        {
            std::filesystem::remove(PIPE_PATH);
        }
        catch(std::exception& ex)
        {
            std::cerr << "Unable to remove previous named pipe at: " << PIPE_PATH << std::endl
                << "Check if you have permissions to remove it" << std::endl
                << "Ex: " << ex.what() << std::endl;
            return -1;
        }
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
            std::this_thread::sleep_for(std::chrono::milliseconds(PIPE_POLL_PERIOD_MS));
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
        }
    }

    return 0;
}