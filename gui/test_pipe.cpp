#include <unistd.h>
#include <limits.h>

#include <string>

#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
    // print current working directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
       std::cout << "Current working dir: " << cwd << std::endl;
    } else {
       std::cerr << "getcwd() error" << std::endl;
    }

    // find pipe name in command line arguments (-p)
    std::string pipe_name = "";
    for (int i = 0; i < argc - 1; i++) {
        if (std::string(argv[i]) == "-p") {
            pipe_name = argv[i + 1];
        }
    }

    // if pipe name was provided:
    if (pipe_name != "") {
        std::ofstream pipe(pipe_name.c_str());
        pipe << "hi\n";
    }
}