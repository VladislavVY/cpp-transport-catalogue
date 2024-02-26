#include "transport_catalogue.h"

namespace transport {

void Catalogue::AddStop(const std::string& stop_name, const geo::Coordinates& coordinates) {
    stops_.push_back({stop_name, coordinates, {}});
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}

    
const Stop* Catalogue::FindStop(const std::string& stop_name) const {
    if (stopname_to_stop_.count(stop_name)) {
        return stopname_to_stop_.at(stop_name);
    } else return nullptr;
}    
    
    
void Catalogue::AddBus(const std::string& route_number, const std::vector<std::string>& route_stops) {
    buses_.push_back({route_number, route_stops});
    for (const auto& route_stop : route_stops) {
        for (auto& stop_ : stops_) {
            if (stop_.name == route_stop) stop_.buses.insert(route_number);
        }
    }
    busname_to_bus_[buses_.back().number] = &buses_.back();
}
      
    
const Bus* Catalogue::FindBus(const std::string& route_number) const {
    if (busname_to_bus_.count(route_number)) {
        return busname_to_bus_.at(route_number);
    } else return nullptr;
}

    
const BusInfo Catalogue::GetBusInfo(const std::string& route_number) const {
    BusInfo bus_info{};
    const Bus* bus = FindBus(route_number);
    bus_info.stops_count = bus->stops->size();
    double route_length = 0.0;
    for (auto iter = bus->stops.value().begin(); iter + 1 != bus->stops.value().end(); ++iter) {
        route_length += ComputeDistance(stopname_to_stop_.find(*iter)->second->coordinates,
        stopname_to_stop_.find(*(iter + 1))->second->coordinates);
    }
    std::unordered_set<std::string> unique_stops;
    for (const auto& stop : busname_to_bus_.at(route_number)->stops.value()) {
        unique_stops.insert(stop);
    }
    bus_info.unique_stops_count = unique_stops.size();
    bus_info.route_length = route_length;
    return bus_info;
}

    
const std::set<std::string> Catalogue::GetBusesOnStop(const std::string& stop_name) const {
    return stopname_to_stop_.at(stop_name)->buses;
}
}
