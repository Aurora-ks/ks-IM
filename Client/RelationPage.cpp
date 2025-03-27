#include "RelationPage.h"
#include <QSplitter>
#include <QStackedWidget>
#include <QBoxLayout>
#include <QStandardItem>
#include <Ela/ElaPushButton.h>
#include <Ela/ElaIconButton.h>
#include <Ela/ElaTreeView.h>
#include <Ela/ElaText.h>
#include <Ela/ElaTheme.h>
#include <Ela/ElaScrollBar.h>
#include <Ela/ElaMessageBar.h>
#include "RelationNotifyWidget.h"
#include "FriendListModel.h"
#include "FriendTreeViewItem.h"
#include "UserInfoCard.h"
#include "net.h"

RelationPage::RelationPage(QWidget *parent) : QWidget(parent) {
    setWindowTitle("好友列表");
    initLayout();
    initContent();

    connect(friendNotifyButton_, &ElaPushButton::clicked, [this]() {
        rightStacked_->setCurrentWidget(userNotifyPage_);
    });
    connect(groupNotifyButton_, &ElaPushButton::clicked, [this]() {
        rightStacked_->setCurrentWidget(groupNotifyPage_);
    });
    connect(userListSwitchButton_, &QPushButton::clicked, [this]() {
        userListSwitchButton_->setIcon(*this->userBluePic_);
        groupListSwitchButton_->setIcon(*this->userGroupPic_);
        leftStacked_->setCurrentWidget(userListView_);
    });
    connect(groupListSwitchButton_, &QPushButton::clicked, [this]() {
        userListSwitchButton_->setIcon(*this->userPic_);
        groupListSwitchButton_->setIcon(*this->userGroupBluePic_);
        leftStacked_->setCurrentWidget(groupListView_);
    });
    connect(userListView_, &ElaTreeView::clicked, this, &RelationPage::showFriendInfo);
    connect(userInfoWidget_, &UserInfoCard::groupingChanged, userListModel_, &FriendListModel::updateFriendGrouping);
    connect(eTheme, &ElaTheme::themeModeChanged, [this](ElaThemeType::ThemeMode themeMode) {
        if (themeMode == ElaThemeType::ThemeMode::Light){
            userListView_->setStyleSheet("ElaTreeView::item{height:50px;}");
            groupListView_->setStyleSheet("ElaTreeView::item{height:50px;}");
        }
        else{
            userListView_->setStyleSheet("ElaTreeView{background-color:#343434;}ElaTreeView::item{height:50px;}");
            groupListView_->setStyleSheet("ElaTreeView{background-color:#343434;}ElaTreeView::item{height:50px;}");
        }
    });
}

RelationPage::~RelationPage() {
    delete userPic_;
    delete userBluePic_;
    delete userGroupPic_;
    delete userGroupBluePic_;
}

