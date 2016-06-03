/*
 * Copyright 2016 Ashish Chaudhari, Franky Romero, Nehal Bhandari, Wesson Altoyan
 *
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "util.h"
#include <math.h>
#include <cassert>
#include <set>
#include <iostream>

static const u32 numPrimes = 52;
static const u32 kPrimes[] = {5, 7, 11, 13, 17, 19, 23, 29, 31, 37,
  41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
  127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197,
  199, 211, 223, 227, 229, 233, 239, 241, 251};
static const std::set<u32> kPrimeSet(kPrimes, kPrimes + numPrimes);

/* Function to determine if a given width is prime. */
bool isPrime(u32 _width) {
  assert(_width < kPrimes[numPrimes - 1]);
  return (kPrimeSet.count(_width) != 0);
}

/* Function to determine if the width has a primitive element. */
static bool isPrimitiveElement(u32 _width, u32 prim) {
  std::vector<bool> satisfy(_width, false);
  satisfy[0] = true;  // exception for primitive elem
  u32 value = 1;
  for (u32 p = 0; p < _width; p++) {
    satisfy[value] = true;
    value = (value * prim) % _width;
  }
  bool isprim = true;
  for (bool s : satisfy) {
    isprim &= s;
  }
  return isprim;
}

/* Function to create the generator set for a computed primitive element. */
u32 createGeneratorSet(
    u32 _width, int delta, std::vector<u32>& X, std::vector<u32>& X_i) {
  u32 prim = 1;
  for (prim = 1; prim < _width; prim++) {
    if (isPrimitiveElement(_width, prim))
      break;
  }
  assert(prim < _width);
  u32 last_pow = (delta == 1) ? _width - 3 : _width - 2;
  X.clear();
  X_i.clear();
  for (u32 p = 0; p <= last_pow; p += 2) {
    if ((delta == -1) && p == (_width + 1) / 2) {
      p--;
    }
    u32 val = pow(prim, p);
    X.push_back(val % _width);
    X_i.push_back((val * prim) % _width);
  }
  return X.size();
}

/* Function to determine the face ID from the address and the width. */
u32 ifaceIdFromAddress(const std::vector<u32>& _address, u32 _width) {
  return (
      _address[2] +
      (_width * _address[1]) +
      (_width * _width * _address[0]));
}
