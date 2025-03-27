#include "FriendTreeViewItem.h"

FriendTreeViewItem::FriendTreeViewItem(QObject *parent) : QObject(parent) {}

FriendTreeViewItem::FriendTreeViewItem(int64_t relationId, int64_t groupId, const QString &alias, const User &user,
                                       QObject *parent)
        : RelationId_(relationId), GroupId_(groupId), Alias_(alias), User_(user), QObject(parent) {}
