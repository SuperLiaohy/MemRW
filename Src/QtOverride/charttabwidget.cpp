//
// Created by liaohy on 9/4/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ChartTabWidget.h" resolved

#include "charttabwidget.h"
#include "ui_charttabwidget.h"


ChartTabWidget::ChartTabWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::ChartTabWidget) {
    ui->setupUi(this);
}

ChartTabWidget::~ChartTabWidget() {
    delete ui;
}
