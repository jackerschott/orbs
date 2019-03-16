#define TIME_BUF_SIZE 100

#include <chrono>
#include <cmath>
#include <thread>
#include <vector>

#include "tmeas.hpp"
#include "res.hpp"

DEF_STANDARD_TYPES

int lapCount;
uint nSections;
uint nLapsUntilEval;
std::vector<std::chrono::high_resolution_clock::time_point> tps;
std::vector<double> rTimes;

bool useLapEvalInterval;
double lapEvalInterval;
std::chrono::high_resolution_clock::time_point tr;

void initTimeMeas(bool _useLapEvalInterval, double _lapEvalInterval) {
  useLapEvalInterval = _useLapEvalInterval;
  lapEvalInterval = _lapEvalInterval;

  tr = std::chrono::high_resolution_clock::now();
}
void setTimeMeasPoint() {
  tps.push_back(std::chrono::high_resolution_clock::now());
}
bool evalLap(std::vector<double>* secTimes) {
  if (lapCount == 0)
    nSections = (uint)tps.size() - 1;

  uint i0 = lapCount * (nSections + 1) + 1;
  for (uint i = i0; i < i0 + nSections; i++) {
    double dt = std::chrono::duration_cast<std::chrono::nanoseconds>(tps[i] - tps[i - 1]).count() / 1.0e9;
    rTimes.push_back(dt);
  }
  lapCount++;

  if (useLapEvalInterval) {
    std::chrono::high_resolution_clock::time_point tn = std::chrono::high_resolution_clock::now();
    double refTime = std::chrono::duration_cast<std::chrono::milliseconds>(tn - tr).count() / 1000.0;
    if (refTime < lapEvalInterval)
      return false;
    tr = std::chrono::high_resolution_clock::now();
  }

  (*secTimes).reserve(nSections);
  for (uint i = 0; i < nSections; i++) {
    double meanTime = 0.0;
    uint nLaps = (uint)rTimes.size() / nSections - nLapsUntilEval;
    for (uint i = nLapsUntilEval; i < nLapsUntilEval + nLaps; i++) {
      meanTime += rTimes[i * nSections];
    }
    meanTime /= nLaps;
    (*secTimes).push_back(meanTime);

    nLapsUntilEval += nLaps;
  }

  return true;
}
void evalTimeMeas(std::vector<double>* meanSecTimes, std::vector<double>* stdDevMeanSecTimes) {
  uint nLaps = (uint)rTimes.size() / nSections;
  meanSecTimes->reserve(nLaps);
  stdDevMeanSecTimes->reserve(nLaps);
  for (uint i = 0; i < nSections; i++) {
    double meanSecTime = 0.0;
    double stdDevMeanSecTime = 0.0;
    for (uint i = 0; i < nLaps; i++) {
      meanSecTime += rTimes[i * nSections];
    }
    meanSecTime /= nLaps;
    for (uint i = 0; i < nLaps; i++) {
      stdDevMeanSecTime += (rTimes[i * nSections] - stdDevMeanSecTime) * (rTimes[i * nSections] - stdDevMeanSecTime);
    }
    stdDevMeanSecTime = sqrt(stdDevMeanSecTime / (nLaps - 1));
    (*meanSecTimes).push_back(meanSecTime);
    (*stdDevMeanSecTimes).push_back(stdDevMeanSecTime);
  }
}
