#include "json_reader.h"
#include "json_builder.h"

#include <iostream>

const json::Node& JsonReader::GetStatRequests() const {
    if (!input_.GetRoot().AsDict().count("stat_requests")) {
        return nul_;
    }
    return input_.GetRoot().AsDict().at("stat_requests");
}

const json::Node& JsonReader::GetRenderSettings() const {
    if (!input_.GetRoot().AsDict().count("render_settings")) {
        return nul_;
    }
    return input_.GetRoot().AsDict().at("render_settings");
}

const json::Node& JsonReader::GetRoutingSettings() const {
    if (!input_.GetRoot().AsDict().count("routing_settings")) {
        return nul_;
    }
    return input_.GetRoot().AsDict().at("routing_settings");
}

void JsonReader::FillCatalogue(transport::Catalogue& catalogue) {
    if (input_.GetRoot().AsDict().count("base_requests") > 0) {
    const auto& base_requests = input_.GetRoot().AsDict().at("base_requests").AsArray();
    for (auto& request : base_requests) {
        const auto& map_request = request.AsDict();
        const auto& type = map_request.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = FillStop(map_request);
            catalogue.AddStop(stop_name, coordinates);
        }
    }
    for (auto& request : base_requests) {
        const auto& map_request = request.AsDict();
        const auto& type = map_request.at("type").AsString();
        if (type == "Stop") {
            FillStopDistances(catalogue, map_request);
        }
    }
    for (auto& request : base_requests) {
        const auto& map_request = request.AsDict();
        const auto& type = map_request.at("type").AsString();
        if (type == "Bus") {
            auto [bus_number, stops, circular_route] = FillRoute(map_request, catalogue);
            catalogue.AddBus(bus_number, stops, circular_route);
        }
    }
}
}

std::tuple<std::string_view, geo::Coordinates, std::map<std::string_view, int>> JsonReader::FillStop(const json::Dict& map_request) const {
    std::string_view stop_name = map_request.at("name").AsString();
    geo::Coordinates coordinates = { map_request.at("latitude").AsDouble(), map_request.at("longitude").AsDouble() };
    std::map<std::string_view, int> stop_distances;
    auto& distances = map_request.at("road_distances").AsDict();
    for (auto& [stop_name, dist] : distances) {
        stop_distances.emplace(stop_name, dist.AsInt());
    }
    return std::make_tuple(stop_name, coordinates, stop_distances);
}

void JsonReader::FillStopDistances(transport::Catalogue& catalogue, const json::Dict& map_request) const {
    auto [stop_name, coordinates, stop_distances] = FillStop(map_request);
    for (auto& [to_name, dist] : stop_distances) {
        auto from = catalogue.FindStop(stop_name);
        auto to = catalogue.FindStop(to_name);
        catalogue.SetStopDistance(from, to, dist);
    }
}


std::tuple<std::string_view, std::vector<const transport::Stop*>, bool> JsonReader::FillRoute(const json::Dict& map_request, transport::Catalogue& catalogue) const {
    std::string_view bus_number = map_request.at("name").AsString();
    bool circular_route = map_request.at("is_roundtrip").AsBool();
    std::vector<const transport::Stop*> stops;
    for (auto& stop : map_request.at("stops").AsArray()) {
        stops.push_back(catalogue.FindStop(stop.AsString()));
    }
    return std::make_tuple(bus_number, stops, circular_route);
}

renderer::MapRenderer JsonReader::FillRenderSettings(const json::Node& settings) const {
    json::Dict map_request = settings.AsDict();
    renderer::RenderSettings render_settings;
    SetBasicSettings(render_settings, map_request);
    SetLabelSettings(render_settings, map_request);
    SetUnderlayerColor(render_settings, map_request);
    SetColorPalette(render_settings, map_request);
    return render_settings;
}

void JsonReader::SetBasicSettings(renderer::RenderSettings& settings, const json::Dict& map_request) const {
    settings.width = map_request.at("width").AsDouble();
    settings.height = map_request.at("height").AsDouble();
    settings.padding = map_request.at("padding").AsDouble();
    settings.stop_radius = map_request.at("stop_radius").AsDouble();
    settings.line_width = map_request.at("line_width").AsDouble();
    settings.underlayer_width = map_request.at("underlayer_width").AsDouble();
}

void JsonReader::SetLabelSettings(renderer::RenderSettings& settings, const json::Dict& map_request) const {
    settings.bus_label_font_size = map_request.at("bus_label_font_size").AsInt();
    auto bus_label_offset = map_request.at("bus_label_offset").AsArray();
    settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };

    settings.stop_label_font_size = map_request.at("stop_label_font_size").AsInt();
    auto stop_label_offset = map_request.at("stop_label_offset").AsArray();
    settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };
}

void JsonReader::SetUnderlayerColor(renderer::RenderSettings& settings, const json::Dict& map_request) const {
    auto underlayer_color = map_request.at("underlayer_color");
    if (underlayer_color.IsString()) {
        settings.underlayer_color = underlayer_color.AsString();
    } else if (underlayer_color.IsArray()) {
        auto color_array = underlayer_color.AsArray();
        settings.underlayer_color = ParseColor(color_array);
    } else {
        throw std::logic_error("Unsupported underlayer color format");
    }
}

