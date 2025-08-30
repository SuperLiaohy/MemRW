#include "mainwindow.h"
#include "./ui_mainwindow.h"


#include "TreeModel.h"

std::shared_ptr<VariTree> get_addr_task();

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    TreeModel* model = new TreeModel(get_addr_task(),this);

    ui->treeView->setModel(model);

    this->dumpObjectTree();

}

MainWindow::~MainWindow()
{
    delete ui;
}
