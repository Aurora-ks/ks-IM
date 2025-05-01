#ifndef MESSAGELISTMODEL_H
#define MESSAGELISTMODEL_H

#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QDialog>
#include "afx.h"

namespace MessageList{
    enum MessageRole {
        IdRole = Qt::UserRole + 1,
        SenderRole, // 0: 对方，1: 自己
        SeqRole,
        TypeRole,
        FileNameRole,  // 文件名
        FileSizeRole,  // 文件大小
        FilePathRole   // 文件路径
    };
    enum MessageType {
        Text,
        Image,
        File
    };

    struct FileInfo {
        QString fileName;
        qint64 fileSize;
        QString filePath;
    };
}

// 原图显示对话框
class ImagePreviewDialog : public QDialog {
    Q_OBJECT
public:
    explicit ImagePreviewDialog(const QPixmap& pixmap, QWidget *parent = nullptr);
};

class MessageListModel : public QStandardItemModel{
    Q_OBJECT
public:
    explicit MessageListModel(QObject *parent = nullptr);
    ~MessageListModel() = default;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void addMessage(const QString &message, int sender, uint64_t seq);
    void addMessage(const QImage &image, int sender, uint64_t seq);
    void addMessage(const MessageList::FileInfo &fileInfo, int sender, uint64_t seq);
    void updateMessageId(uint64_t seq, int64_t messageId);
};

class MessageDelegate : public QStyledItemDelegate {
    MEM_CREATE_D(QColor, bubbleColor)
public:
    MessageDelegate(QObject *parent = nullptr);
    ~MessageDelegate() = default;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

#endif //MESSAGELISTMODEL_H