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
    ~RelationNotifyWidget();

private:
    ElaText *title_;
    ElaIconButton *delButton_;
    ElaListView *listView_;

    void initLayout();
};



#endif //RELATIONNOTIFYWIDGET_H
