#ifndef FRIENDLISTMODEL_H
#define FRIENDLISTMODEL_H

#include <QStandardItemModel>
#include <QMap>
#include "FriendTreeViewItem.h"

class FriendListModel : public QStandardItemModel{
    Q_OBJECT
public:
    enum ItemRoles {
        TypeRole = Qt::UserRole + 1,
        IDRole,
        DataRole
    };
    enum ItemType{
        Group,
        Friend
    };

    explicit FriendListModel(QObject *parent = nullptr);
    ~FriendListModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void addGroup(int64_t groupId, const QString &groupName);
    void addFriend(FriendTreeViewItem *friendItem);
public slots:
    void updateFriendGrouping(int64_t oldGroupId, FriendTreeViewItem *item);
private:
    QMap<int64_t, QStandardItem*> groups_;
};


#endif //FRIENDLISTMODEL_H
