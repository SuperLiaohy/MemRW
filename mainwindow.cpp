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

    auto setupTab = [](QTabWidget *tabWidget) {
        if (tabWidget->count()==0) tabWidget->hide();
        tabWidget->setTabsClosable(true);
        connect(tabWidget, &QTabWidget::tabCloseRequested, tabWidget, [tabWidget](int index){tabWidget->removeTab(index);});
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
    rb.emplace_back();

}

void MainWindow::on_actiondelete_group_triggered() {
    QModelIndex index = ui->group_treeWidget->currentIndex();
    ui->group_treeWidget->takeTopLevelItem(index.row());
    rb.pop_back();
}

void MainWindow::on_actiondelete_item_triggered() {
    auto index = ui->group_treeWidget->currentIndex();
    auto item = ui->group_treeWidget->currentItem()->parent();
    item->removeChild(ui->group_treeWidget->currentItem());
    rb[ui->group_treeWidget->indexOfTopLevelItem(item)].pop_back();
}

void MainWindow::on_actiondisplay_group_dock_triggered() {
    ui->groupDock->show();
}

void MainWindow::on_actiondisplay_variable_dock_triggered() {
    ui->dwarfDock->show();
}

void MainWindow::on_actionadd_chart_tab_triggered() {
    if (ui->group_treeWidget->topLevelItemCount() == 0) {
        QMessageBox::critical(this,"MESSAGE","You have to have at least one group.",QMessageBox::Close);
        return;
    }
    AddChartTabDialog* dlg = new AddChartTabDialog(ui->group_treeWidget, this);
    auto res = dlg->exec();
    if (res == QDialog::Accepted) {

        QChart *chart = new QChart();
        chart->setTitle("test chart");
        // chart->setAnimationOptions(QChart::SeriesAnimations);

        QValueAxis *axisX = new QValueAxis(chart);
        axisX->setTitleText("time (ms)");
        axisX->setRange(0,5000);
        QValueAxis *axisY = new QValueAxis(chart);
        axisY->setTitleText("value");
        axisY->setRange(0, 1000);

        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        auto group = dlg->chartGroup();
        QList<QLineSeries*> series_list(group->childCount());
        for (int count = 0; count < group->childCount(); ++count) {
            series_list[count] = new QLineSeries(chart);
            series_list[count]->setName(group->child(count)->data(0,Qt::DisplayRole).toString());
            series_list[count]->setProperty("color",QColor(group->child(count)->data(4,Qt::DisplayRole).toString()));
            chart->addSeries(series_list[count]);
            series_list[count]->attachAxis(axisX);
            series_list[count]->attachAxis(axisY);
        }

        int tabIndex = ui->chartTab->addTab(new QFrame(this), "Tab" + QString::number(ui->chartTab->currentIndex()+2));
        ui->chartTab->setCurrentIndex(tabIndex);

        auto currentWidget = ui->chartTab->currentWidget();
        QChartView *chartView = new QChartView(chart, currentWidget);
        chartView->setChart(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        QPushButton *startBtn = new QPushButton("Start", currentWidget);
        QPushButton *stopBtn = new QPushButton("Stop", currentWidget);
        QPushButton *reloadBtn = new QPushButton("Reload Group", currentWidget);


        QGridLayout *gridLayout = new QGridLayout(currentWidget);
        currentWidget->setLayout(gridLayout);
        gridLayout->addWidget(reloadBtn, 0, 0, 1, 1);
        gridLayout->addWidget(startBtn,0,1,1,1);
        gridLayout->addWidget(stopBtn,0,2,1,1);
        gridLayout->addWidget(chartView,1,0,1,3);


        auto start_time = (std::chrono::high_resolution_clock::now());

        int rb_index = ui->group_treeWidget->indexOfTopLevelItem(group);
        std::thread([this,group,rb_index,start_time]() {
            while (true) {

                for (int count = 0; count < group->childCount(); ++count) {
                    auto& ringbuffer = rb[rb_index][count];
                    auto addr = group->child(count)->data(2,Qt::DisplayRole).toString().toLongLong(nullptr, 16);
                    // qDebug()<<group->child(count)->data(2,Qt::DisplayRole).toString();
                    auto data = link->read_mem(addr);
                    auto time = std::chrono::duration_cast<std::chrono::microseconds>((std::chrono::high_resolution_clock::now() - start_time)).count()/1000.f;
                    auto point = QPointF(time,data);
                    ringbuffer.write_data_force(&point,1);
                }
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }).detach();


        // 模拟实时数据
        QTimer *timer = new QTimer(currentWidget);
        connect(timer, &QTimer::timeout, currentWidget,[this,rb_index,series_list,chart,start_time](){
            std::vector<QPointF> points;
            points.reserve(30);
            // for (int i = 0; i < 30; ++i) {
            //     points.emplace_back(x++,500+QRandomGenerator::global()->generateDouble()*100);
            // }
            for (int i = 0; i < series_list.size(); ++i) {
                auto& ringbuffer = rb[rb_index][i];
                if (ringbuffer.is_full())
                    series_list[i]->replace(ringbuffer.get_container());
                else
                    series_list[i]->replace(ringbuffer.get_valid_container());
            }
            auto time = std::chrono::duration_cast<std::chrono::microseconds>((std::chrono::high_resolution_clock::now() - start_time)).count()/1000.f;
            // 自动滚动视图
            if(time > 5000) {
                chart->axisX()->setRange(time-5000, time);
            }
        });
        timer->start(30);


    }


}

void MainWindow::on_actionadd_table_tab_triggered() {
}

void MainWindow::on_actionconnect_triggered() {
    try {
        link = std::make_unique<DAPReader>();
        link->attach_to_target();
        link->auto_configure_ap();
    } catch (const std::exception& e) {
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
    if(dlg->exec()==QDialog::Accepted) {
        auto group = dlg->itemGroup();
        auto newItem = new QTreeWidgetItem();
        newItem->setText(0, dlg->itemName());
        newItem->setText(1, dlg->itemType());
        newItem->setText(2, dlg->itemAddr());
        newItem->setText(3, dlg->itemSize());
        newItem->setText(4, dlg->itemColor().name());
        newItem->setData(4, Qt::BackgroundRole, dlg->itemColor());
        group->addChild(newItem);
        qDebug() << "add new item to group: " << ui->group_treeWidget->indexOfTopLevelItem(group);
        rb[ui->group_treeWidget->indexOfTopLevelItem(group)].emplace_back();
    };
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
