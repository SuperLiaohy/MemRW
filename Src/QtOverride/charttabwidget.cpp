//
// Created by liaohy on 9/4/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ChartTabWidget.h" resolved

#include "charttabwidget.h"
#include "ui_charttabwidget.h"

#include "datalogdialog.h"

ChartTabWidget::ChartTabWidget(GroupTreeWidget::Group &group, QWidget *parent) :
        QWidget(parent), ui(new Ui::ChartTabWidget), group(group) {
    ui->setupUi(this);

    QStringList show_mode;
    show_mode.append("only lines");
    show_mode.append("lines and points");
    show_mode.append("lines, points and tips");

    ui->showBox->addItems(show_mode);

    connect(ui->showBox, &QComboBox::currentIndexChanged, this, [this](int index){
        if (index == 0) {
            for (auto series: series_list) {
                series->setPointsVisible(false);
                series->setPointLabelsVisible(false);
            }
        } else if (index==1) {
            for (auto series: series_list) {
                series->setPointsVisible(true);
                series->setPointLabelsVisible(false);
            }
        } else if (index==2) {
            for (auto series: series_list) {
                series->setPointsVisible(true);
                series->setPointLabelsFormat("(@xPoint, @yPoint)");
                series->setPointLabelsVisible(true);
            }
        }
    });

    // create the series to paint the chart and load ringbuffer data
    series_list.resize(group.variables.size());

    int index = 0;
    for (auto & variable: group.variables) {
        series_list[index] = new QLineSeries(ui->chartWidget->chart());
        series_list[index]->setName(variable.first);
        series_list[index]->setProperty("color", variable.second.color);
        ui->chartWidget->addSeriesLine(series_list[index]);
        ++index;
    }
    ++group.bound;
    // create the timer to update QChartView
    timer = new QTimer(this);
    timer->stop();

    connect(timer, &QTimer::timeout, this, &ChartTabWidget::timerUpdate);
}

ChartTabWidget::~ChartTabWidget() {
    --group.bound;
    if (logfile != nullptr) {
        logfile->close();
        logfile.reset();
    }
    delete ui;
}

ChartWidget *ChartTabWidget::chartWidget() {
    return ui->chartWidget;
}

void ChartTabWidget::UpdateFreq() {
    auto time = duration_cast<std::chrono::microseconds>((std::chrono::high_resolution_clock::now() - start_time)).count()/1000.f;
    if ((time - last_time)>1000) {
        ui->freqLabel->setText(QString("freq: %1").arg(freq));
        last_time = time;
        freq = 0;
    }
    ++freq;
}

void ChartTabWidget::on_logcfgBtn_clicked() {
    DataLogDialog *dlg = new DataLogDialog(file_name, islog, this);
    if (dlg->exec()==QDialog::Accepted) {
        file_name = dlg->fileName();
        islog = dlg->islog();
    }
}

void ChartTabWidget::on_startBtn_clicked() {
    if (group.variables.empty()) {
        QMessageBox::critical(this, "MESSAGE", "you can not start because the group you bound is empty!",QMessageBox::Ok);
        return;
    }

    if (islog) {
        if (logfile!=nullptr) {
            logfile->close();
            logfile.reset();
        }
        if (file_name.isEmpty()) {
            QMessageBox::critical(this,"MESSAGE","can not open logfile!",QMessageBox::Ok);
            return;
        }
        logfile = std::make_shared<QFile>(file_name);
        if (!logfile->open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this,"MESSAGE","can not open logfile!",QMessageBox::Ok);
            logfile.reset();
            return;
        }
    }


    for (auto series: series_list) {ui->chartWidget->chart()->removeSeries(series);}

    series_list.resize(group.variables.size());

    QStringList csv_header;
    csv_header << "TimeStamp";

    int index = 0;
    for (auto & variable: group.variables) {
        variable.second.ring_buffers.reset();
        if (islog) csv_header << variable.first;
        series_list[index] = new QLineSeries(ui->chartWidget->chart());
        series_list[index]->setName(variable.first);
        series_list[index]->setProperty("color", variable.second.color);
        ui->chartWidget->addSeriesLine(series_list[index]);
        ++index;
    }
    if (islog) writeCsv({csv_header});

    ++group.used;

    last_time = 0;
    freq = 0;

    ui->chartWidget->loadDefaultRange();
    ui->showBox->setCurrentIndex(0);
    ui->showBox->setEnabled(false);
    ui->logcfgBtn->setEnabled(false);

    start_time = std::chrono::high_resolution_clock::now();
    timer->start(30);
    isRunning= true;


}

void ChartTabWidget::on_stopBtn_clicked() {
    isRunning= false;
    ui->logcfgBtn->setEnabled(true);
    ui->showBox->setEnabled(true);
    if (logfile != nullptr) {
        logfile->close();
        logfile.reset();
    }
    timer->stop();
    --group.used;
}

void ChartTabWidget::timerUpdate() {
    for (auto series : series_list) {
        auto &ringbuffer = group.variables.at(series->name()).ring_buffers;
        if (ringbuffer.is_full())
            series->replace(ringbuffer.get_container());
        else
            series->replace(ringbuffer.get_valid_container());
    }
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(
            (std::chrono::high_resolution_clock::now() - start_time)).count() / 1000.f;
    // 自动滚动视图
    if (time > ui->chartWidget->xRange()) {
        ui->chartWidget->chart()->axisX()->setRange(time - ui->chartWidget->xRange(), time);
    }
}

void ChartTabWidget::writeCsv(const QList<QStringList> &data) {
    QTextStream out(logfile.get());
    for (const QStringList &row: data) {
        out << row.join(',') << "\n";
    }
}

