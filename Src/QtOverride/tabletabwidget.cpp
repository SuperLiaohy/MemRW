//
// Created by liaohy on 9/5/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TableTabWidget.h" resolved

#include <QDoubleSpinBox>
#include "tabletabwidget.h"
#include "ui_tabletabwidget.h"
#include "QLineEdit"
#include <thread>

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
        buffer.emplace(variable.first,buf{});
        QPushButton *readBtn = new QPushButton("read", this);
        QPushButton *writeBtn = new QPushButton("write", this);

        ui->tableWidget->setItem(row,0, new QTableWidgetItem(variable.first));
        ui->tableWidget->setItem(row,1, new QTableWidgetItem(QString::number(1)));

        auto item = ui->tableWidget->item(row,1);
        item->setFlags(item->flags()&~Qt::ItemIsEditable);

        ui->tableWidget->setCellWidget(row,2,readBtn);
        QLineEdit *lineEdit = new QLineEdit(this);
        if (variable.second.type == GroupItemAddDialog::Type::FLOAT || variable.second.type == GroupItemAddDialog::Type::DOUBLE) {
            QDoubleValidator *doubleValidator = new QDoubleValidator(this);
            doubleValidator->setNotation(QDoubleValidator::StandardNotation);
            lineEdit->setValidator(doubleValidator);
        } else {
            QIntValidator *validator = new QIntValidator(this);
            lineEdit->setValidator(validator);
        }
        ui->tableWidget->setCellWidget(row,3,lineEdit);
        ui->tableWidget->setCellWidget(row,4,writeBtn);

        connect(writeBtn, &QPushButton::clicked, this, &TableTabWidget::write_button_clicked_handle);
        connect(readBtn, &QPushButton::clicked, this, &TableTabWidget::read_button_clicked_handle);
        ++row;
    }
}

void TableTabWidget::write_button_clicked_handle() {
    QPushButton *btn = (QPushButton *)sender();
    int x = btn->frameGeometry().x();
    int y = btn->frameGeometry().y();
    QModelIndex index = ui->tableWidget->indexAt(QPoint(x,y));
    int row = index.row();
    int col = index.column();
    const QString& name = ui->tableWidget->item(row, 0)->text();
    auto edit = static_cast<QLineEdit*>(ui->tableWidget->cellWidget(row, 3));
    const GroupTreeWidget::variable &variable = group->variables[name];
    int64_t data = edit->text().toInt();
    union all_form {
        int8_t i8[4];
        uint8_t u8[4];
        int16_t i16[2];
        uint16_t u16[2];
        int32_t i32;
        uint32_t u32;
        float f32;
    } bit_data;
    bit_data.u32 = 0;
    switch (variable.type) {
        case GroupItemAddDialog::Type::INT8:
            if (data>INT8_MAX)
                bit_data.i8[0] = INT8_MAX;
            else if (data<INT8_MIN)
                bit_data.i8[0] = INT8_MIN;
            bit_data.i8[0] = data;
            break;
        case GroupItemAddDialog::Type::UINT8:
            if (data>UINT8_MAX)
                bit_data.u8[0] = UINT8_MAX;
            else if (data < 0)
                bit_data.u8[0] = 0;
            bit_data.u8[0] = data;
            break;
        case GroupItemAddDialog::Type::INT16:
            if (data>INT16_MAX)
                bit_data.i16[0] = INT16_MAX;
            else if (data<INT16_MIN)
                bit_data.i16[0] = INT16_MIN;
            bit_data.i16[0] = data;
            break;
        case GroupItemAddDialog::Type::UINT16:
            if (data>UINT16_MAX)
                bit_data.u16[0] = UINT16_MAX;
            else if (data < 0)
                bit_data.u16[0] = 0;
            bit_data.u16[0] = data;
            break;
        case GroupItemAddDialog::Type::INT32:
            if (data>INT32_MAX)
                bit_data.i32 = INT32_MAX;
            else if (data<INT32_MIN)
                bit_data.i32 = INT32_MIN;
            bit_data.i32 = data;
            break;
        case GroupItemAddDialog::Type::UINT32:
            if (data>UINT32_MAX)
                bit_data.u32 = UINT32_MAX;
            else if (data < 0)
                bit_data.u32 = 0;
            bit_data.u32 = data;
            break;
        case GroupItemAddDialog::Type::INT64:
        case GroupItemAddDialog::Type::UINT64:
        case GroupItemAddDialog::Type::DOUBLE:
            break;
        case GroupItemAddDialog::Type::FLOAT:
            bit_data.f32 = edit->text().toFloat();
            break;
    }
    generate_request(name, variable, bit_data.u32);

}

