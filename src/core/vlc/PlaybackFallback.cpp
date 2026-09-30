#include "PlaybackFallback.h"

namespace Mosuan {

bool PlaybackFallback::shouldUseRawVideo(bool processorReady)
{
    // Until frame processing is fully connected,
    // keep VLC playback path safe.
    return !processorReady;
}

}
