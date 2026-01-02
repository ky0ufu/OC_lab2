#include "bgproc.h"
#include <iostream>
#ifdef _WIN32

#include <windows.h>

namespace proc
{
    ProcessHandle start(const std::string &command)
    {
        ProcessHandle h{};
        h.valid = false;

        STARTUPINFOA si{};
        si.cb = sizeof(si);

        PROCESS_INFORMATION pi{};

        std::string cmd = command;

        if(!CreateProcessA(
            nullptr,
            cmd.data(),
            nullptr, nullptr,
            FALSE,
            0,
            nullptr, nullptr,
            &si, &pi))
            return h;

        h.process = pi.hProcess;
        h.thread = pi.hThread;
        h.pid = pi.dwProcessId;
        h.valid = true;
        return h;
    }

    int wait(const ProcessHandle &h)
    {
        WaitForSingleObject(h.process, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(h.process, &exitCode);

        return static_cast<int>(exitCode);
    }
    
    bool tryWait(ProcessHandle& h, int& exitCode) {
        if (!h.valid) return false;

        DWORD res = WaitForSingleObject(h.process, 0);


        if (res == WAIT_TIMEOUT) return false; // working

        GetExitCodeProcess(h.process, (LPDWORD)&exitCode);
        CloseHandle(h.process);
        CloseHandle(h.thread);
        h.valid = false;
        return true;
    }
}
#else

#include <unistd.h>
#include <sys/wait.h>
namespace proc
{
    ProcessHandle start(const std::string &command)
    {
        ProcessHandle h{};
        h.valid = false;

        pid_t pid = fork();
        if (pid < 0) return h;

        if (pid == 0)
        {
            // дочерний
            execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
            _exit(127);
        }

        // родитель
        h.pid = pid;
        h.valid = true;

        //std::cout << "[START] PID = " << h.pid << "\n";
        return h;
    }

    int wait(const ProcessHandle &h)
    {
        int status = 0;

        waitpid(h.pid, &status, 0);
        if (WIFEXITED(status))
            return  WEXITSTATUS(status);

        return -1;
    }

    bool tryWait(ProcessHandle& h, int& exitCode) {
        if (!h.valid) return false;

        int status = 0;
        pid_t res = waitpid(h.pid, &status, WNOHANG);

        if (res == 0) return false; // working

        if (res == h.pid) {
            exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            h.valid = false;
            return true;
        }
        return false;
    }

} // namespace proc
#endif