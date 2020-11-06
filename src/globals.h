#pragma once

#include <QString>

/// "String reference". An index into the entries of the TLK file.
using StrRef = quint32;

/// \brief "Resource reference". The name of a resource.
///
/// Even if in the game files this
/// is always an ASCII string, so it could be held in a QByteArray, we make it a
/// QString because in any use of it we need to convert it to a QString (e.g.
/// showing it on the screen, or opening a file with QFile).
using ResRef = QString;
