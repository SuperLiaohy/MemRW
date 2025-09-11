#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QColorDialog>
#include <QtCharts>
#include <QDebug>

#include "TreeModel.h"
#include "groupitemadddialog.h"
#include "addcharttabdialog.h"
#include "charttabwidget.h"
#include "addtabletabdialog.h"
#include "tabletabwidget.h"
std::shared_ptr<VariTree> get_addr_task(const std::string& file, DWARF_MODE mode);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/images/monitor.png"));
    this->setWindowTitle("MemRW");

    ui->dwarfDock->setWindowTitle("variables");
    ui->fileEdit->setReadOnly(true);

    ui->groupDock->setWindowTitle("groups");
    ui->group_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->group_treeWidget, &QTreeWidget::customContextMenuRequested, this, &MainWindow::customGroupMenuRequested);
    ui->group_treeWidget->setColumnCount(5);
    ui->group_treeWidget->setHeaderLabels({"name","type","address","size","color"});

    auto setupTab = [this](QTabWidget *tabWidget) {
        if (tabWidget->count()==0) tabWidget->hide();
        tabWidget->setTabsClosable(true);
        connect(tabWidget, &QTabWidget::currentChanged, tabWidget, [tabWidget]() {
            if (tabWidget->count() == 0) tabWidget->hide();
            else tabWidget->show();
        });
    };
    setupTab(ui->tableTab);
    connect(ui->tableTab, &QTabWidget::tabCloseRequested, this, [this](int index) {
        auto widget = (ui->tableTab->widget(index));
        auto name = ui->tableTab->tabText(index);
        ui->tableTab->removeTab(index);
        delete_table(name);
        if (widget!=nullptr) {widget->close();}
    });

    setupTab(ui->chartTab);
    connect(ui->chartTab, &QTabWidget::tabCloseRequested, this, [this](int index) {
        auto widget = (ui->chartTab->widget(index));
        auto name = ui->chartTab->tabText(index);
        ui->chartTab->removeTab(index);
        delete_chart(name);
        if (widget!=nullptr) {widget->close();}
    });

    this->dumpObjectTree();
}

MainWindow::~MainWindow()
{
    is_closing = true;
    if (link_thread!=nullptr&&this->link_thread->joinable())
        this->link_thread->join();
    delete ui;
}

void MainWindow::on_openBtn_clicked() {
    QFileDialog::Options options;
    QString fileName = QFileDialog::getOpenFileName(this, QString("Open File"), QDir::homePath(), tr("dwarf Files *.axf *.elf (*.axf *.elf);;All Files * (*)"));
    if (!fileName.isEmpty()) {
        QCursor cursor;
        cursor.setShape(Qt::WaitCursor);
        this->setCursor(cursor);
        ui->fileEdit->setText(fileName);
        if (ui->modeBox->isChecked())
            mode = DWARF_MODE::COMPLEX;
        else
            mode = DWARF_MODE::SIMPLE;
        TreeModel* new_model = new TreeModel(get_addr_task(ui->fileEdit->text().toStdString(),mode),this);
        if (model!=nullptr) {
            delete model;
            model = nullptr;
        }
        ui->dwarf_treeView->setModel(new_model);
        model = new_model;
        cursor.setShape(Qt::ArrowCursor);
        this->setCursor(cursor);
    }
}

void MainWindow::on_reloadBtn_clicked() {
    if (model!=nullptr) {
        QCursor cursor;
        cursor.setShape(Qt::WaitCursor);
        this->setCursor(cursor);
        if (ui->fileEdit->text().isEmpty()) {return;}
        if (ui->modeBox->isChecked())
            mode = DWARF_MODE::COMPLEX;
        else
            mode = DWARF_MODE::SIMPLE;
        TreeModel* new_model = new TreeModel(get_addr_task(ui->fileEdit->text().toStdString(),mode),this);
        ui->dwarf_treeView->setModel(new_model);
        if (model!=nullptr) {
            delete model;
            model = nullptr;
        }
        model = new_model;
        cursor.setShape(Qt::ArrowCursor);
        this->setCursor(cursor);
    }
}

