#include "mainwindow.h"

#include <QApplication>
#include <QLayout>
#include <QDebug>
#include <QFileDialog>
#include <QPixmap>
#include <QMessageBox>
#include <QCheckBox>
#include <QTimer>

#include <vector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 设置窗口参数
    this->setWindowTitle("Seam Carving");
    const int window_width = 1200;
    const int window_height = 600;
    this->setGeometry(500, 250, window_width, window_height);

    /**
     * 图像操作区
    */

    // 在 Grid Layout 中，按钮的推荐平均宽度为 100，推荐平均高度为 50
    const int n_buttons_line = 5;
    const int button_width = 100;
    const int button_height = 50;

    QWidget *functional_area_widget = new QWidget(this);

    // 如果希望添加其他组件，需要在 functional_widgets 中添加
    QPushButton *open_button = new QPushButton("Open", functional_area_widget);
    QPushButton *save_button = new QPushButton("Save", functional_area_widget);
    QPushButton *reset_button = new QPushButton("Reset", functional_area_widget);
    QPushButton *clean_button = new QPushButton("Clean", functional_area_widget);
    QCheckBox *energy_checkbox = new QCheckBox("Energy", functional_area_widget);
    seam_width_spinbox = new QSpinBox(functional_area_widget);
    seam_button = new QPushButton("Seam", functional_area_widget);
    operator_combobox = new QComboBox(functional_area_widget);
    direction_combobox = new QComboBox(functional_area_widget);
    step_combobox = new QComboBox(functional_area_widget);

    seam_width_spinbox->setValue(50);
    seam_width_spinbox->setMinimum(0);

    operator_combobox->addItem("Sobel");
    operator_combobox->addItem("Prewitt");
    operator_combobox->addItem("Scharr");
    operator_combobox->addItem("Roberts");
    operator_combobox->addItem("Forward");
    operator_combobox->setCurrentIndex(0);

    direction_combobox->addItem("Vertical");
    direction_combobox->addItem("Horizontal");
    direction_combobox->setCurrentIndex(0);

    step_combobox->addItem("By pixels");
    step_combobox->addItem("By ratio");
    step_combobox->setCurrentIndex(0);

    energy_checkbox->setCheckState(Qt::Unchecked);

    open_button->setStyleSheet(normal_button_stylesheet);
    save_button->setStyleSheet(normal_button_stylesheet);
    reset_button->setStyleSheet(normal_button_stylesheet);
    clean_button->setStyleSheet(normal_button_stylesheet);
    energy_checkbox->setStyleSheet(normal_button_stylesheet);
    seam_button->setStyleSheet(stress_button_stylesheet);
    QString merged_spinbox_stylesheet = seam_width_spinbox->styleSheet() + spinbox_stylesheet;
    seam_width_spinbox->setStyleSheet(merged_spinbox_stylesheet);
    QString merged_combobox_stylesheet = operator_combobox->styleSheet() + combobox_stylesheet;
    operator_combobox->setStyleSheet(merged_combobox_stylesheet);
    direction_combobox->setStyleSheet(merged_combobox_stylesheet);
    step_combobox->setStyleSheet(merged_combobox_stylesheet);

    connect(open_button, SIGNAL(clicked()), this, SLOT(on_open_button_clicked()));
    connect(save_button, SIGNAL(clicked()), this, SLOT(on_save_button_clicked()));
    connect(reset_button, SIGNAL(clicked()), this, SLOT(on_reset_button_clicked()));
    connect(clean_button, SIGNAL(clicked()), this, SLOT(on_clean_button_clicked()));
    connect(energy_checkbox, &QCheckBox::stateChanged, this, &MainWindow::on_energy_checkbox_changed);
    connect(seam_width_spinbox, SIGNAL(valueChanged(int)), this, SLOT(on_seam_spinbox_changed(int)));
    connect(seam_button, SIGNAL(clicked()), this, SLOT(on_seam_button_clicked()));

    QGridLayout *operation_layout = new QGridLayout(functional_area_widget);
    functional_area_widget->setLayout(operation_layout);

    functional_widgets = {
        open_button, save_button, reset_button,
        clean_button, energy_checkbox, seam_width_spinbox,
        seam_button, operator_combobox, direction_combobox
    };

    operation_layout->addWidget(open_button, 0, 0);
    operation_layout->addWidget(save_button, 0, 1);
    operation_layout->addWidget(reset_button, 0, 2);
    operation_layout->addWidget(clean_button, 0, 3);
    operation_layout->addWidget(energy_checkbox, 0, 4);

    operation_layout->addWidget(operator_combobox, 1, 0);
    operation_layout->addWidget(direction_combobox, 1, 1);
    operation_layout->addWidget(step_combobox, 1, 2);
    operation_layout->addWidget(seam_width_spinbox, 1, 3);
    operation_layout->addWidget(seam_button, 1, 4);

    const int operation_widget_width = button_width * n_buttons_line;
    const int operation_widget_height = button_height * (functional_widgets.size() / n_buttons_line + 1);

    functional_area_widget->setGeometry(0, 0, operation_widget_width, operation_widget_height);

    /**
     * 图像对比显示区
    */

    QWidget *image_comparison_widget = new QWidget(this);

    image_comparison_widget->setGeometry(0, operation_widget_height, window_width, window_height - operation_widget_height);

    modified_label = new QLabel("Modified Image", image_comparison_widget);
    original_label = new QLabel("Original Image", image_comparison_widget);

    modified_label->setGeometry(0, 0, image_comparison_widget->width() / 2, image_comparison_widget->height());
    original_label->setGeometry(image_comparison_widget->width() / 2, 0, image_comparison_widget->width() / 2, image_comparison_widget->height());

    modified_label->setStyleSheet("QLabel { font-size : 18px; color : gray; border: 1px solid black; }");
    original_label->setStyleSheet("QLabel { font-size : 18px; color : gray; border: 1px solid black; }");

    modified_label->setAlignment(Qt::AlignCenter);
    original_label->setAlignment(Qt::AlignCenter);
}

