#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdexcept>

using namespace std;

class YUVImage {
public:
    int width;
    int height;
    vector<uint8_t> Y; // Luminance
    vector<uint8_t> U; // Chrominance U
    vector<uint8_t> V; // Chrominance V

    // 파일 로드
    bool load(const string& filename) { // 파일 이름
        ifstream file(filename, ios::binary); // 이진 파일로 읽기
        if (!file) { // 파일 열기 실패 시
            cerr << "Error: Could not open file " << filename << endl;
            return false;
        }

        try {
            size_t y_size = width * height; // Y 채널 크기
            size_t uv_size = (width / 2) * (height / 2); // U, V 채널 크기

            Y.resize(y_size); // Y 채널 크기 설정
            U.resize(uv_size); // U 채널 크기 설정 
            V.resize(uv_size);// V 채널 크기 설정

            file.read(reinterpret_cast<char*>(Y.data()), Y.size()); // Y 채널 읽기
            file.read(reinterpret_cast<char*>(U.data()), U.size()); // U 채널 읽기
            file.read(reinterpret_cast<char*>(V.data()), V.size()); // V 채널 읽기

            if (file.fail()) { // 파일 읽기 실패 시
                throw runtime_error("Error reading YUV data from file");
            }
        }
        catch (const exception& e) { // 예외 처리
            cerr << "Exception: " << e.what() << endl;
            return false;
        }

        cout << "Loaded YUV image: " << filename << endl;
        return true;
    }

    // 파일 저장
    void save(const string& filename) const {
        ofstream file(filename, ios::binary); // 이진 파일로 쓰기
        if (!file) {  // 파일 열기 실패 시
            cerr << "Error: Could not open file " << filename << endl;
            return;
        }

        file.write(reinterpret_cast<const char*>(Y.data()), Y.size()); // Y 채널 저장
        file.write(reinterpret_cast<const char*>(U.data()), U.size()); // U 채널 저장
        file.write(reinterpret_cast<const char*>(V.data()), V.size()); // V 채널 저장

        if (file.fail()) {
            cerr << "Error: Failed to write YUV data to file " << filename << endl;
            return;
        }

        cout << "Saved YUV image: " << filename << endl;
    }

    // 다운샘플링
    void downsample() {
        width /= 2; // 너비 절반
        height /= 2; // 높이 절반

        Y = downsampleChannel(Y, width * 2, height * 2, width, height); // Y 채널 다운샘플링
        U = downsampleChannel(U, width, height, width / 2, height / 2); // U 채널 다운샘플링
        V = downsampleChannel(V, width, height, width / 2, height / 2); // V 채널 다운샘플링

        cout << "Downsampled to " << width << "x" << height << endl; // 다운샘플링 크기 출력
    }

    // 업샘플링 (Nearest Neighbor 방식)
    void NN_upsample() {
        int newWidth = width * 2;  // 업샘플링 후 너비
        int newHeight = height * 2; // 업샘플링 후 높이

        Y = NN_upsampleChannel(Y, width, height, newWidth, newHeight); // Y 채널 업샘플링
        U = NN_upsampleChannel(U, width / 2, height / 2, newWidth / 2, newHeight / 2); // U 채널 업샘플링 
        V = NN_upsampleChannel(V, width / 2, height / 2, newWidth / 2, newHeight / 2);// V 채널 업샘플링

        width = newWidth;  // 크기 업데이트
        height = newHeight; // 크기 업데이트

        cout << "Upsampled to " << width << "x" << height << endl; // 업샘플링 크기 출력
    }

