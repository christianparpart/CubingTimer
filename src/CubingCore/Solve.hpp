// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

#include <CubingCore/Puzzle.hpp>

namespace CubingCore
{

/// Penalty applied to a solve.
enum class Penalty : std::uint8_t
{
    Ok = 0,
    PlusTwo = 1,
    Dnf = 2,
};

using Milliseconds = std::chrono::milliseconds;
using Timestamp = std::chrono::system_clock::time_point;

/// A recorded cubing solve. All fields are public so this is a true aggregate.
/// Penalty is stored separately from rawTime so it can be reverted.
struct Solve
{
    std::int64_t id = 0; ///< 0 ⇒ not yet persisted
    std::int64_t sessionId = 0;
    Puzzle puzzle = Puzzle::ThreeByThree;
    Timestamp timestamp = {};
    Milliseconds rawTime { 0 }; ///< measured time before penalties
    Penalty penalty = Penalty::Ok;
    std::string scramble;
    std::string comment;
    std::optional<Milliseconds> inspection; ///< inspection time used (if any)

    /// The time as displayed/ranked: rawTime + 2s for +2, nullopt for DNF.
    [[nodiscard]] std::optional<Milliseconds> effectiveTime() const noexcept
    {
        switch (penalty)
        {
            case Penalty::Ok:
                return rawTime;
            case Penalty::PlusTwo:
                return rawTime + std::chrono::seconds(2);
            case Penalty::Dnf:
                return std::nullopt;
        }
        return rawTime;
    }
};

} // namespace CubingCore
