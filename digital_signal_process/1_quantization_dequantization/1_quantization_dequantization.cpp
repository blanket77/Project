#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <string>
#include <iomanip>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>

using namespace std;

// 비트 단위 패킹 함수
vector<unsigned char> packData(const vector<int>& data, int numBits) {
    vector<unsigned char> packedData; // 패킹된 데이터 저장
    unsigned int buffer = 0; // 비트 버퍼
    int bitsInBuffer = 0;    // 버퍼에 저장된 비트 수

    for (int value : data) {
        // 값이 numBits를 초과하지 않도록 확인
        if (value >= (1 << numBits)) {
            cerr << "Value " << value << " exceeds the maximum for " << numBits << " bits." << endl;
            exit(1);
        }

        buffer |= (value << bitsInBuffer); // 값을 버퍼에 추가
        bitsInBuffer += numBits; // 버퍼에 저장된 비트 수 증가

        // 버퍼에 8비트 이상이 되면 바이트로 저장
        while (bitsInBuffer >= 8) { // 8비트 이상이면
            packedData.push_back(static_cast<unsigned char>(buffer & 0xFF)); /// 버퍼의 마지막 8비트를 저장
            buffer >>= 8; // 마지막 8비트를 버퍼에서 제거
            bitsInBuffer -= 8; // 버퍼에 저장된 비트 수 감소
        }
    }

    // 남은 비트가 있으면 마지막 바이트에 저장
    if (bitsInBuffer > 0) {
        packedData.push_back(static_cast<unsigned char>(buffer & 0xFF)); // 남은 비트를 저장
    }

    return packedData; // 패킹된 데이터 반환
}

// 비트 단위 언패킹 함수
vector<int> unpackData(const vector<unsigned char>& packedData, int numBits, int originalSize) {
    vector<int> unpackedData(originalSize, 0); // 언패킹된 데이터 저장
    unsigned int buffer = 0; // 비트 버퍼
    int bitsInBuffer = 0; // 버퍼에 저장된 비트 수
    size_t byteIndex = 0; // 바이트 인덱스

    for (int i = 0; i < originalSize; ++i) {
        // 버퍼에 충분한 비트가 될 때까지 추가
        while (bitsInBuffer < numBits && byteIndex < packedData.size()) {
            buffer |= (static_cast<unsigned int>(packedData[byteIndex]) << bitsInBuffer); // 버퍼에 추가
            bitsInBuffer += 8; // 8비트씩 증가
            byteIndex++; // 바이트 인덱스 증가
        }

        // 버퍼에서 numBits 만큼 추출
        unpackedData[i] = buffer & ((1 << numBits) - 1); // numBits만큼 추출
        buffer >>= numBits; // 추출한 비트 제거
        bitsInBuffer -= numBits; // 버퍼에 저장된 비트 수 감소
    }

    return unpackedData; // 언패킹된 데이터 반환
}

// 파일 크기를 반환하는 함수
long getFileSize(const string& filename) {
    struct stat stat_buf; // 파일 정보를 저장할 구조체
    int rc = stat(filename.c_str(), &stat_buf); // 파일 정보를 가져옴
    return rc == 0 ? stat_buf.st_size : -1; // 파일 정보가 정상적으로 가져오면 파일 크기 반환, 그렇지 않으면 -1 반환
}

// 이미지 파일을 쓰는 함수
void writeImage(const string& filename, const vector<unsigned char>& data) {
    ofstream file(filename, ios::binary); // 이진 파일로 저장
    if (file) { // 파일이 정상적으로 열리면
        file.write(reinterpret_cast<const char*>(data.data()), data.size()); // 데이터를 파일에 씀
        file.close(); // 파일을 닫음
    }
    else {
        cerr << "Failed to write the image data to " << filename << endl; // 파일을 열지 못하면 오류 메시지 출력
    }
}

