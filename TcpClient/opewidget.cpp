#include "opewidget.h"


OpeWidget::OpeWidget(QWidget *parent) : QWidget(parent)
{
    m_listWidget = new QListWidget(this);
    m_listWidget->addItem("聊天");
    m_listWidget->addItem("网盘");

    m_pFriend = new Friend;
    m_pBook = new Book;
    m_pStackWidget = new QStackedWidget;
    m_pStackWidget->addWidget(m_pFriend);
    m_pStackWidget->addWidget(m_pBook);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_listWidget);
    pMain->addWidget(m_pStackWidget);

    setLayout(pMain);
    // 栈容器切换
    connect(m_listWidget, SIGNAL(currentRowChanged(int)), m_pStackWidget, SLOT(setCurrentIndex(int)));


}

OpeWidget &OpeWidget::getInstance()
{
    static OpeWidget instance;
    return instance;
}

Friend *OpeWidget::getFriend()
{
    return m_pFriend;
}

Book *OpeWidget::getBook()
{
    return m_pBook;
}
