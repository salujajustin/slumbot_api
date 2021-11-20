#include <string>
#include <iostream>

#include "cmdline.h"

#define NUM_STREETS 4
#define SMALL_BLIND 50
#define BIG_BLIND 100
#define STACK_SIZE 20000

int main(int argc, char *argv[]) {
    // Create a parser    
    cmdline::parser p;
    
    // Arguments: (long name, short name, description, mandatory, default value, extra constraint)
    p.add<std::string>("username", 'u', "Username registered on host", false, "TestBot");
    p.add<std::string>("password", 'p', "Password for the username", false, "TestBot");

    // Run parser
    p.parse_check(argc, argv);

    std::cout << p.get<std::string>("username") << std::endl;

}
