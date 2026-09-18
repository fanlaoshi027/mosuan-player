#include "VLCInstance.h"

VLCInstance::VLCInstance() = default;

VLCInstance::~VLCInstance()
{
    if (m_instance) {
        libvlc_release(m_instance);
    }
}

bool VLCInstance::initialize()
{
    if (m_instance) {
        return true;
    }

    const char* args[] = {
        "--no-video-title-show"
    };
    m_instance = libvlc_new(1, args);
    return m_instance != nullptr;
}
