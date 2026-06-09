// SPDX-License-Identifier: Apache-2.0
#include <CubingCore/JsonIo.h>

#include <charconv>
#include <sstream>

namespace CubingCore::json
{

namespace
{
    std::string puzzleKey(Puzzle p)
    {
        return std::string(specOf(p).name);
    }

    std::string penaltyKey(Penalty p)
    {
        switch (p)
        {
            case Penalty::Ok: return "OK";
            case Penalty::PlusTwo: return "+2";
            case Penalty::Dnf: return "DNF";
        }
        return "OK";
    }

    std::string escape(std::string_view s)
    {
        std::string out;
        out.reserve(s.size() + 2);
        for (char c: s)
        {
            switch (c)
            {
                case '"': out.append(R"(\")"); break;
                case '\\': out.append(R"(\\)"); break;
                case '\n': out.append(R"(\n)"); break;
                case '\r': out.append(R"(\r)"); break;
                case '\t': out.append(R"(\t)"); break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20)
                    {
                        std::array<char, 8> buf {};
                        auto const n = std::snprintf(buf.data(), buf.size(), R"(\u%04x)", c);
                        out.append(buf.data(), static_cast<std::size_t>(n));
                    }
                    else
                        out.push_back(c);
                    break;
            }
        }
        return out;
    }

    // --- a tiny single-pass parser ---
    struct Cursor
    {
        std::string_view text;
        std::size_t pos = 0;

        [[nodiscard]] bool eof() const noexcept { return pos >= text.size(); }
        [[nodiscard]] char peek() const noexcept { return eof() ? '\0' : text[pos]; }

        void skipWs() noexcept
        {
            while (!eof())
            {
                char const c = peek();
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                    ++pos;
                else
                    break;
            }
        }

        bool consume(char c) noexcept
        {
            skipWs();
            if (peek() == c)
            {
                ++pos;
                return true;
            }
            return false;
        }
    };

    std::expected<std::string, JsonError> readString(Cursor& c)
    {
        c.skipWs();
        if (!c.consume('"'))
            return std::unexpected(JsonError::Malformed);
        std::string out;
        while (!c.eof() && c.peek() != '"')
        {
            char const ch = c.peek();
            if (ch == '\\')
            {
                ++c.pos;
                if (c.eof())
                    return std::unexpected(JsonError::Malformed);
                char const esc = c.peek();
                ++c.pos;
                switch (esc)
                {
                    case '"': out.push_back('"'); break;
                    case '\\': out.push_back('\\'); break;
                    case '/': out.push_back('/'); break;
                    case 'n': out.push_back('\n'); break;
                    case 'r': out.push_back('\r'); break;
                    case 't': out.push_back('\t'); break;
                    case 'u':
                        if (c.pos + 4 > c.text.size())
                            return std::unexpected(JsonError::Malformed);
                        // We don't fully reconstruct UTF-16; accept ASCII range only.
                        {
                            unsigned val = 0;
                            for (int i = 0; i < 4; ++i)
                            {
                                char const h = c.text[c.pos + static_cast<std::size_t>(i)];
                                val <<= 4U;
                                if (h >= '0' && h <= '9')
                                    val |= static_cast<unsigned>(h - '0');
                                else if (h >= 'a' && h <= 'f')
                                    val |= static_cast<unsigned>(h - 'a' + 10);
                                else if (h >= 'A' && h <= 'F')
                                    val |= static_cast<unsigned>(h - 'A' + 10);
                                else
                                    return std::unexpected(JsonError::Malformed);
                            }
                            c.pos += 4;
                            if (val < 0x80)
                                out.push_back(static_cast<char>(val));
                            else
                                return std::unexpected(JsonError::Malformed);
                        }
                        break;
                    default: return std::unexpected(JsonError::Malformed);
                }
            }
            else
            {
                out.push_back(ch);
                ++c.pos;
            }
        }
        if (!c.consume('"'))
            return std::unexpected(JsonError::Malformed);
        return out;
    }

