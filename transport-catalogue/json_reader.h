#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <iostream>

class JsonReader {
public:
    JsonReader(std::istream& input)
        : input_(json::Load(input))
    {}

    const json::Node& GetStatRequests() const;
    const json::Node& GetRenderSettings() const;
    const json::Node& GetRoutingSettings() const;


    void FillCatalogue(transport::Catalogue& catalogue);
    renderer::MapRenderer FillRenderSettings(const json::Node& settings) const;
    void SetBasicSettings(renderer::RenderSettings& settings, const json::Dict& map_request) const;
    void SetLabelSettings(renderer::RenderSettings& settings, const json::Dict& map_request) const;
    void SetUnderlayerColor(renderer::RenderSettings& settings, const json::Dict& map_request) const;
    void SetColorPalette(renderer::RenderSettings& settings, const json::Dict& v) const;
    svg::Color ParseColor(const json::Node& color_node) const;
    transport::TransportRouter::Settings FillRoutingSettings(const json::Node& settings) const;

    void PrintStatRequests(const json::Node& stat_requests, RequestHandler& rh) const;
    const json::Node PrintRoute(const json::Dict& map_request, RequestHandler& rh) const;
    const json::Node PrintStop(const json::Dict& map_request, RequestHandler& rh) const;
    const json::Node PrintMap(const json::Dict& map_request, RequestHandler& rh) const;
    const json::Node CreateRouteItem(const double& time, const std::string& type) const;
    const json::Node PrintRouting(const json::Dict& map_request, RequestHandler& rh) const;

private:
    json::Document input_;
    json::Node nul_ = nullptr;

    std::tuple<std::string_view, geo::Coordinates, std::map<std::string_view, int>> FillStop(const json::Dict& map_request) const;
    void FillStopDistances(transport::Catalogue& catalogue, const json::Dict& map_request) const;
    std::tuple<std::string_view, std::vector<const transport::Stop*>, bool> FillRoute(const json::Dict& map_request, transport::Catalogue& catalogue) const;
};
