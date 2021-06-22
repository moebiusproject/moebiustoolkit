/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2021 Alejandro Exojo Piqueras
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

#pragma once

#include <QObject>

class XpLevels : public QObject
{
    Q_OBJECT

public:
    struct Level { QString name; QVector<quint32> thresholds; };

    /*!
     * \brief Constructs the object and starts to load the data right away
     *
     * If the passed path is not empty, it will be used to load data from a game
     * in that location via ResourceManager. If the path is empty, built-in data
     * will be used.
     *
     * \param path Path to a game, or empty for built-in data.
     * \param parentObject
     */
    explicit XpLevels(const QString& path, QObject* parentObject = nullptr);
    ~XpLevels();

    QStringList classes() const;
    const QVector<Level>& levels() const;
    QVector<quint32> thresholds(const QString& name) const;


signals:
    void loaded();

private:
    struct Private;
    Private& d;
};
