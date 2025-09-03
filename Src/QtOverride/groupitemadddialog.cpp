//
// Created by liaohy on 8/30/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_GroupItemAddDialog.h" resolved

#include "groupitemadddialog.h"
#include "ui_groupitemadddialog.h"
#include <QColorDialog>
#include <QTreeWidget>
#include <QMessageBox>
#include <qrandom.h>

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

    auto generator = QRandomGenerator::global();
    QColor color = QColor(generator->bounded(256),generator->bounded(256),generator->bounded(256));
    ui->colorLabel->setText(color.name());
    ui->colorLabel->setPalette(QPalette(color));
    ui->colorLabel->setAutoFillBackground(true);

    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,[this]() {
        int same = 0;
        for (int i = 0; i < itemGroup()->childCount(); ++i) {
            if (itemName() == itemGroup()->child(i)->text(0)) { ++same; }
        }
        if (same > 0) {
            int reply = QMessageBox::critical(this, tr("MESSAGE"), tr("you can not take two names"), QMessageBox::Retry,
                                              QMessageBox::Abort);
            if (reply == QMessageBox::Abort) {
                this->reject();
                return;
            } else if (reply == QMessageBox::Retry) {
                ui->nameEdit->setFocus();
                ui->nameEdit->selectAll();
                return;
            }
        }
        this->accept();
    });
}

GroupItemAddDialog::~GroupItemAddDialog() {
    delete ui;
}

int GroupItemAddDialog::SizeType(Type type) {
    switch (type) {
        case Type::INT8:
        case Type::UINT8:
            return 1;
        case Type::INT16:
        case Type::UINT16:
            return 2;
        case Type::INT32:
        case Type::UINT32:
        case Type::FLOAT:
            return 4;
        case Type::INT64:
        case Type::UINT64:
        case Type::DOUBLE:
            return 8;
        default:
            return 0;
    }
}

void GroupItemAddDialog::on_colorBtn_clicked() {
    QColorDialog::ColorDialogOptions options;
    QColor color = QColorDialog::getColor(QColor(Qt::blue),this,"select color", options);
    if (color.isValid()) {
        ui->colorLabel->setText(color.name());
        ui->colorLabel->setPalette(QPalette(color));
        ui->colorLabel->setAutoFillBackground(true);
    }
}

void GroupItemAddDialog::on_typeBox_currentIndexChanged(int index) {
    ui->sizeEdit->setText(QString::number(SizeType(itemTypeEnum())));
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

GroupItemAddDialog::Type GroupItemAddDialog::itemTypeEnum() {
    const auto& type_name = ui->typeBox->currentText();
    if (type_name.contains("uint8_t"))
        return GroupItemAddDialog::Type::UINT8;
    if (type_name.contains("uint16_t"))
        return GroupItemAddDialog::Type::UINT16;
    if (type_name.contains("uint32_t"))
        return GroupItemAddDialog::Type::UINT32;
    if (type_name.contains("uint64_t"))
        return GroupItemAddDialog::Type::UINT64;
    if (type_name.contains("int8_t"))
        return GroupItemAddDialog::Type::INT8;
    if (type_name.contains("int16_t"))
        return GroupItemAddDialog::Type::INT16;
    if (type_name.contains("int32_t"))
        return GroupItemAddDialog::Type::INT32;
    if (type_name.contains("int64_t"))
        return GroupItemAddDialog::Type::INT64;
    if (type_name.contains("float"))
        return GroupItemAddDialog::Type::FLOAT;
    if (type_name.contains("double"))
        return GroupItemAddDialog::Type::DOUBLE;
    if (type_name.contains("unsigned")) {
        if (type_name.contains("char"))
            return GroupItemAddDialog::Type::UINT8;
        if (type_name.contains("short"))
            return GroupItemAddDialog::Type::UINT16;
        if (type_name.contains("int"))
            return GroupItemAddDialog::Type::UINT32;
        if (type_name.contains("long")&&type_name.indexOf("long")!=type_name.lastIndexOf("long"))
            return GroupItemAddDialog::Type::UINT64;
    } else {
        if (type_name.contains("char"))
            return GroupItemAddDialog::Type::INT8;
        if (type_name.contains("short"))
            return GroupItemAddDialog::Type::INT16;
        if (type_name.contains("int"))
            return GroupItemAddDialog::Type::INT32;
        if (type_name.contains("long")&&type_name.indexOf("long")!=type_name.lastIndexOf("long"))
            return GroupItemAddDialog::Type::INT64;
    }
    if (type_name.contains("f")) {
        if (type_name.contains("32"))
            return GroupItemAddDialog::Type::FLOAT;
        if (type_name.contains("64"))
            return GroupItemAddDialog::Type::DOUBLE;
    }
    return Type::INT32;
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
