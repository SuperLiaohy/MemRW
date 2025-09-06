//
// Created by liaohy on 9/5/25.
//

#ifndef MEMRW_ADDTABLETABDIALOG_H
#define MEMRW_ADDTABLETABDIALOG_H

#include <QDialog>

#include "groupitemadddialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AddTableTabDialog; }
QT_END_NAMESPACE

class AddTableTabDialog : public QDialog {
Q_OBJECT

public:
    explicit AddTableTabDialog(std::unordered_map<QString, std::shared_ptr<GroupTreeWidget::Group>>& Groups, QTabWidget *tabWidget, QWidget *parent = nullptr);

    ~AddTableTabDialog() override;

    QString tabName();
    std::shared_ptr<GroupTreeWidget::Group>& chartGroup();
private:
    std::unordered_map<QString, std::shared_ptr<GroupTreeWidget::Group>>& Groups;

private:
    Ui::AddTableTabDialog *ui;
};


#endif //MEMRW_ADDTABLETABDIALOG_H
