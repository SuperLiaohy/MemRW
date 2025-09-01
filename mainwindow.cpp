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
            tabWidget->removeTab(index);
            delete_chart(frame);
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
    if (this->raad_thread.joinable())
        this->raad_thread.join();
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
    if (groups[group->text(0)].used) {
        QMessageBox::critical(this,"MESSAGE","You can not delete the group, because the group is being used.",QMessageBox::Close);
        return;
    }
    delete_group(group);
}

void MainWindow::on_actiondelete_item_triggered() {
    auto item = ui->group_treeWidget->currentItem()->parent();
    if (groups[item->text(0)].used) {
        QMessageBox::critical(this,"MESSAGE","You can not delete the group, because the group is being used.",QMessageBox::Close);
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
    item->setText(4, color.name());
    item->setData(4, Qt::BackgroundRole, color);

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
    AddChartTabDialog* dlg = new AddChartTabDialog(ui->group_treeWidget, ui->chartTab, this);
    auto res = dlg->exec();
    if (res == QDialog::Accepted) {
        auto group = dlg->chartGroup();
        auto tabName = dlg->tabName();
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

        QFrame *frame = new QFrame(this);
        frame->setAttribute(Qt::WA_DeleteOnClose);
        frame->setProperty("chart", tabName);

        int tabIndex = ui->chartTab->addTab(frame, tabName);
        ui->chartTab->setCurrentIndex(tabIndex);
        ++groups[group->text(0)].used;
        chartTabs.emplace(tabName,
            chartTab{.state = chartTab::State::Stop, .group = group->text(0), .start_time = std::chrono::high_resolution_clock::now()});

        // QList<QLineSeries *> series_list(group->childCount());
        auto& series_list = chartTabs[tabName].series_list;
        series_list.resize(group->childCount());
        for (int count = 0; count < group->childCount(); ++count) {
            chartTabs[tabName].addr.emplace_back(group->child(count)->data(2, Qt::DisplayRole).toString().toLongLong(nullptr, 16));
            series_list[count] = new QLineSeries(chart);
            series_list[count]->setName(group->child(count)->data(0, Qt::DisplayRole).toString());
            series_list[count]->setProperty("color", QColor(group->child(count)->data(4, Qt::DisplayRole).toString()));
            chart->addSeries(series_list[count]);
            series_list[count]->attachAxis(axisX);
            series_list[count]->attachAxis(axisY);
        }

        auto currentWidget = ui->chartTab->currentWidget();
        chartTabs[tabName].timer = new QTimer(currentWidget);

        QChartView *chartView = new QChartView(chart, currentWidget);
        chartView->setChart(chart);
        QChartView::RubberBands rubber;
        rubber = QChartView::RectangleRubberBand;
        chartView->setRubberBand(rubber);
        chartView->setRenderHint(QPainter::Antialiasing);
        QPushButton *startBtn = new QPushButton("Start", currentWidget);
        QPushButton *stopBtn = new QPushButton("Stop", currentWidget);
        QPushButton *reloadBtn = new QPushButton("Reload Chart", currentWidget);

        QGridLayout *gridLayout = new QGridLayout(currentWidget);
        currentWidget->setLayout(gridLayout);
        gridLayout->addWidget(reloadBtn, 0, 2, 1, 1);
        gridLayout->addWidget(startBtn, 0, 0, 1, 1);
        gridLayout->addWidget(stopBtn, 0, 1, 1, 1);
        gridLayout->addWidget(chartView, 1, 0, 1, 3);

        chartTabs[tabName].timer->stop();
        connect(startBtn, &QPushButton::clicked, currentWidget, [this,tabName,group,chart]() {
            auto& series_list = chartTabs[tabName].series_list;
            for (int count = 0; count < series_list.size(); ++count) {
                chart->removeSeries(series_list[count]);
            }
            series_list.clear();
            series_list.resize(group->childCount());
            auto& addr_list = chartTabs[tabName].addr;
            addr_list.clear();

            for (int count = 0; count < group->childCount(); ++count) {
                addr_list.emplace_back(
                    group->child(count)->data(2, Qt::DisplayRole).toString().toLongLong(nullptr, 16));
                series_list[count] = new QLineSeries(chart);
                series_list[count]->setName(group->child(count)->data(0, Qt::DisplayRole).toString());
                series_list[count]->setProperty(
                    "color", QColor(group->child(count)->data(4, Qt::DisplayRole).toString()));
                chart->addSeries(series_list[count]);
                series_list[count]->attachAxis(chart->axisX());
                series_list[count]->attachAxis(chart->axisY());
            }
            for (auto & ring_buffer : groups[chartTabs[tabName].group].ring_buffers) {
                ring_buffer.reset();
            }
            chart->axisX()->setRange(0, 5000);
            chartTabs[tabName].start_time = std::chrono::high_resolution_clock::now();
            chartTabs[tabName].timer->start(30);
            chartTabs[tabName].state = chartTab::State::Running;
        });
        connect(stopBtn, &QPushButton::clicked, currentWidget, [this,tabName]() {
            chartTabs[tabName].timer->stop();
            chartTabs[tabName].state = chartTab::State::Stop;
        });

        raad_thread = std::thread([this,tabName]() {
            // auto group_name = chartTabs[tabName].group;
            while (true) {
                switch (chartTabs[tabName].state) {
                    case chartTab::State::Stop:
                        continue;
                    case chartTab::State::Running:
                        break;
                    case chartTab::State::Closed:
                        chartTabs.erase(tabName);
                        return;
                }
                for (int count = 0; count < chartTabs[tabName].addr.size(); ++count) {
                    auto &ringbuffer = groups[chartTabs[tabName].group].ring_buffers[count];
                    auto data = link->read_mem(chartTabs[tabName].addr[count]);
                    auto time = std::chrono::duration_cast<std::chrono::microseconds>(
                                    (std::chrono::high_resolution_clock::now() - chartTabs[tabName].start_time)).count()
                                / 1000.f;
                    auto point = QPointF(time, data);
                    ringbuffer.write_data_force(&point, 1);
                }
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });


        // 模拟实时数据
        connect(chartTabs[tabName].timer, &QTimer::timeout, currentWidget, [this,chart,tabName]() {
            auto& series_list = chartTabs[tabName].series_list;
            for (int i = 0; i < series_list.size(); ++i) {
                auto &ringbuffer = groups[chartTabs[tabName].group].ring_buffers[i];
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

void MainWindow::delete_chart(QFrame* frame) {
    auto& chartTab = chartTabs[frame->property("chart").toString()];
    chartTab.timer->stop();
    chartTab.state = chartTab::State::Closed;
    --groups[chartTab.group].used;
    if (frame!=nullptr) {frame->close();};
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
    groups[group->text(0)].ring_buffers.emplace_back();
}

void MainWindow::remove_item(QTreeWidgetItem* item) {
    auto group = item->parent();
    groups[group->text(0)].ring_buffers.pop_back();
    group->removeChild(ui->group_treeWidget->currentItem());
}
