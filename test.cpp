#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

#include "bgproc.h"

std::vector<proc::ProcessHandle> background;
std::mutex bgMutex;
std::atomic<bool> running{true};

void monitorThread()
{
    while (running)
    {
        std::lock_guard<std::mutex> lock(bgMutex);

        for (auto it = background.begin(); it != background.end();)
        {
            int exitCode;
            if (proc::tryWait(*it, exitCode))
            {
                std::cout << "[END] PID = " << it->pid
                          << ", exit code =" << exitCode << "\n";
                it = background.erase(it);
            }
            else
            {
                ++it;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main()
{

    std::thread monitor(monitorThread);

    while (true)
    {
        std::string cmd;
        std::cout << "\nCommand: ";
        std::getline(std::cin, cmd);

        if (cmd == "exit")
            break;

        int waitFlag;
        std::cout << "Wait for process? (1 = yes, 0 = n0):";
        std::cin >> waitFlag;
        std::cin.ignore();

        auto h = proc::start(cmd);

        if (!h.valid)
        {
            std::cout << "Failed to start process\n";
            continue;
        }

        std::cout << "[START] PID = " << h.pid << "\n";

        if (waitFlag)
        {
            int code = proc::wait(h);
            std::cout << "[END] PID = " << h.pid
                      << ", exit code =" << code << "\n";
        }
        else
        {
            std::lock_guard<std::mutex> lock(bgMutex);
            background.push_back(h);
            std::cout << "Process started in background\n";
        }
    }
    running = false;
    monitor.join();
    std::cout << "Done\n";
    return 0;
}