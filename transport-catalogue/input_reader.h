#pragma once

#include "transport_catalogue.h"

#include <iostream>

namespace transport {
void FillCatalogue(std::istream& input, Catalogue& catalogue);
void FillDistances(std::string& line, Catalogue& catalogue);
}
