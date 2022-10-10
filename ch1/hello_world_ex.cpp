#include <cstdio>
#include <array>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <string>
#include <thread>
#include <iostream>

using namespace std::chrono;

const size_t kBufferSize = 80;
const char* kGrayScaleTable = " .:-=+*#%@";
const size_t kGrayScaleTableSize = sizeof(kGrayScaleTable) / sizeof(char);

void updateWave(const double timeInterval, double* x, double* speed) {
    (*x) += timeInterval * (*speed);
    if ((*x) > 1.0) {
        (*speed) *= -1.0;
        (*x) = 1 + timeInterval * (*speed); // corrects the issue if we stepped over the boundary going right
    }
    if ((*x) < 0.0) {
        (*speed) *= -1.0;
        (*x) = timeInterval * (*speed);
    }
}
// QUESTIONS:
// Does heightField cover the whole span of the graph? Yes.
void accumulateWaveToHeightField(
    const double x,
    const double waveLength,
    const double maxHeight,
    std::array<double, kBufferSize>* heightField) {
    
    const double quarterWaveLength = 0.25 * waveLength;
    const int start = static_cast<int>((x - quarterWaveLength) * kBufferSize); // kBufferSize corresponds to 1 unit?
    const int end = static_cast<int>((x + quarterWaveLength) * kBufferSize);

    for (int i = start; i < end; i++) { // i is just a normal coordinate
        int iNew = i;
        if (i < 0 ) {
            iNew = -i + 1; // reflected across the boundary
        } else if (i >= static_cast<int>(kBufferSize)) {
            iNew = 2 * kBufferSize - i - 1; // wouldn't this be guaranteed to be out of bounds? No, i is greater than kBufferSize already.
            // I think this would only pose an issue if the speed is such that it puts i beyond 2*kBufferSize
        }

        double distance = std::fabs((i + 0.5) / kBufferSize - x); // distance between center point of wave and current coordinate
        double height = maxHeight * 0.5 * (std::cos(std::min(distance * M_PI / quarterWaveLength, M_PI)) + 1.0);
        (*heightField)[iNew] += height;

    }
}

void draw2d(
    const std::array<double, kBufferSize>& heightField) {
    std::vector<std::string> buffers;
    size_t lineHeight = kGrayScaleTableSize;
    for (int i =0; i < lineHeight; i++) {
        buffers.push_back(std::string(kBufferSize, ' '));
    }
    
    for(size_t i = 0; i < lineHeight; ++i){
        for(size_t j = 0; j < kBufferSize; ++j) {
            size_t tableIndex = std::min(static_cast<size_t>(std::floor(kGrayScaleTableSize * heightField[j])), kGrayScaleTableSize - 1);
            if (tableIndex < i) {
                tableIndex = 0;
            }
            buffers[(lineHeight - 1) - i][j] = tableIndex == 0 ? ' ' : '#';
        }
    }
    std::system("clear");
    printf("\e[?25l");

    for (auto& buffer: buffers) {
        printf("%s\n", buffer.c_str());
    }
    fflush(stdout);
}

void draw(
    const std::array<double, kBufferSize>& heightField
) {
    std::string buffer(kBufferSize, ' ');
    for (size_t i = 0; i < kBufferSize; ++i) {
        double height = heightField[i];
        size_t tableIndex = std::min(static_cast<size_t>(std::floor(kGrayScaleTableSize * height)), kGrayScaleTableSize - 1);
        buffer[i] = kGrayScaleTable[tableIndex];
    }

    // clear the buffer
    for(size_t i = 0; i < kBufferSize; ++i) {
        printf("\b");
    }

    // print the buffer
    printf("%s", buffer.c_str());
    fflush(stdout);
}

int main() {
    const double waveLengthX = 0.8;
    const double waveLengthY = 1.2;
    
    const double maxHeightX = 0.5;
    const double maxHeightY = 0.4;

    double x = 0.0;
    double y = 1.0;
    double speedX = 1.0;
    double speedY = -0.5;
    const int fps = 100;
    const double timeinterval = 1.0 / fps; // seconds per frame

    std::array<double, kBufferSize> heightField;

    for (int i = 0; i < 1000; ++i) {
        // Update waves
        updateWave(timeinterval, &x, &speedX);
        updateWave(timeinterval, &y, &speedY);

        // at each frame clear the drawing
        for (double& height : heightField) {
            height = 0.0;
        }

        accumulateWaveToHeightField(x, waveLengthX, maxHeightX, &heightField);
        accumulateWaveToHeightField(y, waveLengthY, maxHeightY, &heightField);
        draw2d(heightField);
        std::this_thread::sleep_for(milliseconds(1000 / fps));
    }
    printf("\n");
    fflush(stdout);
}