void MainWindow::on_actioncreate_group_triggered() {
    create_group();
}

void MainWindow::on_actiondelete_group_triggered() {
    auto group = ui->group_treeWidget->currentItem();
    if (groups[group->text(0)]->bound) {
        QMessageBox::critical(this,"MESSAGE","You can not delete the group, because the group has been bound.",QMessageBox::Close);
        return;
    }
    delete_group(group);
}

void MainWindow::on_actiondelete_item_triggered() {
    auto item = ui->group_treeWidget->currentItem()->parent();
    if (groups[item->text(0)]->used) {
        QMessageBox::critical(this,"MESSAGE","You can not delete the group, because the group items are being used.",QMessageBox::Close);
        return;
    }
    remove_item(ui->group_treeWidget->currentItem());
}

void MainWindow::on_actiondisplay_group_dock_triggered() {
    ui->groupDock->show();
}

void MainWindow::on_actiondisplay_variable_dock_triggered() {
    ui->dwarfDock->show();
}

void MainWindow::on_actionadd_chart_tab_triggered() {
    bool BoundEmpty = true;
    for (auto &group: groups) {
        if (group.second->bound==0) {
            BoundEmpty = false;
            break;
        }
    }
    if (BoundEmpty) {
        QMessageBox::critical(this, "MESSAGE", "You have to have at least one group no bound.", QMessageBox::Close);
        return;
    }
    create_chart();
}

void MainWindow::on_actionadd_table_tab_triggered() {
    if (groups.empty()) {
        QMessageBox::critical(this, "MESSAGE", "You have to have at least one group.", QMessageBox::Close);
        return;
    }
    auto *dlg = new AddTableTabDialog(groups,ui->tableTab, this);
    auto res = dlg->exec();
    if (res == QDialog::Accepted) {
        auto &group = dlg->chartGroup();
        const auto &name = dlg->tabName();
        create_table(group, name);
    }
}

