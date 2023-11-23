#include "online.h"
#include "ui_online.h"

Online::Online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Online)
{
    ui->setupUi(this);


}

Online::~Online()
{
    delete ui;
}

void Online::showUser(PDU *pdu)
{
    if (pdu == NULL) {
        return;
    }
    // 对收到的数据 拆封显示出来
    uint uiSize = pdu->uiMsgLen / 32;
    char caTmp[32];
    for (uint i =0; i<uiSize; i++) {
        memcpy(caTmp, (char*)(pdu->caMsg)+i*32, 32);
        ui->listWidget->addItem(caTmp);
    }
}

void Online::clearListWidget()
{
    // 清空listWidget
    ui->listWidget->clear();
}


// 点击添加好友
void Online::on_addFriendButton_clicked()
{
    // 要加的好友名字
    QString userName = ui->listWidget->currentItem()->text();
    qDebug() << userName;
    // 自己的名字
    QString loginName = TcpClient::getInstance().getLoginName();
    qDebug() << loginName;
    // 封装数据给服务器
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    strncpy(pdu->caData, userName.toStdString().c_str(), userName.size());
    strncpy(pdu->caData+32, loginName.toStdString().c_str(), loginName.size());
    // 给服务器发登录消息
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);

    free(pdu);
    pdu = NULL;


}

