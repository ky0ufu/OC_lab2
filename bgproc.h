#ifndef PROCESS_LIB_H
#define PROCESS_LIB_H
#include <string>

namespace proc {

struct ProcessHandle {
#ifdef _WIN32
        void* process;
        void* thread;
        unsigned long pid;
#else
        int pid;
#endif
        bool valid;
};


ProcessHandle start(const std::string& command);

int wait(const ProcessHandle& handle);

bool tryWait(ProcessHandle& handle, int& exitCode);

}
#endif