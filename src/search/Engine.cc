/*
 * Copyright (c) 2016, Nic McDonald
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * - Neither the name of prim nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "search/Engine.h"

#include <strop/strop.h>
#include <stdlib.h>  // For system

#include <string>
#include <sstream>  // For stringstream
#include <fstream>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <stdexcept>

static const u8 HSE_DEBUG = 0;
static const u32 numPrimes = 52;
static const u32 kPrimes[] = {5, 7, 11, 13, 17, 19, 23, 29, 31, 37,
41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197,
199, 211, 223, 227, 229, 233, 239, 241, 251};

CostFunction::CostFunction() {}
CostFunction::~CostFunction() {}

bool Comparator::operator()(const Slimfly& _lhs, const Slimfly& _rhs) const {
  return _rhs.cost > _lhs.cost;
}

Engine::Engine(u64 _minRadix, u64 _maxRadix,
               u64 _minConcentration, u64 _maxConcentration,
               u64 _minTerminals, u64 _maxTerminals, f64 _minBandwidth,
               u64 _maxResults, const CostFunction* _costFunction)
    : minRadix_(_minRadix),
      maxRadix_(_maxRadix),
      minConcentration_(_minConcentration),
      maxConcentration_(_maxConcentration),
      minTerminals_(_minTerminals),
      maxTerminals_(_maxTerminals),
      minBandwidth_(_minBandwidth),
      maxResults_(_maxResults),
      costFunction_(_costFunction) {

  if (minRadix_ < 2) {
    throw std::runtime_error("minradix must be greater than 1");
  } else if (maxRadix_ < minRadix_) {
    throw std::runtime_error("maxradix must be greater than or equal to "
                             "minradix");
  } else if (maxConcentration_ < minConcentration_) {
    throw std::runtime_error("maxconcentration must be greater than or equal "
                             "to minconcentration");
  } else if (minTerminals_ < minRadix_) {
    throw std::runtime_error("minterminals must be greater than or equal to "
                             "minradix");
  } else if (maxTerminals_ < minTerminals_) {
    throw std::runtime_error("maxterminals must be greater than or equal to "
                             "minterminals");
  } else if (minBandwidth_ <= 0) {
    throw std::runtime_error("minbandwidth must be greater than 0.0");
  }
}

Engine::~Engine() {}

void Engine::run() {
  slimfly_ = Slimfly();

  /* Set dimensions to 2. 
   * No particular use so far 
   */
  slimfly_.dimensions = 2;

  results_.clear();

  stage1();
}

const std::deque<Slimfly>& Engine::results() const {
  return results_;
}

void Engine::stage1() {
  /*
   * Number of dimensions is fixed
   */
  // find the maximum width of any one dimension
  u64 maxWidth = ceil(2 * (maxRadix_ - minConcentration_) / 3.0);

  if (maxWidth < 5) {
    return;
  }

  if (HSE_DEBUG >= 6) {
    printf("1: maxRadix=%lu maxWidth=%lu\n",
               maxRadix_, maxWidth);
  }

  /*
   * generate possible dimension widths (S)
   */
  slimfly_.width = 5;
  u32 prime_idx = 1;
  while (true) {
    // determine the number of routers
    u64 coeff = ceil(slimfly_.width / 4.0);
    int delta = slimfly_.width - 4*coeff;
    slimfly_.routers = 2 * slimfly_.width * slimfly_.width;

    // minimum current radix
    u64 baseRadix = 1 + (3 * slimfly_.width - delta) / 2;

    // find reasons to skip this case
    //  expr 1: at minimum, there would be 1 terminal per router
    //  expr 2: check minimum current router radix
    if ((slimfly_.routers <= maxTerminals_) &&
        (baseRadix <= maxRadix_)) {
      // if this configuration appears to work so far, use it
      stage2();
    } else if (HSE_DEBUG >= 7) {
      printf("1s: SKIPPING S=%lu\n", slimfly_.width);
    }
    // detect when done
    if (slimfly_.width == maxWidth) {
      break;
    }
    // find the next widths configuration
    slimfly_.width = kPrimes[prime_idx++];
  }
}

void Engine::stage2() {
  if (HSE_DEBUG >= 5) {
    printf("2: S=%lu P=%lu\n", slimfly_.width, slimfly_.routers);
  }

  // compute the baseRadix (no terminals)
  u64 baseRadix = 0;
  u64 coeff = ceil(slimfly_.width / 4.0);
  int delta = slimfly_.width - 4*coeff;
  slimfly_.routers = 2 * slimfly_.width * slimfly_.width;
  baseRadix = (3 * slimfly_.width - delta) / 2;

  // try possible values for terminals per router ratio
  for (slimfly_.concentration = minConcentration_;
       slimfly_.concentration <= maxConcentration_;
       slimfly_.concentration++) {
    slimfly_.terminals = slimfly_.routers * slimfly_.concentration;
    u64 baseRadix2 = baseRadix + slimfly_.concentration;
    if ((slimfly_.terminals >= minTerminals_) &&
        (slimfly_.terminals <= maxTerminals_) &&
        (baseRadix2 <= maxRadix_)) {
      stage3();
    } else {
      if (HSE_DEBUG >= 7) {
        printf("2s: SKIPPING S=%lu P=%lu T=%lu\n", slimfly_.width,
               slimfly_.routers, slimfly_.concentration);
      }
    }
    if ((slimfly_.terminals > maxTerminals_) ||
        (baseRadix2 > maxRadix_)) {
      break;
    }
  }
}

