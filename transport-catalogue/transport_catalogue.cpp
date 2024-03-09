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
    std::vector<const Stop*> stops;
	for (auto& stop : route_stops) {
		const auto& st = FindStop(stop);
		stops.push_back(st);
	} 
	buses_.push_back({std::string(route_number), stops});
    for (const auto& route_stop : stops) {
        for (auto& stop_ : stops_) {
            if (stop_.name == route_stop->name) stop_.buses.insert(std::string(route_number));
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
    bus_info.stops_count = bus->stops.size();
    double route_length = 0.0;
    double geographic_length = 0.0;
    for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
        auto first = bus->stops[i];
        auto second = bus->stops[i + 1];
        route_length += GetStopDistance(first, second);
        geographic_length += geo::ComputeDistance(first->coordinates, second->coordinates);
    }
    std::unordered_set<std::string_view> unique_stops;
    for (const auto& stop : busname_to_bus_.at(route_number)->stops) {
        unique_stops.insert(stop->name);
    }
    bus_info.unique_stops_count = unique_stops.size();
    bus_info.route_length = route_length;
    bus_info.curvature = route_length / geographic_length;
    return bus_info;
}

    
const std::set<std::string> Catalogue::GetBusesOnStop(const std::string& stop_name) const {
    return stopname_to_stop_.at(stop_name)->buses;
}
    
void Catalogue::SetStopDistance(const Stop* first, const Stop* second, const int distance) {
    stop_distances_[{first, second}] = distance;
}

int Catalogue::GetStopDistance(const Stop* first, const Stop* second) const {
    if (stop_distances_.count({first, second})){        
        return stop_distances_.at({first, second});
    } else if (stop_distances_.count({second, first})){        
        return stop_distances_.at({second, first});
    } else return 0;
}    
}