void
MainWindow::on_open_button_clicked() {
    QFileDialog dialog(this);
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("选择图片文件"), "", tr("图片文件 (*.png *.jpg *.bmp *jpeg)"));
    if (fileName.isEmpty()) {
        return;
    }

    original_image = QImage(fileName);
    modified_image = QImage(fileName);

    auto original_pixmap = QPixmap::fromImage(original_image);
    auto modified_pixmap = QPixmap::fromImage(modified_image);

    // 记录缩放比例 scale_factor = 窗口宽度（高度） / 原图像宽度（高度）
    double label_ratio = (double) original_label->width() / (double) original_label->height();
    double image_ratio = (double) original_image.width() / (double) original_image.height();
    if (label_ratio > image_ratio) {
        scale_factor = (double) original_label->height() / (double) original_image.height();
    } else {
        scale_factor = (double) original_label->width() / (double) original_image.width();
    }

    show_image(original_label, original_image);
    show_image(modified_label, modified_image);

    // 设置 SpinBox 的最大值
    on_step_combobox_changed(step_combobox->currentText());
}

void
MainWindow::on_save_button_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存图片"), "", tr("图片文件 (*.png *.bmp *.jpg *jpeg)"));
    if (fileName.isEmpty()) {
        return;
    }
    modified_image.save(fileName);
}

void MainWindow::on_reset_button_clicked() {
    modified_image = original_image;
    show_modified();
}

void
MainWindow::on_clean_button_clicked() {
    original_label->setText("Original Image");
    modified_label->setText("Modified Image");
}

void
MainWindow::on_energy_checkbox_changed(const int state) {
    energy_toggled = state == Qt::Checked;
    show_modified();
}

void
MainWindow::on_seam_spinbox_changed(const int value) {
    n_seam_width = value;
}

