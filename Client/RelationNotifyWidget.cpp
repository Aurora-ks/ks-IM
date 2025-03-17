#include "include/RelationNotifyWidget.h"
#include <QBoxLayout>
#include <Ela/ElaText.h>
#include <Ela/ElaIconButton.h>
#include <Ela/ElaListView.h>

RelationNotifyWidget::RelationNotifyWidget(QString title, QWidget *parent) :QWidget(parent){
    initLayout();
    title_->setText(title);
}

RelationNotifyWidget::~RelationNotifyWidget() {
}

void RelationNotifyWidget::initLayout() {
    // title and del button
    title_ = new ElaText(this);
    title_->setTextPixelSize(15);
    delButton_ = new ElaIconButton(ElaIconType::TrashCan, 15, this);

    QHBoxLayout *topHLayout = new QHBoxLayout();
    topHLayout->setContentsMargins(0, 0, 0, 0);
    topHLayout->addWidget(title_);
    topHLayout->addStretch();
    topHLayout->addWidget(delButton_);

    listView_ = new ElaListView(this);

    QVBoxLayout *mainVLayout = new QVBoxLayout();
    mainVLayout->setContentsMargins(3, 2, 3, 2);
    mainVLayout->addLayout(topHLayout);
    mainVLayout->addWidget(listView_);
    setLayout(mainVLayout);
}
