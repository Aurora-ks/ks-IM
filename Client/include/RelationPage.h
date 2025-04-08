#ifndef RELATIONPAGE_H
#define RELATIONPAGE_H

#include <QStyledItemDelegate>

class QPushButton;
class QStackedWidget;
class ElaPushButton;
class ElaIconButton;
class ElaTreeView;
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
private:
    ElaPushButton *friendNotifyButton_{nullptr};
    ElaPushButton *groupNotifyButton_{nullptr};
    QPushButton *userListSwitchButton_{nullptr};
    QPushButton *groupListSwitchButton_{nullptr};
    QStackedWidget *leftStacked_{nullptr};
    ElaTreeView *userListView_{nullptr};
    ElaTreeView *groupListView_{nullptr};
    FriendListModel *userListModel_{nullptr};
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
