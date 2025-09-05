//
// Created by liaohy on 8/31/25.
//

#ifndef MEMRW_ADDCHARTTABDIALOG_H
#define MEMRW_ADDCHARTTABDIALOG_H

#include <QDialog>
#include "groupitemadddialog.h"

class QTreeWidget;
class QTreeWidgetItem;
class QTabWidget;
QT_BEGIN_NAMESPACE

namespace Ui {
    class AddChartTabDialog;
}

QT_END_NAMESPACE

class AddChartTabDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddChartTabDialog(std::unordered_map<QString, std::shared_ptr<GroupTreeWidget::Group>>& Groups, QTabWidget *tabWidget, QWidget *parent = nullptr);

    ~AddChartTabDialog() override;

public slots:
//    void on_groupBox_currentTextChanged(QString string);

public:
    enum mode {
        Simple = 0,
        Full = 1,
        Advanced = 2,
    };

    QString tabName();
    std::shared_ptr<GroupTreeWidget::Group>& chartGroup();
    int chartMode();

private:
    std::unordered_map<QString, std::shared_ptr<GroupTreeWidget::Group>>& Groups;

private:
    Ui::AddChartTabDialog *ui;
};


#endif //MEMRW_ADDCHARTTABDIALOG_H