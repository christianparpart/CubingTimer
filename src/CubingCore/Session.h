// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <CubingCore/Puzzle.h>

#include <chrono>
#include <cstdint>
#include <string>

namespace CubingCore
{

/// A grouping of solves under a profile, typically tied to one puzzle.
struct Session
{
    std::int64_t id = 0;
    std::int64_t profileId = 0;
    std::string name;
    Puzzle puzzle = Puzzle::ThreeByThree;
    std::chrono::system_clock::time_point createdAt {};
};

} // namespace CubingCore
