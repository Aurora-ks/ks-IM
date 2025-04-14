#include "SessionPage.h"
#include "ws.h"

#include <QBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <Ela/ElaListView.h>
#include <Ela/ElaTheme.h>
#include <Ela/ElaText.h>
#include <Ela/ElaPlainTextEdit.h>
#include <Ela/ElaScrollBar.h>
#include <Ela/ElaIconButton.h>
#include <Ela/ElaPushButton.h>
#include <Ela/ElaMessageBar.h>
#include "SessionListModel.h"
#include "MessageListModel.h"
#include "FriendTreeViewItem.h"
#include "net.h"

SessionPage::SessionPage(QWidget *parent) : QWidget(parent){
    initLayout();
    initConnect();
    connect(sessionList_->selectionModel(), &QItemSelectionModel::currentChanged, this, &SessionPage::onSessionSelected);

    // 连接WebSocket消息接收信号
    connect(&WebSocketManager::getInstance(), &WebSocketManager::messageReceived, this, &SessionPage::onMessageReceived);
}

void SessionPage::initLayout() {
    // left session list
    sessionList_ = new ElaListView(this);
    sessionList_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sessionList_->setMinimumWidth(200);
    sessionList_->setMaximumWidth(300);
    ElaScrollBar *sessionScrollBar = new ElaScrollBar(sessionList_->verticalScrollBar(), sessionList_);
    sessionScrollBar->setIsAnimation(true);

    sessionModel_ = new SessionListModel(this);
    SessionDelegate* delegate = new SessionDelegate(this);
    sessionList_->setItemDelegate(delegate);
    sessionList_->setModel(sessionModel_);

    // right top bar
    QHBoxLayout *titleBarHLayout = new QHBoxLayout();
    titleBarHLayout->setContentsMargins(5, 5, 0, 0);
    title_ = new ElaText(this);
    title_->setTextPixelSize(18);
    titleBarHLayout->addWidget(title_);

    // message stack
    messageStack_ = new QStackedWidget(this);
    messageStack_->setStyleSheet("QStackedWidget{background:transparent;}");
    messageDelegate_ = new MessageDelegate(this);

    // edit toolbar
    QHBoxLayout *toolBarHLayout = new QHBoxLayout();
    toolBarHLayout->setContentsMargins(0, 0, 0, 0);
    ElaIconButton *clearTextButton = new ElaIconButton(ElaIconType::TrashCan, 12, this);
    toolBarHLayout->addWidget(clearTextButton);
    toolBarHLayout->addStretch();
    // plain edit
    ElaPlainTextEdit *messageEdit = new ElaPlainTextEdit(this);
    connect(clearTextButton, &ElaIconButton::clicked, messageEdit, &ElaPlainTextEdit::clear);

    // button
    QHBoxLayout *buttonHLayout = new QHBoxLayout();
    buttonHLayout->setContentsMargins(0, 0, 0, 0);
    ElaPushButton *sendButton = new ElaPushButton("发送", this);
    buttonHLayout->addWidget(sendButton);
    buttonHLayout->addStretch();

    connect(sendButton, &ElaPushButton::clicked, [messageEdit, this]() {
       if(messageEdit->toPlainText().isEmpty()) return;
       QString message = messageEdit->toPlainText();
       messageEdit->clear();
       ElaListView *view = static_cast<ElaListView*>(messageStack_->currentWidget());
       if(view == nullptr) return;
       MessageListModel *model = static_cast<MessageListModel*>(view->model());

       // 获取当前会话ID
       QModelIndex currentIndex = sessionList_->currentIndex();
       if(!currentIndex.isValid()) return;
       int64_t sessionId = sessionModel_->itemFromIndex(currentIndex)->data(SessionIdRole).toLongLong();

       // 使用WebSocket发送消息
       QByteArray content = message.toUtf8();
       if(!WebSocketManager::getInstance().sendSingleChatMessage(sessionId, sessionIdMap_.key(sessionId), content, 1)) {
           ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "发送失败", "消息发送失败，请检查网络连接", 2000, this);
           return;
       }

       // 添加到本地消息列表
       model->addMessage(model->rowCount(), message, 1);
       view->scrollToBottom();
    });

    // toolbar & edit & button widget
    QWidget *messageSplitterWidget = new QWidget(this);
    messageSplitterWidget->setMinimumHeight(200);
    messageSplitterWidget->setMaximumHeight(400);
    QVBoxLayout *messageSplitterVLayout = new QVBoxLayout(messageSplitterWidget);
    messageSplitterVLayout->setContentsMargins(0, 0, 0, 0);
    messageSplitterVLayout->addLayout(toolBarHLayout);
    messageSplitterVLayout->addWidget(messageEdit);
    messageSplitterVLayout->addLayout(buttonHLayout);

    // message splitter
    QSplitter *messageSplitter = new QSplitter(Qt::Vertical, this);
    messageSplitter->setChildrenCollapsible(false);
    messageSplitter->setHandleWidth(1);
    messageSplitter->addWidget(messageStack_);
    messageSplitter->addWidget(messageSplitterWidget);
    messageSplitter->setStretchFactor(0, 5);
    messageSplitter->setStretchFactor(1, 3);
    connect(eTheme, &ElaTheme::themeModeChanged, [messageSplitter](ElaThemeType::ThemeMode themeMode) {
        if (themeMode == ElaThemeType::ThemeMode::Light)
            messageSplitter->setStyleSheet("QSplitter::handle{background-color:rgb(233,233,233);}");
        else
            messageSplitter->setStyleSheet("QSplitter::handle{background-color:#444444;}");
    });

    QVBoxLayout *rightVLayout = new QVBoxLayout();
    rightVLayout->setContentsMargins(0, 0, 0, 0);
    rightVLayout->addLayout(titleBarHLayout);
    rightVLayout->addWidget(messageSplitter);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(1);
    splitter->addWidget(sessionList_);
    QWidget *rightWidget = new QWidget(this);
    rightWidget->setLayout(rightVLayout);
    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);
    connect(eTheme, &ElaTheme::themeModeChanged, [splitter](ElaThemeType::ThemeMode themeMode) {
        if (themeMode == ElaThemeType::ThemeMode::Light)
            splitter->setStyleSheet("QSplitter::handle{background-color:rgb(233,233,233);}");
        else
            splitter->setStyleSheet("QSplitter::handle{background-color:#444444;}");
    });

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(splitter);

    connect(eTheme, &ElaTheme::themeModeChanged, [this, delegate](ElaThemeType::ThemeMode themeMode) {
        if(themeMode == ElaThemeType::Light){
            delegate->setbackgroundColor(Qt::white);
            delegate->setbackgroundColorSelected(QColor(204, 235, 255));
            delegate->setbackgroundColorHover(QColor(240, 240, 240));
            messageDelegate_->setbubbleColor(QColor("#F5F5F5"));
        }
        else{
            delegate->setbackgroundColor(QColor(52, 52, 52));
            delegate->setbackgroundColorSelected(QColor(22, 42, 63));
            delegate->setbackgroundColorHover(QColor(59, 59, 59));
            messageDelegate_->setbubbleColor(QColor(Qt::gray));
        }
        sessionList_->viewport()->update();
        for(int i = 0; i < messageStack_->count(); ++i){
            static_cast<ElaListView*>(messageStack_->widget(i))->viewport()->update();
        }
    });
}

