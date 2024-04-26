// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "dlf.h"
#include "kaitai/exceptions.h"

dlf_t::dlf_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, dlf_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = this;
    m_header = 0;
    m_logical_screen = 0;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void dlf_t::_read() {
    m_header = new header_t(m__io, this, m__root);
    m_logical_screen = new logical_screen_t(m__io, this, m__root);
}

dlf_t::~dlf_t() {
    _clean_up();
}

void dlf_t::_clean_up() {
    if (m_header) {
        delete m_header; m_header = 0;
    }
    if (m_logical_screen) {
        delete m_logical_screen; m_logical_screen = 0;
    }
}

dlf_t::header_t::header_t(kaitai::kstream* p__io, dlf_t* p__parent, dlf_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void dlf_t::header_t::_read() {
    m_magic = m__io->read_bytes(3);
    if (!(magic() == std::string("\x47\x49\x46", 3))) {
        throw kaitai::validation_not_equal_error<std::string>(std::string("\x47\x49\x46", 3), magic(), _io(), std::string("/types/header/seq/0"));
    }
    m_version = m__io->read_bytes(3);
}

dlf_t::header_t::~header_t() {
    _clean_up();
}

void dlf_t::header_t::_clean_up() {
}

dlf_t::logical_screen_t::logical_screen_t(kaitai::kstream* p__io, dlf_t* p__parent, dlf_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void dlf_t::logical_screen_t::_read() {
    m_image_width = m__io->read_u2le();
    m_image_height = m__io->read_u2le();
    m_flags = m__io->read_u1();
    m_bg_color_index = m__io->read_u1();
    m_pixel_aspect_ratio = m__io->read_u1();
}

dlf_t::logical_screen_t::~logical_screen_t() {
    _clean_up();
}

void dlf_t::logical_screen_t::_clean_up() {
}
