/*We are building a cli tool to auto set proxy with minimal steps -- college proxy in wifi setting and also in npm and git*/
#include<iostream>
#include<string>
#include<cstdio> //<-- gives C standard I/O like FILE, _popen, _pclose, fgets
#include <windows.h>
#include <wincred.h>
#pragma comment(lib, "Advapi32.lib")

using namespace std;

bool loadProxyCredentials(
    std::string& username,
    std::string& password
) {
    PCREDENTIALW cred = nullptr;

    if (!CredReadW(L"COLLEGE_PROXY", CRED_TYPE_GENERIC, 0, &cred)) {
        DWORD err = GetLastError();
        std::cerr << "CredRead failed. Error code: " << err << "\n";
        return false;
    }

    // Username (WCHAR* → std::string)
    if (cred->UserName) {
        std::wstring wsUser(cred->UserName);
        username.assign(wsUser.begin(), wsUser.end());
    }

    // Password: treat blob as UTF-16 WCHAR[]
    if (cred->CredentialBlob && cred->CredentialBlobSize > 0) {
        WCHAR* widePass = reinterpret_cast<WCHAR*>(cred->CredentialBlob);
        size_t charCount = cred->CredentialBlobSize / sizeof(WCHAR);

        std::wstring wsPass(widePass, charCount);
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
    return output.find("172.19.4.1") !=string::npos;
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

void verifyGitProxy() {
    string result =runCommand("git config --global --get http.proxy");

    cout << "Git proxy currently set to:\n"
              << result << "\n";
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
              << result << "\n";
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

        setGitProxy(proxy);
        setNpmProxy(proxy);

        verifyGitProxy();
        verifyNpmProxy();

    } else {
       cout << "Not college network detected\n";

        unsetGitProxy();
        unsetNpmProxy();

        verifyGitProxy();
        verifyNpmProxy();
    }
    return 0;
}

