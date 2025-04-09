#ifndef SESSIONPAGE_H
#define SESSIONPAGE_H

#include <QWidget>

class QStackedWidget;
class QEventLoop;
class ElaListView;
class ElaText;
class SessionListModel;
class MessageDelegate;
class FriendTreeViewItem;

class SessionPage : public QWidget{
    Q_OBJECT
public:
    explicit SessionPage(QWidget *parent = nullptr);
    ~SessionPage() = default;
public slots:
    void selectOrCreateSession(FriendTreeViewItem *user);
private slots:
    void onSessionSelected(const QModelIndex &current, const QModelIndex &previous);
private:
    ElaListView *sessionList_{nullptr};
    SessionListModel *sessionModel_{nullptr};
    ElaText *title_{nullptr};
    QStackedWidget *messageStack_{nullptr};
    MessageDelegate *messageDelegate_{nullptr};
    QMap<int64_t, ElaListView*> messageViewMap_; // sessionId-messageList
    QMap<int64_t, int64_t> sessionIdMap_; // friendUserId-sessionId
    QEventLoop *loop_;

    void initLayout();
    void initConnect();
};

#endif //SESSIONPAGE_H