void MainWindow::on_actionconnect_triggered() {
    try {
        if (link!=nullptr) {
            is_disconnect = true;
            if (link_thread!=nullptr) {
                if (link_thread->joinable())
                    link_thread->join();
                link_thread.reset();
            }
            link.reset();
            for (auto &tab:chartTabs) {
                tab.second->startBtnEnable(false);
            }
            for (auto &tab:tableTabs) {
                tab.second->btnEnable(false);
            }
            ui->actionconnect->setText("connect");
            ui->actionconnect->setIcon(QIcon(":/images/connect.png"));
            ui->statusbar->showMessage("DAPlink has been disconnected");
            return;
        }
        link = std::make_unique<DAPReader>();
        link->attach_to_target();
        link->auto_configure_ap();
        ui->statusbar->showMessage("DAPlink connection is successful");
        ui->actionconnect->setText("disconnect");
        ui->actionconnect->setIcon(QIcon(":/images/disconnect.png"));
        is_disconnect = false;

        if (link_thread!=nullptr) {
            if (link_thread->joinable())
                link_thread->join();
            link_thread.reset();
        }
        for (auto &tab:chartTabs) {
            tab.second->startBtnEnable(true);
        }
        for (auto &tab:tableTabs) {
            tab.second->btnEnable(true);
        }
        link_thread = std::make_unique<std::thread>([this]() {
            // auto group_name = chartTabs[tabName].group;
            std::vector<DAP::TransferRequest> send;
            std::vector<DAP::TransferResponse> receive(255);
            std::map<uint32_t ,all_form > addrMap;
            std::vector<bool> read_flag;
            send.reserve(255);
            read_flag.reserve(255);
            while (true) {
                if (is_closing) { return; }
                if (is_disconnect) { return; }
                send.clear();
                receive.clear();
                addrMap.clear();
                read_flag.clear();

                auto chartTabCount = ui->chartTab->count();
                std::vector<QString> charts_state;
                for (int chart_count = 0; chart_count < chartTabCount; ++chart_count) {
                    auto name = ui->chartTab->tabText(chart_count);
                    if (!chartTabs.count(name)) {continue;}
                    auto chartTabWidget = chartTabs[name];
                    if (!chartTabWidget->isRun()) continue;
                    charts_state.push_back(name);
                    for (int count = 0; count < chartTabWidget->seriesList().size(); ++count) {
                        const auto &variable_name = chartTabWidget->seriesList()[count]->name();
                        const auto &variable = chartTabWidget->GroupBound()->variables.at(variable_name);
                        uint32_t addr_base = variable.address&0xfffffffC;
                        addrMap.emplace(addr_base,all_form{});
                        if (((variable.address&0x00000003)+variable.size())>4) {
                            addrMap.emplace(addr_base + 4, all_form{});
                            if (((variable.address&0x00000003)+variable.size())>8)
                                addrMap.emplace(addr_base + 8, all_form{});
                        }
                    }
                }

                for (auto &addr_item:addrMap) {
                    if (DAPReader::sw.ap.tar.has_value()) {
                        if (DAPReader::sw.ap.tar->data != addr_item.first) {
                            send.push_back(DAPReader::APWriteRequest(SW::MEM_AP::TAR, addr_item.first));
                            read_flag.push_back(false);
                        }
                        DAPReader::sw.ap.tar->data = addr_item.first+4;
                        if ((DAPReader::sw.ap.tar->data)&(0x400-1)) {
                            DAPReader::sw.ap.tar.reset();
                        }
                    } else {
                        send.push_back(DAPReader::APWriteRequest(SW::MEM_AP::TAR, addr_item.first));
                        read_flag.push_back(false);
                        DAPReader::sw.ap.tar = SW::MEM_AP::TARReg(addr_item.first+4);
                        if ((DAPReader::sw.ap.tar->data)&(0x400-1)) {
                            DAPReader::sw.ap.tar.reset();
                        }
                    }
                    send.push_back(DAPReader::APReadRequest(SW::MEM_AP::DRW));
                    read_flag.push_back(true);
                }

                auto tableTabCount = ui->tableTab->count();
                std::vector<std::unordered_map<QString,int>> tabVars;
                tabVars.resize(tableTabCount);
                for (int table_count = 0; table_count < tableTabCount; ++table_count) {
                    auto name = ui->tableTab->tabText(table_count);
                    if (!tableTabs.count(name)) {continue;}
                    auto tableTabWidget = tableTabs[name];
                    for (auto &buf:tableTabWidget->buffer) {
                        auto &request_buf = buf.second.requests;
                        if (request_buf->is_empty()) { continue;}
                        auto size = request_buf->size();
                        send.resize(send.size() + size);
                        request_buf->get_data(&send[send.size()-size],size);
                        if (size>1 && send[send.size()-1].request.RnW == 1) {
                            tabVars[table_count].emplace(buf.first, send.size()-1);
                        }
                    }
                }
                DAPReader::sw.ap.tar.reset();

                if (send.empty()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
                receive.resize(send.size());
                link->transfer(send, receive);

                int flag_index = 0;
                for (auto &addr:addrMap) {
                    while (!read_flag[flag_index])
                        ++flag_index;
                    addr.second = receive[flag_index].bit_data;
                    ++flag_index;
                }


                auto time = std::chrono::high_resolution_clock::now();
                for (int chart_count = chartTabCount - 1; chart_count >= 0; --chart_count) {
                    auto name = ui->chartTab->tabText(chart_count);
                    if (!chartTabs.count(name)) {continue;}
                    if (charts_state.empty()) {continue;}
                    if (charts_state[charts_state.size()-1]!=name) {continue;}
                    charts_state.pop_back();
                    auto chartTabWidget = chartTabs[name];
                    QStringList csv_data;
                    auto time_spent = chartTabWidget->TimeStamp(time);
                    csv_data.append(QString::number(time_spent));
                    for (int count = 0; count < chartTabWidget->seriesList().size(); ++count) {
                        auto &variable = chartTabWidget->GroupBound()->variables[chartTabWidget->seriesList()[count]->name()];
                        auto &ringbuffer = variable.ring_buffers;
                        uint32_t addr_base = variable.address&0xfffffffc;
                        uint8_t addr_offset = variable.address&0x3;
                        switch (variable.type) {
                            case GroupItemAddDialog::Type::INT8: {
                                int8_t data = addrMap[addr_base].i8[addr_offset];
                                auto point = QPointF(time_spent, data);
                                ringbuffer.write_data_force(&point, 1);
                                if (chartTabWidget->isLog()) { csv_data.append(QString::number(data)); }
                            }
                            break;
                            case GroupItemAddDialog::Type::UINT8: {
                                uint8_t data = addrMap[addr_base].u8[addr_offset];;
                                auto point = QPointF(time_spent, data);
                                ringbuffer.write_data_force(&point, 1);
                                if (chartTabWidget->isLog()) { csv_data.append(QString::number(data)); }
                            }
                            break;
                            case GroupItemAddDialog::Type::INT16: {
                                int16_t data = 0;
                                if (addr_offset > 2)
                                    data = std::bit_cast<int16_t>(static_cast<uint16_t>((addrMap[addr_base].u8[addr_offset] | addrMap[addr_base + 4].u8[0] << 8)));
                                else
                                    data = std::bit_cast<int16_t>(static_cast<uint16_t>((addrMap[addr_base].u8[addr_offset] | addrMap[addr_base].u8[addr_offset+1] << 8)));
                                auto point = QPointF(time_spent, data);
                                ringbuffer.write_data_force(&point, 1);
                                if (chartTabWidget->isLog()) { csv_data.append(QString::number(data)); }
                            }
                            break;
                            case GroupItemAddDialog::Type::UINT16: {
                                uint16_t data = 0;
                                if (addr_offset > 2)
                                    data = (static_cast<uint16_t>((addrMap[addr_base].u8[addr_offset] | addrMap[addr_base + 4].u8[0] << 8)));
                                else
                                    data = (static_cast<uint16_t>((addrMap[addr_base].u8[addr_offset] | addrMap[addr_base].u8[addr_offset+1] << 8)));
                                auto point = QPointF(time_spent, data);
                                ringbuffer.write_data_force(&point, 1);
                                if (chartTabWidget->isLog()) { csv_data.append(QString::number(data)); }
                            }
                            break;
                            case GroupItemAddDialog::Type::INT32: {
                                int32_t data = 0;
                                switch (addr_offset) {
                                    case 0:
                                        data = addrMap[addr_base].i32;
                                        break;
                                    case 1:
                                        data = static_cast<int32_t>(addrMap[addr_base].u8[1]
                                                | addrMap[addr_base].u8[2] << 8
                                                | addrMap[addr_base].u8[3] << 16
                                                | addrMap[addr_base+4].u8[0] << 24);
                                        break;
                                    case 2:
                                        data = static_cast<int32_t>(addrMap[addr_base].u16[1]
                                                | addrMap[addr_base+4].u16[0] << 16);
                                        break;
                                    case 3:
                                        data = static_cast<int32_t>(addrMap[addr_base].u8[3]
                                                | addrMap[addr_base+4].u8[0] << 8
                                                | addrMap[addr_base+4].u8[1] << 16
                                                | addrMap[addr_base+4].u8[2] << 24);
                                        break;
                                    default:
                                        break;
                                }
                                auto point = QPointF(time_spent, data);
                                ringbuffer.write_data_force(&point, 1);
                                if (chartTabWidget->isLog()) { csv_data.append(QString::number(data)); }
                            }
                            break;
                            case GroupItemAddDialog::Type::UINT32: {
                                uint32_t data = 0;
                                switch (addr_offset) {
                                    case 0:
                                        data = addrMap[addr_base].u32;
                                        break;
                                    case 1:
                                        data = (addrMap[addr_base].u8[1]
                                                | addrMap[addr_base].u8[2] << 8
                                                | addrMap[addr_base].u8[3] << 16
                                                | addrMap[addr_base + 4].u8[0] << 24);
                                        break;
                                    case 2:
                                        data = (addrMap[addr_base].u16[1]
                                                | addrMap[addr_base + 4].u16[0] << 16);
                                        break;
                                    case 3:
                                        data = (addrMap[addr_base].u8[3]
                                                | addrMap[addr_base + 4].u8[0] << 8
                                                | addrMap[addr_base + 4].u8[1] << 16
                                                | addrMap[addr_base + 4].u8[2] << 24);
                                        break;
                                    default:
                                        break;
                                }
                                auto point = QPointF(time_spent, data);
                                ringbuffer.write_data_force(&point, 1);
                                if (chartTabWidget->isLog()) { csv_data.append(QString::number(data)); }
                            }
                            break;
                            case GroupItemAddDialog::Type::FLOAT: {
                                float data = 0;
                                switch (addr_offset) {
                                    case 0:
                                        data = addrMap[addr_base].f32;
                                        break;
                                    case 1:
                                        data = std::bit_cast<float>(addrMap[addr_base].u8[1]
                                                | addrMap[addr_base].u8[2] << 8
                                                | addrMap[addr_base].u8[3] << 16
                                                | addrMap[addr_base + 4].u8[0] << 24);
                                        break;
                                    case 2:
                                        data = std::bit_cast<float>(addrMap[addr_base].u16[1]
                                                | addrMap[addr_base + 4].u16[0] << 16);
                                        break;
                                    case 3:
                                        data = std::bit_cast<float>(addrMap[addr_base].u8[3]
                                                | addrMap[addr_base + 4].u8[0] << 8
                                                | addrMap[addr_base + 4].u8[1] << 16
                                                | addrMap[addr_base + 4].u8[2] << 24);
                                        break;
                                    default:
                                        break;
                                }
                                auto point = QPointF(time_spent, data);
                                ringbuffer.write_data_force(&point, 1);
                                if (chartTabWidget->isLog()) { csv_data.append(QString::number(data)); }
                            }
                            break;
                            case GroupItemAddDialog::Type::DOUBLE:
                            case GroupItemAddDialog::Type::INT64:
                            case GroupItemAddDialog::Type::UINT64:
                                break;
                        }
                    }
                    if (chartTabWidget->isLog()) chartTabWidget->writeCsv({csv_data});
                    chartTabWidget->UpdateFreq();
                }

                for (int table_count = 0; table_count < tableTabCount; ++table_count) {
                    auto name = ui->tableTab->tabText(table_count);
                    if (!tableTabs.count(name)) {continue;}
                    auto tableTabWidget = tableTabs[name];
                    for (auto & var: tabVars[table_count]) {
                        tableTabWidget->buffer[var.first].responses->write_data(&receive[var.second],1);
                    }
                }
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

    } catch (const std::exception& e) {
        ui->statusbar->showMessage("DAPlink connection is failed");
        qDebug() << "catch error: " << e.what();
    }
}

void MainWindow::on_dwarf_treeView_doubleClicked(const QModelIndex &index) {
    auto item = static_cast<TreeItem *>(index.internalPointer());
    if(item->childCount()!=0) {return;}
    if (ui->group_treeWidget->topLevelItemCount() == 0) {
        QMessageBox::critical(this,"MESSAGE","You have to have at least one group.",QMessageBox::Close);
        return;
    }
    GroupItemAddDialog* dlg = new GroupItemAddDialog(ui->group_treeWidget, item,this);
    if(dlg->exec()==QDialog::Accepted) {add_item(dlg);};
}

void MainWindow::on_group_treeWidget_doubleClicked(const QModelIndex &index) {
    if (index.parent()==QModelIndex()) {return;}
    if(index.column()!=4) {return;}
    auto item = static_cast<QTreeWidgetItem*>(ui->group_treeWidget->currentIndex().internalPointer());
    QColor color = QColorDialog::getColor(item->text(4), this, "select color");
    if (color.isValid()) {
        item->setText(4, color.name());
        item->setData(4, Qt::BackgroundRole, color);
        groups[item->parent()->text(0)]->variables[item->text(0)].color = color;
    }
}

void MainWindow::customGroupMenuRequested(const QPoint &pos) {
    QTreeWidgetItem* item = ui->group_treeWidget->itemAt(pos);
    QModelIndex index = ui->group_treeWidget->indexAt(pos);
    QMenu menu(this);

    if (item==nullptr) {
        menu.addAction(ui->actioncreate_group);
    } else if (index.parent()==QModelIndex()) {
        menu.addAction(ui->actiondelete_group);
    } else if (index.parent()!=QModelIndex()) {
        menu.addAction(ui->actiondelete_item);
    }
    menu.exec(QCursor::pos());

}

void MainWindow::create_chart() {
    // create the chart dialog
    AddChartTabDialog* dlg = new AddChartTabDialog(groups, ui->chartTab, this);
    auto res = dlg->exec();
    // judge the return value
    if (res == QDialog::Accepted) {
        ui->chartTab->dumpObjectTree();
        // give the group and the tabName key
        auto& group = dlg->chartGroup();
        const auto& tabName = dlg->tabName();

        // create chartTab
        chartTabs.emplace(tabName, new ChartTabWidget(group, nullptr));
        auto& chartTabWidget = chartTabs[tabName];

        if (is_disconnect) {chartTabWidget->startBtnEnable(false);}

        // create the tab base widget
        chartTabWidget->setAttribute(Qt::WA_DeleteOnClose);

        int tabIndex = ui->chartTab->addTab(chartTabWidget, tabName);
        ui->chartTab->setCurrentIndex(tabIndex);

    }
}

void MainWindow::delete_chart(const QString& tabName) {
    chartTabs.erase(tabName);
}

void MainWindow::create_table(const std::shared_ptr<GroupTreeWidget::Group>& group, const QString &tabName) {

    tableTabs.emplace(tabName, new TableTabWidget(group, nullptr));
    auto& tableTabWidget = tableTabs[tabName];

    if (is_disconnect) {tableTabWidget->btnEnable(false);}

    tableTabWidget->setAttribute(Qt::WA_DeleteOnClose);

    int tabIndex = ui->tableTab->addTab(tableTabWidget,tabName);
    ui->tableTab->setCurrentIndex(tabIndex);
}

void MainWindow::delete_table(const QString &tabName) {
    tableTabs.erase(tabName);
}

void MainWindow::create_group() {
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
    groups.emplace(name, std::make_shared<GroupTreeWidget::Group>());
    item->setText(0, name);
}

void MainWindow::delete_group(QTreeWidgetItem* item) {
    auto index = ui->group_treeWidget->indexOfTopLevelItem(item);
    groups.erase(item->text(0));
    ui->group_treeWidget->takeTopLevelItem(index);
}

void MainWindow::add_item(GroupItemAddDialog* dlg) {
    auto group = dlg->itemGroup();
    auto newItem = new QTreeWidgetItem();
    newItem->setText(0, dlg->itemName());
    newItem->setText(1, dlg->itemType());
    newItem->setText(2, dlg->itemAddr());
    newItem->setText(3, dlg->itemSize());
    newItem->setText(4, dlg->itemColor().name());
    newItem->setData(4, Qt::BackgroundRole, dlg->itemColor());
    group->addChild(newItem);
    // groups[group->text(0)].ring_buffers.emplace_back();
    groups[group->text(0)]->variables.emplace(dlg->itemName(), GroupTreeWidget::variable{.address = static_cast<uint64_t>(dlg->itemAddr().toLongLong(nullptr,16)), .type = dlg->itemTypeEnum(), .color = dlg->itemColor()});
}

void MainWindow::remove_item(QTreeWidgetItem* item) {
    auto group = item->parent();
    // groups[group->text(0)].ring_buffers.pop_back();
    groups[group->text(0)]->variables.erase(item->text(0));
    group->removeChild(ui->group_treeWidget->currentItem());
}