    // RGB to YUV 변환 함수 추가
    void convertRGBtoYUV(const vector<uint8_t>& RGB, int w, int h) {
        Y.resize(w * h); // Y 채널 크기 설정
        U.resize((w / 2) * (h / 2)); // U 채널 크기 설정
        V.resize((w / 2) * (h / 2)); // V 채널 크기 설정

        // Temporary accumulators for U and V
        vector<int> U_acc((w / 2) * (h / 2), 0); // U 채널 누적값
        vector<int> V_acc((w / 2) * (h / 2), 0); // V 채널 누적값
        vector<int> count((w / 2) * (h / 2), 0); // 픽셀 수

        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                int rgbIdx = (i * w + j) * 3; // RGB 인덱스

                // RGB 배열의 범위가 넘지 않도록 체크
                if (rgbIdx + 2 >= RGB.size()) {
                    cerr << "Error: Index out of bounds when accessing RGB" << endl; // 인덱스 오류
                    return;
                }

                int R = RGB[rgbIdx]; // R 채널
                int G = RGB[rgbIdx + 1]; // G 채널
                int B = RGB[rgbIdx + 2]; // B 채널

                // RGB to YUV 변환
                double Yval = 0.299 * R + 0.587 * G + 0.114 * B;
                double Uval = -0.14713 * R - 0.28886 * G + 0.436 * B + 128;
                double Vval = 0.615 * R - 0.51499 * G - 0.10001 * B + 128;

                // Y 값을 각 픽셀에 대해 계산
                Y[i * w + j] = static_cast<uint8_t>(max(0, min(255, static_cast<int>(round(Yval)))));

                // U와 V는 2x2 블록으로 평균
                if (i % 2 == 0 && j % 2 == 0) {
                    int uvIdx = (i / 2) * (w / 2) + (j / 2); // U, V 인덱스
                    U_acc[uvIdx] += static_cast<int>(round(Uval)); // U 누적값
                    V_acc[uvIdx] += static_cast<int>(round(Vval)); // V 누적값
                    count[uvIdx] += 1; // 픽셀 수
                }
            }
        }

        // U와 V를 계산
        for (size_t idx = 0; idx < U.size(); ++idx) {
            if (count[idx] > 0) {
                U[idx] = static_cast<uint8_t>(max(0, min(255, U_acc[idx] / count[idx]))); // 2x2 블록 평균 계산
                V[idx] = static_cast<uint8_t>(max(0, min(255, V_acc[idx] / count[idx]))); // 2x2 블록 평균 계산
            }
            else {
                U[idx] = 128;  // 기본값
                V[idx] = 128;  // 기본값
            }
        }
    }
    // RGB 변환
    vector<uint8_t> convertToRGB() const {
        vector<uint8_t> RGB(width * height * 3, 0); // R, G, B 각각 1 byte
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int yIdx = i * width + j;
                int uvIdx = (i / 2) * (width / 2) + (j / 2); // U, V 채널은 절반 크기

                uint8_t Yval = Y[yIdx]; // Y 채널 값
                uint8_t Uval = U[uvIdx]; // U 채널 값
                uint8_t Vval = V[uvIdx]; // V 채널 값

                // YUV to RGB 변환 공식
                double C = Yval - 16; // Y값 조정
                double D = Uval - 128; // U값 조정
                double E = Vval - 128; // V값 조정

                int R = static_cast<int>(round(1.164 * C + 1.596 * E)); // R 채널 계산
                int G = static_cast<int>(round(1.164 * C - 0.392 * D - 0.813 * E)); // G 채널 계산
                int B = static_cast<int>(round(1.164 * C + 2.017 * D)); // B 채널 계산

                R = max(0, min(255, R)); // R 채널 범위 조정
                G = max(0, min(255, G)); // G 채널 범위 조정
                B = max(0, min(255, B)); // B 채널 범위 조정

                int rgbIdx = (i * width + j) * 3; // RGB 인덱스
                RGB[rgbIdx] = static_cast<uint8_t>(R); // R 채널
                RGB[rgbIdx + 1] = static_cast<uint8_t>(G); // G 채널
                RGB[rgbIdx + 2] = static_cast<uint8_t>(B); // B 채널
            }
        }
        return RGB; // RGB 값 반환
    }

    // 업스케일링 (6-tap DCT-IF 필터 적용)
    void BI_interpolation_upscale_withDCTIF() {
        int newWidth = width * 2; // 너비 2배
        int newHeight = height * 2; // 높이 2배

        // Step 1: Convert to RGB
        vector<uint8_t> RGB = convertToRGB(); // RGB로 변환

        // Step 2: Separate RGB channels
        vector<uint8_t> R, G, B; // R, G, B 채널
        R.reserve(width * height); // R 채널 크기 설정
        G.reserve(width * height); // G 채널 크기 설정
        B.reserve(width * height); // B 채널 크기 설정
        for (size_t i = 0; i < RGB.size(); i += 3) { // R, G, B 채널 분리
            R.push_back(RGB[i]); // R 채널
            G.push_back(RGB[i + 1]); // G 채널
            B.push_back(RGB[i + 2]); // B 채널
        }

        // Step 3: Apply 6-tap DCT-IF filter to each channel (R, G, B)
        vector<uint8_t> upscaledR = BI_upsampleChannel_DCTIF(R, width, height, newWidth, newHeight); // R 채널 업샘플링
        vector<uint8_t> upscaledG = BI_upsampleChannel_DCTIF(G, width, height, newWidth, newHeight); // G 채널 업샘플링
        vector<uint8_t> upscaledB = BI_upsampleChannel_DCTIF(B, width, height, newWidth, newHeight); // B 채널 업샘플링

        // Step 4: Combine upscaled RGB channels
        vector<uint8_t> upscaledRGB(newWidth * newHeight * 3, 0); // 업샘플링된 RGB 값
        for (int i = 0; i < newWidth * newHeight; ++i) { // R, G, B 채널 결합
            upscaledRGB[i * 3] = upscaledR[i]; // R 채널
            upscaledRGB[i * 3 + 1] = upscaledG[i]; // G 채널
            upscaledRGB[i * 3 + 2] = upscaledB[i]; // B 채널
        }

        // Step 5: Calculate Y from RGB directly (avoid filtering)
        vector<uint8_t> Y = calculateYFromRGB(upscaledRGB, newWidth, newHeight); // Y 채널 계산

        // Step 6: Convert RGB to YUV
        convertRGBtoYUV(upscaledRGB, newWidth, newHeight); // RGB to YUV 변환

        width = newWidth; // 너비 업데이트
        height = newHeight; // 높이 업데이트

        cout << "Upscaled with 6-tap DCT-IF filter to " << width << "x" << height << endl; // 업스케일링 크기 출력
    }

    // Y 값은 직접 계산
    vector<uint8_t> calculateYFromRGB(const vector<uint8_t>& RGB, int w, int h) { // RGB 값, 너비, 높이
        vector<uint8_t> Y(w * h); // Y 채널 크기 설정

        for (int i = 0; i < h; i++) { // 각 픽셀에 대해
            for (int j = 0; j < w; j++) { // 각 픽셀에 대해
                int rgbIdx = (i * w + j) * 3; // RGB 인덱스

                int R = RGB[rgbIdx]; // R 채널
                int G = RGB[rgbIdx + 1];  // G 채널
                int B = RGB[rgbIdx + 2]; // B 채널

                // RGB to Y 계산
                int Yval = static_cast<int>(0.299 * R + 0.587 * G + 0.114 * B); // Y 계산
                Y[i * w + j] = static_cast<uint8_t>(max(0, min(255, Yval))); // Y 채널 값
            }
        }

        return Y;
    }

    // 가우시안 필터 적용 (Y 채널에만 적용)
    void applyGaussianFilterToY(double sigma) {
        vector<uint8_t> filteredY = Y;
        const int filterSize = 5;  // 5x5 필터
        vector<vector<double>> gaussianKernel(filterSize, vector<double>(filterSize));

        // 가우시안 커널 계산
        double sum = 0.0; // 가우시안 커널 합
        int halfSize = filterSize / 2; // 필터 크기의 절반
        for (int i = -halfSize; i <= halfSize; ++i) { // 필터 크기만큼 반복
            for (int j = -halfSize; j <= halfSize; ++j) { // 필터 크기만큼 반복
                gaussianKernel[i + halfSize][j + halfSize] = exp(-(i * i + j * j) / (2 * sigma * sigma)); // 가우시안 커널 계산
                sum += gaussianKernel[i + halfSize][j + halfSize]; // 가우시안 커널 합
            }
        }

        // 커널 정규화
        for (int i = 0; i < filterSize; ++i) { // 각 픽셀에 대해
            for (int j = 0; j < filterSize; ++j) { // 각 픽셀에 대해 
                gaussianKernel[i][j] /= sum; // 가우시안 커널 정규화
            }
        }

        // Y 채널에 가우시안 필터 적용
        for (int y = 0; y < height; ++y) { // 각 픽셀에 대해
            for (int x = 0; x < width; ++x) { // 각 픽셀에 대해
                double value = 0.0; // 필터링된 값

                // 필터 적용
                for (int ky = -halfSize; ky <= halfSize; ++ky) { // 필터 크기만큼 반복
                    for (int kx = -halfSize; kx <= halfSize; ++kx) {  // 필터 크기만큼 반복
                        int px = x + kx; // 픽셀 x 좌표
                        int py = y + ky; // 픽셀 y 좌표
                        if (px >= 0 && px < width && py >= 0 && py < height) { // 경계 체크
                            value += Y[py * width + px] * gaussianKernel[ky + halfSize][kx + halfSize]; // 가중치 적용
                        }
                    }
                }

                filteredY[y * width + x] = static_cast<uint8_t>(max(0, min(255, static_cast<int>(round(value))))); // 필터링된 Y 값
            }
        }

        Y = filteredY;
        cout << "Applied Gaussian filter to Y channel" << endl;
    }

    // MSE 계산
    double calculateMSE(const vector<uint8_t>& original, const vector<uint8_t>& reconstructed) const {
        if (original.size() != reconstructed.size()) {
            cerr << "Error: Image sizes do not match!" << endl;
            return -1;
        }

        double mse = 0.0;
        for (size_t i = 0; i < original.size(); ++i) { // 각 픽셀에 대해
            double diff = original[i] - reconstructed[i]; // 차이 계산
            mse += diff * diff; // 제곱 오차 누적
        }

        mse /= original.size(); // 평균 제곱 오차 계산
        return mse;
    }


