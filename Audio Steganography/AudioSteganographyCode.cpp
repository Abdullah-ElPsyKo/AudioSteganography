#include <iostream>
#include <sndfile.h>
#include <vector>
#include <bitset>
#include <string>
#include "AudioSteganographyCode.h"

// Converts a string message to a vector of bits.
std::vector<bool> stringToBinary(const std::string& message) {
    std::vector<bool> binaryMessage;
    for (char c : message) {
        std::bitset<8> charBits(c);
        for (int i = 7; i >= 0; --i) {
            binaryMessage.push_back(charBits.test(i));
        }
    }
    return binaryMessage;
}

// Embeds the binary message into the LSB of each audio sample.
bool hideMessageInAudio(const std::string& audioFilePath, const std::string& outputFilePath, const std::string& message) {
    std::vector<bool> binaryMessage = stringToBinary(message + "\0"); // Include null terminator for the message.

    SF_INFO sfinfo;
    SNDFILE* sndfile = sf_open(audioFilePath.c_str(), SFM_READ, &sfinfo);
    if (!sndfile) {
        std::cerr << "Failed to open input file." << std::endl;
        return false;
    }

    std::vector<short> samples(sfinfo.frames * sfinfo.channels);
    if (sf_read_short(sndfile, samples.data(), samples.size()) != samples.size()) {
        sf_close(sndfile);
        std::cerr << "Failed to read audio data." << std::endl;
        return false;
    }
    sf_close(sndfile);

    if (samples.size() < binaryMessage.size()) {
        std::cerr << "Audio file too short for the message." << std::endl;
        return false;
    }

    // Embed the message into the LSB of samples.
    for (size_t i = 0; i < binaryMessage.size(); ++i) {
        if (binaryMessage[i]) {
            samples[i] |= 1; // Set LSB to 1 if bit is 1.
        }
        else {
            samples[i] &= ~1; // Set LSB to 0 if bit is 0.
        }
    }

    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16; // Ensure the format is appropriate for the output.

    sndfile = sf_open(outputFilePath.c_str(), SFM_WRITE, &sfinfo);
    if (!sndfile) {
        std::cerr << "Failed to open output file." << std::endl;
        return false;
    }

    if (sf_write_short(sndfile, samples.data(), samples.size()) != samples.size()) {
        sf_close(sndfile);
        std::cerr << "Failed to write modified audio data." << std::endl;
        return false;
    }

    sf_close(sndfile);
    return true;
}

std::string audioToMessage(const std::string& filePath) {
    SF_INFO sfinfo;
    SNDFILE* sndfile = sf_open(filePath.c_str(), SFM_READ, &sfinfo);
    if (!sndfile) {
        std::cerr << "Failed to open input file." << std::endl;
        return "";
    }

    std::vector<short> samples(sfinfo.frames * sfinfo.channels);
    if (sf_read_short(sndfile, samples.data(), samples.size()) != samples.size()) {
        sf_close(sndfile);
        std::cerr << "Failed to read audio data." << std::endl;
        return "";
    }
    sf_close(sndfile);

    std::string message;
    std::bitset<8> charBits;
    size_t bitIndex = 0;

    for (size_t i = 0; i < samples.size(); ++i) {
        charBits[7 - bitIndex] = (samples[i] & 1) != 0; // Extract the LSB from the sample.

        bitIndex++;
        if (bitIndex == 8) { // Once we have 8 bits, convert to character.
            char character = static_cast<char>(charBits.to_ulong());
            if (character == '\0') break;
            message += character;
            bitIndex = 0;
            charBits.reset();
        }
    }

    return message;
}

//int main() {
//    std::string audioFilePath = "C:\\Users\\ASUS\\Documents\\C++\\AudioSteganography\\AudioSteganography\\AudioSteganography\\secret.wav";
//    std::string outputFilePath = "C:\\Users\\ASUS\\Documents\\C++\\AudioSteganography\\AudioSteganography\\AudioSteganography\\output.wav";
//    std::string secretMessage = "you are monke";
//
//    // Embed the message
//    if (hideMessageInAudio(audioFilePath, outputFilePath, secretMessage)) {
//        std::cout << "Message hidden successfully in " << outputFilePath << std::endl;
//    }
//    else {
//        std::cerr << "Failed to hide the message." << std::endl;
//    }
//
//    // Extract the message
//    std::string decodedMessage = audioToMessage(outputFilePath);
//    if (!decodedMessage.empty()) {
//        std::cout << "Decoded message: " << decodedMessage << std::endl;
//    }
//    else {
//        std::cerr << "Failed to decode the message." << std::endl;
//    }
//
//    return 0;
//}