void RelationPage::initLayout() {
    QString buttonStyleSheet = "ElaPushButton{font-size:14px;}ElaPushButton:hover{background-color:#f1f3f5;}";
    QString switchButtonLightStyleSheet = "QPushButton{border:none;}QPushButton:hover{background-color:#f1f3f5;}";
    QString switchButtonDarkStyleSheet = "QPushButton{border:none;}QPushButton:hover{background-color:#444444;}";
    // left widget
    QWidget *leftWidget = new QWidget(this);
    leftWidget->setMinimumWidth(150);
    leftWidget->setMaximumWidth(300);

    QVBoxLayout *leftVLayout = new QVBoxLayout(leftWidget);
    leftVLayout->setContentsMargins(3, 2, 3, 2);

    // left notify button
    friendNotifyButton_ = new ElaPushButton("好友通知", this);
    friendNotifyButton_->setStyleSheet(buttonStyleSheet);
    groupNotifyButton_ = new ElaPushButton("群组通知", this);
    groupNotifyButton_->setStyleSheet(buttonStyleSheet);

    // left switch button
    userPic_ = new QIcon(":/images/resource/pic/User.png");
    userBluePic_ = new QIcon(":/images/resource/pic/User-Blue.png");
    userGroupPic_ = new QIcon(":/images/resource/pic/UserGroup.png");
    userGroupBluePic_ = new QIcon(":/images/resource/pic/UserGroup-Blue.png");

    userListSwitchButton_ = new QPushButton(this);
    userListSwitchButton_->setIconSize(QSize(20, 20));
    userListSwitchButton_->setFixedHeight(30);
    userListSwitchButton_->setStyleSheet(switchButtonLightStyleSheet);

    groupListSwitchButton_ = new QPushButton(this);
    groupListSwitchButton_->setIconSize(QSize(20, 20));
    groupListSwitchButton_->setFixedHeight(30);
    groupListSwitchButton_->setStyleSheet(switchButtonLightStyleSheet);

    connect(eTheme, &ElaTheme::themeModeChanged,
            [this, switchButtonLightStyleSheet, switchButtonDarkStyleSheet](ElaThemeType::ThemeMode themeMode) {
                if (themeMode == ElaThemeType::ThemeMode::Light) {
                    userListSwitchButton_->setStyleSheet(switchButtonLightStyleSheet);
                    groupListSwitchButton_->setStyleSheet(switchButtonLightStyleSheet);
                } else {
                    userListSwitchButton_->setStyleSheet(switchButtonDarkStyleSheet);
                    groupListSwitchButton_->setStyleSheet(switchButtonDarkStyleSheet);
                }
            });

    QHBoxLayout *switchButtonHLayout = new QHBoxLayout();
    switchButtonHLayout->setContentsMargins(5, 0, 5, 0);
    switchButtonHLayout->addWidget(userListSwitchButton_);
    switchButtonHLayout->addWidget(groupListSwitchButton_);

    // left tree widget
    leftStacked_ = new QStackedWidget(this);
    leftStacked_->setAutoFillBackground(false);
    leftStacked_->setStyleSheet("QStackedWidget:{background-color: transparent;}");

    userListView_ = new ElaTreeView(this);
    ElaScrollBar *userScrollBar = new ElaScrollBar(userListView_->verticalScrollBar(), userListView_);
    userScrollBar->setIsAnimation(true);
    userListView_->setHeaderHidden(true);
    userListView_->setItemDelegate(new NoChildIndentDelegate(this));
    userListView_->setIconSize(QSize(40, 40));
    userListView_->setStyleSheet("ElaTreeView::item{height:50px;}");

    userListModel_ = new FriendListModel(this);
    userListView_->setModel(userListModel_);

    groupListView_ = new ElaTreeView(this);
    ElaScrollBar *groupScrollBar = new ElaScrollBar(groupListView_->verticalScrollBar(), groupListView_);
    groupScrollBar->setIsAnimation(true);
    groupListView_->setHeaderHidden(true);
    groupListView_->setItemDelegate(new NoChildIndentDelegate(this));
    groupListView_->setIconSize(QSize(40, 40));
    groupListView_->setStyleSheet("ElaTreeView::item{height:50px;}");

    leftStacked_->setContentsMargins(0, 0, 0, 0);
    leftStacked_->addWidget(userListView_);
    leftStacked_->addWidget(groupListView_);

    // left layout
    leftVLayout->addWidget(friendNotifyButton_);
    leftVLayout->addWidget(groupNotifyButton_);
    leftVLayout->addSpacing(10);
    leftVLayout->addLayout(switchButtonHLayout);
    leftVLayout->addWidget(leftStacked_);

    // right user notify
    rightStacked_ = new QStackedWidget(this);
    rightStacked_->setAutoFillBackground(false);
    rightStacked_->setStyleSheet("QStackedWidget:{background-color: transparent;}");
    rightStacked_->setContentsMargins(0, 0, 0, 0);

    userNotifyPage_ = new RelationNotifyWidget("好友通知", this);
    rightStacked_->addWidget(userNotifyPage_);

    groupNotifyPage_ = new RelationNotifyWidget("群通知", this);
    rightStacked_->addWidget(groupNotifyPage_);

    // right user info
    userInfoWidget_ = new UserInfoCard(this);
    rightStacked_->addWidget(userInfoWidget_);

    // splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setStyleSheet("QSplitter::handle{background-color:rgb(233,233,233);}");
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightStacked_);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 4);
    connect(eTheme, &ElaTheme::themeModeChanged, [splitter](ElaThemeType::ThemeMode themeMode) {
        if (themeMode == ElaThemeType::ThemeMode::Light)
            splitter->setStyleSheet("QSplitter::handle{background-color:rgb(233,233,233);}");
        else
            splitter->setStyleSheet("QSplitter::handle{background-color:#444444;}");
    });

    // main layout
    userListSwitchButton_->click();
    QHBoxLayout *mainHLayout = new QHBoxLayout();
    mainHLayout->setContentsMargins(0, 0, 0, 0);
    mainHLayout->addWidget(splitter);
    setLayout(mainHLayout);
}

