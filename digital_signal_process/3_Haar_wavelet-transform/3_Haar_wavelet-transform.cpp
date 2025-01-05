#include <iostream> // 표준 입출력 사용
#include <fstream>// 파일 입출력 사용
#include <vector> // 벡터 사용
#include <cmath> // 수학 함수 사용
#include <regex> // 정규 표현식 사용
#include <iomanip> // MSE 출력 형식 지정

using namespace std;

// 함수 선언

void parseDimensions(const string& filename, int& width, int& height); // 파일 이름에서 이미지 크기 추출
void haarTransform1D(vector<int>& data); // 1D Haar 변환
void haarTransform2D(vector<vector<int>>& image, int levels); // 2D Haar 변환 (다단계)
void inverseHaarTransform1D(vector<int>& data); // 1D Haar 역변환
void inverseHaarTransform2D(vector<vector<int>>& image, int levels); // 2D Haar 역변환 (다단계)
void quantizeImage(vector<vector<int>>& image, int bitDepth); // 이미지 양자화
void inverseQuantizeImage(vector<vector<int>>& image, int bitDepth); // 이미지 역양자화
void readRawImage(const string& filename, vector<vector<int>>& image); // RAW 파일에서 이미지 읽기
void writeRawImage(const string& filename, const vector<vector<int>>& image); // RAW 파일로 이미지 저장
double calculateMSE(const vector<vector<int>>& original, const vector<vector<int>>& reconstructed); // MSE 계산

void quantizeBlock(int startHeight, int endHeight, int startWidth, int endWidth, int bitDepth, vector<vector<int>>& image); // 블록에 대해 양자화 수행
void performQuantization(int currentHeight, int currentWidth, vector<vector<int>>& image, int bitDepths[4]); // 전체 이미지에 대해 양자화 수행 (Case 3에서 사용)
void dequantizeBlock(int startHeight, int endHeight, int startWidth, int endWidth, int bitDepth, vector<vector<int>>& image); // 블록에 대해 역양자화 수행
void performDequantization(int currentHeight, int currentWidth, vector<vector<int>>& image, int bitDepths[4]); // 전체 이미지에 대해 역양자화 수행 (Case 3에서 사용)

