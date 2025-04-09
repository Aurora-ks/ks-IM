#ifndef MESSAGELISTMODEL_H
#define MESSAGELISTMODEL_H

#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include "afx.h"

enum MessageRole {
    IdRole = Qt::UserRole + 1,
    SenderRole // 0: 对方，1: 自己
};

class MessageListModel : public QStandardItemModel{
    Q_OBJECT
public:
    explicit MessageListModel(QObject *parent = nullptr);
    ~MessageListModel() = default;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void addMessage(int64_t messageId, const QString &message, int senderRole);
};

class MessageDelegate : public QStyledItemDelegate {
    MEM_CREATE_D(QColor, bubbleColor)
public:
    MessageDelegate(QObject *parent = nullptr);
    ~MessageDelegate() = default;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif //MESSAGELISTMODEL_H
