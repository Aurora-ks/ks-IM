#ifndef RELATIONNOTIFYWIDGET_H
#define RELATIONNOTIFYWIDGET_H
// 好友申请通知/群申请通知

#include <QWidget>

class ElaText;
class ElaIconButton;
class ElaListView;

class RelationNotifyWidget : public QWidget{
    Q_OBJECT
public:
    explicit RelationNotifyWidget(QString title, QWidget *parent = nullptr);
    ~RelationNotifyWidget() = default;

private:
    ElaText *title_{nullptr};
    ElaIconButton *delButton_{nullptr};
    ElaListView *listView_{nullptr};

    void initLayout();
};



#endif //RELATIONNOTIFYWIDGET_H
