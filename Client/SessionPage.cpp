#include "SessionPage.h"

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
#include "SessionListModel.h"
#include "MessageListModel.h"
#include "FriendTreeViewItem.h"

SessionPage::SessionPage(QWidget *parent) : QWidget(parent){
    initLayout();
    initConnect();
    connect(sessionList_->selectionModel(), &QItemSelectionModel::currentChanged, this, &SessionPage::onSessionSelected);
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

    loop_ = new QEventLoop(this);

    connect(sendButton, &ElaPushButton::clicked, [messageEdit, this]() {
       if(messageEdit->toPlainText().isEmpty()) return;
       QString message = messageEdit->toPlainText();
       messageEdit->clear();
       ElaListView *view = static_cast<ElaListView*>(messageStack_->currentWidget());
       if(view == nullptr) return;
       MessageListModel *model = static_cast<MessageListModel*>(view->model());
       model->addMessage(model->rowCount(), message, 1);
       loop_->exec();
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
    // TODO:init sessionIdMap
    // Test---------
    /*QStringList groupNames = {
            "A1", "乔伊咕咕", "惩罚灰色渡鸦",
            "R6S", "B1"
    };
    QStringList contents = {
            "[@全体成员] 踏雪路...", "[@全体成员] decad...",
            "枭：你怎么不把黄毛放...", "[@全体成员] Sonci_...",
            "111OK"
    };
    QStringList times = {
            "16:04", "16:03", "15:58", "15:56", "15:55"
    };
    QList<int> unreadCounts = {99, 99, 5, 99, 0};

    for (int i = 0; i < 5; ++i) {
        sessionModel_->addSession(i, groupNames[i], ":/images/avatar.png", contents[i], times[i], unreadCounts[i]);
    }*/
    // End Test---------
/*    ElaListView *messageList = new ElaListView(this);
    messageList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ElaScrollBar *messageScrollBar = new ElaScrollBar(messageList->verticalScrollBar(), messageList);
    messageScrollBar->setIsAnimation(true);
    messageList->setItemDelegate(messageDelegate_);
    connect(messageScrollBar, &ElaScrollBar::rangeAnimationFinished, loop_, &QEventLoop::quit);
    MessageListModel *model = new MessageListModel(this);
    messageList->setModel(model);
    messageStack_->addWidget(messageList);*/
    // Test---------
    /*QStringList messages = {
            "当晨光穿透云层，城市在苏醒中舒展筋骨。街角的咖啡店飘出第一缕香气，与通勤者的脚步声交织成独特的都市韵律。在科技园区的实验室里，一群工程师正盯着量子计算机的数据流，试图破解常温超导材料的终极秘密。而在千里之外的热带雨林，生态学家们正用无人机追踪濒危物种的迁徙路径，为气候变化研究收集Precious data.",
            "街道上，自动驾驶巴士悄然驶过，车窗倒映着建筑外墙的光伏玻璃，将阳光转化为清洁能源。公园长椅上，老人戴着智能眼镜与远方的",
            "当夜晚降临时，城市天际线被智能照明系统勾勒成流动的银河。体育馆内，仿生机器人组成的球队正在进行跨物种友谊赛，观众席上既有人类球迷，也有戴着翻译项圈的宠物犬。当零点钟声敲响，全球同步启动的碳捕捉网络开始高效运转，将工业废气转化为建筑材料，为可持续未来注入新动能。",
            "在马里亚纳海沟万米深处，仿生章鱼机器人正用生物荧光伪装探索热液喷口。它们的机械触手采集着含硫矿物样本，通过声波通讯将数据传回海面研究站。实验室里，遗传学家将耐热古菌的基因片段植入珊瑚幼虫，培育能适应海洋酸化的新型生态系统。而在水面光伏平台上，能源工程师正将海浪动能转化为氢能，通过水下输能管道输送至沿海城市。",
            "当最后一抹夕阳沉入太平洋，全球的碳捕捉塔同时启动。这些纳米纤维构成的 \"人工肺\" 每年吸收 300 亿吨二氧化碳，将其转化为建筑材料和燃料。在南极冰盖下的地下城市，人工智能系统通过量子加密网络协调着全球资源，其算法核心融合了人类的情感模块与生态学家的知识库。当新年钟声敲响时，所有电子设备的屏幕同时浮现出一句话：\"我们创造的未来，正在学会自我修正。\""
    };
    // 添加对方消息
    for(int i = 0; i < 2; ++i)
        model->addMessage(i, messages[i], 0);

    // 添加自己消息
    for(int i = 2; i < 4; ++i)
        model->addMessage(i, messages[i], 1);*/
    // End Test---------
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
        // TODO: net creat session id
        // test static session id
        int64_t sid = 0; // 使用sid为0创建新会话
        static int64_t cnt = 0;
        QString name = user->getAlias().isEmpty() ? user->getUser().getUserName() : user->getAlias();
        QString avatar = ":/images/avatar.png"; // 默认头像
//        sessionIdMap_.insert(uId, sid);
        sessionIdMap_.insert(uId, cnt);
//        sessionModel_->addSession(sid, name, avatar, "", "", 0);
        sessionModel_->addSession(cnt, name, avatar, "", "", 0);

        // 选中新创建的会话
        QModelIndex index = sessionModel_->indexFromItem(sessionModel_->getSessionItem(sid));
        sessionList_->setCurrentIndex(index);
        ++cnt;
    }
}

void SessionPage::onSessionSelected(const QModelIndex &current, const QModelIndex &previous) {
    if(!current.isValid()) return;
    QStandardItem *sessionItem = sessionModel_->itemFromIndex(current);
    if(!sessionItem) return;

    int64_t sessionId = sessionItem->data(SessionIdRole).toLongLong();
    QString name = sessionItem->data(NameRole).toString();
    title_->setText(name);
    // 切换到消息列表
    if(messageViewMap_.contains(sessionId)){
        messageStack_->setCurrentWidget(messageViewMap_.value(sessionId));
    }else{
        // 创建消息列表
        ElaListView *messageList = new ElaListView(this);
        messageList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ElaScrollBar *messageScrollBar = new ElaScrollBar(messageList->verticalScrollBar(), messageList);
        messageScrollBar->setIsAnimation(true);
        messageList->setItemDelegate(messageDelegate_);
        connect(messageScrollBar, &ElaScrollBar::rangeAnimationFinished, loop_, &QEventLoop::quit);
        MessageListModel *model = new MessageListModel(this);
        messageList->setModel(model);

        messageViewMap_.insert(sessionId, messageList);
        messageStack_->addWidget(messageList);
        messageStack_->setCurrentWidget(messageList);
    }
}