#ifndef SESSIONLISTMODEL_H
#define SESSIONLISTMODEL_H

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include "afx.h"

enum MessageRoles {
    SessionIdRole = Qt::UserRole + 1,
    AvatarRole,
    NameRole,
    MessageRole,
    TimeRole,
    UnreadCountRole,
    LastAckRole,
    IsGroupRole
};

class SessionListModel : public QStandardItemModel{
public:
    Q_OBJECT
public:
    explicit SessionListModel(QObject* parent = nullptr);
    ~SessionListModel() = default;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void addSession(int64_t sessionId, const QString& sessionName, const QString& sessionAvatar, const QString& message, const QString& time, int unreadCount);
    void addSession(int64_t sessionId, const QString &name, int64_t lastAck, bool isGroup);
    void updateSession(int64_t sessionId, const QString &message, const QString& time, int unreadCount);
    QStandardItem* getSessionItem(int64_t sessionId);
private:
    QMap<int64_t, QStandardItem*> sessions_; // sessionId-sessionItem
};

class SessionDelegate : public QStyledItemDelegate {
    MEM_CREATE_D(QColor, backgroundColor)
    MEM_CREATE_D(QColor, backgroundColorSelected)
    MEM_CREATE_D(QColor, backgroundColorHover)
public:
    SessionDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};


#endif //SESSIONLISTMODEL_H
