#include "book.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include "opewidget.h"
#include "sharefile.h"
#include <QThread>

Book::Book(QWidget *parent) : QWidget(parent)
{
     m_cur_enter_dir = "";
     m_ptimer = new QTimer;
     m_isDownload = false;
     m_recev = 0;
     m_total = 0;



     m_pBookListWidget = new QListWidget;

     m_pFlushFilePB= new QPushButton("刷新文件");
     m_pReturnPB= new QPushButton("返回上一级");
     m_pCreateDirPB= new QPushButton("创建文件夹");
     m_pDelDirPB= new QPushButton("删除文件夹");
     m_pRenamePB= new QPushButton("重命名文件");


     QVBoxLayout *pDirVBL = new QVBoxLayout;
     pDirVBL->addWidget(m_pFlushFilePB);
     pDirVBL->addWidget(m_pReturnPB);
     pDirVBL->addWidget(m_pCreateDirPB);
     pDirVBL->addWidget(m_pDelDirPB);
     pDirVBL->addWidget(m_pRenamePB);



     m_pUploadPB= new QPushButton("上传文件");
     m_pDownloadPB= new QPushButton("下载文件");
     m_pDelFilePB= new QPushButton("删除文件");
     m_pShareFilePB= new QPushButton("共享文件");
     m_pMoveFilePB = new QPushButton("移动文件");
     m_pSelectDirPB = new QPushButton("目标目录");
     m_pSelectDirPB->setEnabled(false);

     QVBoxLayout *pFileVBL = new QVBoxLayout;
     pFileVBL->addWidget(m_pUploadPB);
     pFileVBL->addWidget(m_pDownloadPB);
     pFileVBL->addWidget(m_pDelFilePB);
     pFileVBL->addWidget(m_pShareFilePB);
     pFileVBL->addWidget(m_pMoveFilePB);
     pFileVBL->addWidget(m_pSelectDirPB);

     QHBoxLayout *pMain = new QHBoxLayout;
     pMain->addWidget(m_pBookListWidget);
     pMain->addLayout(pDirVBL);
     pMain->addLayout(pFileVBL);

     setLayout(pMain);

     // 关联信号与槽
     connect(m_pCreateDirPB, SIGNAL(clicked(bool)), this, SLOT(createDir()));
     connect(m_pFlushFilePB, SIGNAL(clicked(bool)), this, SLOT(flushFile()));
     connect(m_pDelDirPB, SIGNAL(clicked(bool)), this, SLOT(delDir()));
     connect(m_pRenamePB, SIGNAL(clicked(bool)), this, SLOT(renameFile()));
     connect(m_pBookListWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(enterDir(QModelIndex)));
     connect(m_pReturnPB, SIGNAL(clicked(bool)), this, SLOT(returnPreDir()));
     connect(m_pUploadPB, SIGNAL(clicked(bool)), this, SLOT(uploadFile()));
     connect(m_ptimer, SIGNAL(timeout()), this, SLOT(uploadFileData())); // 发出上传文件请求后，设置个定时器，然后再真正上传文件
     connect(m_pDelFilePB, SIGNAL(clicked(bool)), this, SLOT(delFile()));
     connect(m_pDownloadPB, SIGNAL(clicked(bool)), this, SLOT(downloadFile()));
     connect(m_pShareFilePB, SIGNAL(clicked(bool)), this, SLOT(shareFile()));
     connect(m_pMoveFilePB, SIGNAL(clicked(bool)), this, SLOT(moveFile()));
     connect(m_pSelectDirPB, SIGNAL(clicked(bool)), this, SLOT(selectDestDir()));



}

