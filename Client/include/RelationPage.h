#ifndef RELATIONPAGE_H
#define RELATIONPAGE_H

#include <Ela/ElaWidget.h>

class QPushButton;
class QStackedWidget;
class ElaPushButton;
class ElaIconButton;
class ElaTreeView;
class RelationNotifyWidget;

class RelationPage : public QWidget{
    Q_OBJECT
public:
    explicit RelationPage(QWidget *parent = nullptr);
    ~RelationPage();
private:
    ElaPushButton *friendNotifyButton_{nullptr};
    ElaPushButton *groupNotifyButton_{nullptr};
    QPushButton *userListSwitchButton_{nullptr};
    QPushButton *groupListSwitchButton_{nullptr};
    QStackedWidget *leftStacked_{nullptr};
    ElaTreeView *userListView_{nullptr};
    ElaTreeView *groupListView_{nullptr};
    QStackedWidget *rightStacked_{nullptr};
    RelationNotifyWidget *userNotifyPage_{nullptr};
    RelationNotifyWidget *groupNotifyPage_{nullptr};
    QWidget *userInfoWidget_{nullptr};
    QIcon *userPic_{nullptr};
    QIcon *userBluePic_{nullptr};
    QIcon *userGroupPic_{nullptr};
    QIcon *userGroupBluePic_{nullptr};

    void initLayout();
    void initContent();
};



#endif //RELATIONPAGE_H
