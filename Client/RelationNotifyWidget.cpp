#include "include/RelationNotifyWidget.h"
#include <QBoxLayout>
#include <Ela/ElaText.h>
#include <Ela/ElaIconButton.h>
#include <Ela/ElaListView.h>
#include <Ela/ElaTheme.h>
RelationNotifyWidget::RelationNotifyWidget(QString title, QWidget *parent) : QWidget(parent) {
    initLayout();
    title_->setText(title);
}

void RelationNotifyWidget::initLayout() {
    // title and del button
    title_ = new ElaText(this);
    connect(eTheme, &ElaTheme::themeModeChanged, [this](ElaThemeType::ThemeMode themeMode) {
        if(themeMode == ElaThemeType::Dark) title_->setStyleSheet("color: #FFFFFF;");
        else title_->setStyleSheet("");
    });
    title_->setTextPixelSize(16);
    delButton_ = new ElaIconButton(ElaIconType::TrashCan, 15, this);

    QHBoxLayout *topHLayout = new QHBoxLayout();
    topHLayout->setContentsMargins(0, 0, 0, 0);
    topHLayout->addWidget(title_);
    topHLayout->addStretch();
    topHLayout->addWidget(delButton_);

    listView_ = new ElaListView(this);

    QVBoxLayout *mainVLayout = new QVBoxLayout();
    mainVLayout->setContentsMargins(5, 5, 5, 5);
    mainVLayout->addLayout(topHLayout);
    mainVLayout->addWidget(listView_);
    setLayout(mainVLayout);
}
