#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <sstream>

class RequestHandler {
public:
    RequestHandler(const transport::Catalogue& catalogue, const renderer::MapRenderer& renderer)
        : catalogue_(catalogue)
        , renderer_(renderer)
    {
    }

    svg::Document RenderMap() const;
    std::optional<transport::BusInfo> GetBusStat(const std::string_view bus_number) const;
    const std::set<std::string> GetBusesOnStop(std::string_view stop_name) const;
    bool SearchBusNumber(const std::string_view bus_number) const;
    bool SearchStopName(const std::string_view stop_name) const;

private:
    const transport::Catalogue& catalogue_;
    const renderer::MapRenderer& renderer_;
};