// 코드북을 텍스트 파일로 저장하는 함수
void saveCodebook(const string& filename, const vector<vector<double>>& codebook) {
    ofstream file(filename); // 파일을 열음
    if (file) {
        for (const auto& centroid : codebook) {
            file << fixed << setprecision(2); // 소수점 이하 두 자리까지 출력
            file << centroid[0] << " " << centroid[1] << " " << centroid[2] << "\n"; // 코드북을 파일에 씀
        }
        file.close(); // 파일을 닫음
    }
    else {
        cerr << "Failed to save the codebook to " << filename << endl; // 파일을 열지 못하면 오류 메시지 출력
    }
}

// 벡터 양자화 클래스 (3차원 RGB 벡터를 대상으로 함)
class QuantizerVector {
private:
    vector<vector<double>> centroids;  // 클러스터 중심
    vector<int> assignments;           // 각 데이터 포인트의 클러스터 할당 정보

    // 유클리드 거리 계산 함수 (제곱 거리 반환)
    double calculateDistance(const vector<unsigned char>& v1, const vector<double>& v2) {
        double dist = 0.0; // 거리 초기화
        for (int i = 0; i < 3; ++i) { // RGB 채널에 대해
            double diff = static_cast<double>(v1[i]) - v2[i]; // 차이 계산
            dist += diff * diff; // 제곱 거리 계산
        }
        return dist; // 제곱 거리
    }

public:
    QuantizerVector() {}

    // 코드북 반환 함수
    const vector<vector<double>>& getCodebook() const {
        return centroids;
    }

    // K-Means 클러스터링 수행
    void kMeansClustering(const vector<vector<unsigned char>>& data, int numClusters) {
        int dataSize = data.size(); // 데이터 크기
        centroids.resize(numClusters, vector<double>(3, 0.0)); // 클러스터 중심 초기화 
        assignments.resize(dataSize, -1); // 클러스터 할당 정보 초기화

        // 초기 클러스터 중심 설정 (데이터 중에서 무작위 선택)
        srand(static_cast<unsigned int>(time(0))); // 난수 시드 설정
        for (int i = 0; i < numClusters; ++i) { // 클러스터 수만큼 반복
            int idx = rand() % dataSize; // 무작위 인덱스 선택
            for (int j = 0; j < 3; ++j) { // RGB 채널에 대해
                centroids[i][j] = static_cast<double>(data[idx][j]); // 클러스터 중심 설정
            }
        }

        bool changed = true; // 클러스터 할당 정보 변경 여부
        int iter = 0; // 반복 횟수
        double previousLoss = numeric_limits<double>::max(); // 이전 손실 값

        while (changed && iter < 10) {  // 최대 100번의 반복 수행
            changed = false; // 변경 여부 초기화
            iter++; // 반복 횟수 증가

            // 각 데이터 포인트에 가장 가까운 클러스터 할당
            for (int i = 0; i < dataSize; ++i) {
                double minDist = numeric_limits<double>::max(); // 최소 거리 초기화
                int nearCluster = -1;  // 가장 가까운 클러스터 인덱스

                for (int k = 0; k < numClusters; ++k) { // 모든 클러스터에 대해
                    double dist = calculateDistance(data[i], centroids[k]); // 거리 계산
                    if (dist < minDist) { // 최소 거리보다 작으면
                        minDist = dist; // 최소 거리 갱신 
                        nearCluster = k; // 가장 가까운 클러스터 인덱스 저장
                    }
                }

                if (assignments[i] != nearCluster) { // 클러스터 할당 정보가 변경되면
                    assignments[i] = nearCluster; // 클러스터 할당 정보 갱신
                    changed = true; // 변경 여부 갱신
                }
            }

            // 클러스터 중심 업데이트
            vector<vector<double>> updateCentroids(numClusters, vector<double>(3, 0.0));
            vector<int> clusterSizes(numClusters, 0);

            for (int i = 0; i < dataSize; ++i) {
                int cluster = assignments[i]; // 클러스터 인덱스
                for (int j = 0; j < 3; ++j) {
                    updateCentroids[cluster][j] += static_cast<double>(data[i][j]); // 클러스터 중심 업데이트
                }
                clusterSizes[cluster]++; // 클러스터 크기 증가
            }

            for (int k = 0; k < numClusters; ++k) { // 모든 클러스터에 대해
                if (clusterSizes[k] > 0) { // 클러스터 크기가 0보다 크면
                    for (int j = 0; j < 3; ++j) { //
                        centroids[k][j] = updateCentroids[k][j] / clusterSizes[k]; // 클러스터 중심 업데이트
                    }
                }
            }

            // 손실(Loss)과 PSNR 계산
            double loss = 0.0;
            for (int i = 0; i < dataSize; ++i) { // 모든 데이터 포인트에 대해
                int cluster = assignments[i]; // 클러스터 인덱스
                for (int j = 0; j < 3; ++j) {
                    double diff = static_cast<double>(data[i][j]) - centroids[cluster][j]; // 차이 계산
                    loss += diff * diff; // 손실 계산
                }
            }

            double mse = loss / (dataSize * 3); // MSE 계산
            double psnr = (mse == 0) ? numeric_limits<double>::infinity() : 10 * log10((255.0 * 255.0) / mse); // PSNR 계산

            // 손실과 PSNR 출력
            cout << "Iteration " << iter << ": Loss = " << fixed << setprecision(2) << loss // 반복 횟수와 손실 출력
                << ", MSE = " << mse << ", PSNR = " << psnr << " dB" << endl; // MSE와 PSNR 출력

            // 반복 종료 조건 개선 (손실 변화가 매우 작을 경우)
            if (abs(previousLoss - loss) < 1e-5) { // 손실 변화가 매우 작으면
                cout << "Converged based on loss change threshold." << endl;
                break;
            }
            previousLoss = loss; // 이전 손실 값 갱신
        }
    }

