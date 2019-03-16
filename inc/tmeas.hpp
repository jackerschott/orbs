#ifndef TMEAS_HPP
#define TMEAS_HPP

#include <vector>

void initTimeMeas(bool _useLapEvalInterval = false, double _lapEvalInterval = 0.0);
void setTimeMeasPoint();
bool evalLap(std::vector<double>* secTimes);
void evalTimeMeas(std::vector<double>* meanSecTimes, std::vector<double>* stdDevMeanSecTimes);

#endif
