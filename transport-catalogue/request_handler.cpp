#include "request_handler.h"

std::optional<transport::BusInfo> RequestHandler::GetBusStat(const std::string_view bus_number) const {
    transport::BusInfo bus_stat{};
    const transport::Bus* bus = catalogue_.FindBus(bus_number);
    if (!bus) {
		throw std::out_of_range("The bus is not in the catalog");
	}
    if (bus->is_roundtrip) {
        bus_stat.stops_count = bus->stops.size();
    }
    else {
        bus_stat.stops_count = bus->stops.size() * 2 - 1;
    }
    int route_length = 0;
    double geographic_length = 0.0;
    for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
        auto from = bus->stops[i];
        auto to = bus->stops[i + 1];
        if (bus->is_roundtrip) {
            route_length += catalogue_.GetStopDistance(from, to);
            geographic_length += geo::ComputeDistance(from->coordinates, to->coordinates);
        }
        else {
            route_length += catalogue_.GetStopDistance(from, to) + catalogue_.GetStopDistance(to, from);
            geographic_length += geo::ComputeDistance(from->coordinates, to->coordinates) * 2;
        }
    }
    bus_stat.unique_stops_count = catalogue_.GetNumberOfUniqueStops(bus_number);
    bus_stat.route_length = route_length;
    bus_stat.curvature = route_length / geographic_length;

    return bus_stat;
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.RenderMap(catalogue_.GetSortedBuses());
}

const std::set<std::string> RequestHandler::GetBusesOnStop(std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name)->buses;
}

bool RequestHandler::SearchBusNumber(const std::string_view bus_number) const {
    return catalogue_.FindBus(bus_number);
}

bool RequestHandler::SearchStopName(const std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name);
}

const std::optional<graph::Router<double>::RouteInfo> RequestHandler::GetRouting(const std::string_view stop_name_from, const std::string_view stop_name_to) const {
    return router_.GetRouter(stop_name_from, stop_name_to);
}

const graph::DirectedWeightedGraph<double>& RequestHandler::GetGraph() const {
    return router_.GetGraph();
}
