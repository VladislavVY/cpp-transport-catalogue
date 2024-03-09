#include "input_reader.h"

namespace transport {

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}


/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}


/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

                                                                        
/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        auto stops = Split(route, '>');
        std::vector<std::string> route_stops(stops.begin(), stops.end());
        return route_stops;
    }
    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());
    std::vector<std::string> route_stops(results.begin(), results.end());
    return route_stops;
}

    
void FillDistances(std::string& line, Catalogue& catalogue) {
    if (!line.empty()) {
        std::string stop_name = line.substr(1, line.find_first_of(':') - line.find_first_of(' ') - 1);
        line.erase(0, line.find_first_of(',') + 2);
        line.erase(0, line.find_first_of(',') + 2);
        const Stop* first = catalogue.FindStop(stop_name);
        while (!line.empty()) {
            int distanse = 0;
            std::string stop_name_to;
            distanse = std::stoi(line.substr(0, line.find_first_of("m to ")));
            line.erase(0, line.find_first_of("m to ") + 5);
            if (line.find("m to ") == line.npos) {
                stop_name_to = line.substr(0, line.npos - 1);
                const Stop* second = catalogue.FindStop(stop_name_to);
                catalogue.SetStopDistance(first, second, distanse);
                if (!catalogue.GetStopDistance(first, second)) {
                    catalogue.SetStopDistance(second, first, distanse);
                }
                line.clear();
            }
            else {
                stop_name_to = line.substr(0, line.find_first_of(','));
                const Stop* second = catalogue.FindStop(stop_name_to);
                catalogue.SetStopDistance(first, second, distanse);
                if (!catalogue.GetStopDistance(first, second)) {
                    catalogue.SetStopDistance(second, first, distanse);
                }
                line.erase(0, line.find_first_of(',') + 2);
            }
        }
    }
}    

void FillCatalogue(std::istream& input, Catalogue& catalogue) {
    std::vector<std::string> query_bus;
    std::vector<std::string> query_stop;
    std::vector<std::string> query_stop_distances;
    size_t requests_count;
    input >> requests_count;
    for (size_t i = 0; i < requests_count; ++i) {
        std::string command, line;
        input >> command;
        std::getline(input, line);
        if (command == "Stop") {
            query_stop.push_back(line);
        }
        if (command == "Bus") {
            query_bus.push_back(line);
        }
    }
    query_stop_distances = query_stop;
    for (auto& stop : query_stop) {
            std::string stop_name = stop.substr(1, stop.find_first_of(':') - stop.find_first_of(' ') - 1);
            stop.erase(1, stop.find_first_of(':'));
            geo::Coordinates stop_coordinates = ParseCoordinates(stop);
            catalogue.AddStop(std::move(stop_name), stop_coordinates);
    }
    for (auto& stop : query_stop_distances) {
        FillDistances(stop, catalogue);
    }
    for (auto& bus : query_bus) {
        std::string route_number = bus.substr(1, bus.find_first_of(':') - 1);
        bus.erase(0, bus.find_first_of(':') + 2);    
        auto route_stops = ParseRoute(bus);
        catalogue.AddBus(std::move(route_number), std::move(route_stops));
    }
}
}
