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

void catch_error(std::string msg) {
    std::cerr << "Error: " << msg << std::endl;
    exit(-1);
}

nlohmann::json parse_action(std::string &action) {
    int st = 0;                                 // street
    int street_last_bet_to = BIG_BLIND;         // total chips bet on this street
    int total_last_bet_to = BIG_BLIND;          // total chips bet
    int last_bet_size = BIG_BLIND - SMALL_BLIND;
    int last_bettor = 0;
    int sz = action.length();
    int pos = 1;                                // -1 when hand is over

    nlohmann::json info_json;

    if (sz == 0) {
        info_json["st"] = st;
        info_json["pos"] = pos;
        info_json["street_last_bet_to"] = street_last_bet_to;
        info_json["total_last_bet_to"] = total_last_bet_to;
        info_json["last_bet_size"] = last_bet_size;
        info_json["last_bettor"] = last_bettor;
        return info_json;
    }

    bool check_or_call_ends_street = false;
    int i = 0;
    while (i < sz) {
        if (st >= NUM_STREETS) catch_error("Street exceeds 4");
        char c = action[i];     // the first character
        i++;
        switch (c) {
            case 'k' : {
                if (last_bet_size > 0) catch_error("Illegal check");
                // After a check that ends a pre-river street, expect either a '/' or end of string
                if (check_or_call_ends_street) {
                    if (st < (NUM_STREETS - 1) && i < sz) {
                        if (action[i] != '/') catch_error("Missing slash");
                        i++;
                    } else if (st == NUM_STREETS - 1) {
                        pos = -1;
                    } else {
                        pos = 0;
                        st++;
                    }
                    street_last_bet_to = 0;
                    check_or_call_ends_street = false;
                } else {
                    pos = (pos + 1) % 2;
                    check_or_call_ends_street = true;
                }
                break;
            }
            case 'c' : {
                if (last_bet_size == 0) catch_error("Illegal call");
                if (total_last_bet_to == STACK_SIZE) {
                    // Call of an all-in bet
                    // Either allow no slashes, or slashes temination all street prior to the river
                    if (i != sz) {
                        for (int st1 = 0; st < NUM_STREETS; st1++) {
                            if (i == sz) {
                                catch_error("Missing slash (end of string)");
                            } else {
                                c = action[i];
                                i++;
                                if (c != '/') catch_error("Missing slash");
                            }
                        }
                    }
                    if (i != sz) catch_error("Extra characters at end of action");
                    st = NUM_STREETS - 1;
                    pos = -1;
                    last_bet_size = 0;
                } else if (check_or_call_ends_street) {
                    if ((st < NUM_STREETS - 1) && (i < sz)) {
                        if (action[i] != '/') catch_error("Missing slash");
                        i++;
                    } else if (st == NUM_STREETS - 1) {
                        pos = -1;
                    } else {
                        pos = 0;
                        st++;
                    }
                    street_last_bet_to = 0;
                    check_or_call_ends_street = false;
                } else {
                    pos = (pos + 1) % 2;
                    check_or_call_ends_street = true;
                }
                last_bet_size = 0;
                last_bettor = -1;
                break;
            }
            case 'f' : {
                if (last_bet_size == 0) catch_error("Illegal fold");
                if (i != sz) catch_error("Extra characters at end of action");
                pos = -1;
                break;
            }
            case 'b' : {
                int j = i;
                while (i < sz && action[i] >= '0' && action[i] <= '9') i++;
                if (i == j) catch_error("Missing bet size");
                int new_street_last_bet_to = std::stoi(action.substr(j,i));
                int new_last_bet_size = new_street_last_bet_to - street_last_bet_to;

                // Validate that the bet is legal
                int remaining = STACK_SIZE - street_last_bet_to;
                int min_bet_size = (last_bet_size > 0) ? last_bet_size : BIG_BLIND;
                if (min_bet_size < BIG_BLIND) min_bet_size = BIG_BLIND;

                // Can always go all in
                if (min_bet_size > remaining) min_bet_size = remaining;
                if (new_last_bet_size < min_bet_size) catch_error("Bet too small");
                int max_bet_size = remaining;
                if (new_last_bet_size > max_bet_size) catch_error("Bet too big");

                // Update
                last_bet_size = new_last_bet_size;
                street_last_bet_to = new_street_last_bet_to;
                total_last_bet_to += last_bet_size;
                last_bettor = pos;
                pos = (pos + 1) % 2;
                check_or_call_ends_street = true;
                break;
            }
            default :
                catch_error("Unexpected character in action");
        }
    }

    info_json["st"] = st;
    info_json["pos"] = pos;
    info_json["street_last_bet_to"] = street_last_bet_to;
    info_json["total_last_bet_to"] = total_last_bet_to;
    info_json["last_bet_size"] = last_bet_size;
    info_json["last_bettor"] = last_bettor;
    return info_json;
}

std::string take_sample_action(nlohmann::json &info_json) {
    // Naive action to always check or call
    std::string incr = (info_json["last_bettor"] != -1) ? "c" : "k";
    return incr;
}

auto act(std::string &token, std::string &action) {

    // Set up SSL client
    httplib::SSLClient cli("slumbot.com");

    // Send a POST request
    nlohmann::json body_json = {{"token", token}, {"incr", action}};
    auto res = cli.Post("/api/act", body_json.dump(), "application/json");
    bool success = (res->status == 200) ? 1 : 0;
    if (!success) {
        std::cout << "Status code: " << res->status << "\t";
        std::cout << httplib::detail::status_message(res->status) << std::endl;
        auto err = static_cast<int>(res.error());
        std::cout << "Error response: " << err << std::endl;
        exit(-1);
    }

    std::cout << res->body << std::endl;
    return res;
}

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

        std::string action = res_json["action"];
        int client_pos = res_json["client_pos"];
        std::vector<std::string> hole_cards = res_json["hole_cards"];
        std::vector<std::string> board = res_json["board"];

        std::cout << "Action: " << res_json["action"] << std::endl;
        std::cout << "Client pos: " << res_json["client_pos"] << std::endl;
        std::cout << "Client hole cards: " << res_json["hole_cards"] << std::endl;
        std::cout << "Board: " << res_json["board"] << std::endl;

        if (!res_json["winnings"].is_null()) {
            auto winnings = res_json["winnings"];
            std::cout << "Hand winnings: " << winnings << std::endl;
            return winnings;
        }

        // Need to check or call
        nlohmann::json info_json = parse_action(action);

        // IMPLEMENT COMPUTED STRATEGY HERE
        std::string incr = take_sample_action(info_json);

        // Send action to API
        res = act(token, incr);
        res_json = nlohmann::json::parse(res->body);
    }
    // Should never get here
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
