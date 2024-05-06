#include "seam_carver.h"

#include <QImage>
#include <QTransform>

#include <array>

typedef int Kernel[3][3];

void rgb2gray(const QImage &image, QImage &output) {
    output = image;
    for (int i = 0; i < image.width(); i++) {
        for (int j = 0; j < image.height(); j++) {
            QRgb pixel = image.pixel(i, j);
            int gray = qGray(pixel);
            output.setPixel(i, j, qRgb(gray, gray, gray));
        }
    }
}

void calc_energy_conv(
    const QImage& image, QImage& output,
    const Kernel& kernelX, const Kernel& kernelY
) {
    output = image;
    Mat2d energy(image.width(), std::vector<int>(image.height(), 0));

    int max_grad = 0;
    int min_grad = std::numeric_limits<int>::max();

    const int col = image.width();
    const int row = image.height();
    for (int x = 0; x < col; x++) {
        for (int y = 0; y < row; y++) {
            int gx = 0;
            int gy = 0;
            for (int k = 0; k < 3; ++k) {
                for (int l = 0; l < 3; ++l) {
                    int _x = qBound(0, x + k - 1, col - 1);
                    int _y = qBound(0, y + l - 1, row - 1);
                    int pixel = qGray(image.pixel(_x, _y));
                    gx += pixel * kernelX[k][l];
                    gy += pixel * kernelY[k][l];
                }
            }
            energy[x][y] = (qAbs(gx) + qAbs(gy)) / 2;
            max_grad = qMax(max_grad, energy[x][y]);
            min_grad = qMin(min_grad, energy[x][y]);
        }
    }

    // 正则化
    normalize(output, energy, max_grad, min_grad);
}

void calc_energy_forward(
    QImage &image, QImage &output
) {
    const int row = image.height();
    const int col = image.width();
    int max_energy = 0;
    int min_energy = std::numeric_limits<int>::max();
    output = QImage(col, row, image.format());

    Mat2d energy(col, std::vector<int>(row, 0));

    for (int y = 0; y < row; y++) {
        for (int x = 0; x < col; x++) {
            int top = qBound(0, y - 1, row - 1);
            int below = qBound(0, y + 1, row - 1);
            int left = qBound(0, x - 1, col - 1);
            int right = qBound(0, x + 1, col - 1);

            int cT = qAbs(qGray(image.pixel(left, y)) - qGray(image.pixel(right, y)));
            int cL = qAbs(qGray(image.pixel(x, top)) - qGray(image.pixel(left, y))) + cT;
            int cR = qAbs(qGray(image.pixel(x, top)) - qGray(image.pixel(right, y))) + cT;

            std::array<int, 3> cTLR = {cT, cL, cR};
            energy[x][y] = *min_element(cTLR.begin(), cTLR.end());

            min_energy = qMin(min_energy, energy[x][y]);
            max_energy = qMax(max_energy, energy[x][y]);
        }
    }

    // 正则化
    normalize(output, energy, max_energy, min_energy);
}

void seam_carve(
    QImage& image, QImage &energy,
    const Kernel& kernelX, const Kernel& kernelY
) {
    calc_energy_conv(image, energy, kernelX, kernelY);
    find_seam_and_carve(image, energy);
}

void seam_carve_horizontally(
    QImage& image, QImage &energy,
    const Kernel& kernelX, const Kernel& kernel
) {
    transpose(image);
    transpose(energy);
    seam_carve(image, energy, kernelX, kernel);
    transpose(image);
    transpose(energy);
}

void seam_carve_forward(QImage& image, QImage &energy) {
    calc_energy_forward(image, energy);
    find_seam_and_carve(image, energy);
}

void seam_carve_forward_horizontally(QImage& image, QImage &energy) {
    transpose(image);
    transpose(energy);
    seam_carve_forward(image, energy);
    transpose(image);
    transpose(energy);
}

void find_seam_and_carve(QImage& image, QImage &energy) {
    const int row = energy.height();
    const int col = energy.width();
    // dp_sum[i][j] 表示以 (j, i) 结尾的最小路径的总能量
    Mat2d dp_sum;
    // dp_from[i][j] 表示以 (j, i) 结尾的最小路径在 i - 1 行的 x 坐标
    Mat2d dp_from;
    dp_sum.resize(col);
    dp_from.resize(col);
    for (int i = 0; i < col; i++) {
        dp_sum[i].resize(row);
        dp_from[i].resize(row);
    }
    for (int i = 0; i < col; i++) {
        dp_sum[i][0] = qGray(energy.pixel(i, 0));
        dp_from[i][0] = i;
    }

    for (int i = 1; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            int sum_left_top = (j == 0) ? INT_MAX : dp_sum[j - 1][i - 1];
            int sum_right_top = (j == col - 1) ? INT_MAX : dp_sum[j + 1][i - 1];
            int sum_top = dp_sum[j][i - 1];

            std::array<int, 3> sums = {sum_top, sum_left_top, sum_right_top};
            int sum_min = *min_element(sums.begin(), sums.end());

            dp_from[j][i] = (sum_min == sum_top) ? j : ((sum_min == sum_left_top) ? j - 1 : j + 1);
            dp_sum[j][i] = sum_min + qGray(energy.pixel(j, i));
        }
    }

    // 找到最小路径的结束点
    int min_energy_col = 0;
    int min_energy = dp_sum[0][row - 1];
    for (int i = 1; i < col; ++i) {
        if (dp_sum[i][row - 1] < min_energy) {
            min_energy = dp_sum[i][row - 1];
            min_energy_col = i;
        }
    }

    // 构造最小路径
    // seam[i] 表示 seam 在第 i 行的 x 坐标
    std::vector<int> seam;
    seam.resize(row);
    seam[row - 1] = min_energy_col;
    for (int i = row - 2; i >= 0; --i)
        seam[i] = dp_from[seam[i + 1]][i + 1];

    // 剪切最小路径
    QImage new_image = QImage(col - 1, row, image.format());
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < seam[i]; j++) {
            new_image.setPixel(j, i, image.pixel(j, i));
        }
        for (int j = seam[i]; j < col - 1; j++) {
            new_image.setPixel(j, i, image.pixel(j + 1, i));
        }
    }
    image = new_image;
}

void transpose(QImage& image) {
    image = image.transformed(QTransform().rotate(90).scale(-1, 1));
}

void normalize(
    QImage &image, Mat2d &energy,
    int max_energy, int min_energy
) {
    int range = max_energy - min_energy;
    if (range == 0) range = 1;
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            int g = (energy[x][y] - min_energy) * 255 / range;
            image.setPixel(x, y, qRgb(g, g, g));
        }
    }
}