void Book::updateFileList(const PDU *pdu)
{
    if (pdu == NULL) {
        return;
    }
    // 更新前先清除
    m_pBookListWidget->clear();

    FileInfo *pFileInfo = NULL;
    int size = pdu->uiMsgLen/sizeof(FileInfo);
    for (int i =0; i< size ;i++ ) {
        pFileInfo = (FileInfo*)(pdu->caMsg)+i;
        qDebug() << pFileInfo->fileName << pFileInfo->fileType;
        QListWidgetItem *pItem = new QListWidgetItem;


        if (pFileInfo->fileType ==0) {
            pItem->setIcon(QIcon(QPixmap(":/img/dir.jpg")));
        } else if (pFileInfo->fileType == 1) {
            pItem->setIcon(QIcon(QPixmap(":/img/file.jpg")));
        }
        pItem->setText(pFileInfo->fileName);

        // . 和 .. 不显示
//        if (pFileInfo->fileName != QString(".") && pFileInfo->fileName != QString("..")) {
//            m_pBookListWidget->addItem(pItem);
//        }
        if (pFileInfo->fileName != QString("..")) {
            m_pBookListWidget->addItem(pItem);
        }


    }

}

void Book::clearCurEnterDir()
{
    m_cur_enter_dir = "";
}

QString Book::getCurEnterDir()
{
    return m_cur_enter_dir;
}


QString Book::getDownloadPath()
{
    return downloadFilePath;
}

QString Book::getShareFileName()
{
    return shareFileName;
}

void Book::createDir()
{
    // 封装数据： 用户名、用户路径 、文件夹名字
    QString strNewDir = QInputDialog::getText(this, "新建文件夹", "输入文件夹名称");
    if (!strNewDir.isEmpty()) {
        if (strNewDir.size()>32){
            QMessageBox::warning(this, "提示", "输入的文件夹名字不可超过32字符");
        } else{
            QString strName =  TcpClient::getInstance().getLoginName();
            QString curPath =  TcpClient::getInstance().getCurPath();

            PDU *pdu = mkPDU(curPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData, strName.toStdString().c_str(), strName.size());
            strncpy(pdu->caData+32, strNewDir.toStdString().c_str(), strNewDir.size());
            memcpy(pdu->caMsg, curPath.toStdString().c_str(),curPath.size());

            // 发送给服务器
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }



    } else {
        QMessageBox::warning(this, "提示", "输入的文件夹名字不可为空");
    }


}

