#pragma once

namespace Mosuan {

// Playback safety helper.
// Frame processing should never prevent VLC from starting playback.
class PlaybackFallback
{
public:
    static bool shouldUseRawVideo(bool processorReady);
};

}
