#include "friend.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QDebug>
#include "privatechat.h"

Friend::Friend(QWidget *parent) : QWidget(parent)
{
    m_pShowMsgTE = new QTextEdit;
    m_pFriendListWidget = new QListWidget;
    m_pInputMsgLE= new QLineEdit;

    m_pDelFriendPB = new QPushButton("删除好友");
    m_pFlushFriendPB = new QPushButton("好友列表");
    m_pShowOnlineUserPB =new  QPushButton("显示在线用户");
    m_pSearchUserPB =new  QPushButton("查找用户");
    m_pMsgSendPB  =new  QPushButton("群发");
    m_pPrivateChatPB =new  QPushButton("私聊");

    QVBoxLayout *pRightPBVBL = new QVBoxLayout;
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUserPB);
    pRightPBVBL->addWidget(m_pSearchUserPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout * pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(pRightPBVBL);



    QHBoxLayout * pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline = new Online;
    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();

    setLayout(pMain);

    // 关联信号和槽函数
    connect(m_pShowOnlineUserPB, SIGNAL(clicked(bool)), this, SLOT(showOnline()));
    connect(m_pSearchUserPB, SIGNAL(clicked(bool)), this, SLOT(searchUser()));
    connect(m_pFlushFriendPB, SIGNAL(clicked(bool)), this, SLOT(flushFriend()));
    connect(m_pDelFriendPB, SIGNAL(clicked(bool)), this, SLOT(deleteFriend()));
    connect(m_pPrivateChatPB, SIGNAL(clicked(bool)), this, SLOT(privateChat()));
    connect(m_pMsgSendPB, SIGNAL(clicked(bool)), this, SLOT(groupChat()));

}

void Friend::showAllOnlineUser(PDU *pdu)
{
    if (pdu == NULL) {
        return;
    }
    m_pOnline->showUser(pdu);
}

void Friend::showFriendList(PDU *pdu)
{
    if (pdu == NULL) {
        return;
    }

//     // 显示好友 name
//    uint  uiSize = pdu->uiMsgLen/32;
//    char caName[32] = {'\0'};
//    for (uint i = 0; i< uiSize; i++) {
//        memcpy(caName, (char*)(pdu->caMsg)+i*32, 32);
//        m_pFriendListWidget->addItem(caName);
//    }

    // 显示好友 name + online
    uint  uiSize = pdu->uiMsgLen/64;
    char caMsg[64] = {'\0'};
    for (uint i = 0; i< uiSize; i++) {
        memcpy(caMsg, (char*)(pdu->caMsg)+i*64, 64);
        qDebug() << caMsg;
        m_pFriendListWidget->addItem(caMsg);
    }

}

void Friend::updateGroupMsg(PDU *pdu)
{
    if (pdu == NULL) {
        return;
    }
    char send_name[32] = {'\0'};
    memcpy(send_name, pdu->caData, 32);
    QString msg = QString("%1 : %2").arg(send_name).arg((char*)(pdu->caMsg));
    m_pShowMsgTE->append(msg);

}

QListWidget *Friend::getFriendList()
{
    return m_pFriendListWidget;
}

void Friend::showOnline()
{
    if (m_pOnline->isHidden()) {
        m_pOnline->show();
        // 封装数据
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        // 发请求给服务器
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);

        free(pdu);
        pdu = NULL;

    } else {
        m_pOnline->hide();
        // 清空online的listWidgit的内容
        m_pOnline->clearListWidget();
    }
}

void Friend::searchUser()
{
        QString name = QInputDialog::getText(this, "搜索用户", "用户名");
        m_searchName = name;
        if (!name.isEmpty()) {
            qDebug() << name;
            // 打包数据发给服务器
            PDU *pdu = mkPDU(0);
            memcpy(pdu->caData, name.toStdString().c_str(), name.size());
            pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_REQUEST;
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);

            free(pdu);
            pdu = NULL;
        }
}

void Friend::flushFriend()
{
    // 获取客户端的name
    QString loginname = TcpClient::getInstance().getLoginName();
//    qDebug() << loginname;
    // 点击先清空好友列
    m_pFriendListWidget->clear();
    // 打包数据
    PDU *pdu = mkPDU(0);
    memcpy(pdu->caData, loginname.toStdString().c_str(), loginname.size());
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    // 发给服务器
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);

    free(pdu);
    pdu = NULL;
}

void Friend::deleteFriend()
{
    if (m_pFriendListWidget->currentItem() != NULL) {
        QString str = m_pFriendListWidget->currentItem()->text();
        QString f_name = str.split(" ")[0];
        QString u_name = TcpClient::getInstance().getLoginName();
//        qDebug() << f_name << u_name;

        // 打包数据
        PDU *pdu = mkPDU(0);
        memcpy(pdu->caData, u_name.toStdString().c_str(), u_name.size());
        memcpy(pdu->caData+32, f_name.toStdString().c_str(), f_name.size());

        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        // 发给服务器
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);

        free(pdu);
        pdu = NULL;

    }
}

void Friend::privateChat()
{
    if (m_pFriendListWidget->currentItem() != NULL) {
        // 私聊好友的名字
        QString friend_name = m_pFriendListWidget->currentItem()->text().split(" ")[0];
        // 设置要私聊的好友姓名
        PrivateChat::getInstance().setChatName(friend_name);
        PrivateChat::getInstance().setWindowTitle(friend_name);
        // 显示私聊窗口
        if (PrivateChat::getInstance().isHidden()) {
            PrivateChat::getInstance().show();
        }
    } else {
        QMessageBox::warning(this, "提示", "请选择一个要私聊的好友");
    }
}

void Friend::groupChat()
{
    QString msg = m_pInputMsgLE->text();
    m_pInputMsgLE->clear();
    QString str = QString("I : %1").arg(msg);
    m_pShowMsgTE->append(str);

    if (!msg.isEmpty()) {
        PDU *pdu = mkPDU(msg.toUtf8().size()+1);
        // 打包数据：name+msg
        QString name = TcpClient::getInstance().getLoginName();
        memcpy(pdu->caData, name.toStdString().c_str(), name.size());
        memcpy(pdu->caMsg, msg.toStdString().c_str(), msg.toUtf8().size());
        pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        // 发给服务器
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);

        free(pdu);
        pdu = NULL;
    } else {
        QMessageBox::warning(this, "群聊", "消息不能为空！");
    }
}