void TableTabWidget::read_button_clicked_handle() {
    QPushButton *btn = (QPushButton *)sender();
    int x = btn->frameGeometry().x();
    int y = btn->frameGeometry().y();
    QModelIndex index = ui->tableWidget->indexAt(QPoint(x,y));
    int row = index.row();
    int col = index.column();
    const QString& name = ui->tableWidget->item(row, 0)->text();
    const GroupTreeWidget::variable &variable = group->variables[name];
    generate_response(name,variable,row);
}

void TableTabWidget::btnEnable(bool able) {

}

void TableTabWidget::generate_request(const QString &name, const GroupTreeWidget::variable &variable, uint32_t data) {
    std::vector<DAP::TransferRequest> reg_request;
    reg_request.reserve(4);
    switch (variable.type) {
        case GroupItemAddDialog::Type::INT8:
        case GroupItemAddDialog::Type::UINT8:
            DAPReader::sw.ap.csw->Size = 0b000;
            reg_request.push_back(DAPReader::APWriteRequest(SW::MEM_AP::CSW, std::bit_cast<uint32_t>(DAPReader::sw.ap.csw.value())));
            reg_request.push_back(DAPReader::APWriteRequest(SW::MEM_AP::TAR, variable.address));
            reg_request.push_back(DAPReader::APWriteRequest(SW::MEM_AP::DRW, data));
            DAPReader::sw.ap.csw->Size = 0b010;
            reg_request.push_back(DAPReader::APWriteRequest(SW::MEM_AP::CSW, std::bit_cast<uint32_t>(DAPReader::sw.ap.csw.value())));
            break;
        case GroupItemAddDialog::Type::INT16:
        case GroupItemAddDialog::Type::UINT16:
            DAPReader::sw.ap.csw->Size = 0b001;
            reg_request.push_back(DAPReader::APWriteRequest(SW::MEM_AP::CSW, std::bit_cast<uint32_t>(DAPReader::sw.ap.csw.value())));
            reg_request.push_back(DAPReader::APWriteRequest(SW::MEM_AP::TAR, variable.address));
            reg_request.push_back(DAPReader::APWriteRequest(SW::MEM_AP::DRW, data));
            DAPReader::sw.ap.csw->Size = 0b010;
            reg_request.push_back(DAPReader::APWriteRequest(SW::MEM_AP::CSW, std::bit_cast<uint32_t>(DAPReader::sw.ap.csw.value())));
            break;
        case GroupItemAddDialog::Type::INT32:
        case GroupItemAddDialog::Type::UINT32:
        case GroupItemAddDialog::Type::FLOAT:
            reg_request.push_back(DAPReader::APWriteRequest(SW::MEM_AP::TAR, variable.address));
            reg_request.push_back(DAPReader::APWriteRequest(SW::MEM_AP::DRW, data));
            break;
        case GroupItemAddDialog::Type::INT64:
        case GroupItemAddDialog::Type::UINT64:
        case GroupItemAddDialog::Type::DOUBLE:
            break;
    }
    buffer[name].requests->write_data(reg_request.data(), reg_request.size());
}

void TableTabWidget::generate_response(const QString &name, const GroupTreeWidget::variable &variable, int row) {
    std::vector<DAP::TransferRequest> reg_request;
    reg_request.reserve(2);
    reg_request.push_back(DAPReader::APWriteRequest(SW::MEM_AP::TAR, variable.address));
    reg_request.push_back(DAPReader::APReadRequest(SW::MEM_AP::DRW));
    buffer[name].requests->write_data(reg_request.data(), reg_request.size());
    std::vector<DAP::TransferResponse> reg_response;
    reg_response.resize(2);
    while (!buffer[name].responses->get_data(reg_response.data(), 1)){
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
    auto edit = ui->tableWidget->item(row, 1);
    switch (variable.type) {
        case GroupItemAddDialog::Type::INT8:
            edit->setText(QString::number(reg_response[0].data8i[0]));
            break;
        case GroupItemAddDialog::Type::UINT8:
            edit->setText(QString::number(reg_response[0].data8u[0]));
            break;
        case GroupItemAddDialog::Type::INT16:
            edit->setText(QString::number(reg_response[0].data16i[0]));
            break;
        case GroupItemAddDialog::Type::UINT16:
            edit->setText(QString::number(reg_response[0].data16u[0]));
            break;
        case GroupItemAddDialog::Type::INT32:
            edit->setText(QString::number(reg_response[0].data32i));
            break;
        case GroupItemAddDialog::Type::UINT32:
            edit->setText(QString::number(reg_response[0].data));
            break;
        case GroupItemAddDialog::Type::FLOAT:
            edit->setText(QString::number(reg_response[0].data32f));
            break;
        case GroupItemAddDialog::Type::DOUBLE:
        case GroupItemAddDialog::Type::INT64:
        case GroupItemAddDialog::Type::UINT64:
            break;
    }
}

