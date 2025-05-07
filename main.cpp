#include <iostream>
#include <fstream>
#include <windows.h>
#include <vector>
#include <cmath>

// Структура для заголовка BMP файла
#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t bfType;       // Тип файла (должен быть "BM")
    uint32_t bfSize;       // Размер файла в байтах
    uint16_t bfReserved1;   // Зарезервировано
    uint16_t bfReserved2;   // Зарезервировано
    uint32_t bfOffBits;     // Смещение до начала данных изображения
};
#pragma pack(pop)

// Структура для заголовка информации BMP
#pragma pack(push, 1)
struct BMPInfoHeader {
    uint32_t biSize;          // Размер этого заголовка в байтах
    int32_t  biWidth;         // Ширина изображения в пикселях
    int32_t  biHeight;        // Высота изображения в пикселях
    uint16_t biPlanes;        // Число плоскостей (должно быть 1)
    uint16_t biBitCount;      // Число бит на пиксель
    uint32_t biCompression;   // Тип сжатия
    uint32_t biSizeImage;     // Размер изображения в байтах
    int32_t  biXPelsPerMeter;  // Горизонтальное разрешение, пикселей на метр
    int32_t  biYPelsPerMeter;  // Вертикальное разрешение, пикселей на метр
    uint32_t biClrUsed;       // Число используемых цветов
    uint32_t biClrImportant;  // Число важных цветов
};
#pragma pack(pop)

// Класс для хранения информации о пикселе
class Pixel{
public:
    uint8_t B;
    uint8_t G;
    uint8_t R;
    bool check;

    Pixel() : B(0), G(0), R(0), check(false) {}

    void setPixel(uint8_t r, uint8_t g, uint8_t b){
        R = r;
        G = g;
        B = b;
    }
    bool isBlack() const{
        return (B == 0 && G == 0 && R == 0);
    }
};

// Основной класс для работы с BMP файлом
class BMP_Processor {
private:
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    std::vector<Pixel> pixels;
    int width;
    int height;
    bool open_file;

public:
    BMP_Processor() {}

    void readBMP(const std::string& filePath) {
        std::ifstream inputFile(filePath, std::ios::binary);
        if (!inputFile) {
            std::cerr << "Error: Could not open input file." << std::endl;
            open_file = 0;
            return;
        }
        open_file = 1;
        inputFile.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
        inputFile.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

        width = infoHeader.biWidth;
        height = std::abs(infoHeader.biHeight);

        inputFile.seekg(fileHeader.bfOffBits);
        pixels.resize(width * height);
        int padding = (3 * width) % 4;
        if (padding != 0) {
            padding = 4 - padding;
        }
        
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                uint8_t blue = inputFile.get();
                uint8_t green = inputFile.get();
                uint8_t red = inputFile.get();
                pixels[i * width + j].setPixel(red, green, blue);
            }
            inputFile.seekg(padding, std::ios::cur);
        }

        inputFile.close();
    }

    void displayImage() const {
        for (int y = height - 1; y >= 0; --y) {
            for (int x = 0; x < width; ++x) {
                if (pixels[y * width + x].isBlack()) {
                    std::cout << '1';
                } else {
                    std::cout << '0';
                }
            }
            std::cout << std::endl;
        }
        
    }

    bool is_open_file()
    {
        return open_file;
    }

    void drawLine(int x1, int y1, int x2, int y2) {
        int dx = std::abs(x2 - x1);
        int dy = std::abs(y2 - y1);
        int sx = x1 < x2 ? 1 : -1;
        int sy = y1 < y2 ? 1 : -1;
        int err = dx - dy;

        //Алгоритм Брезенхэма//
        while (true) {
            if (!pixels[y1 * width + x1].check){
                if (pixels[y1 * width + x1].isBlack())
                {
                    pixels[y1 * width + x1].setPixel(255, 255, 255);
                }
                else
                {
                    pixels[y1 * width + x1].setPixel(0, 0, 0);
                }
            }
            pixels[y1 * width + x1].check = true;

            if (x1 == x2 && y1 == y2) break;
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x1 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y1 += sy;
            }
        }
    }

    void saveBMP(const std::string& filePath) const {
        std::ofstream outputFile(filePath, std::ios::binary);
        if (!outputFile) {
            std::cerr << "Error: Could not open output file." << std::endl;
            return;
        }

        int padding = (4 - (width * 3) % 4) % 4;

        outputFile.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
        outputFile.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));
  
        for (int y = height - 1; y >= 0; --y) {
            for (int x = 0; x < width; ++x) {
                const Pixel& pixel = pixels[y * width + x];
                outputFile.write(reinterpret_cast<const char*>(&pixel.B), 1);
                outputFile.write(reinterpret_cast<const char*>(&pixel.G), 1);
                outputFile.write(reinterpret_cast<const char*>(&pixel.R), 1);
            }

            for (int p = 0; p < padding; ++p) {
                uint8_t pad = 0;
                outputFile.write(reinterpret_cast<const char*>(&pad), 1);
            }
        }

        outputFile.close();
    }

    ~BMP_Processor() {
    }
};

int main() {
    std::string inputFilePath;
    std::cout << "Enter input BMP file name: ";
    std::cin >> inputFilePath;

    BMP_Processor bmpProcessor;
    bmpProcessor.readBMP(inputFilePath);

    if (bmpProcessor.is_open_file() == 1) 
    {
        std::cout << "Original Image:" << std::endl;
        bmpProcessor.displayImage();

        int x1, y1, x2, y2;
        std::cout << "Enter coordinates for the first line (x1 y1 x2 y2): ";
        std::cin >> x1 >> y1 >> x2 >> y2;

        bmpProcessor.drawLine(x1, y1, x2, y2);

        std::cout << "Enter coordinates for the second line (x1 y1 x2 y2): ";
        std::cin >> x1 >> y1 >> x2 >> y2;

        bmpProcessor.drawLine(x1, y1, x2, y2);

        std::cout << "Modified Image:" << std::endl;
        bmpProcessor.displayImage();

        std::string outputFilePath;
        std::cout << "Enter output BMP file name: ";
        std::cin >> outputFilePath;

        bmpProcessor.saveBMP(outputFilePath);
    }

    return 0;
}