    // 양자화된 데이터 생성
    vector<vector<unsigned char>> quantize(const vector<vector<unsigned char>>& data) { // 양자화된 데이터 생성
        int dataSize = data.size(); // 데이터 크기
        vector<vector<unsigned char>> quantizedData(dataSize, vector<unsigned char>(3)); // 양자화된 데이터 저장

        for (int i = 0; i < dataSize; ++i) { // 모든 데이터 포인트에 대해
            int cluster = assignments[i]; // 클러스터 인덱스
            for (int j = 0; j < 3; ++j) {
                quantizedData[i][j] = static_cast<unsigned char>(round(centroids[cluster][j])); // 반올림
                // 클램핑 추가
                if (quantizedData[i][j] > 255) quantizedData[i][j] = 255; // 255보다 크면 255로 설정
                if (quantizedData[i][j] < 0) quantizedData[i][j] = 0; // 0보다 작으면 0으로 설정
            }
        }

        return quantizedData; // 양자화된 데이터 반환
    }

    // MSE 계산 함수
    double getMSE(const vector<vector<unsigned char>>& original, const vector<vector<unsigned char>>& quantized) {
        double mse = 0.0; // MSE 초기화
        int dataSize = original.size(); // 데이터 크기

        for (int i = 0; i < dataSize; ++i) {
            for (int j = 0; j < 3; ++j) {
                double diff = static_cast<double>(original[i][j]) - static_cast<double>(quantized[i][j]); // 차이 계산
                mse += diff * diff; // 제곱 오차 누적
            }
        }

        return mse / (dataSize * 3); // MSE 반환
    }

    // PSNR 계산 함수
    double getPSNR(double mse) {
        if (mse == 0) return numeric_limits<double>::infinity(); // MSE가 0이면 무한대 반환
        return 10 * log10((255.0 * 255.0) / mse); // PSNR 계산
    }

    // 클러스터 할당 정보 반환 함수
    const vector<int>& getInformation() const {
        return assignments;
    }

