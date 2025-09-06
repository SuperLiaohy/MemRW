//
// Created by liaohy on 9/5/25.
//

#ifndef MEMRW_TABLETABWIDGET_H
#define MEMRW_TABLETABWIDGET_H

#include <QWidget>

#include "groupitemadddialog.h"
#include "DAPReader.h"
QT_BEGIN_NAMESPACE
namespace Ui { class TableTabWidget; }
QT_END_NAMESPACE

class TableTabWidget : public QWidget {
Q_OBJECT

public:
    explicit TableTabWidget(const std::shared_ptr<GroupTreeWidget::Group>& group, QWidget *parent = nullptr);

    ~TableTabWidget() override;

public slots:
    void write_button_clicked_handle();
    void read_button_clicked_handle();

public:
    using request_buf = std::shared_ptr<RingBuffer<128,DAP::TransferRequest>>;
    using response_buf = std::shared_ptr<RingBuffer<128,DAP::TransferResponse>>;
    struct buf {
        request_buf requests;
        response_buf responses;
        buf() {
            requests = std::make_shared<RingBuffer<128,DAP::TransferRequest>>();
            responses = std::make_shared<RingBuffer<128,DAP::TransferResponse>>();
        }
    };
    void btnEnable(bool able);
    std::unordered_map<QString, buf> buffer;

private:
    void generate_request(const QString &name, const GroupTreeWidget::variable& variable, uint32_t data);
    void generate_response(const QString &name, const GroupTreeWidget::variable& variable, int row);
    std::shared_ptr<GroupTreeWidget::Group> group;
    void load_variables();

private:
    Ui::TableTabWidget *ui;
};


#endif //MEMRW_TABLETABWIDGET_H
