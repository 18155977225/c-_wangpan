#ifndef ONLINE_H
#define ONLINE_H

#include <QWidget>
#include "protocol.h"
#include <QListWidget>
#include "tcpclient.h"

namespace Ui {
class Online;
}

class Online : public QWidget
{
    Q_OBJECT

public:
    explicit Online(QWidget *parent = nullptr);
    ~Online();

    void showUser(PDU *pdu);

    void clearListWidget();

private slots:
    void on_addFriendButton_clicked();

private:
    Ui::Online *ui;
};



#endif // ONLINE_H