    std::expected<std::int64_t, JsonError> readInt(Cursor& c)
    {
        c.skipWs();
        std::size_t const start = c.pos;
        if (!c.eof() && (c.peek() == '-' || c.peek() == '+'))
            ++c.pos;
        while (!c.eof() && c.peek() >= '0' && c.peek() <= '9')
            ++c.pos;
        std::int64_t v = 0;
        auto const sv = c.text.substr(start, c.pos - start);
        auto const [p, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), v);
        (void) p;
        if (ec != std::errc {})
            return std::unexpected(JsonError::Malformed);
        return v;
    }

    /// Reads an int field that may be `null`, returning std::nullopt for `null`.
    std::expected<std::optional<std::int64_t>, JsonError> readIntOrNull(Cursor& c)
    {
        c.skipWs();
        if (c.pos + 4 <= c.text.size() && c.text.substr(c.pos, 4) == "null")
        {
            c.pos += 4;
            return std::optional<std::int64_t> {};
        }
        auto const v = readInt(c);
        if (!v)
            return std::unexpected(v.error());
        return std::optional<std::int64_t> { *v };
    }

    /// Per-field assignment. The parser dispatches each known JSON key to one
    /// of these handlers — keeping fromJson's complexity tractable and making
    /// it a one-row change to add a new field.
    std::expected<void, JsonError> assignField(Solve& s, std::string_view key, Cursor& c)
    {
        if (key == "timestamp_ms")
        {
            auto const v = readInt(c);
            if (!v) return std::unexpected(v.error());
            s.timestamp = Timestamp { Milliseconds { *v } };
            return {};
        }
        if (key == "puzzle")
        {
            auto const v = readString(c);
            if (!v) return std::unexpected(v.error());
            if (*v != "222" && *v != "333" && *v != "444")
                return std::unexpected(JsonError::Malformed);
            s.puzzle = puzzleFromKey(*v);
            return {};
        }
        if (key == "raw_time_ms")
        {
            auto const v = readInt(c);
            if (!v) return std::unexpected(v.error());
            s.rawTime = Milliseconds { *v };
            return {};
        }
        if (key == "penalty")
        {
            auto const v = readString(c);
            if (!v) return std::unexpected(v.error());
            if (*v == "OK") s.penalty = Penalty::Ok;
            else if (*v == "+2") s.penalty = Penalty::PlusTwo;
            else if (*v == "DNF") s.penalty = Penalty::Dnf;
            else return std::unexpected(JsonError::Malformed);
            return {};
        }
        if (key == "scramble")
        {
            auto const v = readString(c);
            if (!v) return std::unexpected(v.error());
            s.scramble = *v;
            return {};
        }
        if (key == "comment")
        {
            auto const v = readString(c);
            if (!v) return std::unexpected(v.error());
            s.comment = *v;
            return {};
        }
        if (key == "inspection_ms")
        {
            auto const v = readIntOrNull(c);
            if (!v) return std::unexpected(v.error());
            if (*v) s.inspection = Milliseconds { **v };
            else s.inspection.reset();
            return {};
        }
        return std::unexpected(JsonError::UnknownField);
    }

    /// Parses a single `{...}` solve object. Cursor must be positioned at `{`.
    std::expected<Solve, JsonError> parseObject(Cursor& c)
    {
        if (!c.consume('{'))
            return std::unexpected(JsonError::Malformed);
        Solve s;
        bool firstField = true;
        while (true)
        {
            c.skipWs();
            if (c.consume('}'))
                return s;
            if (!firstField && !c.consume(','))
                return std::unexpected(JsonError::Malformed);
            firstField = false;

            auto const key = readString(c);
            if (!key) return std::unexpected(key.error());
            if (!c.consume(':'))
                return std::unexpected(JsonError::Malformed);
            c.skipWs();
            if (auto const r = assignField(s, *key, c); !r)
                return std::unexpected(r.error());
        }
    }
} // namespace

std::string toJson(std::span<Solve const> solves)
{
    std::ostringstream out;
    out << "[";
    bool first = true;
    for (auto const& s: solves)
    {
        if (!first)
            out << ',';
        first = false;
        auto const ts =
            std::chrono::duration_cast<std::chrono::milliseconds>(s.timestamp.time_since_epoch()).count();
        out << R"({"timestamp_ms":)" << ts << R"(,"puzzle":")" << puzzleKey(s.puzzle)
            << R"(","raw_time_ms":)" << s.rawTime.count() << R"(,"penalty":")" << penaltyKey(s.penalty)
            << R"(","scramble":")" << escape(s.scramble) << R"(","comment":")" << escape(s.comment)
            << R"(","inspection_ms":)";
        if (s.inspection)
            out << s.inspection->count();
        else
            out << "null";
        out << '}';
    }
    out << "]";
    return out.str();
}

std::expected<std::vector<Solve>, JsonError> fromJson(std::string_view text)
{
    Cursor c { .text = text, .pos = 0 };
    std::vector<Solve> result;
    if (!c.consume('['))
        return std::unexpected(JsonError::Malformed);
    c.skipWs();
    if (c.consume(']'))
        return result;

    while (true)
    {
        auto solve = parseObject(c);
        if (!solve)
            return std::unexpected(solve.error());
        result.push_back(std::move(*solve));

        c.skipWs();
        if (c.consume(']'))
            return result;
        if (!c.consume(','))
            return std::unexpected(JsonError::Malformed);
    }
}

} // namespace CubingCore::json
