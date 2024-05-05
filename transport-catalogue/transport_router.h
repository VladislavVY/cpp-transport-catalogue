#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>

    
namespace transport {

constexpr static double K_MH_TO_M_MIN = 1000.0 / 60.0;
    
class TransportRouter {
    
public:
    struct Settings {
        int bus_wait_time = 0;
        double bus_velocity = 0.0;
    };

    TransportRouter() = default;

    explicit TransportRouter(const Settings& settings, const Catalogue& catalogue)
        : settings_(settings) {
        BuildGraph(catalogue);
    }
    
    using RouteInfo = graph::Router<double>::RouteInfo;
    const std::optional<RouteInfo> FindRoute(const std::string_view stop_from, const std::string_view stop_to) const;
    
    const graph::DirectedWeightedGraph<double>& GetGraph() const;

private:
    Settings settings_;
    const graph::DirectedWeightedGraph<double>& BuildGraph(const Catalogue& catalogue);
    void BuildStopsGraph(const Catalogue& catalogue);
    void BuildBusesGraph(const Catalogue& catalogue);
    
    graph::DirectedWeightedGraph<double> graph_;
    std::map<std::string, graph::VertexId> stop_id_;
    std::unique_ptr<graph::Router<double>> router_;
};
} // namespace transport