void Engine::stage3() {
  if (HSE_DEBUG >= 4) {
    printf("3: S=%lu T=%lu N=%lu P=%lu\n", slimfly_.width,
           slimfly_.concentration, slimfly_.terminals,
           slimfly_.routers);
  }

  // find the base radix
  slimfly_.routerRadix = slimfly_.concentration;
  u64 coeff = ceil(slimfly_.width / 4.0);
  int delta = slimfly_.width - 4*coeff;
  slimfly_.routers = 2 * slimfly_.width * slimfly_.width;
  slimfly_.routerRadix += (3 * slimfly_.width - delta) / 2;

  bool tooSmallRadix = (slimfly_.routerRadix < minRadix_);
  bool tooBigRadix = (slimfly_.routerRadix > maxRadix_);

  // if not already skipped, compute bisection bandwidth
  // TODO(ashish): Compute bisection bandwidth
  bool tooSmallBandwidth = false;
  int sysOutput = 0;
  if (!tooSmallRadix && !tooBigRadix) {
    slimfly_.bisections.clear();
    slimfly_.bisections.resize(slimfly_.dimensions, 0.0);
    f64 smallestBandwidth = 9999999999;
    for (u64 dim = 0; dim < slimfly_.dimensions; dim++) {
      // TODO(ashish): Generate input to metis
      sysOutput = system("gpmetis sf_bb.txt 2 > sf_bb.out");
      if (sysOutput) {
        printf("Error: %d with gpmetis! Now exiting...\n", sysOutput);
        return;
      }

      // Parse METIS output to obtain edgecuts
      std::ifstream metisfile("sf_bb.out");
      std::string nextline;
      std::string edgecuts_s;
      while (std::getline(metisfile, nextline)) {
        if (nextline.find("Edgecut:") != std::string::npos) {
          std::istringstream ss(nextline);
          // Need the third word
          for (int i = 0; i < 3; ++i)
            ss >> edgecuts_s;
          break;
        }
      }

      int edgecuts_i = std::stoi(edgecuts_s, nullptr, 10);

      printf("Edgecuts is: %d", edgecuts_i);

      slimfly_.bisections.at(dim) =
        edgecuts_i / slimfly_.terminals;

      if (slimfly_.bisections.at(dim) < smallestBandwidth) {
        smallestBandwidth = slimfly_.bisections.at(dim);
      }
    }
    if (smallestBandwidth < minBandwidth_) {
      tooSmallBandwidth = true;
      if (HSE_DEBUG >= 7) {
        printf("3s: SKIPPING S=%lu T=%lu N=%lu P=%lu R=%lu B=%s\n",
                             slimfly_.width,
               slimfly_.concentration, slimfly_.terminals,
               slimfly_.routers,
               slimfly_.routerRadix,
               strop::vecString<f64>(slimfly_.bisections).c_str());
      }
    }
    // if passed all tests, send to next stage
    if (!tooSmallRadix && !tooBigRadix && !tooSmallBandwidth) {
      stage4();
    } else {
      return;
    }
  }
}

void Engine::stage4() {
  if (HSE_DEBUG >= 3) {
    printf("4: S=%lu T=%lu N=%lu B=%s\n", slimfly_.width,
           slimfly_.concentration, slimfly_.terminals,
           strop::vecString<f64>(slimfly_.bisections).c_str());
  }

  // compute the number of channels (only router to router)
  slimfly_.channels = slimfly_.routers *
                      (slimfly_.routerRadix - slimfly_.concentration);
  stage5();
}

void Engine::stage5() {
  if (HSE_DEBUG >= 2) {
    printf("5: S=%lu T=%lu N=%lu P=%lu R=%lu B=%s\n", slimfly_.width,
           slimfly_.concentration, slimfly_.terminals, slimfly_.routers,
           slimfly_.routerRadix,
           strop::vecString<f64>(slimfly_.bisections).c_str());
  }

  slimfly_.cost = costFunction_->cost(slimfly_);

  results_.push_back(slimfly_);
  std::sort(results_.begin(), results_.end(), comparator_);

  if (results_.size() > maxResults_) {
    results_.pop_back();
  }
}