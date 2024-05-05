#include "transport_router.h"

namespace transport {

const graph::DirectedWeightedGraph<double>& TransportRouter::BuildGraph(const Catalogue& catalogue) {
    BuildStopsGraph(catalogue);
    BuildBusesGraph(catalogue);
    return graph_;
}

void TransportRouter::BuildStopsGraph(const Catalogue& catalogue) {
    const auto& all_stops = catalogue.GetSortedStops();
    graph::DirectedWeightedGraph<double> graph_stops(all_stops.size() * 2);
    std::map<std::string, graph::VertexId> stop_id;
    graph::VertexId vertex_id = 0;
    std::string type = "Stop";
    for (const auto& [stop_name, info] : all_stops) {
        stop_id[info->name] = vertex_id;
        graph_stops.AddEdge({type, vertex_id, ++vertex_id, static_cast<double>(bus_wait_time_)});
        ++vertex_id;
    }
    graph_ = std::move(graph_stops);
    stop_id_ = std::move(stop_id); 
}

void TransportRouter::BuildBusesGraph(const Catalogue& catalogue) {
    const auto& all_buses = catalogue.GetSortedBuses();
    for (const auto& [_, info] : all_buses) {
        const auto& stops = info->stops;
        size_t stops_count = stops.size();
        std::string type = "Bus";
        for (size_t i = 0; i < stops_count; ++i) {
            for (size_t j = i + 1; j < stops_count; ++j) {
                const Stop* stop_from = stops[i];
                const Stop* stop_to = stops[j];
                int dist = 0;
                int dist_reverse = 0;
                for (size_t k = i + 1; k <= j; ++k) {
                    dist += catalogue.GetStopDistance(stops[k - 1], stops[k]); dist_reverse += catalogue.GetStopDistance(stops[k], stops[k - 1]);
                }
                graph_.AddEdge({type, stop_id_.at(stop_from->name) + 1, stop_id_.at(stop_to->name), static_cast<double>(dist) / (bus_velocity_ * K_MH_TO_M_MIN)});
                if (!info->is_roundtrip) {
                    graph_.AddEdge({type, stop_id_.at(stop_to->name) + 1, stop_id_.at(stop_from->name), static_cast<double>(dist_reverse) / (bus_velocity_ * K_MH_TO_M_MIN)});
                }
            }
        }
    }
    router_ = std::make_unique<graph::Router<double>>(graph_);
}

const std::optional<graph::Router<double>::RouteInfo> TransportRouter::GetRouter(const std::string_view stop_from, const std::string_view stop_to) const {
	return router_->BuildRoute(stop_id_.at(std::string(stop_from)),stop_id_.at(std::string(stop_to)));
}

const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
	return graph_;
}
} // namespace transport
