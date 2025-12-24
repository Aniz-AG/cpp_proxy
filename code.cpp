/*We are building a cli tool to auto set proxy with minimal steps -- college proxy in wifi setting and also in npm and git*/
// ===== Configuration =====
constexpr const char* COLLEGE_GATEWAY = "172.19.4.1";
constexpr const char* PROXY_SERVER = "172.31.2.4:8080";
constexpr const wchar_t* CRED_NAME = L"COLLEGE_PROXY";


#include<iostream>
#include<string>
#include<cstdio> //<-- gives C standard I/O like FILE, _popen, _pclose, fgets
#include <windows.h>
#include <wincred.h>
#pragma comment(lib, "Advapi32.lib")
#include<windows.h>
#include <wininet.h>


using namespace std;

bool loadProxyCredentials(string& username,string& password) {
    PCREDENTIALW cred = nullptr;

    if (!CredReadW(CRED_NAME, CRED_TYPE_GENERIC, 0, &cred)) {
        DWORD err = GetLastError();
        cerr << "CredRead failed. Error code: " << err << "\n";
        return false;
    }

    // Username (WCHAR* → string)
    if (cred->UserName) {
        wstring wsUser(cred->UserName);
        username.assign(wsUser.begin(), wsUser.end());
    }

    // Password: treat blob as UTF-16 WCHAR[]
    if (cred->CredentialBlob && cred->CredentialBlobSize > 0) {
        WCHAR* widePass = reinterpret_cast<WCHAR*>(cred->CredentialBlob);
        size_t charCount = cred->CredentialBlobSize / sizeof(WCHAR);

        wstring wsPass(widePass, charCount);
        password.assign(wsPass.begin(), wsPass.end());
    }

    CredFree(cred);
    return true;
}


string runCommand(const char* cmd){ //we use char* as _popen expects C string , not cpp styles string
    char buffer[256];
    string result;
    // signature: FILE *_popen(const char *command, const char *mode);
    FILE* pipe=_popen(cmd,"r"); //open a pipe to run the command and read its output
    if(!pipe) return "ERROR";
    // fgets() signature: char *fgets(char *buffer, int size, FILE *stream);
    while(fgets(buffer,sizeof(buffer),pipe)){
        result+=buffer;
    }

    _pclose(pipe);
    return result;
}

bool isCollegeNetwork() {
   string output = runCommand("ipconfig");
    return output.find(COLLEGE_GATEWAY) !=string::npos;
}

void setGitProxy(const string& proxy)
{
    string cmd1="git config --global http.proxy " +proxy;
    string cmd2="git config --global https.proxy " +proxy;

    system(cmd1.c_str());
    system(cmd2.c_str());
}

void unsetGitProxy() {
    system("git config --global --unset http.proxy");
    system("git config --global --unset https.proxy");
}

string maskProxyPassword(const string& proxy) 
{ 
    // Find the '@' which separates credentials from host
     size_t atPos = proxy.find('@');
     size_t colonPos = proxy.find(':', proxy.find("://") + 3); 
     if (colonPos != string::npos && atPos != string::npos && colonPos < atPos) 
     { 
        // Keep scheme + username, mask password 
        string prefix = proxy.substr(0, colonPos + 1);  // includes ':' 
        string suffix = proxy.substr(atPos); // includes '@' and host 
        return prefix + "****" + suffix; // replace password with **** 
    } return proxy; 
// no password found 
}

void verifyGitProxy() {
    string result =runCommand("git config --global --get http.proxy");

    cout << "Git proxy currently set to:\n"
              << maskProxyPassword(result) << "\n";
}

void setNpmProxy(const string& proxy) {
    system(("npm config set proxy " + proxy).c_str());
    system(("npm config set https-proxy " + proxy).c_str());
}

void unsetNpmProxy() {
    system("npm config delete proxy");
    system("npm config delete https-proxy");
}

void verifyNpmProxy() {
   string result =
        runCommand("npm config get proxy");

   cout << "NPM proxy currently set to:\n"
              << maskProxyPassword(result) << "\n";
}

void enableWinInetProxy() {
    HKEY hKey;
    const char* subKey ="Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";

    if (RegOpenKeyExA(
            HKEY_CURRENT_USER,
            subKey,
            0,
            KEY_SET_VALUE,
            &hKey
        ) != ERROR_SUCCESS) {
        std::cerr << "Failed to open registry key\n";
        return;
    }

    DWORD enable = 1;
    RegSetValueExA(
        hKey,
        "ProxyEnable",
        0,
        REG_DWORD,
        reinterpret_cast<const BYTE*>(&enable),
        sizeof(enable)
    );

    const char* proxyServer = PROXY_SERVER;
    RegSetValueExA(
        hKey,
        "ProxyServer",
        0,
        REG_SZ,
        reinterpret_cast<const BYTE*>(proxyServer),
        strlen(proxyServer) + 1
    );

    RegCloseKey(hKey);

    // Notify system about the change
    InternetSetOptionA(nullptr, INTERNET_OPTION_SETTINGS_CHANGED, nullptr, 0);
    InternetSetOptionA(nullptr, INTERNET_OPTION_REFRESH, nullptr, 0);
}

void disableWinInetProxy() {
    HKEY hKey;
    const char* subKey =
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";

    if (RegOpenKeyExA(
            HKEY_CURRENT_USER,
            subKey,
            0,
            KEY_SET_VALUE,
            &hKey
        ) != ERROR_SUCCESS) {
        std::cerr << "Failed to open registry key\n";
        return;
    }

    DWORD disable = 0;
    RegSetValueExA(
        hKey,
        "ProxyEnable",
        0,
        REG_DWORD,
        reinterpret_cast<const BYTE*>(&disable),
        sizeof(disable)
    );
    RegCloseKey(hKey);

    InternetSetOptionA(nullptr, INTERNET_OPTION_SETTINGS_CHANGED, nullptr, 0);
    InternetSetOptionA(nullptr, INTERNET_OPTION_REFRESH, nullptr, 0);
}



int main()
{
  if (isCollegeNetwork()) {

    string username, password;
    if (!loadProxyCredentials(username, password)) {
        cerr << "Failed to load proxy credentials.\n";
        return 1;
    }

       string proxy ="http://" + username + ":" + password + "@172.31.2.4:8080";

       cout << "College network detected\n";

        enableWinInetProxy();
        setGitProxy(proxy);
        setNpmProxy(proxy);

        verifyGitProxy();
        verifyNpmProxy();

    } else {
       cout << "Not college network detected\n";
        disableWinInetProxy();
        unsetGitProxy();
        unsetNpmProxy();

        verifyGitProxy();
        verifyNpmProxy();
    }
    return 0;
}

