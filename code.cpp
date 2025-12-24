/*We are building a cli tool to auto set proxy with minimal steps -- college proxy in wifi setting and also in npm and git*/
#include<iostream>
#include<string>
#include<cstdio> //<-- gives C standard I/O like FILE, _popen, _pclose, fgets

using namespace std;

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

void setGitProxy(const string& proxy)
{
    string cmd1="git config --global http.proxy " +proxy;
    string cmd2="git config --global https.proxy " +proxy;

    system(cmd1.c_str());
    system(cmd2.c_str());
}
void verifyGitProxy() {
    string result =runCommand("git config --global --get http.proxy");

    cout << "Git proxy currently set to:\n"
              << result << "\n";
}

int main()
{
    string output=runCommand("ipconfig");

    if(output.find("172.19.4.1")!=string::npos){
        cout<<"college proxy detected\n";
    }
    else{
        cout<<"college proxy not detected\n";
        return 0;
    }

    string username,password;
    cout<<"Enter proxy username"<<endl;
    cin>>username;
    cout<<"enter proxy password"<<endl;
    cin>>password;

    string proxy="http://"+username+":"+password+"@172.31.2.4:8080";
    cout<<proxy<<endl;

    std::cout << "Setting git proxy...\n";
    setGitProxy(proxy);

    verifyGitProxy();
    return 0;
}