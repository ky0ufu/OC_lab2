#ifndef PROCESS_LIB_H
#define PROCESS_LIB_H

#include <string>

namespace proc {

struct ProcessHandle {
#ifdef _WIN32
    void* process = nullptr;
    void* thread  = nullptr;
    unsigned long pid = 0;
#else
    int pid = -1;
#endif
    bool valid = false;
};

ProcessHandle start(const std::string& command);

int wait(ProcessHandle& handle);

bool tryWait(ProcessHandle& handle, int& exitCode);

} // namespace proc
#endif