private:
    // 채널 다운샘플링
    vector<uint8_t> downsampleChannel(const vector<uint8_t>& channel, int w, int h, int newW, int newH) const {
        vector<uint8_t> result(newW * newH);
        for (int y = 0; y < newH; y++) {
            for (int x = 0; x < newW; x++) {
                result[y * newW + x] = channel[(y * 2) * w + (x * 2)];
            }
        }
        return result;
    }

    // 채널 업샘플링 (Nearest Neighbor 방식)
    vector<uint8_t> NN_upsampleChannel(const vector<uint8_t>& channel, int w, int h, int newW, int newH) const {
        vector<uint8_t> result(newW * newH); // 결과 저장
        for (int y = 0; y < h; y++) { // For each row
            for (int x = 0; x < w; x++) { // For each pixel
                int srcIdx = y * w + x; //소스 인덱스 
                int destIdx = (y * 2) * newW + (x * 2); // 대상 인덱스

                // 좌상단 복제 
                result[destIdx] = channel[srcIdx];
                // 우상단 복제
                if ((x * 2 + 1) < newW) {
                    result[destIdx + 1] = channel[srcIdx];
                }
                // 좌하단 복제
                if ((y * 2 + 1) < newH) {
                    result[destIdx + newW] = channel[srcIdx];
                }
                // 우하단 복제
                if ((x * 2 + 1) < newW && (y * 2 + 1) < newH) {
                    result[destIdx + 1 + newW] = channel[srcIdx];
                }
            }
        }
        return result;
    }

    // 채널 업샘플링 (6-tap DCT-IF 필터 적용)
    vector<uint8_t> BI_upsampleChannel_DCTIF(const vector<uint8_t>& channel, int w, int h, int newW, int newH) const {
        const int filter_size = 6; // 6-tap DCT-IF 필터
        const double filter_coeffs[6] = { 11 / 256.0, -43 / 256.0, 160 / 256.0, 160 / 256.0, -43 / 256.0, 11 / 256.0 }; // 6-tap DCT-IF 필터 계수

        // First, perform horizontal filtering
        vector<double> tempBuffer(w * 2 * h, 0.0);

        for (int y = 0; y < h; y++) { // For each row
            for (int x = 0; x < w; x++) { // For each pixel
                tempBuffer[y * newW + (x * 2)] = channel[y * w + x]; // Copy the pixel
            }
        }

        // Horizontal interpolation
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w - 1; x++) {
                double interpolated = 0.0; // Interpolated value
                for (int k = -2; k <= 3; k++) { //  6-tap DCT-IF 필터 적용
                    int src_x = x + k; // Source pixel
                    src_x = max(0, min(w - 1, src_x)); // Clamp to valid range
                    interpolated += filter_coeffs[k + 2] * channel[y * w + src_x]; // Filter coefficient * pixel value
                }
                tempBuffer[y * newW + (x * 2 + 1)] = interpolated; // Store the interpolated value
            }
        }

        // Vertical filtering
        vector<uint8_t> finalBuffer(newW * newH, 0);

        for (int x = 0; x < newW; x++) { // For each column
            for (int y = 0; y < newH; y++) { // For each pixel
                if (y % 2 == 0) { // Copy the pixel
                    finalBuffer[y * newW + x] = static_cast<uint8_t>(round(tempBuffer[(y / 2) * newW + x])); // Copy the pixel
                }
                else {
                    double interpolated = 0.0; // Interpolated value 
                    int src_y = y / 2;// Source pixel
                    for (int k = -2; k <= 3; k++) { // 6-tap DCT-IF 필터 적용
                        int current_y = src_y + k; // Source pixel
                        current_y = max(0, min(h - 1, current_y)); // Clamp to valid range
                        interpolated += filter_coeffs[k + 2] * tempBuffer[current_y * newW + x]; // Filter coefficient * pixel value
                    }
                    interpolated = max(0.0, min(255.0, interpolated)); // Clamp to valid range
                    finalBuffer[y * newW + x] = static_cast<uint8_t>(round(interpolated)); // Store the interpolated value
                }
            }
        }

        // Diagonal Half-pel Sample Interpolation
        for (int y = 0; y < newH - 1; y += 2) {
            for (int x = 0; x < newW - 1; x += 2) {
                // Take the average of 4 surrounding pixels (top-left, top-right, bottom-left, bottom-right)
                int avg = (finalBuffer[(y * newW) + x] +
                    finalBuffer[(y * newW) + (x + 1)] +
                    finalBuffer[((y + 1) * newW) + x] +
                    finalBuffer[((y + 1) * newW) + (x + 1)]) / 4;

                // Set the 4 neighboring pixels to the averaged value
                finalBuffer[(y * newW) + x] = static_cast<uint8_t>(avg); // Top-left
                finalBuffer[(y * newW) + (x + 1)] = static_cast<uint8_t>(avg); // Top-right
                finalBuffer[((y + 1) * newW) + x] = static_cast<uint8_t>(avg); // Bottom-left
                finalBuffer[((y + 1) * newW) + (x + 1)] = static_cast<uint8_t>(avg); // Bottom-right
            }
        }

        return finalBuffer;
    }

};

