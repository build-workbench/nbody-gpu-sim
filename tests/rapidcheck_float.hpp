#pragma once

#include <rapidcheck.h>

namespace nbody::test {

// rapidcheck's gen::inRange only supports integer types; map a scaled
// integer generator to floats in [lo, hi) with 1/1000 granularity.
// Use this instead of RC_PRE-filtering arbitrary floats: filtering
// full-range values almost never hits small valid ranges, so the
// property gives up without testing anything.
inline rc::Gen<float> genFloatInRange(float lo, float hi) {
  constexpr int kScale = 1000;
  return rc::gen::map(
      rc::gen::inRange<int>(static_cast<int>(lo * kScale), static_cast<int>(hi * kScale)),
      [](int v) { return static_cast<float>(v) / kScale; });
}

}  // namespace nbody::test