int main() {
    string filename = "Baboon_256x256_yuv400_8bit.raw"; // 파일 이름 설정
    cout << "filename is " << filename << "\n\n";
    int width, height; // 너비와 높이 변수 선언
    parseDimensions(filename, width, height); // 이미지 크기 추출

    vector<vector<int>> image(height, vector<int>(width)); // 이미지 데이터를 저장할 벡터 선언
    vector<vector<int>> originalImage(height, vector<int>(width)); // 원본 이미지 데이터를 저장할 벡터 선언
    vector<vector<int>> origin_haar_image(height, vector<int>(width)); // Haar 변환된 이미지 데이터를 저장할 벡터 선언


    readRawImage(filename, image); // 이미지 읽기
    originalImage = image; // 원본 이미지 설정
    // for(int i = 0; i < height; i++) {
    //     for(int j = 0; j < width; j++) {
    //         cout << image[i][j] << " ";
    //     }
    // }

    int levels = 3; // 다단계 Haar 변환 수행
    haarTransform2D(image, levels); // 2D Haar 변환 수행
    origin_haar_image = image; // Haar 변환된 이미지 설정

    // Case 1: 양자화 없이 변환 수행
    inverseHaarTransform2D(image, levels); // 2D Haar 역변환 수행
    writeRawImage(filename + "_reconstructed_image_case1.raw", image); // 결과 이미지 저장
    double mse = calculateMSE(originalImage, image); // MSE 계산
    cout << fixed << setprecision(2) << "Case 1 - 평균 제곱 오차 (MSE): " << mse << endl; // 결과 출력


    // Case 2: 각 영역에 대해 8-bit 양자화 수행 
    image = origin_haar_image; // Haar 변환된 이미지 설정
    int currentHeight = image.size(); // 현재 높이 설정
    int currentWidth = image[0].size(); // 현재 너비 설정
    int bitDepths[4] = { 8, 8, 8, 8 }; // 각 영역에 대해 8-bit 양자화 수행
    performQuantization(currentHeight, currentWidth, image, bitDepths); // 양자화 수행
    performDequantization(currentHeight, currentWidth, image, bitDepths); // 역양자화 수행

    mse = calculateMSE(origin_haar_image, image); // MSE 계산

    cout << fixed << setprecision(2) << "Case 2 - Haar 변환 후 평균 제곱 오차 (MSE): " << mse << endl; // 결과 출력
    inverseHaarTransform2D(image, levels); // 2D Haar 역변환 수행
    writeRawImage(filename + "_reconstructed_image_case2.raw", image); // 결과 이미지 저장
    mse = calculateMSE(originalImage, image); // MSE 계산
    cout << fixed << setprecision(2) << "Case 2 - 평균 제곱 오차 (MSE): " << mse << endl; // 결과 출력

    // Case 3: 각 영역에 대해 7-bit, 7-bit, 6-bit, 5-bit 양자화 수행
    image = origin_haar_image; // Haar 변환된 이미지 설정
    int bitDepths_3[4] = { 7, 7, 6, 5 };
    performQuantization(currentHeight, currentWidth, image, bitDepths_3); // 양자화 수행
    performDequantization(currentHeight, currentWidth, image, bitDepths_3); // 역양자화 수행
    inverseHaarTransform2D(image, levels); // 2D Haar 역변환 수행
    writeRawImage(filename + "_reconstructed_image_case3.raw", image); // 결과 이미지 저장
    mse = calculateMSE(originalImage, image); // MSE 계산
    cout << "7bit, 7bit, 6bit, 5bit 양자화 후 평균 제곱 오차 (MSE): " << mse << endl; // 결과 출력
    cout << fixed << setprecision(2) << "Case 3 - 평균 제곱 오차 (MSE): " << mse << endl; // 결과 출력

    //add Case 3: 각 영역에 대해 8-bit, 7-bit, 4-bit, 3-bit 양자화 수행
    image = origin_haar_image; // Haar 변환된 이미지 설정
    int bitDepths_4[4] = { 6, 7, 6, 4 }; // 각 영역에 대해 8-bit, 7-bit, 4-bit, 3-bit 양자화 수행
    performQuantization(currentHeight, currentWidth, image, bitDepths_4); // 양자화 수행
    performDequantization(currentHeight, currentWidth, image, bitDepths_4); // 역양자화 수행
    inverseHaarTransform2D(image, levels); // 2D Haar 역변환 수행
    writeRawImage(filename + "_reconstructed_image_case3_add.raw", image); // 결과 이미지 저장
    mse = calculateMSE(originalImage, image); // MSE 계산
    cout << "6bit, 7bit, 6bit, 4bit 양자화 후 평균 제곱 오차 (MSE): " << mse << endl; // 결과 출력
    cout << fixed << setprecision(2) << "Case 3 - 평균 제곱 오차 (MSE): " << mse << endl; // 결과 출력

    cout << "Haar 웨이블릿 변환 및 복원 완료!" << endl; // 완료 메시지 출력
    return 0;
}

// 파일 이름에서 이미지 크기 추출
void parseDimensions(const string& filename, int& width, int& height) {
    regex re(R"((\d+)x(\d+))"); // 정규 표현식으로 크기 추출
    smatch match; // 매치 변수 선언

    if (regex_search(filename, match, re)) {
        width = stoi(match[1]);  // 첫 번째 그룹: 너비
        height = stoi(match[2]); // 두 번째 그룹: 높이
    }
    else {
        cerr << "오류: 파일 이름에서 크기를 추출할 수 없습니다!" << endl;
        exit(1);
    }
}

// 1D Haar 변환
void haarTransform1D(vector<int>& data) {
    int n = data.size(); // 데이터 크기
    vector<int> temp(data); // 임시 벡터 생성

    for (int i = 0; i < n / 2; i++) { // 반복문
        data[i] = (temp[2 * i] + temp[2 * i + 1]) / 2;  // 저주파 성분
        data[i + n / 2] = (temp[2 * i] - temp[2 * i + 1]) / 2;  // 고주파 성분
    }
}

// 1D Haar 역변환
void inverseHaarTransform1D(vector<int>& data) {
    int n = data.size(); // 데이터 크기
    vector<int> temp(n); // 임시 벡터 생성

    for (int i = 0; i < n / 2; i++) { // 반복문
        temp[2 * i] = data[i] + data[i + n / 2]; // 역변환 수행
        temp[2 * i + 1] = data[i] - data[i + n / 2]; // 역변환 수행
    }

    for (int i = 0; i < n; i++) {
        data[i] = temp[i]; // 결과 저장
    }
}