void
MainWindow::on_seam_button_clicked() {
    if (modified_image.isNull()) {
        return;
    }

    int seam_pixels;
    if (step_combobox->currentText() == "By pixels") {
        seam_pixels = seam_width_spinbox->value();
    } else if (step_combobox->currentText() == "By ratio") {
        if (direction_combobox->currentText() == "Vertical") {
            seam_pixels = (int) ((double) seam_width_spinbox->value() / 100.0 * modified_image.width());
        } else if (direction_combobox->currentText() == "Horizontal") {
            seam_pixels = (int) ((double) seam_width_spinbox->value() / 100.0 * modified_image.height());
        }
    } else {
        return;
    }

    for (int i = 0; i < functional_widgets.size(); i++) {
        functional_widgets[i]->setEnabled(false);
    }
    for (int i = 0; i < seam_pixels; i++) {
        seam_button->setText(QString::number(i + 1) + "/" + QString::number(seam_pixels));

        last_operator = operator_combobox->currentText();
        if (operator_combobox->currentText() == "Forward") {
            if (direction_combobox->currentText() == "Vertical") {
                seam_carve_forward(modified_image, modified_image_energy);
            } else if (direction_combobox->currentText() == "Horizontal") {
                seam_carve_forward_horizontally(modified_image, modified_image_energy);
            } else {
                break;
            }
        }
        else {
            const Kernel *kernelX = name2kernel[operator_combobox->currentText()].first;
            const Kernel *kernelY = name2kernel[operator_combobox->currentText()].second;
            if (direction_combobox->currentText() == "Vertical") {
                seam_carve(modified_image, modified_image_energy, *kernelX, *kernelY);
            } else if (direction_combobox->currentText() == "Horizontal") {
                seam_carve_horizontally(modified_image, modified_image_energy, *kernelX, *kernelY);
            } else {
                break;
            }
        }

        show_modified();
    }
    seam_button->setText("Seam");
    for (int i = 0; i < functional_widgets.size(); i++) {
        functional_widgets[i]->setEnabled(true);
    }
}

void MainWindow::on_step_combobox_changed(const QString &text) {
    if (text == "By pixels") {
        seam_width_spinbox->setMinimum(0);
        seam_width_spinbox->setMaximum(qMin(modified_image.width() - 1, modified_image.height() - 1));
        seam_width_spinbox->setValue(qMin(50, seam_width_spinbox->maximum()));
    } else if (text == "By ratio") {
        seam_width_spinbox->setMinimum(0);
        seam_width_spinbox->setMaximum(100);
        seam_width_spinbox->setValue(qMin(50, seam_width_spinbox->maximum()));
    }
}

// 显示修改后的图片（根据是否勾选 energy checkbox 进行调整）
void MainWindow::show_modified() {
    if (energy_toggled) {
        if (
            modified_image_energy.isNull() ||
            modified_image_energy.width() != modified_image.width() ||
            modified_image_energy.height() != modified_image.height() ||
            last_operator != operator_combobox->currentText()
        ) {
            if (operator_combobox->currentText() == "Forward") {
                calc_energy_forward(modified_image, modified_image_energy);
            } else {
                const Kernel *kernelX = name2kernel[operator_combobox->currentText()].first;
                const Kernel *kernelY = name2kernel[operator_combobox->currentText()].second;
                calc_energy_conv(modified_image, modified_image_energy, *kernelX, *kernelY);
            }
        }
        // if (operator_combobox->currentText() == "Forward") {
        //     calc_energy_forward(modified_image, modified_image_energy);
        // } else {
        //     const Kernel *kernelX = name2kernel[operator_combobox->currentText()].first;
        //     const Kernel *kernelY = name2kernel[operator_combobox->currentText()].second;
        //     calc_energy_conv(modified_image, modified_image_energy, *kernelX, *kernelY);
        // }
        show_image(modified_label, modified_image_energy);
    } else {
        show_image(modified_label, modified_image);
    }
}

void MainWindow::show_image(QLabel *label, QImage image) {
    QPixmap pixmap = QPixmap::fromImage(image);
    label->setPixmap(pixmap.scaled((int)(scale_factor * image.width()),
                                    (int)(scale_factor * image.height()),
                                    Qt::KeepAspectRatio));
    label->setAlignment(Qt::AlignCenter);
    label->show();
    update();
    QEventLoop loop;
    QTimer::singleShot(10, &loop, SLOT(quit()));
    loop.exec();
}

MainWindow::~MainWindow() {}
