//
// Created by liaohy on 9/5/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_AddTableTabDialog.h" resolved

#include "addtabletabdialog.h"
#include "ui_addtabletabdialog.h"


AddTableTabDialog::AddTableTabDialog(std::unordered_map<QString, std::shared_ptr<GroupTreeWidget::Group>>& Groups, QTabWidget *tabWidget, QWidget *parent) :
        QDialog(parent), ui(new Ui::AddTableTabDialog), Groups(Groups) {
    ui->setupUi(this);
    ui->nameEdit->setText("Tab" + QString::number(tabWidget->currentIndex() + 2));

    QStringList groups;

    for (auto &group: Groups) {
            groups.append(group.first);
    }
    ui->groupBox->addItems(groups);
}

AddTableTabDialog::~AddTableTabDialog() {
    delete ui;
}

QString AddTableTabDialog::tabName() {
    return ui->nameEdit->text();
}

std::shared_ptr<GroupTreeWidget::Group> &AddTableTabDialog::chartGroup() {
    return Groups[ui->groupBox->currentText()];
}
