//
// Created by liaohy on 9/8/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ChartSettingDialog.h" resolved

#include "chartsettingdialog.h"
#include "ui_chartsettingdialog.h"


ChartSettingDialog::ChartSettingDialog(double xMin, double xMax, double yMin, double yMax, uint32_t buffer_size, QWidget *parent) :
        QDialog(parent), ui(new Ui::ChartSettingDialog) {
    ui->setupUi(this);
    ui->sizeBox->setValue(buffer_size);
    ui->xMinBox->setValue(xMin);
    ui->xMaxBox->setValue(xMax);
    ui->yMinBox->setValue(yMin);
    ui->yMaxBox->setValue(yMax);

}

ChartSettingDialog::~ChartSettingDialog() {
    delete ui;
}

double ChartSettingDialog::xMin() {
    return ui->xMinBox->value();
}

double ChartSettingDialog::xMax() {
    return ui->xMaxBox->value();
}

double ChartSettingDialog::yMin() {
    return ui->yMinBox->value();
}

double ChartSettingDialog::yMax() {
    return ui->yMaxBox->value();
}

int ChartSettingDialog::bufferSize() {
    return ui->sizeBox->value();
}
