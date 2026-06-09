// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <CubingCore/Solve.hpp>

namespace CubingCore::json
{

enum class JsonError : std::uint8_t
{
    Malformed,
    UnknownField,
};

/// Serialises solves as a minimal JSON array of objects. Intentionally
/// hand-rolled to avoid pulling in a JSON library for what is a trivial schema.
/// @param solves solves to write.
/// @return JSON array text.
[[nodiscard]] std::string toJson(std::span<Solve const> solves);

/// Parses a JSON array produced by `toJson` (or compatible).
/// @param text JSON input (array of objects).
/// @return parsed solves, or a JsonError on the first issue.
[[nodiscard]] std::expected<std::vector<Solve>, JsonError> fromJson(std::string_view text);

} // namespace CubingCore::json
