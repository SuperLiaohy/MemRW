#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

#include "TreeModel.h"
#include "groupitemadddialog.h"


std::shared_ptr<VariTree> get_addr_task(const std::string& file);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->fileEdit->setReadOnly(true);

    ui->group_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->group_treeWidget, &QTreeWidget::customContextMenuRequested, this, &MainWindow::customGroupMenuRequested);
    ui->group_treeWidget->setColumnCount(5);
    ui->group_treeWidget->setHeaderLabels({"name","type","address","size","color"});


    ui->tableTab->hide();

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
        ui->dwarf_treeView->setModel(new_model);
        model = new_model;
    }
}

void MainWindow::on_reloadBtn_clicked() {
    if (model!=nullptr) {
        if (ui->fileEdit->text().isEmpty()) {return;}
        TreeModel* new_model = new TreeModel(get_addr_task(ui->fileEdit->text().toStdString()),this);
        ui->dwarf_treeView->setModel(new_model);
        if (model!=nullptr) {
            delete model;
            model = nullptr;
        }
        model = new_model;
    }
}

void MainWindow::on_actioncreate_group_triggered() {
    QString name;
    while (1) {
        bool ok = false;
        name = QInputDialog::getText(this,"group name","name", QLineEdit::Normal,
            "group" + QString::number(ui->group_treeWidget->topLevelItemCount()+1), &ok);
        if (ok) {
            int same=0;
            for (int i = 0; i < ui->group_treeWidget->topLevelItemCount(); ++i) {
                if (name==ui->group_treeWidget->topLevelItem(i)->text(0)) {++same;}
            }
            if (same>0) {
                int reply = QMessageBox::critical(this,tr("MESSAGE"),tr("you can not take two names"),QMessageBox::Retry,QMessageBox::Abort);
                if (reply == QMessageBox::Abort) {
                    return;
                } else if (reply == QMessageBox::Retry) {
                    continue;
                }
            }
            break;
        }
        return;
    }
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->group_treeWidget);
    item->setText(0, name);

}

void MainWindow::on_actiondelete_group_triggered() {
    QModelIndex index = ui->group_treeWidget->currentIndex();
    ui->group_treeWidget->takeTopLevelItem(index.row());
}

void MainWindow::on_dwarf_treeView_doubleClicked(const QModelIndex &index) {
    auto item = static_cast<TreeItem *>(index.internalPointer());
    if(item->childCount()!=0) {return;}
    GroupItemAddDialog* dlg = new GroupItemAddDialog(ui->group_treeWidget, item,this);
    if(dlg->exec()==QDialog::Accepted) {

    };

}

void MainWindow::customGroupMenuRequested(const QPoint &pos) {
    QTreeWidgetItem* item = ui->group_treeWidget->itemAt(pos);
    QModelIndex index = ui->group_treeWidget->indexAt(pos);
    QMenu menu(this);

    if (item==nullptr) {
        menu.addAction(ui->actioncreate_group);
    }
    if (index.parent()==QModelIndex()&&item!=nullptr) {
        menu.addAction(ui->actiondelete_group);
    }
    menu.exec(QCursor::pos());

}
