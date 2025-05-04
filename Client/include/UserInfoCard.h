#ifndef USERINFOCARD_H
#define USERINFOCARD_H

#include <QWidget>

class QLabel;
class ElaText;
class ElaLineEdit;
class ElaComboBox;
class ElaPushButton;
class User;
class FriendTreeViewItem;

class UserInfoCard : public QWidget{
    Q_OBJECT
public:
    explicit UserInfoCard(QWidget *parent = nullptr);
    ~UserInfoCard() = default;
signals:
    void groupingChanged(int64_t oldGroupId, FriendTreeViewItem *item);
    void sendMessageClicked(FriendTreeViewItem *user);
private slots:
    void updateAlias();
    void updateUserGrouping(int index);
public slots:
    void updateUserInfo(FriendTreeViewItem *user);
    void updateGrouping();
    void updateGroupingList();
private:
    QLabel *avatar_{nullptr};
    ElaText *name_{nullptr};
    ElaText *id_{nullptr};
    ElaLineEdit *aliasLineEdit_{nullptr};
    ElaComboBox *groupingComboBox_{nullptr};
    ElaPushButton *communicateButton_{nullptr};
    FriendTreeViewItem *user_{nullptr};
    QMap<int64_t, QString> groupingMap_{};

    void initLayout();
};


#endif //USERINFOCARD_H
