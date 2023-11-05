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

#include "pathutils.h"

QString path::baseName(const QString& path)
{
#if 1 // TODO: For the science, benchmark this two implementations to see the impact.
    const int lastSlash = path.lastIndexOf(QLatin1Char('/'));
    const int firstDot = path.indexOf(QLatin1Char('.'), lastSlash + 1);
    // The first parameter to mid() is a postion, but the 2nd is a size. We need
    // to count how many characters to remove from the start and from the end.
    return path.mid(lastSlash + 1, (path.size() - lastSlash) - (path.size() - firstDot + 1));
#else
    return path.section(QLatin1Char('/'), -1).section(QLatin1Char('.'), 0, 0);
#endif
}

QString path::completeBaseName(const QString& path)
{
#if 1 // TODO: For the science, benchmark this two implementations to see the impact.
    const int lastSlash = path.lastIndexOf(QLatin1Char('/'));
    const int lastDot = path.lastIndexOf(QLatin1Char('.'));
    return path.mid(lastSlash + 1, (path.size() - lastSlash) - (path.size() - lastDot + 1));
#else
    return path.section(QLatin1Char('/'), -1).section(QLatin1Char('.'), 0, -2);
#endif
}

QString path::suffix(const QString& path)
{
    return path.section(QLatin1Char('.'), -1);
}
