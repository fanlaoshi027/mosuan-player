#pragma once

#include <vlc/vlc.h>

class VLCInstance
{
public:
    VLCInstance();
    ~VLCInstance();

    VLCInstance(const VLCInstance&) = delete;
    VLCInstance& operator=(const VLCInstance&) = delete;

    bool initialize();
    libvlc_instance_t* instance() const { return m_instance; }

private:
    libvlc_instance_t* m_instance = nullptr;
};
