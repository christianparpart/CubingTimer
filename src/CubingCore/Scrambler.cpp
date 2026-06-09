// SPDX-License-Identifier: Apache-2.0
#include <CubingCore/Scrambler.h>

#include <cassert>
#include <string>

namespace CubingCore
{

namespace
{
    constexpr char faceChar(Face face) noexcept
    {
        switch (face)
        {
            case Face::U: return 'U';
            case Face::D: return 'D';
            case Face::L: return 'L';
            case Face::R: return 'R';
            case Face::F: return 'F';
            case Face::B: return 'B';
        }
        return '?';
    }
} // namespace

Scrambler::Scrambler(PuzzleSpec const& spec, RandomFn rng): _spec(&spec), _rng(std::move(rng))
{
    assert(_rng && "RNG must be callable");
    assert(!_spec->faces.empty() && "PuzzleSpec must provide at least one face");
}

std::vector<Move> Scrambler::generate() const
{
    std::vector<Move> result;
    result.reserve(static_cast<std::size_t>(_spec->length));

    Face lastFace = Face::U;
    int lastAxis = -1;
    int axisRunLength = 0; // how many consecutive moves have shared the same axis

    while (result.size() < static_cast<std::size_t>(_spec->length))
    {
        auto const faceIndex = static_cast<std::size_t>(_rng() % _spec->faces.size());
        auto const face = _spec->faces[faceIndex];
        auto const axis = axisOf(face);

        if (!result.empty())
        {
            if (face == lastFace)
                continue;
            if (axis == lastAxis && axisRunLength >= 2)
                continue;
        }

        auto const amountIndex = _rng() % 3U;
        auto const amount = static_cast<Amount>(amountIndex);

        bool wide = false;
        if (_spec->allowWide)
        {
            // For 4x4 the WCA convention uses both outer and wide turns; we use
            // wide turns roughly half the time to produce realistic scrambles.
            wide = (_rng() & 1U) != 0;
        }

        result.push_back(Move { .face = face, .amount = amount, .wide = wide });

        if (axis == lastAxis)
            ++axisRunLength;
        else
            axisRunLength = 1;
        lastFace = face;
        lastAxis = axis;
    }

    return result;
}

std::string Scrambler::generateString() const
{
    return format(generate());
}

std::string Scrambler::formatMove(Move move)
{
    std::string out;
    out.reserve(4);
    out.push_back(faceChar(move.face));
    if (move.wide)
        out.push_back('w');
    switch (move.amount)
    {
        case Amount::Cw: break;
        case Amount::Half: out.push_back('2'); break;
        case Amount::Ccw: out.push_back('\''); break;
    }
    return out;
}

std::string Scrambler::format(std::vector<Move> const& moves)
{
    std::string out;
    out.reserve(moves.size() * 4U);
    bool first = true;
    for (auto const& m: moves)
    {
        if (!first)
            out.push_back(' ');
        out.append(formatMove(m));
        first = false;
    }
    return out;
}

} // namespace CubingCore
