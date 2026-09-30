#pragma once

namespace Mosuan {

enum class MediaState {
    Idle,
    Opening,
    Buffering,
    Playing,
    Paused,
    Error,
    Ended
};

}
