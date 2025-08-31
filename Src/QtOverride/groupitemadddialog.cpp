//
// Created by liaohy on 8/30/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_GroupItemAddDialog.h" resolved

#include "../../groupitemadddialog.h"
#include "ui_groupitemadddialog.h"
#include <QColorDialog>
#include <QTreeWidget>
#include <QMessageBox>

#include "TreeItem.h"

GroupItemAddDialog::GroupItemAddDialog(QTreeWidget* tree, TreeItem* item, QWidget *parent) : QDialog(parent), treeWidget(tree), item(item), ui(new Ui::GroupItemAddDialog) {
    ui->setupUi(this);

    QStringList groups;
    for (int row = 0; row < tree->topLevelItemCount(); ++row) {
        groups<<tree->topLevelItem(row)->text(0);
    }
    ui->groupBox->addItems(groups);
    ui->groupBox->setCurrentIndex(0);

    ui->nameEdit->setText(item->data(0).toString());
    ui->addrEdit->setText(item->data(2).toString());

    ui->typeBox->addItems({"uint8_t","uint16_t","uint32_t","uint64_t","int8_t","int16_t","int32_t","int64_t", "float","double", item->data(1).toString()});
    ui->typeBox->setCurrentIndex(10);

    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,[this]() {
        if (ui->colorLabel->text()=="wait for selection") {
            int reply = QMessageBox::critical(this, tr("MESSAGE"), tr("you should first select a color before accepting"), QMessageBox::Retry,
                                              QMessageBox::Abort);
            if (reply == QMessageBox::Retry) {return;}
            if (reply == QMessageBox::Abort) {this->reject();return;}
        }
        this->accept();
    });
}

GroupItemAddDialog::~GroupItemAddDialog() {
    delete ui;
}

void GroupItemAddDialog::on_colorBtn_clicked() {
    QColorDialog::ColorDialogOptions options;
    QColor color = QColorDialog::getColor(QColor(Qt::blue),this,"select color", options);
    ui->colorLabel->setText(color.name());
    ui->colorLabel->setPalette(QPalette(color));
    ui->colorLabel->setAutoFillBackground(true);
}

void GroupItemAddDialog::on_typeBox_currentIndexChanged(int index) {
    int size;
    switch (index) {
        case 0:
        case 4:
            size = 1;
            break;
        case 1:
        case 5:
            size = 2;
            break;
        case 2:
        case 6:
            size = 4;
            break;
        case 3:
        case 7:
            size = 8;
            break;
        case 8:
            size = 4;
            break;
        case 9:
            size = 8;
            break;
        default:
            size = 4;
            break;
    }
    ui->sizeEdit->setText(QString::number(size));
}

void GroupItemAddDialog::on_groupBox_currentIndexChanged(int index) {
    group = treeWidget->topLevelItem(index);
}

QString GroupItemAddDialog::itemName() {
    return ui->nameEdit->text();
}

QString GroupItemAddDialog::itemAddr() {
    return ui->addrEdit->text();
}

QString GroupItemAddDialog::itemType() {
    return ui->typeBox->currentText();
}

QString GroupItemAddDialog::itemSize() {
    return ui->sizeEdit->text();
}

QColor GroupItemAddDialog::itemColor() {
    return ui->colorLabel->text();
}

QTreeWidgetItem* GroupItemAddDialog::itemGroup() {
    return group;
}
