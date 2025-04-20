#ifndef SESSIONPAGE_H
#define SESSIONPAGE_H

#include <QWidget>
#include "protocol.pb.h"
#include "ws.h"

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
    void onMessageReceived(uint64_t seq, const protocol::Msg& message);
private:
    ElaListView *sessionList_{nullptr};
    SessionListModel *sessionModel_{nullptr};
    ElaText *title_{nullptr};
    QStackedWidget *messageStack_{nullptr};
    MessageDelegate *messageDelegate_{nullptr};
    QMap<int64_t, ElaListView*> messageViewMap_; // sessionId-messageList
    QMap<int64_t, int64_t> peer2SessionMap_; // peerId-sessionId
    QMap<int64_t, int64_t> session2PeerMap_; // sessionId-peerId

    void initLayout();
    void initConnect();
    void addMessageView(int64_t sessionId);
};

#endif //SESSIONPAGE_H
