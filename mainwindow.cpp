#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>

#include "TreeModel.h"

std::shared_ptr<VariTree> get_addr_task(const std::string& file);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->fileEdit->setReadOnly(true);

    this->dumpObjectTree();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openBtn_clicked() {
    QFileDialog::Options options;
    QString fileName = QFileDialog::getOpenFileName(this, QString("Open File"), QDir::homePath(), tr("dwarf Files *.axf *.elf (*.axf *.elf);;All Files * (*)"));
    if (!fileName.isEmpty()) {
        ui->fileEdit->setText(fileName);
        TreeModel* new_model = new TreeModel(get_addr_task(ui->fileEdit->text().toStdString()),this);
        if (model!=nullptr) {
            delete model;
            model = nullptr;
        }
        ui->treeView->setModel(new_model);
        model = new_model;
    }
}

void MainWindow::on_reloadBtn_clicked() {
    if (model!=nullptr) {
        if (ui->fileEdit->text().isEmpty()) {return;}
        TreeModel* new_model = new TreeModel(get_addr_task(ui->fileEdit->text().toStdString()),this);
        ui->treeView->setModel(new_model);
        if (model!=nullptr) {
            delete model;
            model = nullptr;
        }
        model = new_model;
    }
}
