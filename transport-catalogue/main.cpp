#include "json_reader.h"
#include "request_handler.h"

int main() {
    transport::Catalogue catalogue;
    JsonReader json_doc(std::cin);
    
    json_doc.FillCatalogue(catalogue);
    
    const auto& stat_requests = json_doc.GetStatRequests();
    const auto& render_settings = json_doc.GetRenderSettings();
    const auto& routing_settings = json_doc.GetRoutingSettings();
    const auto& renderer = json_doc.FillRenderSettings(render_settings);
    const auto& full_routing = json_doc.FillRoutingSettings(routing_settings);
    const transport::TransportRouter router = {full_routing, catalogue};
    
    RequestHandler req_hand(catalogue, renderer, router);
    
    json_doc.PrintStatRequests(stat_requests, req_hand);
}