// 2D Haar 변환 (다단계)
void haarTransform2D(vector<vector<int>>& image, int levels) {
    int height = image.size(); // 높이
    int width = image[0].size(); // 너비

    for (int level = 0; level < levels; level++) { // 다단계 변환 수행
        int currentHeight = height >> level; // 현재 높이
        int currentWidth = width >> level; // 현재 너비

        // 행 변환 수행
        for (int i = 0; i < currentHeight; i++) {
            vector<int> row(currentWidth); // 행 벡터 생성
            for (int j = 0; j < currentWidth; j++) {
                row[j] = image[i][j]; // 행 데이터 설정
            }
            haarTransform1D(row); // 1D Haar 변환 수행
            for (int j = 0; j < currentWidth; j++) { // 결과 저장
                image[i][j] = row[j]; // 결과 저장
            }
        }

        // 열 변환 수행
        for (int j = 0; j < currentWidth; j++) {
            vector<int> column(currentHeight); // 열 벡터 생성
            for (int i = 0; i < currentHeight; i++) {
                column[i] = image[i][j]; // 열 데이터 설정
            }
            haarTransform1D(column); // 1D Haar 변환 수행
            for (int i = 0; i < currentHeight; i++) {
                image[i][j] = column[i]; // 결과 저장
            }
        }
    }
}

// 2D Haar 역변환 (다단계)
void inverseHaarTransform2D(vector<vector<int>>& image, int levels) {
    int height = image.size(); // 높이
    int width = image[0].size(); // 너비

    for (int level = levels - 1; level >= 0; level--) {
        int currentHeight = height >> level; // 현재 높이
        int currentWidth = width >> level; // 현재 너비

        // 열 역변환 수행
        for (int j = 0; j < currentWidth; j++) {
            vector<int> column(currentHeight); // 열 벡터 생성
            for (int i = 0; i < currentHeight; i++) {
                column[i] = image[i][j]; // 열 데이터 설정
            }
            inverseHaarTransform1D(column); // 1D Haar 역변환 수행
            for (int i = 0; i < currentHeight; i++) {
                image[i][j] = column[i]; // 결과 저장
            }
        }

        // 행 역변환 수행
        for (int i = 0; i < currentHeight; i++) {
            vector<int> row(currentWidth); // 행 벡터 생성
            for (int j = 0; j < currentWidth; j++) {
                row[j] = image[i][j]; // 행 데이터 설정
            }
            inverseHaarTransform1D(row); // 1D Haar 역변환 수행
            for (int j = 0; j < currentWidth; j++) {
                image[i][j] = row[j]; // 결과 저장
            }
        }
    }
}

// RAW 파일에서 이미지 읽기
void readRawImage(const string& filename, vector<vector<int>>& image) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "파일 열기 오류!" << endl;
        exit(1);
    }

    int height = image.size(); // 높이
    int width = image[0].size(); // 너비

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            unsigned char pixel; // 픽셀 변수 선언
            file.read(reinterpret_cast<char*>(&pixel), sizeof(unsigned char)); // 파일에서 픽셀 읽기
            image[i][j] = static_cast<int>(pixel); // 이미지 데이터 설정
        }
    }

    file.close();
}

// RAW 파일로 이미지 저장
void writeRawImage(const string& filename, const vector<vector<int>>& image) {
    ofstream file(filename, ios::binary);
    if (!file) {
        cerr << "파일 열기 오류!" << endl;
        exit(1);
    }

    int height = image.size(); // 높이
    int width = image[0].size(); // 너비

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            unsigned char pixel = static_cast<unsigned char>(image[i][j]); // 픽셀 값 설정
            file.write(reinterpret_cast<const char*>(&pixel), sizeof(unsigned char)); // 파일에 픽셀 쓰기
        }
    }

    file.close();
}

// MSE 계산
double calculateMSE(const vector<vector<int>>& original, const vector<vector<int>>& reconstructed) {
    double mse = 0.0; // MSE 변수 선언
    int height = original.size(); // 높이
    int width = original[0].size(); // 너비

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            mse += pow(original[i][j] - reconstructed[i][j], 2); // MSE 계산
        }
    }
    return mse / (height * width); // MSE 반환
}

