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
#ifndef NETWORK_SLIMFLY_UTIL_H_
#define NETWORK_SLIMFLY_UTIL_H_

#include <prim/prim.h>
#include <vector>

bool isPrime(u32 _width);
u32 createGeneratorSet(
  u32 _width, int delta, std::vector<u32>& X, std::vector<u32>& X_i);
u32 ifaceIdFromAddress(
    const std::vector<u32>& _address, u32 _width);

#endif  // NETWORK_SLIMFLY_UTIL_H_
