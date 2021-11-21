#include <string>
#include <iostream>
#include <sstream>

#include "cmdline.h"
#include "HTTPRequest.hpp"

#define NUM_STREETS 4
#define SMALL_BLIND 50
#define BIG_BLIND 100
#define STACK_SIZE 20000

std::string login(const std::string &username, const std::string &password) {
    
    
    try
    {
        http::Request request{"http://slumbot.com/api/login"};
        std::ostringstream oss;
        oss << "{\"username\": " << username << ", \"password\": " << password << "}";
        const std::string body = oss.str(); 
        const auto response = request.send("POST", body, {
            "Content-Type: application/json"
        });

        if (response.status != http::Response::Ok) {
            std::cout << "Status code: " << response.status << std::endl;
            std::cout << "Error desc: " << response.description << std::endl;
            exit(-1);
        } 
        std::cout << std::string{response.body.begin(), response.body.end()} << '\n'; // print the result
    }
    catch (const std::exception& e)
    {
        std::cerr << "Request failed, error: " << e.what() << '\n';
    }  
    return "guh";
}

int main(int argc, char *argv[]) {
    // Create a parser    
    cmdline::parser p;
    
    // Arguments: (long name, short name, description, mandatory, default value, extra constraint)
    p.add<std::string>("username", 'u', "Username registered on host", false, "TestBot");
    p.add<std::string>("password", 'p', "Password for the username", false, "TestBot");

    // Run parser
    p.parse_check(argc, argv);

    // Login if username and password are provided
    std::string token;
    if (p.exist("username") && p.exist("password")) {
        token = login(p.get<std::string>("username"), p.get<std::string>("password"));
    } else {
        token = "None";
    }
        

}
