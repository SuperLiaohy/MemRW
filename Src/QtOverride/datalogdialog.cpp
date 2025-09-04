//
// Created by liaohy on 9/4/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DataLogDialog.h" resolved

#include "datalogdialog.h"
#include "ui_datalogdialog.h"

#include <QFileDialog>

DataLogDialog::DataLogDialog(const QString& default_path, bool islog, QWidget *parent) :
        QDialog(parent), ui(new Ui::DataLogDialog) {
    ui->setupUi(this);
    ui->lineEdit->setText(default_path);
    ui->checkBox->setChecked(islog);
    connect(ui->toolButton, &QToolButton::clicked, this, [this](){
        QString file_path = QFileDialog::getOpenFileName(this,"select a file to log data", QDir::homePath());
        if (file_path.isEmpty()) {return;}
        ui->lineEdit->setText(file_path);
    });
}

DataLogDialog::~DataLogDialog() {
    delete ui;
}

QString DataLogDialog::fileName() {
    return ui->lineEdit->text();
}

bool DataLogDialog::islog() {
    return ui->checkBox->isChecked();
}
