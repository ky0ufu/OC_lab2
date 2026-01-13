# Cross-platform background process library
### library can:
- start an external command in a background
- **wait** for the child process to finish and get the exit code
- **check** (non-blocking) whether the process has finished

### Platforms:
- **Windows** (WinAPI: `CreateProcess`, `WaitForSingleObject`, `GetExitCodeProcess`) 
- **POSIX** (`fork`, `exec*`, `waitpid`)

### Build:
##### Windows
```bat
./bulld_win.bat
```

##### POSIX
```sh
./build.sh
```
### Run
##### Windows
```bat
./bulld-win/test_app.exe
```
Commands example:
```bat
Command: ping 127.0.0.1 -n 10 > nul & echo Hello World & exit 7
Wait for process? (1 = yes, 0 = no): 0
```
##### POSIX
```bash
./build/test_app
```
Command example:
```bash
Command: sleep 9; echo Hello World
Wait for process? (1 = yes, 0 = no): 0
```
