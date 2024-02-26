#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace transport {

struct Bus {
    std::string number;
    std::optional<std::vector<std::string>> stops;
};

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
    std::set<std::string> buses;
};

struct BusInfo {
    size_t stops_count;
    size_t unique_stops_count;
    double route_length;
};

class Catalogue {
public:
    void AddStop(const std::string stop_name, geo::Coordinates& coordinates);
    const Stop* FindStop(const std::string& stop_name) const;
    void AddBus(const std::string route_number, const std::vector<std::string> route_stops);
    const Bus* FindBus(const std::string& route_number) const;
    const BusInfo GetBusInfo(const std::string& route_number) const;
    const std::set<std::string> GetBusesOnStop(const std::string& stop_name) const;

private:
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
};
}
