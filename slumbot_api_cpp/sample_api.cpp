#include <string>
#include <iostream>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "cmdline.h"
#include "json.hpp"

#define NUM_STREETS 4
#define SMALL_BLIND 50
#define BIG_BLIND 100
#define STACK_SIZE 20000

auto new_hand(std::string &token) {
    // Set up SSL client
    httplib::SSLClient cli("slumbot.com");

    // Send a POST request
    nlohmann::json body_json;
    if (token != "") body_json["token"] = token;
    std::string body = (body_json.is_null()) ? "{}" : body_json.dump();
    auto res = cli.Post("/api/new_hand", body, "application/json");
    bool success = (res->status == 200) ? 1 : 0;
    if (!success) {
        std::cout << "Status code: " << res->status << "\t";
        std::cout << httplib::detail::status_message(res->status) << std::endl;
        auto err = static_cast<int>(res.error());
        std::cout << "Error response: " << err << std::endl;
        exit(-1);
    }

    return res;
}

int play_hand(std::string &token) {
    auto res = new_hand(token);
    auto res_json = nlohmann::json::parse(res->body);

    // If result sends back new token use that
    if (token == "") token = res_json["token"];

    std::cout << "Token: " << token << std::endl;
    while (42) {
        std::cout << "-----------------" << std::endl;
        std::cout << res_json.dump() << std::endl;

        auto action = res_json["action"];
        auto client_pos = res_json["client_pos"];
        auto hole_cards = res_json["hole_cards"];
        auto board = res_json["board"];

        std::cout << "Action: " << action << std::endl;
        std::cout << "Client pos: " << client_pos << std::endl;
        std::cout << "Client hole cards: " << hole_cards << std::endl;
        std::cout << "Board: " << board << std::endl;

        // Need to check or call

        return 0;
    }



    return 0;
}

std::string login(const std::string &username, const std::string &password) {

    // Set up SSL client
    httplib::SSLClient cli("slumbot.com");   

    // Send a POST request
    nlohmann::json body_json = {{"username", username}, {"password", password}};
    auto res = cli.Post("/api/login", body_json.dump(), "application/json");
    bool success = (res->status == 200) ? 1 : 0;
    if (!success) {
        std::cout << "Status code: " << res->status << "\t";
        std::cout << httplib::detail::status_message(res->status) << std::endl;
        auto err = static_cast<int>(res.error());
        std::cout << "Error response: " << err << std::endl;
        exit(-1);
    }

    auto token_json = nlohmann::json::parse(res->body);
    return token_json["token"];
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
        token = "";
    }

    int num_hands = 1;
    int winnings = 0; int hand_winnings = 0;

    for (int h = 0; h < num_hands; ++h) {
        hand_winnings = play_hand(token);
        winnings += hand_winnings;
    }
    std::cout << "Total winnings: " << winnings << std::endl;

}
