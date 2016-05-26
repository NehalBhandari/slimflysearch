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
#include <grid/Grid.h>
#include <prim/prim.h>
#include <strop/strop.h>
#include <tclap/CmdLine.h>

#include <deque>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "search/Calculator.h"
#include "search/CalculatorFactory.h"
#include "search/Engine.h"

s32 main(s32 _argc, char** _argv) {
  // u64 minDimensions;
  // u64 maxDimensions;
  u64 minRadix;
  u64 maxRadix;
  u64 minConcentration;
  u64 maxConcentration;
  u64 minTerminals;
  u64 maxTerminals;
  f64 minBandwidth;
  // bool fixedWidth;
  // bool fixedWeight;
  u64 maxResults;
  bool printSettings;
  std::string costCalc;

  std::string version = "1.1";
  std::string description =
      ("Search Slim Fly topologies for optimal solutions.");
  try {
    // create the command line parser
    TCLAP::CmdLine cmd(description, ' ', version);

    // define command line args
    // TCLAP::ValueArg<u64> minDimensionsArg(
    //     "", "mindimensions", "minimum number of dimensions",
    //     false, 1, "u64", cmd);
    // TCLAP::ValueArg<u64> maxDimensionsArg(
    //     "", "maxdimensions", "maximum number of dimensions",
    //     false, 4, "u64", cmd);
    TCLAP::ValueArg<u64> minRadixArg(
        "", "minradix", "minimum router radix",
        false, 2, "u64", cmd);
    TCLAP::ValueArg<u64> maxRadixArg(
        "", "maxradix", "maximum router radix",
        false, 64, "u64", cmd);
    TCLAP::ValueArg<u64> minConcentrationArg(
        "", "minconcentration", "minimum router concentration",
        false, 1, "u64", cmd);
    TCLAP::ValueArg<u64> maxConcentrationArg(
        "", "maxconcentration", "maximum router concentration",
        false, U32_MAX - 1, "u64", cmd);
    TCLAP::ValueArg<u64> minTerminalsArg(
        "", "minterminals", "minimum number of terminals",
        false, 32768, "u64", cmd);
    TCLAP::ValueArg<u64> maxTerminalsArg(
        "", "maxterminals", "maximum number of terminals",
        false, 0, "u64", cmd);
    TCLAP::ValueArg<f64> minBandwidthArg(
        "", "minbandwidth", "minimum relative bisection bandwidth",
        false, 1.0, "f64", cmd);
    // TCLAP::SwitchArg fixedWidthArg(
    //     "", "fixedwidth", "only search fixed width (fbfly) topologies",
    //     cmd, false);
    // TCLAP::SwitchArg fixedWeightArg(
    //     "", "fixedweight", "only search fixed weight (fbfly) topologies",
    //     cmd, false);
    TCLAP::ValueArg<u64> maxResultsArg(
        "", "maxresults", "maximum number of results",
        false, 10, "u64", cmd);
    TCLAP::ValueArg<std::string> costCalcArg(
        "", "costcalc", "cost calculator to use",
        false, "router_channel_count", "string", cmd);
    TCLAP::SwitchArg printSettingsArg(
        "p", "printsettings", "print the input settings",
        cmd, false);

    // parse the command line
    cmd.parse(_argc, _argv);

    // copy values out to variables
    // minDimensions = minDimensionsArg.getValue();
    // maxDimensions = maxDimensionsArg.getValue();
    minRadix = minRadixArg.getValue();
    maxRadix = maxRadixArg.getValue();
    minConcentration = minConcentrationArg.getValue();
    maxConcentration = maxConcentrationArg.getValue();
    minTerminals = minTerminalsArg.getValue();
    maxTerminals = maxTerminalsArg.getValue();
    if (maxTerminals == 0) {
      maxTerminals = minTerminals * 2;
    }
    minBandwidth = minBandwidthArg.getValue();
    // fixedWidth = fixedWidthArg.getValue();
    // fixedWeight = fixedWeightArg.getValue();
    maxResults = maxResultsArg.getValue();
    printSettings = printSettingsArg.getValue();
    costCalc = costCalcArg.getValue();
  } catch (TCLAP::ArgException& e) {
    throw std::runtime_error(e.error().c_str());
  }

  // if in verbose mode, print input settings
  if (printSettings) {
    printf("input settings:\n"
           // "  minDimensions = %lu\n"
           // "  maxDimensions = %lu\n"
           "  minRadix = %lu\n"
           "  maxRadix = %lu\n"
           "  minConcentration = %lu\n"
           "  maxConcentration = %lu\n"
           "  minTerminals = %lu\n"
           "  maxTerminals = %lu\n"
           "  minBandwidth = %f\n"
           // "  fixedWidth = %s\n"
           // "  fixedWeight = %s\n"
           "  maxResults = %lu\n"
           "  costCalc = %s\n"
           "\n",
           // minDimensions,
           // maxDimensions,
           minRadix,
           maxRadix,
           minConcentration,
           maxConcentration,
           minTerminals,
           maxTerminals,
           minBandwidth,
           // (fixedWidth ? "yes" : "no"),
           // (fixedWeight ? "yes" : "no"),
           maxResults,
           costCalc.c_str());
  }

  // create the cost calculator
  Calculator* calc = CalculatorFactory::createCalculator(costCalc);

  // create and run the engine
  Engine engine(
      minRadix, maxRadix, minConcentration, maxConcentration,
      minTerminals, maxTerminals, minBandwidth, maxResults, calc);
  engine.run();

  // gather the results
  const std::deque<Slimfly>& results = engine.results();

  // create the output grid
  const std::vector<std::string>& extFields = calc->extFields();
  grid::Grid grid(1 + results.size(), 11 + extFields.size());

  // format the regular header
  grid.set(0, 0, "#");
  grid.set(0, 1, "Dimensions");
  grid.set(0, 2, "Widths");
  grid.set(0, 3, "Concentration");
  grid.set(0, 4, "Terminals");
  grid.set(0, 5, "Routers");
  grid.set(0, 6, "Radix");
  grid.set(0, 7, "Channels");
  grid.set(0, 8, "Bisections");
  grid.set(0, 9, "Cost");

  // format the extension header
  for (u64 ext = 0; ext < extFields.size(); ext++) {
    grid.set(0, 11 + ext, extFields.at(ext));
  }

  // format the data section
  for (u64 idx = 0; idx < results.size(); idx++) {
    u64 row = idx + 1;

    // get the results
    const Slimfly& res = results.at(idx);

    // format the regular values in the row
    grid.set(row, 0, std::to_string(row));
    grid.set(row, 1, std::to_string(res.dimensions));
    grid.set(row, 2, std::to_string(res.width));
    grid.set(row, 3, std::to_string(res.concentration));
    grid.set(row, 4, std::to_string(res.terminals));
    grid.set(row, 5, std::to_string(res.routers));
    grid.set(row, 6, std::to_string(res.routerRadix));
    grid.set(row, 7, std::to_string(res.channels));
    grid.set(row, 8, strop::vecString<f64>(res.bisections, ',', 2).c_str());
    grid.set(row, 9, std::to_string(res.cost));

    // get extension values from the calculator
    const std::unordered_map<std::string, std::string>& extValues =
        calc->extValues(res);

    // format the extensions values in the row
    for (u64 ext = 0; ext < extFields.size(); ext++) {
      grid.set(row, 11 + ext, extValues.at(extFields.at(ext)));
    }
  }

  // print the output grid
  printf("%s", grid.toString().c_str());

  // cleanup
  delete calc;

  return 0;
}