    // 클러스터 할당 정보 설정 함수
    void setInformation(const vector<int>& loadedAssignments) {
        assignments = loadedAssignments;
    }
};

// 역 양자화 함수 (클러스터 인덱스를 코드북을 통해 원래의 RGB 값으로 복원)
vector<vector<unsigned char>> inverseQuantize(const vector<int>& indices, const vector<vector<double>>& codebook) {
    size_t dataSize = indices.size(); // 데이터 크기
    vector<vector<unsigned char>> reconstructedData(dataSize, vector<unsigned char>(3, 0)); // 복원된 데이터 저장

    for (size_t i = 0; i < dataSize; ++i) {
        int cluster = indices[i]; // 클러스터 인덱스
        if (cluster >= static_cast<int>(codebook.size())) { // 클러스터 인덱스가 코드북 크기보다 크면
            cerr << "Cluster index out of range: " << cluster << endl; // 오류 메시지 출력
            reconstructedData[i] = { 0, 0, 0 }; // 오류 발생 시 검은색 픽셀
            continue;
        }
        for (int j = 0; j < 3; ++j) {
            double value = round(codebook[cluster][j]); // 반올림
            // 클램핑 추가
            if (value > 255) value = 255; // 255보다 크면 255로 설정
            if (value < 0) value = 0; // 0보다 작으면 0으로 설정
            reconstructedData[i][j] = static_cast<unsigned char>(value); // 복원된 데이터 저장
        }
    }

    return reconstructedData; // 복원된 데이터 반환
}

