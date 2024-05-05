#pragma once

#include "geo.h"
#include "domain.h"

#include <deque>
#include <map>
#include <optional>
#include <stdexcept>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace transport {

class Catalogue {
public:
    
    void AddStop(std::string_view stop_name, const geo::Coordinates coordinates);
    const Stop* FindStop(std::string_view stop_name) const;
    void AddBus(std::string_view bus_number, const std::vector<const Stop*> stops, bool is_circle);
    const Bus* FindBus(std::string_view bus_number) const;
    size_t GetNumberOfUniqueStops(std::string_view bus_number) const;
    void SetStopDistance(const Stop* from, const Stop* to, const int distance);
    int GetStopDistance(const Stop* from, const Stop* to) const;
    const std::map<std::string_view, const Bus*> GetSortedBuses() const;
    const std::map<std::string_view, const Stop*> GetSortedStops() const;
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

} // namespace transport