void Book::flushFile()
{
    m_pBookListWidget->clear();
    QString curPath =  TcpClient::getInstance().getCurPath();
    PDU *pdu = mkPDU(curPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    memcpy(pdu->caMsg, curPath.toStdString().c_str(), curPath.size());

    // 发送给服务器
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::delDir()
{
    QString curPath =  TcpClient::getInstance().getCurPath();

    if (m_pBookListWidget->currentItem() == NULL) {
        QMessageBox::warning(this, "删除文件夹", "请选择要删除的文件夹");
    } else {
        // 选中的文件夹名
        QString del_dir_name = m_pBookListWidget->currentItem()->text();
        // 打包数据
        PDU *pdu = mkPDU(curPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_REQUEST;
        memcpy(pdu->caData, del_dir_name.toStdString().c_str(),del_dir_name.size());
        memcpy(pdu->caMsg, curPath.toStdString().c_str(),curPath.size());

        // 发送给服务器
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::renameFile()
{

    // 当前路径
    QString curPath =  TcpClient::getInstance().getCurPath();

    if (m_pBookListWidget->currentItem() == NULL) {
        QMessageBox::warning(this, "重命名文件", "请选择要重命名的文件");
    } else {
        // 选中的文件名
        QString old_file_name = m_pBookListWidget->currentItem()->text();
        // 新文件名
        QString new_file_name = QInputDialog::getText(this, "重命名文件", "输入新的文件名");

        if (!new_file_name.isEmpty()) {
            if (new_file_name.size()>32){
                QMessageBox::warning(this, "提示", "输入的文件名字不可超过32字符");
            } else{

                PDU *pdu = mkPDU(curPath.size()+1);
                pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
                // 封装数据
                strncpy(pdu->caData, old_file_name.toStdString().c_str(), old_file_name.size());
                strncpy(pdu->caData+32, new_file_name.toStdString().c_str(), new_file_name.size());
                memcpy(pdu->caMsg, curPath.toStdString().c_str(),curPath.size());

                // 发送给服务器
                TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
                free(pdu);
                pdu = NULL;
            }



        } else {
            QMessageBox::warning(this, "重命名文件", "新文件名字不可为空");
        }

    }

}


// 进入文件夹（槽函数形参与信号保持一致）
void Book::enterDir(const QModelIndex &index)
{
    QString dir_name = index.data().toString();
    m_cur_enter_dir = dir_name;
    QString cur_path = TcpClient::getInstance().getCurPath();
    // 打包数据
    PDU *pdu = mkPDU(cur_path.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    memcpy(pdu->caData, dir_name.toStdString().c_str(), dir_name.size());
    memcpy(pdu->caMsg, cur_path.toStdString().c_str(), cur_path.size());
    // 发送给服务器
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;

}

void Book::returnPreDir()
{
    // 当前路径
    QString cur_path = TcpClient::getInstance().getCurPath();
    // 顶层路径
    QString root_path = QString("D:/ServerFiles/%1").arg(TcpClient::getInstance().getLoginName());
    if (cur_path == root_path){
        QMessageBox::warning(this, "返回", "返回失败，已经是根目录");
    } else {
        // 获取上一级目录
        int index = cur_path.lastIndexOf('/');
        cur_path.remove(index, cur_path.size() - index);
        TcpClient::getInstance().setCurPath(cur_path);
        // 要清空未返回前进入的当前目录名
        clearCurEnterDir();
        // 这里直接刷新文件即可，不用再发请求
        flushFile();

    }
}

void Book::uploadFile()
{

    // 上传的文件路径
    uploadFilePath = QFileDialog::getOpenFileName();

    if (uploadFilePath.isEmpty()) {
        QMessageBox::warning(this, "上传文件", "文件名字不能为空");
    } else {
        // 解析文件名
        int index = uploadFilePath.lastIndexOf('/');
        QString uploadFileName = uploadFilePath.right(uploadFilePath.size() - index - 1);
        qDebug() << uploadFilePath << uploadFileName;

        QFile file(uploadFilePath);
        // 获得文件大小
        qint64 filesize = file.size();

        // 打包数据：当前路径+文件名+大小
        // 当前路径
        QString cur_path = TcpClient::getInstance().getCurPath();
        PDU *pdu = mkPDU(cur_path.size()+32);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        // 当前路径
        memcpy(pdu->caMsg, cur_path.toStdString().c_str(), cur_path.size()+32);
        // 文件名+大小
        sprintf(pdu->caData, "%s %lld", uploadFileName.toStdString().c_str(), filesize);

        // 发请求给服务器
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;

        // 设置定时器：1s后才真正上传文件数据  防止文件数据脏包，不好解析
        m_ptimer->start(1000);
    }
}

void Book::uploadFileData()
{

    // 关闭定时器
    m_ptimer->stop();
    QFile file(uploadFilePath);


    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }

    char *buf = new char[4096];
    qint64 len = 0;
    qint64 sendSize = 0;

    while((len = file.read(buf, 4096)) > 0)
    {

        qint64 bytesWritten = TcpClient::getInstance().getTcpSocket().write(buf, len);
        if(bytesWritten == -1)
        {
            QMessageBox::warning(this, "上传文件", "写入文件失败!");
            break;

        }
        sendSize += bytesWritten;

//        if(len > bytesWritten)       //读写字节数不一致，可能缓冲区满了
//        {
//            QThread::msleep(100);
//        }

        if (len < 0)
        {
            QMessageBox::warning(this, "上传文件", "读取文件失败!");
            break;
        }
    }

    file.close();
    delete[] buf;
    buf = NULL;



}

void Book::delFile()
{
    QString curPath =  TcpClient::getInstance().getCurPath();

    if (m_pBookListWidget->currentItem() == NULL) {
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件");
    } else {
        // 选中要删除的文件名
        QString del_file_name = m_pBookListWidget->currentItem()->text();
        // 打包数据 ： cur_path + del_file_name
        PDU *pdu = mkPDU(curPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_REQUEST;
        memcpy(pdu->caData, del_file_name.toStdString().c_str(),del_file_name.size());
        memcpy(pdu->caMsg, curPath.toStdString().c_str(),curPath.size());

        // 发送给服务器
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::downloadFile()
{
    QString curPath =  TcpClient::getInstance().getCurPath();

    if (m_pBookListWidget->currentItem() == NULL) {
        QMessageBox::warning(this, "下载文件", "请选择要下载的文件");
    } else {

        // 指定要下载的位置
        QString save_file_path = QFileDialog::getSaveFileName();
        if (save_file_path.isEmpty()) {
            QMessageBox::warning(this, "下载文件", "请选择要下载文件的位置");
            downloadFilePath.clear();
        } else {
            downloadFilePath = save_file_path;

        }

        // 选中的文件名
        QString download_file_name = m_pBookListWidget->currentItem()->text();
        // 打包数据
        PDU *pdu = mkPDU(curPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        strcpy(pdu->caData, download_file_name.toStdString().c_str());
        memcpy(pdu->caMsg, curPath.toStdString().c_str(),curPath.size());

        // 发送给服务器
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::shareFile()
{
    QListWidgetItem *pItem = m_pBookListWidget->currentItem();
    if ( pItem == NULL) {
        QMessageBox::warning(this, "分享文件", "请选择要分享的文件");
        return;
    } else {
        shareFileName = pItem->text();
    }
    // 先调用刷新好友的请求
    Friend *pFriend = OpeWidget::getInstance().getFriend();
    // 获得好友列表的ListWidget
    pFriend->flushFriend();

    QListWidget *pFriendList = pFriend->getFriendList();

    // 显示好友列表到分享文件的界面
    ShareFile::getInstance().updateFriend(pFriendList);
    if(ShareFile::getInstance().isHidden()) {
        ShareFile::getInstance().show();
    }
}

void Book::moveFile()
{
    QListWidgetItem *pItem = m_pBookListWidget->currentItem();
    if ( pItem == NULL) {
        QMessageBox::warning(this, "移动文件", "请选择你要移动的文件");
        return;
    } else {
        moveFileName = pItem->text();
        QString curPath =  TcpClient::getInstance().getCurPath();
        moveFilePath = curPath + '/' + moveFileName;
        m_pSelectDirPB->setEnabled(true);
        QMessageBox::information(this, "提示", "请选择你要移动文件的目标位置，并点击目标目录");
    }
}

void Book::selectDestDir()
{
    QListWidgetItem *pItem = m_pBookListWidget->currentItem();
    if ( pItem == NULL) {
        QMessageBox::warning(this, "移动文件", "请选择要移动的文件夹");
        return;
    } else {
        QString destDirName = pItem->text();
        QString curPath =  TcpClient::getInstance().getCurPath();
        destDirPath = curPath + '/' + destDirName;

        // 打包要移动的文件路径 + 目的地文件夹路径 给服务器
        int movePathSize = moveFilePath.size();
        int destPathSize = destDirPath.size();
        PDU *pdu = mkPDU(movePathSize+destPathSize+2);
        pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        // 两个路径的大小+文件名
        sprintf(pdu->caData, "%d %d %s", movePathSize, destPathSize, moveFileName.toStdString().c_str());
        // 两个路径名
        memcpy(pdu->caMsg, moveFilePath.toStdString().c_str(),movePathSize);
        memcpy((char*)(pdu->caMsg)+(movePathSize+1), destDirPath.toStdString().c_str(),destPathSize);

        // 发送给服务器
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;

    }
    m_pSelectDirPB->setEnabled(false);
}
