//
// Created by liaohy on 8/30/25.
//

#ifndef MEMRW_GROUPITEMADDDIALOG_H
#define MEMRW_GROUPITEMADDDIALOG_H

#include <QDialog>

class QTreeWidget;
class TreeItem;
QT_BEGIN_NAMESPACE

namespace Ui {
    class GroupItemAddDialog;
}

QT_END_NAMESPACE

class GroupItemAddDialog : public QDialog {
    Q_OBJECT

public:
    explicit GroupItemAddDialog(QTreeWidget* tree, TreeItem* item, QWidget *parent = nullptr);

    ~GroupItemAddDialog() override;

public slots:
    void on_colorBtn_clicked();
    void on_typeBox_currentIndexChanged(int index);
public:
    QString itemName();
    QString itemAddr();
    QString itemType();
    QString itemSize();
    QColor itemColor();
    QString itemGroup();
private:
    QTreeWidget* treeWidget;
    TreeItem* item;

private:
    Ui::GroupItemAddDialog *ui;
};


#endif //MEMRW_GROUPITEMADDDIALOG_H