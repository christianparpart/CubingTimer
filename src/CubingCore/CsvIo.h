// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <CubingCore/Solve.h>

#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace CubingCore::csv
{

enum class CsvError
{
    Malformed,
    UnknownPuzzle,
    UnknownPenalty,
};

/// Serialises solves to a CSV string with header row.
/// Columns: `timestamp_ms, puzzle, raw_time_ms, penalty, scramble, comment, inspection_ms`.
/// @param solves solves to write.
/// @return CSV text with trailing newline per row.
[[nodiscard]] std::string toCsv(std::span<Solve const> solves);

/// Parses a CSV string produced by `toCsv` (or compatible).
/// @param text CSV input; the first row may be the header or a data row.
/// @return parsed solves, or a CsvError describing the first issue encountered.
[[nodiscard]] std::expected<std::vector<Solve>, CsvError> fromCsv(std::string_view text);

} // namespace CubingCore::csv
