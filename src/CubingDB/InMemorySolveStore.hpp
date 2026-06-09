// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <CubingDB/SqliteSolveStore.hpp>

namespace CubingDB
{

/// Convenience subclass that opens SQLite at ":memory:". Used by WASM (where
/// disk I/O is not available without a special FS) and by tests.
class InMemorySolveStore final: public SqliteSolveStore
{
  public:
    InMemorySolveStore():
        SqliteSolveStore(":memory:")
    {
    }
};

} // namespace CubingDB
