#ifndef OPEWIDGET_H
#define OPEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>

#include "friend.h"
#include "book.h"

class OpeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OpeWidget(QWidget *parent = nullptr);

    // 单例模式
    static OpeWidget &getInstance();
    Friend *getFriend();
    Book * getBook();

signals:

private:
    QListWidget *m_listWidget;
    Friend *m_pFriend;
    Book *m_pBook;
    QStackedWidget *m_pStackWidget;
};

#endif // OPEWIDGET_H