void SessionPage::initConnect() {
    // 好友会话
    QMap<QString, QString> query;
    query["user_id"] = QString::number(User::GetUid());
    query["is_group"] = "false";
    auto resp = Net::GetTo("/rel/session", query);
    if(!resp){
        qWarning() << "[Net] [SessionPage::initConnect] get session failed, err:" << resp.errorString();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取会话失败", 2000, this);
        return;
    }
    auto data = resp.data();
    if(!data){
        qWarning() << "[Net] [SessionPage::initConnect] get session error, code:" << data.code() << " msg:" << data.message();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取会话失败", 2000, this);
        return;
    }

    auto sessionArray = data.dataArray();
    if(sessionArray.empty()) return;

    for(auto session : sessionArray){
        auto obj = session.toObject();
        if(obj.empty()) continue;
        int64_t sessionId = obj["session_id"].toInteger();
        sessionModel_->addSession(sessionId, obj["name"].toString(), obj["last_ack_msg_id"].toInteger(), false);
        sessionIdMap_.insert(obj["peer_id"].toInteger(), sessionId);
        addMessageView(sessionId);
    }
}

void SessionPage::selectOrCreateSession(FriendTreeViewItem *user) {
    if (!user || !sessionModel_ || !sessionList_) return;
    // 查找是否已存在该用户的会话
    int64_t uId = user->getUser().getUserID();
    if(sessionIdMap_.contains(uId)){
        int64_t sessionId = sessionIdMap_.value(uId);
        QModelIndex index = sessionModel_->indexFromItem(sessionModel_->getSessionItem(sessionId));
        sessionList_->setCurrentIndex(index);
    }else {
        // 如果不存在，创建新会话
        QJsonObject req;
        req["user_id"] = User::GetUid();
        req["peer_id"] = uId;
        req["is_group"] = false;
        auto resp = Net::PostTo("/rel/session", QJsonDocument(req).toJson());
        if(!resp){
            qWarning() << "[Net] [SessionPage::selectOrCreateSession] create session failed, code:" << resp.statusCode() << " err:" << resp.errorString();
            ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "创建会话失败", 2000, this);
            return;
        }
        auto data = resp.data();
        if(!data){
            qWarning() << "[Net] [SessionPage::selectOrCreateSession] create session error, code:" << data.code() << " msg:" << data.message();
            ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "创建会话失败", 2000, this);
            return;
        }
        auto json = data.data();
        if(!json.isObject()){
            qWarning() << "[Net] [SessionPage::selectOrCreateSession] create session received invalid";
            ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "错误", "创建会话失败", 2000, this);
            return;
        }

        int64_t sid = json["session_id"].toInteger();
        QString name = user->getAlias().isEmpty() ? user->getUser().getUserName() : user->getAlias();
        sessionIdMap_.insert(uId, sid);
        sessionModel_->addSession(sid, name, 0, false);

        // 选中新创建的会话
        addMessageView(sid);
        QModelIndex index = sessionModel_->indexFromItem(sessionModel_->getSessionItem(sid));
        sessionList_->setCurrentIndex(index);
    }
}

