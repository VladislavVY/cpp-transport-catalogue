#pragma once

#include "geo.h"

#include <set>
#include <string>
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
    bool is_roundtrip;
};

struct BusInfo {
    size_t stops_count;
    size_t unique_stops_count;
    double route_length;
    double curvature;
};

} // namespace transport