void RelationPage::initContent() {
    // 获取组列表
    userListModel_->addGroup(0, "我的好友");
    QMap<QString, QString> query;
    query.insert("uid", QString::number(User::GetUid()));
    auto resp = Net::GetTo("/rel/friend_grouping", query);
    if(!resp){
        qWarning() << "[Net] [RelationPage::initContent] get grouping net work failed, err:" << resp.errorString();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友分组失败", 2000);
        return;
    }

    auto json = resp.data();
    if(!json){
        qWarning() << "[Net] [RelationPage::initContent] get grouping error, code: " << json.code() << ", message: " << json.message();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友分组失败", 2000);
        return;
    }

    auto groupingData = json.dataArray();
    for(const auto &value : groupingData){
        if(value.isObject()){
            auto obj = value.toObject();
            QString name = obj.value("name").toString();
            int64_t  id = obj.value("id").toInteger();
            userListModel_->addGroup(id, name);
        }
    }
    // 获取好友列表
    resp = Net::GetTo("/rel/friend-list", query);
    if(!resp){
        qWarning() << "[Net] [RelationPage::initContent] get friend list net work failed, err:" << resp.errorString();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友列表失败", 2000);
        return;
    }

    json = resp.data();
    if(!json){
        qWarning() << "[Net] [RelationPage::initContent] get friend list error, code: " << json.code() << ", message: " << json.message();
        ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友列表失败", 2000);
        return;
    }

    auto friendListData = json.dataArray();
    for(const auto &value : friendListData){
        if(value.isObject()){
            auto obj = value.toObject();
            FriendTreeViewItem *item = new FriendTreeViewItem();
            item->setRelationId(obj.value("id").toInteger());
            item->setGroupId(obj.value("group_id").toInteger());
            item->setAlias(obj.value("alias").toString());

            auto userObj = obj.value("user").toObject();
            if (userObj.isEmpty()) {
                qWarning() << "[Net] [RelationPage::initContent] get friend list error, code: " << json.code() << ", message: " << json.message();
                ElaMessageBar::error(ElaMessageBarType::PositionPolicy::Top, "网络错误", "获取好友列表失败", 2000);
                return;
            }
            item->setUser(User(userObj["id"].toInteger(), userObj["name"].toString(), userObj["gender"].toInt(), userObj["icon"].toString(), userObj["emal"].toString(), userObj["phone"].toString()));
            userListModel_->addFriend(item);
        }
    }
}

void RelationPage::showFriendInfo(const QModelIndex &index) {
    if (!index.parent().isValid()) return;

    QStandardItem *friendItem = dynamic_cast<QStandardItemModel*>(userListView_->model())->itemFromIndex(index);
    if (!friendItem) return;

    FriendTreeViewItem *item = friendItem->data(FriendListModel::DataRole).value<FriendTreeViewItem*>();
    if(!item) return;
    userInfoWidget_->updateUserInfo(item);
    rightStacked_->setCurrentWidget(userInfoWidget_);
}
