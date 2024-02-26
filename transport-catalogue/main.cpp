#include "stat_reader.h"
#include "input_reader.h"

using namespace transport;

int main() {
    Catalogue transport_catalogue;
    FillCatalogue(std::cin, transport_catalogue);
    ParseAndPrintStat(transport_catalogue, std::cout);
}