int main() {
    string inputFileName = "Lenna_512x512_original.raw";
    ifstream file(inputFileName, ios::binary | ios::ate);
    if (!file) {
        cerr << "Error opening file: " << inputFileName << endl;
        return 1;
    }

    streamsize size = file.tellg(); // 파일 크기 계산
    file.seekg(0, ios::beg); // 파일 포인터를 파일의 시작으로 이동

    vector<unsigned char> buffer(size); // 파일 크기만큼 버퍼 생성
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) { // 파일을 읽어서 버퍼에 저장
        cerr << "Error reading file: " << inputFileName << endl; // 파일을 읽지 못하면 오류 메시지 출력
        return 1; // 프로그램 종료
    }
    file.close(); // 파일을 닫음

    // RGB 데이터를 3차원 벡터로 변환
    vector<vector<unsigned char>> imageData; // 이미지 데이터 저장
    imageData.reserve(buffer.size() / 3); // 저장 공간 할당
    for (size_t i = 0; i + 2 < buffer.size(); i += 3) { // 모든 픽셀에 대해
        vector<unsigned char> pixel = { buffer[i], buffer[i + 1], buffer[i + 2] }; // 픽셀 데이터 생성
        imageData.push_back(pixel); // 이미지 데이터 저장
    }

    // 양자화 비트 수 설정 (1부터 18까지)
    for (int bits = 1; bits <= 18; ++bits) {
        int numClusters = static_cast<int>(pow(2, bits)); // 클러스터 수 계산
        cout << "\nPerforming vector quantization with " << bits << " bits (" << numClusters << " clusters)..." << endl; // 비트 수 출력

        QuantizerVector quantizer; // 벡터 양자화 객체 생성
        quantizer.kMeansClustering(imageData, numClusters); // K-Means 클러스터링 수행
        vector<vector<unsigned char>> quantizedData = quantizer.quantize(imageData); // 양자화된 데이터 생성

        // MSE 및 PSNR 계산 
        double mse = quantizer.getMSE(imageData, quantizedData); // MSE 계산
        double psnr = quantizer.getPSNR(mse); // PSNR 계산

        cout << fixed << setprecision(2); // 소수점 이하 두 자리까지 출력
        cout << "Final MSE: " << mse << ", Final PSNR: " << psnr << " dB" << endl; // MSE와 PSNR 출력

        // 코드북 저장 (텍스트 파일로) 
        string codebookFilename = "Lenna_codebook_" + to_string(bits) + "bits.txt"; // 코드북 파일 이름
        saveCodebook(codebookFilename, quantizer.getCodebook()); // 코드북 저장
        cout << "Codebook saved to " << codebookFilename << endl; // 코드북 파일 이름 출력

        // 클러스터 할당 정보 저장
        // 클러스터 인덱스를 int로 저장
        vector<int> assignmentsPacked = quantizer.getInformation(); // 클러스터 할당 정보 저장

        // 클러스터 인덱스 패킹
        vector<unsigned char> packedIndices = packData(assignmentsPacked, bits); // 패킹된 클러스터 할당 정보 저장
        string packedFilename = "Lenna_vector_quantized_" + to_string(bits) + "bits.raw"; // 이름 변경
        writeImage(packedFilename, packedIndices); // 패킹된 클러스터 할당 정보 저장
        cout << "Packed indices saved to " << packedFilename << endl; // 패킹된 클러스터 할당 정보 파일 이름 출력

        // 파일 크기 출력
        long fileSize = getFileSize(packedFilename); // 파일 크기 계산
        if (fileSize != -1) {
            cout << "File size of " << packedFilename << ": " << fileSize << " bytes" << endl; // 파일 크기 출력
        }
        else {
            cout << "Could not determine file size for " << packedFilename << endl; // 파일 크기를 계산할 수 없으면 오류 메시지 출력
        }

        // 역 양자화 수행 (패킹된 데이터를 언패킹하여 복원)
        // 패킹된 데이터를 다시 언패킹
        vector<int> unpackedAssignments = unpackData(packedIndices, bits, quantizer.getInformation().size());

        // 클러스터 할당 정보가 올바르게 복원되었는지 검증
        bool mismatch = false; // 불일치 플래그
        for (size_t i = 0; i < quantizer.getInformation().size(); ++i) { // 모든 클러스터 인덱스에 대해
            if (quantizer.getInformation()[i] != unpackedAssignments[i]) { // 클러스터 인덱스가 일치하지 않으면
                mismatch = true; // 불일치 플래그 설정
                cerr << "Mismatch at index " << i << ": original=" << quantizer.getInformation()[i] // 오류 메시지 출력
                    << ", unpacked=" << unpackedAssignments[i] << endl; // 오류 메시지 출력
                break; // 루프 종료
            }
        }

        if (mismatch) { // 불일치가 발생하면
            cerr << "Cluster index mismatch after unpacking!" << endl; // 오류 메시지 출력
            continue; // 다음 비트 레벨로 넘어갑니다.
        }
        else {
            cout << "Cluster indices successfully packed and unpacked." << endl; // 클러스터 인덱스가 올바르게 복원되었으면 메시지 출력
        }

        // 역 양자화 수행 (올바른 코드북 전달)
        vector<vector<unsigned char>> reconstructedData = inverseQuantize(unpackedAssignments, quantizer.getCodebook());

        // 역 양자화된 데이터로부터 파일 생성
        vector<unsigned char> reconstructedOutput; // 역 양자화된 데이터 저장
        reconstructedOutput.reserve(reconstructedData.size() * 3); // 저장 공간 할당
        for (const auto& pixel : reconstructedData) { // 모든 픽셀에 대해
            reconstructedOutput.insert(reconstructedOutput.end(), pixel.begin(), pixel.end()); // 역 양자화된 데이터 저장
        }
        // 역 양자화된 이미지 저장
        string reconstructedFilename = "Lenna_reconstructed_" + to_string(bits) + "bits.raw"; // 이름 변경
        writeImage(reconstructedFilename, reconstructedOutput); // 역 양자화된 이미지 저장
        cout << "Reconstructed image saved to " << reconstructedFilename << endl; // 역 양자화된 이미지 파일 이름 출력


    }

    return 0;
}