#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main() {
    // 파일 이름 목록
    vector<string> filenames = {
        "BQTerrace_1920x1080_yuv420_8bit_frame0",
        "Cactus_1920x1080_yuv420_8bit_frame200",
        "Kimono1_1920x1080_yuv420_8bit_frame0",
        "ParkScene_1920x1080_yuv420_8bit_frame0",
        "ParkScene_1920x1080_yuv420_8bit_frame200"
    };

    // 파일 처리 반복
    for (const string& filename : filenames) {
        string inputPath = "./" + filename + ".yuv";

        // 원본 이미지 로드
        YUVImage originalImage;
        originalImage.width = 1920;
        originalImage.height = 1080;

        if (!originalImage.load(inputPath)) {
            cerr << "Error: Failed to load image " << filename << endl;
            continue;
        }

        // 복원된 이미지 로드 (같은 파일을 다시 로드)
        YUVImage image;
        image.width = 1920;
        image.height = 1080;

        if (!image.load(inputPath)) {
            cerr << "Error: Failed to load image " << filename << endl;
            continue;
        }

        // Y 채널에 가우시안 필터 적용
        //image.applyGaussianFilterToY(1);

        // 다운샘플링 수행
        image.downsample();
        string outputPath = "./" + filename + "_downsampled.yuv";
        image.save(outputPath);

        // 다운샘플링된 이미지 로드
        YUVImage downsampledImage_NN;
        downsampledImage_NN.width = 960;
        downsampledImage_NN.height = 540;

        if (!downsampledImage_NN.load(outputPath)) {
            cerr << "Error: Failed to load downsampled image for " << filename << endl;
            continue;
        }

        // 다운샘플링된 이미지에서 업스케일 수행 (6-tap DCT-IF 필터 적용)
        downsampledImage_NN.NN_upsample();
        string upsampledPath = "./" + filename + "_upsampled_NN.yuv";
        downsampledImage_NN.save(upsampledPath);

        // 원본과 필터링 후 MSE 계산
        double mse_Y = image.calculateMSE(originalImage.Y, downsampledImage_NN.Y);  // Y 채널 MSE
        double mse_U = image.calculateMSE(originalImage.U, downsampledImage_NN.U);  // U 채널 MSE
        double mse_V = image.calculateMSE(originalImage.V, downsampledImage_NN.V);  // V 채널 MSE

        // 각 채널별 MSE 출력
        cout << "Results for NN" << filename << ":" << endl;
        cout << "MSE between original and filtered Y: " << mse_Y << endl;
        cout << "MSE between original and filtered U: " << mse_U << endl;
        cout << "MSE between original and filtered V: " << mse_V << endl;

        // 전체 평균 MSE 계산
        cout << "Average MSE (Y, U, V): " << (mse_Y + mse_U + mse_V) / 3 << endl;
        cout << "------------------------------------" << endl;


        // 다운샘플링된 이미지 로드
        YUVImage downsampledImage_BI;
        downsampledImage_BI.width = 960;
        downsampledImage_BI.height = 540;

        if (!downsampledImage_BI.load(outputPath)) {
            cerr << "Error: Failed to load downsampled image for " << filename << endl;
            continue;
        }

        // 다운샘플링된 이미지에서 업스케일 수행 (6-tap DCT-IF 필터 적용)
        downsampledImage_BI.BI_interpolation_upscale_withDCTIF();
        upsampledPath = "./" + filename + "_upsampled_DCTIF.yuv";
        downsampledImage_BI.save(upsampledPath);

        // 원본과 필터링 후 MSE 계산
        mse_Y = image.calculateMSE(originalImage.Y, downsampledImage_BI.Y);  // Y 채널 MSE
        mse_U = image.calculateMSE(originalImage.U, downsampledImage_BI.U);  // U 채널 MSE
        mse_V = image.calculateMSE(originalImage.V, downsampledImage_BI.V);  // V 채널 MSE

        // 각 채널별 MSE 출력
        cout << "Results for BI" << filename << ":" << endl;
        cout << "MSE between original and filtered Y: " << mse_Y << endl;
        cout << "MSE between original and filtered U: " << mse_U << endl;
        cout << "MSE between original and filtered V: " << mse_V << endl;

        // 전체 평균 MSE 계산
        cout << "Average MSE (Y, U, V): " << (mse_Y + mse_U + mse_V) / 3 << endl;
        cout << "------------------------------------" << endl;
    }

    return 0;
}
