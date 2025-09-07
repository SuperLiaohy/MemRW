//
// Created by liaohy on 8/30/25.
//

#ifndef MEMRW_GROUPITEMADDDIALOG_H
#define MEMRW_GROUPITEMADDDIALOG_H

#include <QDialog>
#include <qtreewidget.h>

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

    enum class Type { INT8, UINT8, INT16, UINT16, INT32, UINT32, INT64, UINT64, FLOAT, DOUBLE};
    static int SizeType(Type type);

public slots:
    void on_colorBtn_clicked();
    void on_typeBox_currentIndexChanged(int index);
    void on_groupBox_currentIndexChanged(int index);
public:
    QString itemName();
    QString itemAddr();
    QString itemType();
    Type itemTypeEnum();
    QString itemSize();
    QColor itemColor();
    QTreeWidgetItem* itemGroup();
private:
    QTreeWidget* treeWidget;
    TreeItem* item;
    QTreeWidgetItem* group;

private:
    Ui::GroupItemAddDialog *ui;
};

#include "../../RingBuffer.h"
namespace GroupTreeWidget {
    struct variable {
        uint64_t address{};
        GroupItemAddDialog::Type type{GroupItemAddDialog::Type::INT32};
        QColor color;
        RingBuffer<8000, QPointF, QList<QPointF>> ring_buffers;
        uint8_t size() const {
            switch (type) {
                case GroupItemAddDialog::Type::INT8:
                case GroupItemAddDialog::Type::UINT8:
                    return 1;
                case GroupItemAddDialog::Type::INT16:
                case GroupItemAddDialog::Type::UINT16:
                    return 2;
                case GroupItemAddDialog::Type::INT32:
                case GroupItemAddDialog::Type::UINT32:
                case GroupItemAddDialog::Type::FLOAT:
                    return 4;
                case GroupItemAddDialog::Type::INT64:
                case GroupItemAddDialog::Type::UINT64:
                case GroupItemAddDialog::Type::DOUBLE:
                    return 8;
            }
        }
    };
    struct Group {
        int bound{};
        int used{};
        std::unordered_map<QString, variable> variables;
    };
}
#endif //MEMRW_GROUPITEMADDDIALOG_H