void SessionPage::onSessionSelected(const QModelIndex &current, const QModelIndex &previous) {
    if(!current.isValid()) return;
    QStandardItem *sessionItem = sessionModel_->itemFromIndex(current);
    if(!sessionItem) return;

    int64_t sessionId = sessionItem->data(SessionIdRole).toLongLong();
    QString name = sessionItem->data(NameRole).toString();
    title_->setText(name);

    if(messageViewMap_.contains(sessionId))
        messageStack_->setCurrentWidget(messageViewMap_.value(sessionId));
    else
        addMessageView(sessionId);
}

void SessionPage::addMessageView(int64_t sessionId) {
    ElaListView *messageList = new ElaListView(this);
    messageList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    ElaScrollBar *messageScrollBar = new ElaScrollBar(messageList->verticalScrollBar(), messageList);
//    messageScrollBar->setIsAnimation(true);
    messageList->setItemDelegate(messageDelegate_);
//    connect(messageScrollBar, &ElaScrollBar::rangeAnimationFinished, [messageList](){
//        messageList->scrollToBottom();
//    });
    MessageListModel *model = new MessageListModel(this);
    messageList->setModel(model);
    messageViewMap_.insert(sessionId, messageList);
    messageStack_->addWidget(messageList);
}

void SessionPage::onMessageReceived(const protocol::Msg& message) {
    // 检查是否是当前会话的消息
    int64_t sessionId = message.conversation_id();
    if(!messageViewMap_.contains(sessionId)) {
        return;
    }

    // 获取消息视图和模型
    ElaListView* view = messageViewMap_.value(sessionId);
    MessageListModel* model = static_cast<MessageListModel*>(view->model());

    // 添加消息到列表
    QString content = QString::fromUtf8(message.content().data(), message.content().size());
    model->addMessage(model->rowCount(), content, message.sender_id() == User::GetUid() ? 1 : 0);
    view->scrollToBottom();
}
