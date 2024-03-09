#pragma once

#include "geo.h"

#include <deque>
#include <optional>
#include <stdexcept>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace transport {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
    std::set<std::string> buses;
};    
    
struct Bus {
    std::string number;
    std::vector<const Stop*> stops;
};

struct BusInfo {
    size_t stops_count;
    size_t unique_stops_count;
    double route_length;
    double curvature;
};

class Catalogue {
public:
    void AddStop(const std::string& stop_name, const geo::Coordinates& coordinates);
    const Stop* FindStop(const std::string& stop_name) const;
    void AddBus(const std::string& route_number, const std::vector<std::string>& route_stops);
    const Bus* FindBus(const std::string& route_number) const;
    const BusInfo GetBusInfo(const std::string& route_number) const;
    const std::set<std::string> GetBusesOnStop(const std::string& stop_name) const;
    void SetStopDistance(const Stop* from, const Stop* to, const int distance);
    int GetStopDistance(const Stop* from, const Stop* to) const;
    struct StopDistancesHasher {
        size_t operator()(const std::pair<const Stop*, const Stop*>& points) const {
            size_t hash_first = std::hash<const void*>{}(points.first);
            size_t hash_second = std::hash<const void*>{}(points.second);
            return hash_first + hash_second * 37;
        }
    };

private:
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopDistancesHasher> stop_distances_;
};
}
