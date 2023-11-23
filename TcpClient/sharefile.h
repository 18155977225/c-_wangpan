#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QScrollArea>
#include <QCheckBox>

class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);

    static ShareFile& getInstance();

    void updateFriend(QListWidget *friendList);

signals:

public slots:
    void cancelSelect();
    void selectAll();
    void confirmShare();
    void cancelShare();

private:
    QPushButton *m_pSelectAllPB;
    QPushButton *m_pCancelSelectPB;

    QPushButton *m_pOKPB;
    QPushButton *m_pCancelPB;

    QScrollArea *m_pSA;
    QWidget *m_pFriendW;
    QButtonGroup *m_pButtonGroup;

    // 用于刷新好友用的垂直布局
    QVBoxLayout *m_pfriendWVBL;


};

#endif // SHAREFILE_H
