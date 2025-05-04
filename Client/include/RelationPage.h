#ifndef RELATIONPAGE_H
#define RELATIONPAGE_H

#include <QStyledItemDelegate>

class QPushButton;
class QStackedWidget;
class ElaPushButton;
class ElaIconButton;
class ElaTreeView;
class ElaLineEdit;
class ElaToolButton;
class RelationNotifyWidget;
class UserInfoCard;
class FriendListModel;
class FriendTreeViewItem;

class RelationPage : public QWidget{
    Q_OBJECT
public:
    explicit RelationPage(QWidget *parent = nullptr);
    ~RelationPage();
signals:
    void sendMessageClicked(FriendTreeViewItem *user);
private slots:
    void showFriendInfo(const QModelIndex &index);
    void showGroupInfo(const QModelIndex &index);
    void addUser();
    void addGroup();
    void createGroup();
    void createGrouping(); // 创建好友分组
private:
    ElaLineEdit *searchEdit_{nullptr};
    ElaToolButton *addButton_{nullptr};
    ElaPushButton *friendNotifyButton_{nullptr};
    ElaPushButton *groupNotifyButton_{nullptr};
    QPushButton *userListSwitchButton_{nullptr};
    QPushButton *groupListSwitchButton_{nullptr};
    QStackedWidget *leftStacked_{nullptr};
    ElaTreeView *userListView_{nullptr};
    ElaTreeView *groupListView_{nullptr};
    FriendListModel *userListModel_{nullptr};
    FriendListModel *groupListModel_{nullptr};
    QStackedWidget *rightStacked_{nullptr};
    RelationNotifyWidget *userNotifyPage_{nullptr};
    RelationNotifyWidget *groupNotifyPage_{nullptr};
    UserInfoCard *userInfoWidget_{nullptr};
    QIcon *userPic_{nullptr};
    QIcon *userBluePic_{nullptr};
    QIcon *userGroupPic_{nullptr};
    QIcon *userGroupBluePic_{nullptr};

    void initLayout();
    void initContent();
    void getUserList();
    void getGroupList();
};

class NoChildIndentDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;
        // 检查是否有子节点
        if (!index.model()->hasChildren(index)) {
            opt.rect.setLeft(0); // 无子节点，移除缩进
        }
        // 绘制项内容
        QStyledItemDelegate::paint(painter, opt, index);
    }
};

#endif //RELATIONPAGE_H
