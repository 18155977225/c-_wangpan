#include "privatechat.h"
#include "ui_privatechat.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QMessageBox>


PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
    m_strChatName = "";
    m_loginName = "";


}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::setChatName(QString strName)
{
    m_strChatName = strName;
    m_loginName = TcpClient::getInstance().getLoginName();
}

// 收到消息进行显示
void PrivateChat::receveMsg(const PDU *pdu)
{
    if (pdu == NULL) {
        return;
    }
    char send_name[32] = {'\0'};
    memcpy(send_name, pdu->caData, 32);
    QString msg = QString("%1 : %2").arg(send_name).arg((char*)(pdu->caMsg));

    this->ui->chatTextEdit->append(msg);


}

// 点击发送消息按钮
void PrivateChat::on_pushButton_clicked()
{
    QString strmsg = ui->lineEdit->text();
    ui->lineEdit->clear();

    QString msg = QString("I : %1").arg(strmsg);
    ui->chatTextEdit->append(msg);


    if (!strmsg.isEmpty()) {
        // 打包数据
        PDU *pdu = mkPDU(strmsg.toUtf8().size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        // 发送方name 接收方name 消息内容
        memcpy(pdu->caData, m_loginName.toStdString().c_str(), m_loginName.size());
        memcpy(pdu->caData+32, m_strChatName.toStdString().c_str(), m_strChatName.size());
        strcpy((char*)(pdu->caMsg), strmsg.toStdString().c_str());

        // 发给服务器
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;

    } else {
        QMessageBox::warning(this, "警告", "输入的消息不能为空");
    }

}

