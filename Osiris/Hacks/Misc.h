#pragma once

#include "../JsonForward.h"

enum class FrameStage;
class GameEvent;
struct ImDrawList;
struct UserCmd;

namespace Misc
{
    bool isMenuKeyPressed() noexcept;
}
