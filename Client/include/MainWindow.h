#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Ela/ElaWindow.h>
#include "net.h"
#include "user.h"

class MainWindow :public ElaWindow{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void bindUser(const QString &uid);
    void updateUserInfo();

private:
    Net http_;
    Net ws_;
    User user_;
};



#endif //MAINWINDOW_H
