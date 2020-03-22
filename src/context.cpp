#include "esc.h"

esc::Context::Context(std::string src): m_src(src) { }

const std::string& esc::Context::src() const {
    return m_src;
}

esc::LocationId esc::Context::add_location(size_t offset, size_t len) {
    return this->add_location(esc::Location {offset, len});
}

esc::LocationId esc::Context::add_location(Location loc) {
    LocationId id = m_locs.size();
    m_locs.push_back(loc);
    return id;
}

const esc::Location& esc::Context::location(esc::LocationId id) {
    return m_locs[id];
}
