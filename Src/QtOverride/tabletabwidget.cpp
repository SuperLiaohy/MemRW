//
// Created by liaohy on 9/5/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TableTabWidget.h" resolved

#include <QDoubleSpinBox>
#include "tabletabwidget.h"
#include "ui_tabletabwidget.h"


TableTabWidget::TableTabWidget(const std::shared_ptr<GroupTreeWidget::Group>& group, QWidget *parent) :
        QWidget(parent), ui(new Ui::TableTabWidget), group(group) {
    ui->setupUi(this);
    QStringList headers;
    headers << "variable name" << "read value" << "" << "write value" << "";
    ui->tableWidget->setColumnCount(headers.size());
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    load_variables();
}

TableTabWidget::~TableTabWidget() {
    delete ui;
}

void TableTabWidget::load_variables() {
    int row = 0;
    ui->tableWidget->setRowCount(group->variables.size());
    for (auto &variable:group->variables) {
        QPushButton *readBtn = new QPushButton("read", this);
        QPushButton *writeBtn = new QPushButton("write", this);

        ui->tableWidget->setItem(row,0, new QTableWidgetItem(variable.first));
        ui->tableWidget->setItem(row,1, new QTableWidgetItem(QString::number(1)));
        ui->tableWidget->setCellWidget(row,2,readBtn);
//        ui->tableWidget->setItem(row,3, new QTableWidgetItem(QString::number(2)));

        QDoubleSpinBox *dSpinBox = new QDoubleSpinBox(this);
        dSpinBox->setDecimals(6);
        ui->tableWidget->setCellWidget(row,3,dSpinBox);

        ui->tableWidget->setCellWidget(row,4,writeBtn);

        ++row;
    }

}

void TableTabWidget::button_clicked_handle() {
    QPushButton *btn = (QPushButton *)sender();
    int x = btn->frameGeometry().x();
    int y = btn->frameGeometry().y();
    QModelIndex index = ui->tableWidget->indexAt(QPoint(x,y));
    int row = index.row();
    int col = index.column();

}
