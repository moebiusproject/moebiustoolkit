#include "gamebrowserpage.h"

#include "ui_gamebrowserpage.h"

#include "bifffile.h"
#include "keyfile.h"
#include "resourcemanager.h"
#include "resourcetype.h"

#include <QDebug>
#include <QStandardItemModel>
#include <QThreadPool>

enum Row {
    BiffsRow = 0,
    SpellsRow,
    ItemsRow,
    TdasRow,
    CresRow,
};

struct GameBrowserPage::Private
{
    Private(GameBrowserPage& page_)
        : page(page_)
    {}
    GameBrowserPage& page;
    Ui::GameBrowserPage ui;

    QStandardItemModel model;

    ResourceManager manager;

    QString location;
    QVector<BiffFile> biffs;

    void loaded();
};

GameBrowserPage::GameBrowserPage(QWidget* parent)
    : QWidget(parent)
    , d(new Private(*this))
{
    d->ui.setupUi(this);
    d->ui.resources->setModel(&d->model);
    d->model.setColumnCount(2);
    d->model.insertRow(BiffsRow, new QStandardItem(tr("BIFFs")));
    d->model.insertRow(SpellsRow, new QStandardItem(tr("Spells")));
    d->model.insertRow(ItemsRow, new QStandardItem(tr("Items")));
    d->model.insertRow(TdasRow, new QStandardItem(tr("2da")));
    d->model.insertRow(CresRow, new QStandardItem(tr("Creature")));

    connect(&d->manager, &ResourceManager::loaded,
            this, [this]() { d->loaded(); });
}

GameBrowserPage::~GameBrowserPage()
{
    delete d;
    d = nullptr;
}

void GameBrowserPage::start(const QString& name, const QString& location)
{
    d->ui.header->setText(tr("%1 (%2)").arg(name).arg(location));
    d->location = location.section(QLatin1Char('/'), 0, -2); // Get the root only.

    QThreadPool::globalInstance()->start(
        QRunnable::create([this, location] { d->manager.load(location); }
    ));
}

bool GameBrowserPage::event(QEvent *event)
{
    // On first show, start browsing.
    if (event->type() == QEvent::Show && d->location.isEmpty())
        start(m_currentName, m_currentLocation);

    return QWidget::event(event);
}

void GameBrowserPage::Private::loaded()
{
    const KeyFile& chitinKey = manager.chitinKey();

    QStandardItem* biffsItem = model.item(BiffsRow, 0);
    for (const KeyFile::BiffEntry& biff : chitinKey.biffEntries) {
        // TODO: Add if it's valid or not, size, etc. And do it from the biffs
        // that we REALLY have parsed.
        biffsItem->appendRow(new QStandardItem(biff.name));
    }

    for (const KeyFile::ResourceEntry& resource : chitinKey.resourceEntries) {
        const int rowEnum = resource.type == SplType ? SpellsRow
                          : resource.type == ItmType ? ItemsRow
                          : resource.type == TdaType ? TdasRow
                          : resource.type == CreType ? CresRow
                                                     : -1;
        if (rowEnum == -1)
            continue;

        const int biffIndex = resource.source;
        const QString biffName = chitinKey.biffEntries.at(biffIndex).name;

        QStandardItem* item = model.item(rowEnum, 0);
        item->appendRow(QList<QStandardItem*>()
                             << new QStandardItem(resource.name)
                             << new QStandardItem(biffName)
                        );
    }
}