#pragma once

#include "transport_catalogue.h"

#include <iostream>

namespace transport {

void ParseAndPrintStat(const Catalogue& transport_catalogue, std::ostream& output);
    
}
