#include "common.h"
#include <fstream>

std::string ws2s(const std::wstring &ws, UINT page)
{
    int len = WideCharToMultiByte(page, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    auto buf = new char[len]{0};
    WideCharToMultiByte(page, 0, ws.c_str(), -1, buf, len, nullptr, nullptr);
    std::string s = buf;
    delete[] buf;
    return s;
}

std::wstring s2ws(const std::string &s, UINT page)
{
    int len = MultiByteToWideChar(page, 0, s.c_str(), -1, nullptr, 0);
    auto buf = new wchar_t[len]{0};
    MultiByteToWideChar(page, 0, s.c_str(), -1, buf, len);
    std::wstring ws = buf;
    delete[] buf;
    return ws;
}

std::string read_file(const std::wstring &path)
{
    std::ifstream in_file(path);
    if (!in_file) {
        throw std::runtime_error("failed to read file: {}"_format(ws2s(path)));
    }
    std::stringstream ss;
    ss << in_file.rdbuf();
    return ss.str();
}

std::pair<int, std::string> exec(const std::wstring &cmd)
{
    SECURITY_ATTRIBUTES sa = {0};
    sa.bInheritHandle = TRUE;
    sa.nLength = sizeof(sa);
    HANDLE hr = NULL;
    HANDLE hw = NULL;
    if (!CreatePipe(&hr, &hw, &sa, 0)) {
        throw std::runtime_error("failed to create pipe");
    }
    SetHandleInformation(hr, HANDLE_FLAG_INHERIT, 0);
    STARTUPINFOW si = {sizeof(si)};
    si.dwFlags = STARTF_FORCEOFFFEEDBACK | STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdError = hw;
    si.hStdOutput = hw;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    PROCESS_INFORMATION pi = {0};
    if (!CreateProcessW(NULL, (LPWSTR)cmd.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL,
                        &si, &pi)) {
        CloseHandle(hr);
        CloseHandle(hw);
        throw std::runtime_error(
            "failed to create process for '{}': {}"_format(ws2s(cmd), GetLastError()));
    }
    CloseHandle(hw);
    std::string result;
    const int bsize = 1024;
    char buf[bsize] = {0};
    for (;;) {
        DWORD n_read = 0;
        BOOL ok = ReadFile(hr, buf, bsize, &n_read, NULL);
        if (!ok || n_read == 0) {
            break;
        }
        result.append(buf, n_read);
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(hr);
    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return {exit_code, result};
}
