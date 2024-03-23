#include "map_renderer.h"

namespace renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

std::vector<svg::Polyline> MapRenderer::RenderRoute(const std::map<std::string_view, const transport::Bus*>& buses, const SphereProjector& proj) const {
    std::vector<svg::Polyline> results;
    size_t color_index = 0;
    for (const auto& [bus_number, bus] : buses) {
        if (bus->stops.empty()) continue;
        std::vector<const transport::Stop*> stops_for_route(bus->stops.begin(), bus->stops.end());
        if (!bus->is_roundtrip) {
            stops_for_route.insert(stops_for_route.end(), std::next(bus->stops.rbegin()), bus->stops.rend());
        }
        svg::Polyline polyline;
        for (const auto stop : stops_for_route) {
            polyline.AddPoint(proj(stop->coordinates));
        }
        polyline.SetStrokeColor(render_settings_.color_palette.at(color_index++ % render_settings_.color_palette.size()))
                .SetFillColor("none")
                .SetStrokeWidth(render_settings_.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        results.push_back(std::move(polyline));
    }
    return results;
}

std::vector<svg::Text> MapRenderer::RenderBusName(const std::map<std::string_view, const transport::Bus*>& buses, const SphereProjector& proj) const {
    std::vector<svg::Text> results;
    size_t color_index = 0;
    for (const auto& [bus_number, bus] : buses) {
        if (bus->stops.empty()) continue;
        auto create_text = [&](const transport::Stop* stop) -> std::pair<svg::Text, svg::Text> {
            svg::Text text, text_substrate;
            text.SetPosition(proj(stop->coordinates))
                 .SetOffset(render_settings_.bus_label_offset)
                 .SetFontSize(render_settings_.bus_label_font_size)
                 .SetFontFamily("Verdana")
                 .SetFontWeight("bold")
                 .SetData(std::string(bus->number))
                 .SetFillColor(render_settings_.color_palette[color_index]);

            text_substrate = text;
    text_substrate.SetFillColor(render_settings_.underlayer_color)
                  .SetStrokeColor(render_settings_.underlayer_color)
                  .SetStrokeWidth(render_settings_.underlayer_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            return {std::move(text_substrate), std::move(text)};
        };

        auto [first_text_substrate, first_text] = create_text(bus->stops.front());
        results.push_back(std::move(first_text_substrate));
        results.push_back(std::move(first_text));

        if (!bus->is_roundtrip && bus->stops.front() != bus->stops.back()) {
            auto [last_text_substrate, last_text] = create_text(bus->stops.back());
            results.push_back(std::move(last_text_substrate));
            results.push_back(std::move(last_text));
        }
        color_index = (color_index + 1) % render_settings_.color_palette.size();
    }
    return results;
}

std::vector<svg::Circle> MapRenderer::RenderStopCoordinates(const std::map<std::string_view, const transport::Stop*>& stops, const SphereProjector& proj) const {
    std::vector<svg::Circle> results;
    for (const auto& [stop_name, stop] : stops) {
        results.push_back(svg::Circle()
                          .SetCenter(proj(stop->coordinates))
                          .SetRadius(render_settings_.stop_radius)
                          .SetFillColor("white"));
    }
    return results;
}

std::vector<svg::Text> MapRenderer::RenderStopNames(const std::map<std::string_view, const transport::Stop*>& stops, const SphereProjector& proj) const {
    std::vector<svg::Text> results;
    for (const auto& [stop_name, stop] : stops) {
        svg::Text text, text_substrate;
        text.SetPosition(proj(stop->coordinates))
             .SetOffset(render_settings_.stop_label_offset)
             .SetFontSize(render_settings_.stop_label_font_size)
             .SetFontFamily("Verdana")
             .SetData(stop->name)
             .SetFillColor("black");

        text_substrate = text;
text_substrate.SetFillColor(render_settings_.underlayer_color)
              .SetStrokeColor(render_settings_.underlayer_color)
              .SetStrokeWidth(render_settings_.underlayer_width)
              .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
              .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        results.push_back(std::move(text_substrate));
        results.push_back(std::move(text));
    }
    return results;
}

svg::Document MapRenderer::RenderMap(const std::map<std::string_view, const transport::Bus*>& buses) const {
    svg::Document result;
    std::vector<geo::Coordinates> coordinates;
    std::map<std::string_view, const transport::Stop*> all_stops;
    for (const auto& [bus_number, bus] : buses) {
        if (bus->stops.empty()) continue;
        for (const auto stop : bus->stops) {
            coordinates.push_back(stop->coordinates);
            all_stops[stop->name] = stop;
        }
    }
    SphereProjector proj(coordinates.begin(), coordinates.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
    
    for (const auto& line : RenderRoute(buses, proj)) result.Add(line);
    for (const auto& label : RenderBusName(buses, proj)) result.Add(label);
    for (const auto& circle : RenderStopCoordinates(all_stops, proj)) result.Add(circle);
    for (const auto& text : RenderStopNames(all_stops, proj)) result.Add(text);

    return result;
}

} // namespace renderer
