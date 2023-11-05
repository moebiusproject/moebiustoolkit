/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2023 Alejandro Exojo Piqueras
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

#include "gamebrowserresourcefilter.h"

void GameBrowserResourceFilter::setFilter(const QString& filter)
{
    m_name.clear();
    m_types.clear();
    m_locations.clear();

    const QStringList parts = filter.split(QLatin1Char(' '));
    for (const QString& part : parts) {
        if (part.startsWith(QLatin1String("is:")))
            m_types.append(part.right(part.size() - 3).toLower());
        else
        if (part.startsWith(QLatin1String("in:")))
            m_locations.append(part.right(part.size() - 3).toLower());
        else
            m_name = part.toLower();
    }
}

bool GameBrowserResourceFilter::accept(const Row& row) const
{
    if (!m_name.isEmpty() && !row.name.toLower().contains(m_name))
        return false;

    auto matches = [](const QString& value) {
        return [kind = value.toLower()](const QString& item) {
            return kind.contains(item);
        };
    };
    bool success = m_types.isEmpty() ||
                   std::any_of(m_types.begin(), m_types.end(), matches(row.type));
    if (!success)
        return false;

    success = m_locations.isEmpty() ||
              std::any_of(m_locations.begin(), m_locations.end(), matches(row.location));
    if (!success)
        return false;

    return true;
}
