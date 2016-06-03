/*
 * Copyright (c) 2016, Franky Romero, Ashish Chaudhari,
 * Wesson Altoyan, Nehal Bhandari
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
#include <stdio.h>

#include "search/util.h"
#include <string>
#include <sstream>  // For stringstream
#include <fstream>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <set>

static const u8 HSE_DEBUG = 0;
static const u32 numPrimes = 75;
static const u32 kPrimes[] = {
  5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59,
  61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131,
  137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211,
  223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293,
  307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389};

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

  writeSlimflyAdjList(u32(5), u32(1), "sf_bb.txt");
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
  u64 maxWidth = round(2 * (maxRadix_ - minConcentration_) / 3.0);
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
    u64 coeff = round(slimfly_.width / 4.0);
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
    // find the next widths configuration
    if (prime_idx == numPrimes) {
      printf("Prime index has reached %d. Terminating program.\n",
        numPrimes);
      return;
    }
    slimfly_.width = kPrimes[prime_idx++];
    // detect when done
    if (slimfly_.width > maxWidth) {
      break;
    }
  }
}

void Engine::stage2() {
  if (HSE_DEBUG >= 5) {
    printf("2: S=%lu P=%lu\n", slimfly_.width, slimfly_.routers);
  }

  // compute the baseRadix (no terminals)
  u64 baseRadix = 0;
  u64 coeff = round(slimfly_.width / 4.0);
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
  u64 coeff = round(slimfly_.width / 4.0);
  int delta = slimfly_.width - 4*coeff;
  slimfly_.routers = 2 * slimfly_.width * slimfly_.width;
  slimfly_.routerRadix += (3 * slimfly_.width - delta) / 2;

  bool tooSmallRadix = (slimfly_.routerRadix < minRadix_);
  bool tooBigRadix = (slimfly_.routerRadix > maxRadix_);

  // if not already skipped, compute bisection bandwidth
  bool tooSmallBandwidth = false;
  int sysOutput = 0;

  if (!tooSmallRadix && !tooBigRadix) {
    f64 smallestBandwidth = 9999999999;
    
    writeSlimflyAdjList(slimfly_.width, delta, "sf_bb.txt");
    sysOutput = system("gpmetis sf_bb.txt 2 > sf_bb.out");
    if (sysOutput) {
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

    remove("sf_bb.txt");
    remove("sf_bb.txt.part.2");
    remove("sf_bb.out");

    int edgecuts_i = std::stoi(edgecuts_s, nullptr, 10);
    slimfly_.bisections =
      static_cast <f64> (edgecuts_i) / slimfly_.terminals;

    if (slimfly_.bisections < smallestBandwidth) {
      smallestBandwidth = slimfly_.bisections;
    }
    if (smallestBandwidth < minBandwidth_) {
      tooSmallBandwidth = true;
      if (HSE_DEBUG >= 7) {
        printf("3s: SKIPPING S=%lu T=%lu N=%lu P=%lu R=%lu B=%lf\n",
                             slimfly_.width,
               slimfly_.concentration, slimfly_.terminals,
               slimfly_.routers,
               slimfly_.routerRadix,
               slimfly_.bisections);

        /* Used to debug gpmetis output file parser. */
        printf("In stage 3, edgecuts is: %d\n", edgecuts_i);
        printf("In stage 3, terminals is: %lu\n", slimfly_.terminals);
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
    printf("4: S=%lu T=%lu N=%lu B=%lf\n", slimfly_.width,
           slimfly_.concentration, slimfly_.terminals,
           slimfly_.bisections);
  }

  // compute the number of channels (only router to router)
  slimfly_.channels = slimfly_.routers *
                      (slimfly_.routerRadix - slimfly_.concentration) / 2;
  stage5();
}

void Engine::stage5() {
  if (HSE_DEBUG >= 2) {
    printf("5: S=%lu T=%lu N=%lu P=%lu R=%lu B=%lf\n", slimfly_.width,
           slimfly_.concentration, slimfly_.terminals, slimfly_.routers,
           slimfly_.routerRadix,
           slimfly_.bisections);
  }

  slimfly_.cost = costFunction_->cost(slimfly_);

  results_.push_back(slimfly_);
  std::sort(results_.begin(), results_.end(), comparator_);

  if (results_.size() > maxResults_) {
    results_.pop_back();
  }
}

void Engine::writeSlimflyAdjList(u32 width, u32 delta, std::string filename) {

  std::vector<u32> X, X_i;
  createGeneratorSet(width, delta, X, X_i);

  u32 numNodes = 2 * width * width;
  u32 numEdges = 0;
  std::vector< std::vector<u32> > adjList(numNodes, std::vector<u32>());

  static const u32 NUM_GRAPHS = 2;
  // link routers via channels: Intra subgraph connections
  for (u32 graph = 0; graph < NUM_GRAPHS; graph++) {
    std::vector<u32>& dVtr = (graph == 0) ? X : X_i;
    std::set<u32> distSet(dVtr.begin(), dVtr.end());
    for (u32 col = 0; col < width; col++) {
      for (u32 srcRow = 0; srcRow < width; srcRow++) {
        for (u32 dstRow = 0; dstRow < width; dstRow++) {
          // determine the source and destination router
          std::vector<u32> srcAddr = {graph, col, srcRow};
          std::vector<u32> dstAddr = {graph, col, dstRow};
          u32 dist = static_cast<u32>(
            std::abs<int>(static_cast<int>(dstRow) - srcRow));
          if (distSet.count(dist)) {
            adjList.at(ifaceIdFromAddress(srcAddr, width)).push_back(
              ifaceIdFromAddress(dstAddr, width) + 1);
            numEdges++;
          }
        }
      }
    }
  }
  // link routers via channels: Inter subgraph connections
  for (u32 x = 0; x < width; x++) {
    for (u32 y = 0; y < width; y++) {
      for (u32 m = 0; m < width; m++) {
        for (u32 c = 0; c < width; c++) {
          if (y == ((m*x + c) % width)) {
             // determine the source router
            std::vector<u32> addr1 = {0, x, y};
            std::vector<u32> addr2 = {1, m, c};

            adjList.at(ifaceIdFromAddress(addr1, width)).push_back(
              ifaceIdFromAddress(addr2, width) + 1);
            numEdges++;
            adjList.at(ifaceIdFromAddress(addr2, width)).push_back(
              ifaceIdFromAddress(addr1, width) + 1);
            numEdges++;
          }
        }
      }
    }
  }

  std::ofstream adjLstFile;
  adjLstFile.open(filename);
  adjLstFile << numNodes << " " << (numEdges/2) << std::endl;
  for (u32 i = 0; i < numNodes; i++) {
    for (u32 j = 0; j < adjList[i].size(); j++) {
      adjLstFile << adjList[i][j] << " ";
    }
    adjLstFile << std::endl;
  }
  adjLstFile.close();
}
