#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include "protocol.h"

namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

public:
    explicit PrivateChat(QWidget *parent = nullptr);
    ~PrivateChat();

    static PrivateChat & getInstance();

    void setChatName(QString strName);
    void receveMsg(const PDU *pdu);

private slots:
    void on_pushButton_clicked();

private:
    Ui::PrivateChat *ui;
    QString m_strChatName;
    QString m_loginName;
};

#endif // PRIVATECHAT_H
