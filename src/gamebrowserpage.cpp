/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2020-2021 Alejandro Exojo Piqueras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gamebrowserpage.h"

#include "ui_gamebrowserpage.h"

#include "keyfile.h"
#include "resourcemanager.h"
#include "resourcetype.h"

#include <QDebug>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#ifndef Q_OS_WASM
#include <QThreadPool>
#endif

struct GameBrowserPage::Private
{
    Private(GameBrowserPage& page_)
        : page(page_)
    {}

    enum ColumnIndex {
        NameColumn,
        TypeColumn,
        LocationColumn,
    };

    enum {
        DataRole = Qt::UserRole + 1,
    };

    GameBrowserPage& page;
    Ui::GameBrowserPage ui;

    QStandardItemModel model;
    QSortFilterProxyModel filterModel;

    ResourceManager manager;

    void start(const QString& name, const QString& location);
    void loaded();
};

GameBrowserPage::GameBrowserPage(QWidget* parent)
    : BasePage(parent)
    , d(new Private(*this))
{
    d->ui.setupUi(this);
    d->ui.log->setFontFamily(QLatin1String("monospace"));

    d->model.setColumnCount(Private::LocationColumn+1);
    d->model.setHorizontalHeaderLabels(QStringList() << tr("Resource") << tr("Type") << tr("Location"));

    d->filterModel.setSourceModel(&d->model);
    d->filterModel.setFilterKeyColumn(Private::NameColumn);
    d->filterModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    d->ui.resources->setModel(&d->filterModel);

    connect(d->ui.resourcesFilter, &QLineEdit::textChanged, this, [this](const QString& text){
        d->filterModel.setFilterFixedString(text);
    });

    connect(&d->manager, &ResourceManager::loaded,
            this, [this]() { d->loaded(); });

    connect(d->ui.resources, &QAbstractItemView::clicked, [this](const QModelIndex& index) {
        const int row = index.row();
        const auto model = index.model();
        const QString resourceName = model->data(model->index(row, Private::NameColumn)).toString();
        const int type = model->data(model->index(row, Private::TypeColumn), Private::DataRole).toInt();
        QString resource = tr("Viewing this file is not implemented yet.");
        if (type == TdaType) {
            // TODO: will need consideration when there are files to show in
            // both the BIFF files and the override directory.
            const QByteArray resourceData = d->manager.resource(resourceName, static_cast<ResourceType>(type));
            resource = QString::fromUtf8(resourceData);
        }
        d->ui.log->setText(resource);
    });
}

GameBrowserPage::~GameBrowserPage()
{
    delete d;
    d = nullptr;
}

bool GameBrowserPage::event(QEvent *event)
{
    // On first show, start browsing.
    if (event->type() == QEvent::Show && !d->manager.chitinKey().isValid())
        d->start(m_currentName, m_currentLocation);

    return QWidget::event(event);
}

void GameBrowserPage::Private::loaded()
{
    const KeyFile& chitinKey = manager.chitinKey();

#if 0  // Should BIFF files be displayed? Not sure why it could be useful, but initially I had them.
    for (const KeyFile::BiffEntry& biff : chitinKey.biffEntries) {
        // TODO: Add if it's valid or not, size, etc. And do it from the biffs
        // that we REALLY have parsed.
        model.appendRow(new QStandardItem(biff.name));
    }
#endif


    for (const KeyFile::ResourceEntry& resource : chitinKey.resourceEntries) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(resource.name));

        const QString typeName = ResourceManager::resourceTermination(resource.type).toUpper();
        const QString typeText = typeName.isEmpty() ? QString(QLatin1String("0x%1"))
                                                          .arg(resource.type, 4, 16, QLatin1Char('0'))
                                                    : typeName;
        QStandardItem* typeItem = new QStandardItem(typeName);
        typeItem->setData(resource.type, Private::DataRole);
        row.append(typeItem);

        const int biffIndex = resource.source;
        const QString biffName = chitinKey.biffEntries.at(biffIndex).name;
        row.append(new QStandardItem(biffName));

        model.appendRow(row);
    }
    // TODO: add items from override.
}

void GameBrowserPage::Private::start(const QString& name, const QString& location)
{
    ui.header->setText(tr("%1 (%2)").arg(name).arg(location));

    const auto load = [this, location] { manager.load(location); };

#ifndef Q_OS_WASM
    QThreadPool::globalInstance()->start(QRunnable::create(load));
#else
    load();
#endif
}
