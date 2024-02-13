#pragma once
#ifndef AUDIO_STEGANOGRAPHY_H
#define AUDIO_STEGANOGRAPHY_H

#include <iostream>
#include <vector>
#include <bitset>
#include <string>
#include <sndfile.h>

std::vector<bool> stringToBinary(const std::string& message);

bool hideMessageInAudio(const std::string& audioFilePath, const std::string& outputFilePath, const std::string& message);

std::string audioToMessage(const std::string& filePath);

#endif
