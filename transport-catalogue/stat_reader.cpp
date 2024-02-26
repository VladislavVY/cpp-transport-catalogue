#include "stat_reader.h"

#include <iomanip>

namespace transport {

void ParseAndPrintStat(const Catalogue& transport_catalogue, std::ostream& output) {
    size_t requests_count;
    std::cin >> requests_count;
    for (size_t i = 0; i < requests_count; ++i) {
        std::string command, line;
        std::cin >> command;
        std::getline(std::cin, line);
        if (command == "Bus") {
            std::string route_number = line.substr(1, line.npos);
            if (transport_catalogue.FindBus(route_number)) {
                output << "Bus " << route_number << ": " << transport_catalogue.GetBusInfo(route_number).stops_count << " stops on route, "
                       << transport_catalogue.GetBusInfo(route_number).unique_stops_count << " unique stops, " << std::setprecision(6)
                       << transport_catalogue.GetBusInfo(route_number).route_length << " route length" << std::endl;
            }
            else {
                output << "Bus " << route_number << ": not found" << std::endl;
            }
        }
        if (command == "Stop") {
            std::string stop_name = line.substr(1, line.npos);
            if (transport_catalogue.FindStop(stop_name)) {
               output << "Stop " << stop_name << ": ";
                std::set<std::string> buses = transport_catalogue.GetBusesOnStop(stop_name);
                if (!buses.empty()) {
                   output << "buses ";
                    for (const auto& bus : buses) {
                        output << bus << " ";
                    }
                    output << std::endl;
                }
                else {
                    output << "no buses" << std::endl;
                }
            }
            else {
                output << "Stop " << stop_name << ": not found" << std::endl;
            }
        }
    }
}
}
