// SPDX-License-Identifier: Apache-2.0
#include <charconv>
#include <ranges>
#include <sstream>

#include <CubingCore/CsvIo.hpp>

namespace CubingCore::csv
{

namespace
{
    constexpr std::string_view CsvHeader = "timestamp_ms,puzzle,raw_time_ms,penalty,scramble,comment,inspection_ms";

    std::string puzzleKey(Puzzle p)
    {
        return std::string(specOf(p).name);
    }

    std::string penaltyKey(Penalty p)
    {
        switch (p)
        {
            case Penalty::Ok:
                return "OK";
            case Penalty::PlusTwo:
                return "+2";
            case Penalty::Dnf:
                return "DNF";
        }
        return "OK";
    }

    std::expected<Penalty, CsvError> parsePenalty(std::string_view s)
    {
        if (s == "OK")
            return Penalty::Ok;
        if (s == "+2")
            return Penalty::PlusTwo;
        if (s == "DNF")
            return Penalty::Dnf;
        return std::unexpected(CsvError::UnknownPenalty);
    }

    /// Escapes a CSV field: wraps in quotes and doubles inner quotes if it
    /// contains delimiter, quote or newline.
    std::string escape(std::string_view field)
    {
        bool const needsQuote = field.find_first_of(",\"\n\r") != std::string_view::npos;
        if (!needsQuote)
            return std::string(field);
        std::string out;
        out.reserve(field.size() + 2);
        out.push_back('"');
        for (char c: field)
        {
            if (c == '"')
                out.push_back('"');
            out.push_back(c);
        }
        out.push_back('"');
        return out;
    }

    /// Splits one CSV row into fields, honouring "" quoting.
    std::vector<std::string> splitRow(std::string_view line)
    {
        std::vector<std::string> out;
        std::string current;
        bool inQuotes = false;
        for (std::size_t i = 0; i < line.size(); ++i)
        {
            char const c = line[i];
            if (inQuotes)
            {
                if (c == '"')
                {
                    if (i + 1 < line.size() && line[i + 1] == '"')
                    {
                        current.push_back('"');
                        ++i;
                    }
                    else
                        inQuotes = false;
                }
                else
                    current.push_back(c);
            }
            else
            {
                if (c == ',')
                {
                    out.push_back(std::move(current));
                    current.clear();
                }
                else if (c == '"' && current.empty())
                    inQuotes = true;
                else
                    current.push_back(c);
            }
        }
        out.push_back(std::move(current));
        return out;
    }

    std::expected<std::int64_t, CsvError> toI64(std::string_view s)
    {
        std::int64_t v = 0;
        auto const [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), v);
        if (ec != std::errc {} || ptr != s.data() + s.size())
            return std::unexpected(CsvError::Malformed);
        return v;
    }
} // namespace

std::string toCsv(std::span<Solve const> solves)
{
    std::ostringstream out;
    out << CsvHeader << '\n';
    for (auto const& s: solves)
    {
        auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(s.timestamp.time_since_epoch()).count();
        out << ms << ',' << puzzleKey(s.puzzle) << ',' << s.rawTime.count() << ',' << penaltyKey(s.penalty) << ','
            << escape(s.scramble) << ',' << escape(s.comment) << ',';
        if (s.inspection)
            out << s.inspection->count();
        out << '\n';
    }
    return out.str();
}

std::expected<std::vector<Solve>, CsvError> fromCsv(std::string_view text)
{
    std::vector<Solve> out;
    std::size_t lineStart = 0;
    bool first = true;
    while (lineStart <= text.size())
    {
        auto const nl = text.find('\n', lineStart);
        auto end = (nl == std::string_view::npos) ? text.size() : nl;
        std::string_view line = text.substr(lineStart, end - lineStart);
        // strip trailing \r
        if (!line.empty() && line.back() == '\r')
            line.remove_suffix(1);
        lineStart = end + 1;

        if (line.empty())
        {
            if (nl == std::string_view::npos)
                break;
            continue;
        }

        if (first)
        {
            first = false;
            if (line == CsvHeader)
                continue;
            // tolerate a missing header — treat as data
        }

        auto const fields = splitRow(line);
        if (fields.size() < 6 || fields.size() > 7)
            return std::unexpected(CsvError::Malformed);

        Solve s;
        auto const tsMs = toI64(fields[0]);
        if (!tsMs)
            return std::unexpected(tsMs.error());
        s.timestamp = Timestamp { std::chrono::milliseconds { *tsMs } };
        s.puzzle = puzzleFromKey(fields[1]);
        // Validate the puzzle key was actually one we recognise.
        if (fields[1] != "222" && fields[1] != "333" && fields[1] != "444")
            return std::unexpected(CsvError::UnknownPuzzle);
        auto const raw = toI64(fields[2]);
        if (!raw)
            return std::unexpected(raw.error());
        s.rawTime = Milliseconds { *raw };
        auto const pen = parsePenalty(fields[3]);
        if (!pen)
            return std::unexpected(pen.error());
        s.penalty = *pen;
        s.scramble = fields[4];
        s.comment = fields[5];
        if (fields.size() == 7 && !fields[6].empty())
        {
            auto const insp = toI64(fields[6]);
            if (!insp)
                return std::unexpected(insp.error());
            s.inspection = Milliseconds { *insp };
        }
        out.push_back(std::move(s));

        if (nl == std::string_view::npos)
            break;
    }
    return out;
}

} // namespace CubingCore::csv
