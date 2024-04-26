#ifndef DLF_H_
#define DLF_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "kaitai/kaitaistruct.h"
#include <stdint.h>

#if KAITAI_STRUCT_VERSION < 9000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.9 or later is required"
#endif

class dlf_t : public kaitai::kstruct {

public:
    class header_t;
    class logical_screen_t;

    dlf_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, dlf_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~dlf_t();

    class header_t : public kaitai::kstruct {

    public:

        header_t(kaitai::kstream* p__io, dlf_t* p__parent = 0, dlf_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~header_t();

    private:
        std::string m_magic;
        std::string m_version;
        dlf_t* m__root;
        dlf_t* m__parent;

    public:
        std::string magic() const { return m_magic; }
        std::string version() const { return m_version; }
        dlf_t* _root() const { return m__root; }
        dlf_t* _parent() const { return m__parent; }
    };

    class logical_screen_t : public kaitai::kstruct {

    public:

        logical_screen_t(kaitai::kstream* p__io, dlf_t* p__parent = 0, dlf_t* p__root = 0);

    private:
        void _read();
        void _clean_up();

    public:
        ~logical_screen_t();

    private:
        uint16_t m_image_width;
        uint16_t m_image_height;
        uint8_t m_flags;
        uint8_t m_bg_color_index;
        uint8_t m_pixel_aspect_ratio;
        dlf_t* m__root;
        dlf_t* m__parent;

    public:
        uint16_t image_width() const { return m_image_width; }
        uint16_t image_height() const { return m_image_height; }
        uint8_t flags() const { return m_flags; }
        uint8_t bg_color_index() const { return m_bg_color_index; }
        uint8_t pixel_aspect_ratio() const { return m_pixel_aspect_ratio; }
        dlf_t* _root() const { return m__root; }
        dlf_t* _parent() const { return m__parent; }
    };

private:
    header_t* m_header;
    logical_screen_t* m_logical_screen;
    dlf_t* m__root;
    kaitai::kstruct* m__parent;

public:
    header_t* header() const { return m_header; }
    logical_screen_t* logical_screen() const { return m_logical_screen; }
    dlf_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // DLF_H_
