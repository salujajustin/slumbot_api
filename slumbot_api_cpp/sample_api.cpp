#include <string>
#include <iostream>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "cmdline.h"

#define NUM_STREETS 4
#define SMALL_BLIND 50
#define BIG_BLIND 100
#define STACK_SIZE 20000


std::string login(const std::string &username, const std::string &password) {
 
    boost::property_tree::ptree root;
    root.put("username", username);
    root.put("password", password);
    std::ostringstream oss;
    boost::property_tree::write_json(oss, root);
    std::cout << oss.str() << std::endl;
    
    // Set up client with or without SSL
    // httplib::Client cli("https://slumbot.com");   
    httplib::SSLClient cli("slumbot.com");   
    
    const std::string body = "{\"username\": \"TestBot\", \"password\": \"TestBot\"}";
    auto res = cli.Post("/api/login", oss.str(), "application/json");
    bool success = (res->status == 200) ? 1 : 0;

    if (!success) {
        std::cout << "Status code: " << res->status << std::endl;
        auto err = static_cast<int>(res.error());
        std::cout << "Error response: " << err << std::endl;
        exit(-1);
    }

    std::cout << res->status << std::endl;
    std::cout << res->body << std::endl;

    return "Place token value here";
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
