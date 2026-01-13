#include "bgproc.h"


//  - start(): запускает процесс через CreateProcessA и возвращает хэндлы процесса/потока + PID
//  - wait(): ждёт завершения процесса, читает exit code, закрывает хэндлы
//  - tryWait(): не блокируется; если процесс ещё работает -> false
//              если завершился -> возвращает true и exitCode, закрывает хэндлы

#ifdef _WIN32
#include <windows.h>
#include <vector>

namespace proc {

// returns ProcessHandle с valid=true и заполненными полями:
//  - process: HANDLE процесса
//  - thread:  HANDLE первичного потока
//  - pid:     ID процесса

ProcessHandle start(const std::string& command)
{
    ProcessHandle h{};

    STARTUPINFOA si{};
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi{};

    std::string fullCmd = "cmd.exe /c \"" + command + "\"";

    std::vector<char> cmd(fullCmd.begin(), fullCmd.end());
    cmd.push_back('\0');

    // Пытаемся запустить процесс
    if (!CreateProcessA(
            nullptr,
            cmd.data(),
            nullptr, nullptr,
            FALSE,
            0,
            nullptr, nullptr,
            &si, &pi))
    {
        // Не удалось запустить = возвращаем невалидный handle (valid=false)
        return h;
    }

    h.process = pi.hProcess;
    h.thread = pi.hThread;
    h.pid = pi.dwProcessId;
    h.valid = true;
    return h;
}

// закрывает дескрипторы процесса и потока
static void closeHandle(ProcessHandle& h)
{
    if (h.thread)  { CloseHandle((HANDLE)h.thread);  h.thread  = nullptr; }
    if (h.process) { CloseHandle((HANDLE)h.process); h.process = nullptr; }
    h.valid = false;
}

// Блокируется до завершения процесса.
// Возвращает exit code процесса, либо -1 при ошибке.
int wait(ProcessHandle& h)
{
    if (!h.valid) return -1;

    // Ждём бесконечно пока процесс не завершится
    DWORD res = WaitForSingleObject((HANDLE)h.process, INFINITE);
    if (res == WAIT_FAILED) {
        closeHandle(h);
        return -1;
    }

    // Считываем код завершения процесса
    DWORD exitCode = 0;
    if (!GetExitCodeProcess((HANDLE)h.process, &exitCode)) {
        closeHandle(h);
        return -1;
    }

    closeHandle(h);
    return (int)exitCode;
}

// Не блокирует до завершения процесса.
// Возвращает:
//  - false: процесс ещё работает
//  - true:  процесс завершился ИЛИ произошла ошибка ожидания
bool tryWait(ProcessHandle& h, int& exitCode)
{
    if (!h.valid) return false;

    // timeout=0 , проверяем состояние
    DWORD res = WaitForSingleObject((HANDLE)h.process, 0);

    if (res == WAIT_TIMEOUT) return false; // ещё работает
    if (res == WAIT_FAILED) {
        closeHandle(h);
        exitCode = -1;
        return true; // завершили с ошибкой ожидания
    }

    // процесс завершился
    DWORD code = 0;
    if (!GetExitCodeProcess((HANDLE)h.process, &code)) {
        closeHandle(h);
        exitCode = -1;
        return true;
    }

    exitCode = (int)code;
    closeHandle(h);
    return true;
}

} // namespace proc

#else  // POSIX

#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

namespace proc {

// Создаёт дочерний процесс через fork().
//
// В дочернем процессе выполняем:
// sh -c "<command>"
ProcessHandle start(const std::string& command)
{
    ProcessHandle h{};

    pid_t pid = fork();
    if (pid < 0) return h; // fork() не удался

    if (pid == 0) {
        // дочерний
        execlp("sh", "sh", "-c", command.c_str(), (char*)nullptr);
        _exit(127);
    }

    // родительский процесс
    h.pid = (int)pid;
    h.valid = true;
    return h;
}

static int decode_status(int status)
{
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return -1;
}

int wait(ProcessHandle& h)
{
    if (!h.valid) return -1;

    int status = 0;
    pid_t r;
    do {
        r = waitpid((pid_t)h.pid, &status, 0);
    } while (r == -1 && errno == EINTR);

    h.valid = false;
    if (r == -1) return -1;

    return decode_status(status);
}

bool tryWait(ProcessHandle& h, int& exitCode)
{
    if (!h.valid) return false;

    int status = 0;
    pid_t r = waitpid((pid_t)h.pid, &status, WNOHANG);

    if (r == 0) return false; // ещё работает

    h.valid = false;

    if (r == -1) {
        exitCode = -1;
        return true;
    }

    exitCode = decode_status(status);
    return true;
}

} // namespace proc
#endif
