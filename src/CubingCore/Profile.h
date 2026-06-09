// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace CubingCore
{

/// A user profile. Profiles own sessions; sessions own solves.
struct Profile
{
    std::int64_t id = 0;
    std::string name;
    std::chrono::system_clock::time_point createdAt {};
};

} // namespace CubingCore
