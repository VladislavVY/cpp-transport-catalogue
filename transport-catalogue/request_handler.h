#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <sstream>

class RequestHandler {
public:
    RequestHandler(const transport::Catalogue& catalogue, const renderer::MapRenderer& renderer, const transport::TransportRouter& router)
        : catalogue_(catalogue)
        , renderer_(renderer)
        , router_(router)
    {
    }

    svg::Document RenderMap() const;
    std::optional<transport::BusInfo> GetBusStat(const std::string_view bus_number) const;
    const std::set<std::string> GetBusesOnStop(std::string_view stop_name) const;
    bool SearchBusNumber(const std::string_view bus_number) const;
    bool SearchStopName(const std::string_view stop_name) const;
    const std::optional<graph::Router<double>::RouteInfo> GetRouting(const std::string_view stop_name_from, const std::string_view stop_name_to) const;
    const graph::DirectedWeightedGraph<double>& GetGraph() const;

private:
    const transport::Catalogue& catalogue_;
    const renderer::MapRenderer& renderer_;
    const transport::TransportRouter& router_;
};
