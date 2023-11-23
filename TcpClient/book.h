#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include "protocol.h"
#include "tcpclient.h"
#include <QTimer>
#include <QThread>

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);

    void updateFileList(const PDU *pdu);
    void clearCurEnterDir();
    QString getCurEnterDir();
    void setDownloadFlag(bool flag);
    bool getDownloadFlag();
    QString getDownloadPath();

    QString getShareFileName();



    bool m_isDownload;
    qint64 m_total;
    qint64 m_recev;

signals:

public slots:
    void createDir();
    void flushFile();
    void delDir();
    void renameFile();
    void enterDir(const QModelIndex &index);
    void returnPreDir();
    void uploadFile();
    void uploadFileData();
    void delFile();
    void downloadFile();
    void shareFile();
    void moveFile();
    void selectDestDir();

private:
    QListWidget *m_pBookListWidget;

    QPushButton *m_pReturnPB;
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDelDirPB;
    QPushButton *m_pRenamePB;
    QPushButton *m_pFlushFilePB;
    QPushButton *m_pUploadPB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pDownloadPB;
    QPushButton *m_pShareFilePB;
    QPushButton *m_pMoveFilePB;
    QPushButton *m_pSelectDirPB;

    QString m_cur_enter_dir;
    QString uploadFilePath;
    // 定时器
    QTimer *m_ptimer;

    QString downloadFilePath;

    QString shareFileName;
    QString moveFileName;
    QString moveFilePath;
    QString destDirPath;

};

#endif // BOOK_H
