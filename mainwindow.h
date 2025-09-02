#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAbstractItemModel>
#include <QMainWindow>

#include <memory>
#include <thread>
#include <unordered_map>
#include "RingBuffer.h"
#include "DAPReader.h"
class QTreeWidgetItem;
class QFrame;
class QLineSeries;
class QLabel;
class TreeModel;
class GroupItemAddDialog;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void on_openBtn_clicked();
    void on_reloadBtn_clicked();
    void on_actioncreate_group_triggered();
    void on_actiondelete_group_triggered();
    void on_actiondelete_item_triggered();
    void on_actiondisplay_group_dock_triggered();
    void on_actiondisplay_variable_dock_triggered();
    void on_actionadd_chart_tab_triggered();
    void on_actionadd_table_tab_triggered();
    void on_actionconnect_triggered();

    void on_dwarf_treeView_doubleClicked(const QModelIndex &index);
    void on_group_treeWidget_doubleClicked(const QModelIndex &index);
    void customGroupMenuRequested(const QPoint& pos);
public:
private:
    void create_chart();
    void delete_chart(QFrame* frame);

    void create_group();
    void delete_group(QTreeWidgetItem* group);

    void add_item(GroupItemAddDialog* dlg);
    void remove_item(QTreeWidgetItem* item);
    TreeModel* model = nullptr;
    using rb = RingBuffer<8000,QPointF,QList<QPointF>>;
    struct group {
        QList<rb> ring_buffers;
        int used{};
    };
    std::unordered_map<QString, group> groups;
    struct chartTab {
        enum class State {
            Stop,
            Running,
            Closed,
        };
        State state;
        QTimer* timer;
        QString group;
        uint32_t freq;
        uint32_t last_time;
        std::shared_ptr<std::thread> thread;
        RingBuffer<200,DAP::TransferRequest> request_rb;
        RingBuffer<200,DAP::TransferResponse> response_rb;
        QList<qlonglong> addr;
        QList<QLineSeries *> series_list;
        std::chrono::high_resolution_clock::time_point start_time;
        ~chartTab() {
            if (thread && thread->joinable()) {
                thread->join();
            }
        }
    };
    std::unordered_map<QString, chartTab> chartTabs;
    std::unique_ptr<std::thread> link_thread;
    std::unique_ptr<DAPReader> link;
    bool is_closing{false};
    bool is_disconnect{true};

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
