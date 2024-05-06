#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "seam_carver.h"

#include <QMainWindow>
#include <QLabel>
#include <QImage>
#include <QPushButton>
#include <QMap>
#include <QSpinBox>
#include <QComboBox>

#include <vector>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QMap <const QString, std::pair<const Kernel*, const Kernel*>> name2kernel{
        {"Sobel", {&SobelX, &SobelY}},
        {"Prewitt", {&PrewittX, &PrewittY}},
        {"Scharr", {&ScharrX, &ScharrY}},
        {"Roberts", {&RobertsX, &RobertsY}}
    };
    const QString normal_button_stylesheet = "QPushButton { height: 30px; border-radius: 5px; background-color: white; border: 1px solid grey; }";
    const QString stress_button_stylesheet = "QPushButton { height: 30px; border-radius: 5px; background-color: #8764B8; color: white; }";
    const QString combobox_stylesheet = "height: 30px;";
    // const QString combobox_stylesheet = "QComboBox { height: 30px; border-radius: 5px; background-color: white; border: 1px solid grey; }";
    const QString spinbox_stylesheet = "height: 30px;";
    QString last_operator = "";

    QPushButton *seam_button;
    QSpinBox *seam_width_spinbox;
    QComboBox *operator_combobox;
    QComboBox *direction_combobox;
    QComboBox *step_combobox;
    QLabel *original_label;
    QLabel *modified_label;
    QImage original_image;
    QImage modified_image;
    QImage modified_image_energy;
    // 缩放比例 = 窗口宽度 / 原图像宽度
    // 用于在比较原图片和修改后的图片时保持缩放比例一致
    std::vector<QWidget *> functional_widgets;
    double scale_factor;
    int n_seam_width;
    bool energy_toggled = false;

    void show_modified();
    void show_image(QLabel *label, QImage image);

private slots:
    void on_open_button_clicked();
    void on_save_button_clicked();
    void on_reset_button_clicked();
    void on_clean_button_clicked();
    void on_energy_checkbox_changed(const int state);
    void on_seam_spinbox_changed(const int value);
    void on_seam_button_clicked();
    void on_step_combobox_changed(const QString &text);
};
#endif // MAINWINDOW_H
