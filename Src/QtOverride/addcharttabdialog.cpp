//
// Created by liaohy on 8/31/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_AddChartTabDialog.h" resolved

#include "addcharttabdialog.h"
#include "addcharttabdialog.h"
#include "ui_addcharttabdialog.h"

#include <QTreeWidget>
#include <QMessageBox>

AddChartTabDialog::AddChartTabDialog(QTreeWidget* group, QTabWidget *tabWidget, QWidget *parent) : QDialog(parent), tree(group), ui(new Ui::AddChartTabDialog) {
    ui->setupUi(this);

    ui->nameEdit->setText("Tab" + QString::number(tabWidget->currentIndex() + 2));

    QStringList groups;
    for (int row = 0; row < tree->topLevelItemCount(); ++row) {
        groups<<tree->topLevelItem(row)->text(0);
    }
    ui->groupBox->addItems(groups);
    ui->groupBox->setCurrentIndex(0);

    QStringList modes;
    modes<<"Simple"<<"Full"<<"Advanced";
    ui->modeBox->addItems(modes);
    ui->modeBox->setCurrentIndex(0);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [tabWidget, this]() {
        int same = 0;
        for (int i = 0; i < tabWidget->count(); ++i) {
            if (ui->nameEdit->text() == tabWidget->tabText(i)) { ++same; }
        }
        if (same > 0) {
            int reply = QMessageBox::critical(this, tr("MESSAGE"), tr("you can not take two names"), QMessageBox::Retry,
                                              QMessageBox::Abort);
            if (reply == QMessageBox::Abort) {
                this->reject();
            } else if (reply == QMessageBox::Retry) {
                return;
            }
        }
        this->accept();
    });
}

AddChartTabDialog::~AddChartTabDialog() {
    delete ui;
}

void AddChartTabDialog::on_groupBox_currentIndexChanged(int index) {
    group = tree->topLevelItem(index);
}

QString AddChartTabDialog::tabName() {
    return ui->nameEdit->text();
}

QTreeWidgetItem * AddChartTabDialog::chartGroup() {
    return group;
}

int AddChartTabDialog::chartMode() {
    if (ui->modeBox->currentText() == "Simple") return mode::Simple;
    if (ui->modeBox->currentText() == "Full") return mode::Full;
    if (ui->modeBox->currentText() == "Advanced") return mode::Advanced;
    return mode::Simple;
}
