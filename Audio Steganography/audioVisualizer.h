#pragma once

#ifndef AUDIOVISUALIZER_H
#define AUDIOVISUALIZER_H

#include <vector>
#include <iostream>
#include <cmath>

struct SF_INFO;

struct AudioData {
    std::vector<double> samples;
    int sampleRate;
    int channels;
};

AudioData readAudioFile(const char* filename);
std::vector<double> prepareWaveformData(const std::vector<double>& samples, int channels);

#endif