void JsonReader::SetColorPalette(renderer::RenderSettings& settings, const json::Dict& map_request) const {
    auto color_palette = map_request.at("color_palette").AsArray();
    for (const auto& color_element : color_palette) {
        settings.color_palette.push_back(ParseColor(color_element));
    }
}

svg::Color JsonReader::ParseColor(const json::Node& color_node) const {
    if (color_node.IsString()) {
        return color_node.AsString();
    } else if (color_node.IsArray()) {
        auto color_array = color_node.AsArray();
        if (color_array.size() == 3) {
            return svg::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt());
        } else if (color_array.size() == 4) {
            return svg::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble());
        }
    }
    throw std::logic_error("Unsupported color format");
}

transport::TransportRouter JsonReader::FillRoutingSettings(const json::Node& settings) const {
    const auto& wait_time = settings.AsDict().at("bus_wait_time").AsInt();
    const auto& velocity = settings.AsDict().at("bus_velocity").AsDouble();
    return transport::TransportRouter{wait_time, velocity};
}

void JsonReader::PrintStatRequests(const json::Node& stat_requests, RequestHandler& req_hand) const {
    json::Array result;
    for (auto& request : stat_requests.AsArray()) {
        const auto& map_request = request.AsDict();       
        if (map_request.at("type").AsString() == "Stop") {
           result.push_back(PrintStop(map_request, req_hand).AsDict());
        }
        if (map_request.at("type").AsString() == "Bus") {
           result.push_back(PrintRoute(map_request, req_hand).AsDict());
        }
        if (map_request.at("type").AsString() == "Map") {
           result.push_back(PrintMap(map_request, req_hand).AsDict());
        }
        if (map_request.at("type").AsString() == "Route") {
           result.push_back(PrintRouting(map_request, req_hand).AsDict());
        }
    }
    json::Print(json::Document{result}, std::cout);
}

const json::Node GetErrorMessage(const int id) {
            json::Node result;
            return json::Builder{}.StartDict().Key("request_id").Value(id).Key("error_message").Value("not found").EndDict().Build();
        }

const json::Node JsonReader::PrintStop(const json::Dict& map_request, RequestHandler& req_hand) const {
    json::Node result;
    const std::string& stop_name = map_request.at("name").AsString();
    const int id = map_request.at("id").AsInt();
    if (!req_hand.SearchStopName(stop_name)) {
       result = GetErrorMessage(id);
    }
    else {
        json::Array buses;
        for (auto& bus : req_hand.GetBusesOnStop(stop_name)) {
            buses.push_back(bus);
        }
        result = json::Builder{}.StartDict().Key("request_id").Value(id).Key("buses").Value(buses).EndDict().Build();
    }
    return result;
}

const json::Node JsonReader::PrintRoute(const json::Dict& map_request, RequestHandler& req_hand) const {
    json::Node result;
    const std::string& route_number = map_request.at("name").AsString();
    const int id = map_request.at("id").AsInt();
    if (!req_hand.SearchBusNumber(route_number)) {
        result = GetErrorMessage(id);
    }
    else {
        const auto& info = req_hand.GetBusStat(route_number);
        result = json::Builder{}.StartDict().Key("request_id").Value(id).Key("curvature").Value(info->curvature)
            .Key("route_length").Value(info->route_length).Key("stop_count").Value(static_cast<int>(info->stops_count))
            .Key("unique_stop_count").Value(static_cast<int>(info->unique_stops_count)).EndDict().Build();
    }
    return result;
}

const json::Node JsonReader::PrintMap(const json::Dict& map_request, RequestHandler& req_hand) const {
    json::Node result;
    const int id = map_request.at("id").AsInt();
    std::ostringstream out;
    svg::Document map = req_hand.RenderMap();
    map.Render(out);
    result = json::Builder{}.StartDict().Key("request_id").Value(id).Key("map").Value(out.str()).EndDict().Build();      
    return result;
}

const json::Node JsonReader::CreateRouteItem(const double& time, const std::string& type) const {
    return json::Builder{}.StartDict()
            .Key("time").Value(time)
            .Key("type").Value(type).EndDict().Build();
}

const json::Node JsonReader::PrintRouting(const json::Dict& map_request, RequestHandler& req_hand) const {
    json::Node result;
    const int id = map_request.at("id").AsInt();
    const std::string_view stop_from = map_request.at("from").AsString();
    const std::string_view stop_to = map_request.at("to").AsString();
    const auto& routing = req_hand.GetRouting(stop_from, stop_to);
    if (!routing) {
        result = GetErrorMessage(id);
    }
    else {
        const auto& graph = req_hand.GetGraph();
        json::Array items;
        double total_time = 0.0;
        for (auto& edge_id : routing.value().edges) {
            const auto& edge_info = graph.GetEdge(edge_id);
            auto wait_time = edge_info.weight;
            if (edge_info.type == "Stop") {
                items.emplace_back(CreateRouteItem(wait_time, "stop_name"));
            }
            else {
                items.emplace_back(CreateRouteItem(wait_time, "bus"));
            }
            total_time += wait_time;
        }
        result = json::Builder{}.StartDict()
                .Key("request_id").Value(id)
                .Key("total_time").Value(total_time)
                .Key("items").Value(items).EndDict().Build();
    }
    return result;
}
