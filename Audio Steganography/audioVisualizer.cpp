#include "AudioVisualizer.h"
#include <sndfile.h>

AudioData readAudioFile(const char* filename) {
    SNDFILE* file;
    SF_INFO sfinfo;

    file = sf_open(filename, SFM_READ, &sfinfo);
    if (file == nullptr) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(1);
    }

    std::vector<double> samples(sfinfo.frames * sfinfo.channels);
    long long numSamples = sf_read_double(file, samples.data(), samples.size());
    samples.resize(numSamples); // Adjust vector size to the actual number of samples read

    sf_close(file);

    return { samples, sfinfo.samplerate, sfinfo.channels };
}

std::vector<double> prepareWaveformData(const std::vector<double>& samples, int channels) {
    std::vector<double> monoSamples;
    for (size_t i = 0; i < samples.size(); i += channels) {
        double sum = 0.0;
        for (int channel = 0; channel < channels; ++channel) {
            sum += samples[i + channel];
        }
        monoSamples.push_back(sum / channels);
    }
    return monoSamples;
}
