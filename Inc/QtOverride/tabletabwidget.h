//
// Created by liaohy on 9/5/25.
//

#ifndef MEMRW_TABLETABWIDGET_H
#define MEMRW_TABLETABWIDGET_H

#include <QWidget>

#include "groupitemadddialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TableTabWidget; }
QT_END_NAMESPACE

class TableTabWidget : public QWidget {
Q_OBJECT

public:
    explicit TableTabWidget(const std::shared_ptr<GroupTreeWidget::Group>& group, QWidget *parent = nullptr);

    ~TableTabWidget() override;

public slots:
    void button_clicked_handle();

private:
    std::shared_ptr<GroupTreeWidget::Group> group;
    void load_variables();

private:
    Ui::TableTabWidget *ui;
};


#endif //MEMRW_TABLETABWIDGET_H
