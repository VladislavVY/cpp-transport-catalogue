#include "json_reader.h"

#include <iostream>

const json::Node& JsonReader::GetStatRequests() const {
    if (!input_.GetRoot().AsMap().count("stat_requests")) {
        return nul_;
    }
    return input_.GetRoot().AsMap().at("stat_requests");
}

const json::Node& JsonReader::GetRenderSettings() const {
    if (!input_.GetRoot().AsMap().count("render_settings")) {
        return nul_;
    }
    return input_.GetRoot().AsMap().at("render_settings");
}

void JsonReader::FillCatalogue(transport::Catalogue& catalogue) {
    if (input_.GetRoot().AsMap().count("base_requests") > 0) {
    const auto& base_requests = input_.GetRoot().AsMap().at("base_requests").AsArray();
    for (auto& request : base_requests) {
        const auto& map_request = request.AsMap();
        const auto& type = map_request.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = FillStop(map_request);
            catalogue.AddStop(stop_name, coordinates);
        }
    }
    for (auto& request : base_requests) {
        const auto& map_request = request.AsMap();
        const auto& type = map_request.at("type").AsString();
        if (type == "Stop") {
            FillStopDistances(catalogue, map_request);
        }
    }
    for (auto& request : base_requests) {
        const auto& map_request = request.AsMap();
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
    auto& distances = map_request.at("road_distances").AsMap();
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

renderer::MapRenderer JsonReader::FillRenderSettings(const json::Dict& map_request) const {
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

void JsonReader::PrintStatRequests(const json::Node& stat_requests, RequestHandler& req_hand) const {
    json::Array result;
    for (auto& request : stat_requests.AsArray()) {
        const auto& map_request = request.AsMap();       
        if (map_request.at("type").AsString() == "Stop") {
           result.push_back(PrintStop(map_request, req_hand).AsMap());
        }
        if (map_request.at("type").AsString() == "Bus") {
           result.push_back(PrintRoute(map_request, req_hand).AsMap());
        }
        if (map_request.at("type").AsString() == "Map") {
           result.push_back(PrintMap(map_request, req_hand).AsMap());
        } 
    }
    json::Print(json::Document{ result }, std::cout);
}

const json::Node JsonReader::PrintStop(const json::Dict& map_request, RequestHandler& req_hand) const {
    json::Dict result;
    const std::string& stop_name = map_request.at("name").AsString();
    result["request_id"] = map_request.at("id").AsInt();
    if (!req_hand.SearchStopName(stop_name)) {
        result["error_message"] = json::Node{ static_cast<std::string>("not found") };
    }
    else {
        json::Array buses;
        for (auto& bus : req_hand.GetBusesOnStop(stop_name)) {
            buses.push_back(bus);
        }
        result["buses"] = buses;
    }
    return json::Node{result};
}

const json::Node JsonReader::PrintRoute(const json::Dict& map_request, RequestHandler& req_hand) const {
    json::Dict result;
    const std::string& route_number = map_request.at("name").AsString();
    result["request_id"] = map_request.at("id").AsInt();
    if (!req_hand.SearchBusNumber(route_number)) {
        result["error_message"] = json::Node{ static_cast<std::string>("not found") };
    }
    else {
        result["curvature"] = req_hand.GetBusStat(route_number)->curvature;
        result["route_length"] = req_hand.GetBusStat(route_number)->route_length;
        result["stop_count"] = static_cast<int>(req_hand.GetBusStat(route_number)->stops_count);
        result["unique_stop_count"] = static_cast<int>(req_hand.GetBusStat(route_number)->unique_stops_count);
    }
    return json::Node{ result };
}

const json::Node JsonReader::PrintMap(const json::Dict& map_request, RequestHandler& req_hand) const {
    json::Dict result;
    std::ostringstream out;
    svg::Document map = req_hand.RenderMap();
    map.Render(out);
    result["map"] = out.str();
    result["request_id"] = map_request.at("id").AsInt();       
    return json::Node{result};
}
