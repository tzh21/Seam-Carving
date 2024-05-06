#ifndef SEAM_CARVER_H
#define SEAM_CARVER_H

#include <QImage>

typedef int Kernel[3][3];
typedef std::vector<std::vector<int>> Mat2d;

const Kernel SobelX = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
const Kernel SobelY = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
const Kernel PrewittX = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
const Kernel PrewittY = {{-1, -1, -1}, {0, 0, 0}, {1, 1, 1}};
const Kernel ScharrX = {{-3, 0, 3}, {-10, 0, 10}, {-3, 0, 3}};
const Kernel ScharrY = {{-3, -10, -3}, {0, 0, 0}, {3, 10, 3}};
const Kernel RobertsX = {{0, 0, 0}, {0, 1, 0}, {0, 0, -1}};
const Kernel RobertsY = {{0, 0, 0}, {0, 0, 1}, {0, -1, 0}};

void rgb2gray(const QImage &image, QImage &output);

void calc_energy_conv(
    const QImage& image, QImage& output,
    const Kernel& kernelX, const Kernel& kernelY
);

void calc_energy_conv(
    const QImage& image, QImage& output,
    const Kernel& kernelX, const Kernel& kernelY
);

void calc_energy_forward(QImage &image, QImage &output);

void seam_carve(
    QImage& image, QImage &energy,
    const Kernel& kernelX, const Kernel& kernelY
);

void seam_carve_horizontally(
    QImage& image, QImage &energy,
    const Kernel& kernelX, const Kernel& kernel
);

void seam_carve_forward(QImage& image, QImage &energy);

void seam_carve_forward_horizontally(QImage& image, QImage &energy);

void find_seam_and_carve(QImage& image, QImage &energy);

void transpose(QImage& image);

void normalize(
    QImage &image, std::vector<std::vector<int>> &energy,
    int max_energy, int min_energy
);

#endif // SEAM_CARVER_H
