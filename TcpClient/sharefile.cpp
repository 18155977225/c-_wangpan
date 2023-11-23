#include "sharefile.h"
#include "tcpclient.h"
#include "opewidget.h"
#include <cstring>

ShareFile::ShareFile(QWidget *parent) : QWidget(parent)
{
    this->resize(300, 200);
    this->setWindowTitle("分享文件");

    m_pSelectAllPB = new QPushButton("全选");
    m_pCancelSelectPB = new QPushButton("取消全选");

    m_pOKPB = new QPushButton("确定");
    m_pCancelPB = new QPushButton("取消");;

     m_pSA = new QScrollArea;
     m_pFriendW = new QWidget;
     m_pfriendWVBL = new QVBoxLayout(m_pFriendW);
     m_pButtonGroup = new QButtonGroup(m_pFriendW);
     m_pButtonGroup->setExclusive(false);

     QHBoxLayout *pTopHBL = new QHBoxLayout;
     pTopHBL->addWidget(m_pSelectAllPB);
     pTopHBL->addWidget(m_pCancelSelectPB);
     pTopHBL->addStretch();

     QHBoxLayout *pDownHBL = new QHBoxLayout;
     pDownHBL->addWidget(m_pOKPB);
     pDownHBL->addWidget(m_pCancelPB);

     QVBoxLayout *pMainVBL = new QVBoxLayout;
     pMainVBL->addLayout(pTopHBL);
     pMainVBL->addWidget(m_pSA);
     pMainVBL->addLayout(pDownHBL);
     setLayout(pMainVBL);


     // 关联信号槽
     connect(m_pCancelSelectPB, SIGNAL(clicked(bool)), this, SLOT(cancelSelect()));
     connect(m_pSelectAllPB, SIGNAL(clicked(bool)), this, SLOT(selectAll()));
     connect(m_pOKPB, SIGNAL(clicked(bool)), this, SLOT(confirmShare()));
     connect(m_pCancelPB, SIGNAL(clicked(bool)), this, SLOT(cancelShare()));
}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

void ShareFile::updateFriend(QListWidget *friendList)
{
    if (friendList == NULL) {
        return;
    }
    // 移除之前的好友列表
    QAbstractButton *tmp = NULL;
    QList<QAbstractButton*> preFriendList= m_pButtonGroup->buttons();
    for (int i =0 ; i< preFriendList.size(); i++) {
        tmp = preFriendList[i];
        m_pfriendWVBL->removeWidget(tmp);
        m_pButtonGroup->removeButton(tmp);
        preFriendList.removeOne(tmp);
        delete tmp;
        tmp = NULL;
    }
    // 将好友列表显示出来
    QCheckBox *pCB = NULL;
    for (int i =0; i< friendList->count(); i++) {
        pCB = new QCheckBox(friendList->item(i)->text());
        m_pfriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);

    }
    m_pSA->setWidget(m_pFriendW);
}

// 取消全选
void ShareFile::cancelSelect()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for (int i=0; i< cbList.size(); i++) {
        if (cbList[i]->isChecked()) {
            cbList[i]->setChecked(false);
        }
    }
}

// 全选
void ShareFile::selectAll()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for (int i=0; i< cbList.size(); i++) {
        if (!cbList[i]->isChecked()) {
            cbList[i]->setChecked(true);
        }
    }
}

// 确认共享
void ShareFile::confirmShare()
{

    getInstance().setHidden(true);

    // 整理数据发给服务器: 发送者 接收者 当前路径 文件名
    QString send_name = TcpClient::getInstance().getLoginName();
    QString cur_path = TcpClient::getInstance().getCurPath();
    QString share_file_name = OpeWidget::getInstance().getBook()->getShareFileName();
    QString file_path = cur_path + "/" + share_file_name;

    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();

    QStringList rec_names;
    for (int i=0; i< cbList.size(); i++) {
        if (cbList[i]->isChecked()) {
            rec_names.append(cbList[i]->text().split(" ")[0]);

        }
    }
    // 接收者数量
    int num =rec_names.size();

    PDU *pdu = mkPDU(32*num + file_path.size() +1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    // caData 放 ：send_name + num
    sprintf(pdu->caData, "%s %d", send_name.toStdString().c_str(), num);
    // caMsg 放：rec_names + cur_path
    for (int i=0; i< rec_names.size(); i++) {
        memcpy((char*)(pdu->caMsg)+i*32, rec_names[i].toStdString().c_str(), rec_names[i].size());
        qDebug() << rec_names[i];
    }
    memcpy((char*)(pdu->caMsg)+num*32, file_path.toStdString().c_str(), file_path.size());

    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

// 取消共享
void ShareFile::cancelShare()
{
    hide();
}
