#include "json_reader.h"
#include "request_handler.h"

int main() {
    transport::Catalogue catalogue;
    JsonReader json_doc(std::cin);
    
    json_doc.FillCatalogue(catalogue);
    
    const auto& stat_requests = json_doc.GetStatRequests();
    const auto& render_settings = json_doc.GetRenderSettings().AsMap();
    const auto& renderer = json_doc.FillRenderSettings(render_settings);
    
    RequestHandler req_hand(catalogue, renderer);
    
    json_doc.PrintStatRequests(stat_requests, req_hand);
}
