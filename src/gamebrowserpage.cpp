/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2020-2023 Alejandro Exojo Piqueras
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

#include "gamebrowserresourcefilter.h"
#include "keyfile.h"
#include "pathutils.h"
#include "resourcemanager.h"
#include "resourcetype.h"

#include <QDebug>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#ifndef Q_OS_WASM
#include <QThreadPool>
#endif

enum ColumnIndex {
    NameColumn,
    TypeColumn,
    LocationColumn,
};

class FilterModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    void setCustomFilter(const QString& filter)
    {
        m_filter.setFilter(filter);
        invalidateFilter();
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        const QAbstractItemModel* source = sourceModel();
        const QModelIndex nameIndex = source->index(sourceRow, NameColumn, sourceParent);
        const QModelIndex typeIndex = source->index(sourceRow, TypeColumn, sourceParent);
        const QModelIndex loctIndex = source->index(sourceRow, LocationColumn, sourceParent);

        GameBrowserResourceFilter::Row row {
            nameIndex.data().toString(),
            typeIndex.data().toString(),
            loctIndex.data().toString(),
        };
        return m_filter.accept(row);
    }

 private:
    GameBrowserResourceFilter m_filter;
};

struct GameBrowserPage::Private
{
    Private(GameBrowserPage& page_)
        : page(page_)
    {}

    enum {
        DataRole = Qt::UserRole + 1,
    };

    GameBrowserPage& page;
    Ui::GameBrowserPage ui;

    QStandardItemModel model;
    FilterModel filterModel;

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

    d->model.setColumnCount(LocationColumn+1);
    d->model.setHorizontalHeaderLabels(QStringList() << tr("Resource") << tr("Type") << tr("Location"));

    d->filterModel.setSourceModel(&d->model);
    d->filterModel.setFilterKeyColumn(NameColumn);
    d->filterModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    d->ui.resources->setModel(&d->filterModel);

    connect(d->ui.resourcesFilter, &QLineEdit::textChanged, this, [this](const QString& text){
        d->filterModel.setCustomFilter(text);
    });

    connect(&d->manager, &ResourceManager::loaded,
            this, [this]() { d->loaded(); });

    connect(&d->manager, &ResourceManager::failureLoading, this, [this]() {
        QMessageBox::critical(this, tr("Moebius Toolkit"),
                             tr("The game could not be loaded.\n"
                                "Confirm the game files like `chitin.key` are valid"));
    });

    connect(d->ui.resources, &QAbstractItemView::clicked, [this](const QModelIndex& index) {
        const int row = index.row();
        const auto model = index.model();
        const QString resourceName = model->data(model->index(row, NameColumn)).toString();
        const QVariant typeValue = model->data(model->index(row, TypeColumn), Private::DataRole);

        QString resource = tr("Viewing this file is not implemented yet.");

        auto isPlainText = [](ResourceType type) {
            return type == TdaType || type == IdsType;
        };

        if (typeValue.isNull()) { // In override.
            const QString typeString = model->data(model->index(row, TypeColumn)).toString();
            const ResourceType type = ResourceManager::resourceType(typeString);
            if (isPlainText(type)) {
                const QString fileName = resourceName + QLatin1Char('.') + typeString;
                resource = QString::fromUtf8(d->manager.overriddenResource(fileName));
            }
        } else {
            const ResourceType type = static_cast<ResourceType>(typeValue.toInt());
            if (isPlainText(type)) {
                const QByteArray resourceData = d->manager.defaultResource(resourceName, static_cast<ResourceType>(type));
                resource = QString::fromUtf8(resourceData);
            }
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

    // TODO: Qt 6. Remove this workaround which is only for Qt 5.
    filterModel.setSourceModel(nullptr);

    for (const KeyFile::ResourceEntry& resource : chitinKey.resourceEntries) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(resource.name));

        const QString typeName = ResourceManager::resourceTermination(resource.type).toUpper();
        const QString typeText = typeName.isEmpty() ? QString(QLatin1String("0x%1"))
                                                          .arg(resource.type, 4, 16, QLatin1Char('0'))
                                                    : typeName;
        QStandardItem* typeItem = new QStandardItem(typeName);
        // The presence of this role also helps indicate that the file comes from a BIFF, not override.
        typeItem->setData(resource.type, Private::DataRole);
        row.append(typeItem);

        const int biffIndex = resource.source;
        const QString biffName = chitinKey.biffEntries.at(biffIndex).name;
        row.append(new QStandardItem(biffName));

        model.appendRow(row);
    }

    for (const QString& file : manager.overridden()) {
        const QString base = path::baseName(file);
        const QString type = path::suffix(file).toUpper();
        QList<QStandardItem*> row;
        row.append(new QStandardItem(base));
        row.append(new QStandardItem(type));
        row.append(new QStandardItem(QLatin1String("override")));
        model.appendRow(row);
    }


    // TODO: Qt 6. Remove this workaround which is only for Qt 5.
    filterModel.setSourceModel(&model);
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

#include "gamebrowserpage.moc"
