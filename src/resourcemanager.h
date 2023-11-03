/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2020 Alejandro Exojo Piqueras
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

#include "resourcetype.h"

#include <QObject>

struct KeyFile;

class ResourceManager : public QObject
{
    Q_OBJECT

public:
    explicit ResourceManager(QObject* parentObject = nullptr);
    ~ResourceManager();

    const KeyFile& chitinKey() const;

    void load(const QString& path);

    /*!
     * \brief Returns the contents of a resource from a full file name ("abc.xyz").
     *
     * Searches a file by name, and retrieves its contents either from a BIFF
     * file or from the override directory (the later has precedence).
     *
     * \param name Name of the file
     * \return Contents of the file
     */
    QByteArray resource(const QString& name) const;

    /*!
     * \brief Returns the contents of a resource from a base name and type.
     *
     * Searches a file by base name ("abc", without ".xyz") and type, and
     * retrieves its contents either from a BIFF file or from the override
     * directory (the later has precedence).
     *
     * \param name Name of the file
     * \param type Type of resource
     * \return Contents of the file
     */
    QByteArray resource(const QString& name, ResourceType type);

    /*!
     * \brief Return a resource without looking in the override directory.
     * \param name Name of the file
     * \param type Type of the file.
     * \return
     */
    QByteArray defaultResource(const QString& name, ResourceType type) const;

    /*!
     * \brief Returns a file termination "xyz" from the type.
     * \param type
     * \return
     */
    static QString resourceTermination(ResourceType type);

    /*!
     * \brief Returns a file name "abc.xyz" from "abc" and the type
     * \param name
     * \param type
     * \return
     */
    static QString resourceFileName(const QString& name, ResourceType type);

    /*!
     * \brief Returns the resource type from a full file name
     * \param name
     * \return
     */
    static ResourceType resourceType(const QString& name);

signals:
    void loaded();

private:
    struct Private;
    Private& d;
};