// 양자화 수행
void quantizeBlock(int startHeight, int endHeight, int startWidth, int endWidth, int bitDepth, vector<vector<int>>& image) {
    int maxValue = (1 << bitDepth) - 1;  // 최대값 계산

    for (int i = startHeight; i < endHeight; i++) {
        for (int j = startWidth; j < endWidth; j++) {
            image[i][j] = round(image[i][j] * ((static_cast<double>(maxValue) / 255.0))); // 양자화 수행
        }
    }

}

// 전체 이미지에 대해 양자화 수행 (Case 3에서 사용)
void performQuantization(int currentHeight, int currentWidth, vector<vector<int>>& image, int bitDepths[4]) {
    // LL3 영역에 대해 7-bit 양자화 수행
    quantizeBlock(0, currentHeight / 8, 0, currentWidth / 8, bitDepths[0], image);

    // LH3, HL3, HH3 영역에 대해 7-bit 양자화 수행
    quantizeBlock(currentHeight / 8, currentHeight / 4, 0, currentWidth / 8, bitDepths[1], image); // LH3
    quantizeBlock(0, currentHeight / 8, currentWidth / 8, currentWidth / 4, bitDepths[1], image); // HL3
    quantizeBlock(currentHeight / 8, currentHeight / 4, currentWidth / 8, currentWidth / 4, bitDepths[1], image); // HH3

    // LH2, HL2, HH2 영역에 대해 6-bit 양자화 수행
    quantizeBlock(currentHeight / 4, currentHeight / 2, 0, currentWidth / 4, bitDepths[2], image); // LH2
    quantizeBlock(0, currentHeight / 4, currentWidth / 4, currentWidth / 2, bitDepths[2], image); // HL2
    quantizeBlock(currentHeight / 4, currentHeight / 2, currentWidth / 4, currentWidth / 2, bitDepths[2], image); // HH2

    // LH1, HL1, HH1 영역에 대해 5-bit 양자화 수행
    quantizeBlock(currentHeight / 2, currentHeight, 0, currentWidth / 2, bitDepths[3], image); // LH1
    quantizeBlock(0, currentHeight / 2, currentWidth / 2, currentWidth, bitDepths[3], image); // HL1
    quantizeBlock(currentHeight / 2, currentHeight, currentWidth / 2, currentWidth, bitDepths[3], image); // HH1
}

void dequantizeBlock(int startHeight, int endHeight, int startWidth, int endWidth, int bitDepth, vector<vector<int>>& image) {
    int maxValue = (1 << bitDepth) - 1;  // 최대값 계산

    for (int i = startHeight; i < endHeight; i++) {
        for (int j = startWidth; j < endWidth; j++) {
            image[i][j] = round(image[i][j] * (255.0 / maxValue)); // 역양자화 수행
        }
    }
}

void performDequantization(int currentHeight, int currentWidth, vector<vector<int>>& image, int bitDepths[4]) {
    // LL3 영역에 대해 7-bit 역양자화 수행
    dequantizeBlock(0, currentHeight / 8, 0, currentWidth / 8, bitDepths[0], image);

    // LH3, HL3, HH3 영역에 대해 7-bit 역양자화 수행
    dequantizeBlock(currentHeight / 8, currentHeight / 4, 0, currentWidth / 8, bitDepths[1], image); // LH3
    dequantizeBlock(0, currentHeight / 8, currentWidth / 8, currentWidth / 4, bitDepths[1], image); // HL3
    dequantizeBlock(currentHeight / 8, currentHeight / 4, currentWidth / 8, currentWidth / 4, bitDepths[1], image); // HH3

    // LH2, HL2, HH2 영역에 대해 6-bit 역양자화 수행
    dequantizeBlock(currentHeight / 4, currentHeight / 2, 0, currentWidth / 4, bitDepths[2], image); // LH2
    dequantizeBlock(0, currentHeight / 4, currentWidth / 4, currentWidth / 2, bitDepths[2], image); // HL2
    dequantizeBlock(currentHeight / 4, currentHeight / 2, currentWidth / 4, currentWidth / 2, bitDepths[2], image); // HH2

    // LH1, HL1, HH1 영역에 대해 5-bit 역양자화 수행
    dequantizeBlock(currentHeight / 2, currentHeight, 0, currentWidth / 2, bitDepths[3], image); // LH1
    dequantizeBlock(0, currentHeight / 2, currentWidth / 2, currentWidth, bitDepths[3], image); // HL1
    dequantizeBlock(currentHeight / 2, currentHeight, currentWidth / 2, currentWidth, bitDepths[3], image); // HH1
}
