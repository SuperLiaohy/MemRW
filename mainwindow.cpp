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

std::shared_ptr<VariTree> get_addr_task(const std::string& file);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
        connect(tabWidget, &QTabWidget::tabCloseRequested, tabWidget, [this, tabWidget](int index) {
            auto frame = static_cast<QFrame *>(tabWidget->widget(index));
            delete_chart(tabWidget->tabText(index));
            tabWidget->removeTab(index);
            if (frame!=nullptr) {frame->close();}
        });
        connect(tabWidget, &QTabWidget::currentChanged, tabWidget, [tabWidget]() {
            if (tabWidget->count() == 0) tabWidget->hide();
            else tabWidget->show();
        });
    };
    setupTab(ui->tableTab);
    setupTab(ui->chartTab);
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
        TreeModel* new_model = new TreeModel(get_addr_task(ui->fileEdit->text().toStdString()),this);
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
        TreeModel* new_model = new TreeModel(get_addr_task(ui->fileEdit->text().toStdString()),this);
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
    if (groups[group->text(0)].bound) {
        QMessageBox::critical(this,"MESSAGE","You can not delete the group, because the group has been bound.",QMessageBox::Close);
        return;
    }
    delete_group(group);
}

void MainWindow::on_actiondelete_item_triggered() {
    auto item = ui->group_treeWidget->currentItem()->parent();
    if (groups[item->text(0)].used) {
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
    if (ui->group_treeWidget->topLevelItemCount() == 0) {
        QMessageBox::critical(this, "MESSAGE", "You have to have at least one group.", QMessageBox::Close);
        return;
    }
    create_chart();
}

void MainWindow::on_actionadd_table_tab_triggered() {
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
        link_thread = std::make_unique<std::thread>([this]() {
            // auto group_name = chartTabs[tabName].group;
            std::vector<DAP::TransferRequest> send;
            std::vector<DAP::TransferResponse> receive(255);
            send.reserve(255);
            while (true) {
                if (is_closing == true) { return; }
                if (is_disconnect == true) { return; }
                send.clear();
                receive.clear();
                std::unordered_map<QString,int> requests_count;
                for (int chart_count = 0; chart_count < ui->chartTab->count(); ++chart_count) {
                    auto name = ui->chartTab->tabText(chart_count);
                    if (!chartTabs.count(name)) {continue;}
                    auto& chart = chartTabs[name];
                    auto index = send.size();
                    auto count = chart.request_rb.size();
                    if (count == 0) {continue;}
                    send.resize(index + count);
                    chart.request_rb.get_data(&send[index], count);
                    requests_count.emplace(name, count);
                }

                if (send.empty()) {continue;}
                link->transfer(send, receive);

                auto time = std::chrono::high_resolution_clock::now();
                int read_index = 0;
                for (int chart_count = 0; chart_count < ui->chartTab->count(); ++chart_count) {
                    auto name = ui->chartTab->tabText(chart_count);
                    if (!chartTabs.count(name)) {continue;}
                    auto& chart = chartTabs[name];
                    auto size = requests_count[name];
                    for (int i = 0; i < size; ++i) {
                        receive[read_index + i].TimeStamp = std::chrono::duration_cast<std::chrono::microseconds>((time - chart.start_time)).count();
                    }
                    chart.response_rb.write_data(&receive[read_index],size);
                    read_index += size;
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
    AddChartTabDialog* dlg = new AddChartTabDialog(ui->group_treeWidget, ui->chartTab, this);
    auto res = dlg->exec();
    // judge the return value
    if (res == QDialog::Accepted) {
        // give the group and the tabName key
        auto groupWidget = dlg->chartGroup();
        auto& group = groups[groupWidget->text(0)];
        const auto& tabName = dlg->tabName();

        QChart *chart = new QChart();
        chart->setTitle("test chart");
        // chart->setAnimationOptions(QChart::SeriesAnimations);

        QValueAxis *axisX = new QValueAxis(chart);
        axisX->setTitleText("time (ms)");
        axisX->setRange(0, 5000);
        QValueAxis *axisY = new QValueAxis(chart);
        axisY->setTitleText("value");
        axisY->setRange(0, 1000);

        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);

        // create the tab base widget --frame
        QFrame *frame = new QFrame(this);
        frame->setAttribute(Qt::WA_DeleteOnClose);

        int tabIndex = ui->chartTab->addTab(frame, tabName);
        ui->chartTab->setCurrentIndex(tabIndex);
        auto currentWidget = ui->chartTab->currentWidget();

        // add group bound
        ++group.bound;
        // create chartTab
        chartTabs.emplace(tabName,
            chartTab{.state = chartTab::State::Stop, .group = groupWidget->text(0), .start_time = std::chrono::high_resolution_clock::now()});
        auto& chartTab = chartTabs[tabName];

        // create the series to paint the chart and load ringbuffer data
        auto& series_list = chartTab.series_list;
        series_list.resize(groupWidget->childCount());

        // traversal the group treeWidget
        for (int count = 0; count < groupWidget->childCount(); ++count) {
            series_list[count] = new QLineSeries(chart);
            series_list[count]->setName(groupWidget->child(count)->data(0, Qt::DisplayRole).toString());
            series_list[count]->setProperty("color", QColor(groupWidget->child(count)->data(4, Qt::DisplayRole).toString()));
            chart->addSeries(series_list[count]);
            series_list[count]->attachAxis(axisX);
            series_list[count]->attachAxis(axisY);
        }

        // create QChatView
        QChartView *chartView = new QChartView(chart, currentWidget);
        chartView->setChart(chart);
        QChartView::RubberBands rubber;
        rubber = QChartView::RectangleRubberBand;
        chartView->setRubberBand(rubber);
        chartView->setRenderHint(QPainter::Antialiasing);
        QPushButton *startBtn = new QPushButton("Start", currentWidget);
        QPushButton *stopBtn = new QPushButton("Stop", currentWidget);
        QPushButton *logfileBtn = new QPushButton("log config", currentWidget);
        QCheckBox* logfileCheckBox = new QCheckBox("log data into a file", currentWidget);
        logfileCheckBox->setEnabled(true);

        QLabel* freqLabel = new QLabel("freq: 0",currentWidget);
        freqLabel->setAlignment(Qt::AlignCenter);

        QGridLayout *gridLayout = new QGridLayout(currentWidget);
        currentWidget->setLayout(gridLayout);
        gridLayout->addWidget(freqLabel,0,0,1,1);
        gridLayout->addWidget(logfileCheckBox, 0, 2, 1, 1);
        gridLayout->addWidget(logfileBtn, 1, 2, 1, 1);
        gridLayout->addWidget(startBtn, 1, 0, 1, 1);
        gridLayout->addWidget(stopBtn, 1, 1, 1, 1);
        gridLayout->addWidget(chartView, 2, 0, 1, 3);

        // create the timer to update QChartView
        chartTabs[tabName].timer = new QTimer(currentWidget);
        chartTabs[tabName].timer->stop();

        // setup startBtn to start collect data
        connect(startBtn, &QPushButton::clicked, currentWidget, [this,tabName,groupWidget,chart,logfileCheckBox]() {
            // judge whether is in connection
            if (is_disconnect == true) {
                QMessageBox::critical(this,"MESSAGE","you can not start when you are not in connection!",QMessageBox::Ok);
                return;
            }
            // judge whether there is a variable in the group
            auto& chartTab = chartTabs[tabName];
            if (groups[chartTab.group].variables.size() == 0) {
                QMessageBox::critical(this, "MESSAGE", "you can not start because the group you bound is empty!",QMessageBox::Ok);
                return;
            }
            // judge whether the logfile is valid
            if (logfileCheckBox->isChecked()) {
                if (chartTab.logfile!=nullptr) {
                    chartTab.logfile->close();
                    chartTab.logfile.reset();
                }
                if (chartTab.logfile_path.isEmpty()) {
                    QMessageBox::critical(this,"MESSAGE","can not open logfile!",QMessageBox::Ok);
                    return;
                }
                chartTab.logfile = std::make_shared<QFile>(chartTab.logfile_path);
                if (!chartTab.logfile->open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QMessageBox::critical(this,"MESSAGE","can not open logfile!",QMessageBox::Ok);
                    chartTab.logfile.reset();
                    return;
                }
            }
            // you can not change the log state when you have already started
            logfileCheckBox->setEnabled(false);

            // reload the group variables
            auto& series_list = chartTab.series_list;
            for (int count = 0; count < series_list.size(); ++count) {
                chart->removeSeries(series_list[count]);
            }
            series_list.clear();
            series_list.resize(groupWidget->childCount());

            // prepare for csv header
            QStringList csv_header;
            csv_header << "TimeStamp";

            // reload the group variables
            for (int count = 0; count < groupWidget->childCount(); ++count) {
                auto chile_name = groupWidget->child(count)->data(0, Qt::DisplayRole).toString();
                if (logfileCheckBox->isChecked()) csv_header << chile_name;
                series_list[count] = new QLineSeries(chart);
                series_list[count]->setName(chile_name);
                series_list[count]->setProperty(
                    "color", QColor(groupWidget->child(count)->data(4, Qt::DisplayRole).toString()));
                chart->addSeries(series_list[count]);
                series_list[count]->attachAxis(chart->axisX());
                series_list[count]->attachAxis(chart->axisY());
            }

            if (logfileCheckBox->isChecked()) writeCsv(chartTab.logfile, {csv_header});
            // clean up last remaining data of the ringbuffer
            for (auto & variable : groups[chartTab.group].variables) {
                variable.second.ring_buffers.reset();
            }
            // variables be used
            ++groups[chartTab.group].used;
            // reset the last remaining data of the chartTab record
            chartTab.last_time = 0;
            chartTab.freq = 0;
            chart->axisX()->setRange(0, 5000);
            chartTab.start_time = std::chrono::high_resolution_clock::now();
            chartTab.timer->start(30);
            chartTab.state = chartTab::State::Running;
        });

        connect(stopBtn, &QPushButton::clicked, currentWidget, [this,tabName,logfileCheckBox]() {
            auto& chartTab = chartTabs[tabName];
            if (chartTab.logfile != nullptr) {
                chartTab.logfile->close();
                chartTab.logfile.reset();
            }
            logfileCheckBox->setEnabled(true);
            chartTab.timer->stop();
            chartTab.state = chartTab::State::Stop;
            --groups[chartTab.group].used;
        });

        connect(logfileBtn, &QPushButton::clicked, currentWidget, [this,tabName]() {
            auto& chartTab = chartTabs[tabName];
            if (chartTab.state == chartTab::State::Running) {
                QMessageBox::critical(this,"MESSAGE","you can not change logfile when you are collecting the data!\nplease stop first!",QMessageBox::Ok);
                return;
            }
            bool ok=false;
            auto file = QInputDialog::getText(this,"logfile path","please input a logfile path(include file name): ", QLineEdit::Normal, chartTab.logfile_path, &ok);
            if (ok==false) {return;}
            chartTab.logfile_path = file;
        });

        if (chartTabs[tabName].thread!=nullptr) {
            if (chartTabs[tabName].thread->joinable())
                chartTabs[tabName].thread->join();
            chartTabs[tabName].thread.reset();
        }

        chartTabs[tabName].thread = std::make_shared<std::thread>([this,tabName,freqLabel, logfileCheckBox]() {
            auto& chartTab = chartTabs[tabName];
            while (true) {
                if (is_closing==true){return;}
                switch (chartTab.state) {
                    case chartTab::State::Stop:
                        continue;
                    case chartTab::State::Running:
                        break;
                    case chartTab::State::Closed:
                        return;
                }
                std::vector<DAP::TransferRequest> send;
                send.reserve(chartTab.series_list.size()*2);
                std::vector<DAP::TransferResponse> receive(chartTab.series_list.size()*2);
                for (int count = 0; count < chartTab.series_list.size(); ++count) {
                    const auto& variable_name = chartTab.series_list[count]->name();
                    auto& variable = groups[chartTab.group].variables[variable_name];
                    send.push_back(DAPReader::APWriteRequest(SW::MEM_AP::TAR, static_cast<uint32_t>(variable.address)));
                    send.push_back(DAPReader::APReadRequest(SW::MEM_AP::DRW));
                }
                chartTab.request_rb.write_data(send.data(),send.size());

                int timeout = 0;
                while (true) {
                    if (chartTab.response_rb.get_data(receive.data(),receive.size())) break;
                    // ++timeout;
                }

                auto time = std::chrono::duration_cast<std::chrono::microseconds>(
                                (std::chrono::high_resolution_clock::now() - chartTab.start_time)).count()
                            / 1000.f;
                QStringList csv_data;
                if (receive.size()>1)
                    csv_data.append(QString::number(receive[1].TimeStamp/1000.f));
                else
                    csv_data.append(QString::number(time));

                for (int count = 0; count < chartTab.series_list.size(); ++count) {
                    auto& variable = groups[chartTab.group].variables[chartTab.series_list[count]->name()];
                    auto& ringbuffer = variable.ring_buffers;
                    switch (variable.type) {
                        case GroupItemAddDialog::Type::INT8: {
                            int8_t data = receive[count * 2 + 1].data8i[0];
                            auto point = QPointF(receive[count * 2 + 1].TimeStamp / 1000.f, data);
                            ringbuffer.write_data_force(&point, 1);
                            if (logfileCheckBox->isChecked()) { csv_data.append(QString::number(data)); }
                        } break;
                        case GroupItemAddDialog::Type::UINT8: {
                            uint8_t data = receive[count * 2 + 1].data8u[0];
                            auto point = QPointF(receive[count * 2 + 1].TimeStamp / 1000.f, data);
                            ringbuffer.write_data_force(&point, 1);
                            if (logfileCheckBox->isChecked()) { csv_data.append(QString::number(data)); }
                        } break;
                        case GroupItemAddDialog::Type::INT16: {
                            int16_t data = receive[count * 2 + 1].data16i[0];
                            auto point = QPointF(receive[count * 2 + 1].TimeStamp / 1000.f, data);
                            ringbuffer.write_data_force(&point, 1);
                            if (logfileCheckBox->isChecked()) { csv_data.append(QString::number(data)); }
                        } break;
                        case GroupItemAddDialog::Type::UINT16: {
                            int16_t data = receive[count * 2 + 1].data16u[0];
                            auto point = QPointF(receive[count * 2 + 1].TimeStamp / 1000.f, data);
                            ringbuffer.write_data_force(&point, 1);
                            if (logfileCheckBox->isChecked()) { csv_data.append(QString::number(data)); }
                        } break;
                        case GroupItemAddDialog::Type::INT32: {
                            int32_t data = receive[count * 2 + 1].data32i;
                            auto point = QPointF(receive[count * 2 + 1].TimeStamp / 1000.f, data);
                            ringbuffer.write_data_force(&point, 1);
                            if (logfileCheckBox->isChecked()) { csv_data.append(QString::number(data)); }
                        } break;
                        case GroupItemAddDialog::Type::UINT32: {
                            uint32_t data = receive[count * 2 + 1].data;
                            auto point = QPointF(receive[count * 2 + 1].TimeStamp / 1000.f, data);
                            ringbuffer.write_data_force(&point, 1);
                            if (logfileCheckBox->isChecked()) { csv_data.append(QString::number(data)); }
                        } break;
                        case GroupItemAddDialog::Type::FLOAT: {
                            float data = receive[count * 2 + 1].data32f;
                            auto point = QPointF(receive[count * 2 + 1].TimeStamp / 1000.f, data);
                            ringbuffer.write_data_force(&point, 1);
                            if (logfileCheckBox->isChecked()) { csv_data.append(QString::number(data)); }
                        } break;
                            case GroupItemAddDialog::Type::DOUBLE:
                            case GroupItemAddDialog::Type::INT64:
                            case GroupItemAddDialog::Type::UINT64:
                            break;

                    }
                }
                if (logfileCheckBox->isChecked()) writeCsv(chartTab.logfile, {csv_data});

                ++chartTab.freq;
                if ((static_cast<int>(time)-chartTab.last_time)/1000 == 1) {
                    freqLabel->setText(QString("freq: %1").arg(chartTab.freq));
                    chartTab.last_time = time;
                    chartTab.freq = 0;
                }
            }
        });


        // 模拟实时数据
        connect(chartTabs[tabName].timer, &QTimer::timeout, currentWidget, [this,chart,tabName]() {
            auto& series_list = chartTabs[tabName].series_list;
            for (int i = 0; i < series_list.size(); ++i) {
                auto &ringbuffer = groups[chartTabs[tabName].group].variables[series_list[i]->name()].ring_buffers;
                if (ringbuffer.is_full())
                    series_list[i]->replace(ringbuffer.get_container());
                else
                    series_list[i]->replace(ringbuffer.get_valid_container());
            }
            auto time = std::chrono::duration_cast<std::chrono::microseconds>(
                            (std::chrono::high_resolution_clock::now() - chartTabs[tabName].start_time)).count() / 1000.f;
            // 自动滚动视图
            if (time > 5000) {
                chart->axisX()->setRange(time - 5000, time);
            }
        });
    }
}

void MainWindow::delete_chart(const QString& tabName) {
    auto& chartTab = chartTabs[tabName];
    chartTab.timer->stop();
    chartTab.state = chartTab::State::Closed;
    --groups[chartTab.group].bound;
    chartTabs.erase(tabName);
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
    groups.emplace(name,group());
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
    groups[group->text(0)].variables.emplace(dlg->itemName(), variable{.address = static_cast<uint64_t>(dlg->itemAddr().toLongLong(nullptr,16)), .type = dlg->itemTypeEnum(), .color = dlg->itemColor()});
}

void MainWindow::remove_item(QTreeWidgetItem* item) {
    auto group = item->parent();
    // groups[group->text(0)].ring_buffers.pop_back();
    groups[group->text(0)].variables.erase(item->text(0));
    group->removeChild(ui->group_treeWidget->currentItem());
}

void MainWindow::writeCsv(const std::shared_ptr<QFile>& file, const QList<QStringList> &data) {
    QTextStream out(file.get());
    for (const QStringList &row: data) {
        out << row.join(',') << "\n";
    }
}
