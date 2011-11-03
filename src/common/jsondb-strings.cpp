/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtAddOn.JsonDb module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "jsondb-strings.h"

namespace QtAddOn { namespace JsonDb {

const QString JsonDbString::kUuidStr    = QString::fromLatin1("_uuid");
const QString JsonDbString::kVersionStr = QString::fromLatin1("_version");
const QString JsonDbString::kIdStr      = QString::fromLatin1("id");
const QString JsonDbString::kResultStr  = QString::fromLatin1("result");
const QString JsonDbString::kErrorStr   = QString::fromLatin1("error");
const QString JsonDbString::kFieldNameStr   = QString::fromLatin1("fieldName");
const QString JsonDbString::kCodeStr    = QString::fromLatin1("code");
const QString JsonDbString::kMessageStr = QString::fromLatin1("message");
const QString JsonDbString::kNameStr    = QString::fromLatin1("name");
const QString JsonDbString::kCountStr   = QString::fromLatin1("count");
const QString JsonDbString::kCurrentStr = QString::fromLatin1("_current");
const QString JsonDbString::kDomainStr    = QString::fromLatin1("_domain");
const QString JsonDbString::kOwnerStr   = QString::fromLatin1("_owner");
const QString JsonDbString::kTypeStr    = QString::fromLatin1("_type");
const QString JsonDbString::kTypesStr   = QString::fromLatin1("types");
const QString JsonDbString::kParentStr  = QString::fromLatin1("_parent");
const QString JsonDbString::kSchemaTypeStr = QString::fromLatin1("_schemaType");

const QString JsonDbString::kActionStr   = QString::fromLatin1("action");
const QString JsonDbString::kActionsStr  = QString::fromLatin1("actions");
const QString JsonDbString::kActiveStr   = QString::fromLatin1("active");
const QString JsonDbString::kAddIndexStr = QString::fromLatin1("addIndex");
const QString JsonDbString::kCreateStr   = QString::fromLatin1("create");
const QString JsonDbString::kDropStr   = QString::fromLatin1("drop");
const QString JsonDbString::kConnectStr  = QString::fromLatin1("connect");
const QString JsonDbString::kDataStr     = QString::fromLatin1("data");
const QString JsonDbString::kDeletedStr   = QString::fromLatin1("_deleted");
const QString JsonDbString::kDisconnectStr  = QString::fromLatin1("disconnect");
const QString JsonDbString::kExplanationStr = QString::fromLatin1("explanation");
const QString JsonDbString::kFindStr     = QString::fromLatin1("find");
const QString JsonDbString::kLengthStr   = QString::fromLatin1("length");
const QString JsonDbString::kLimitStr    = QString::fromLatin1("limit");
const QString JsonDbString::kMapTypeStr  = QString::fromLatin1("Map");
const QString JsonDbString::kNotifyStr   = QString::fromLatin1("notify");
const QString JsonDbString::kNotificationTypeStr = QString::fromLatin1("notification");
const QString JsonDbString::kObjectStr   = QString::fromLatin1("object");
const QString JsonDbString::kOffsetStr   = QString::fromLatin1("offset");
const QString JsonDbString::kQueryStr    = QString::fromLatin1("query");
const QString JsonDbString::kReduceTypeStr   = QString::fromLatin1("Reduce");
const QString JsonDbString::kRemoveStr   = QString::fromLatin1("remove");
const QString JsonDbString::kSchemaStr   = QString::fromLatin1("schema");
const QString JsonDbString::kUpdateStr   = QString::fromLatin1("update");
const QString JsonDbString::kTokenStr    = QString::fromLatin1("token");
const QString JsonDbString::kSettingsStr    = QString::fromLatin1("settings");
const QString JsonDbString::kViewTypeStr = QString::fromLatin1("View");
const QString JsonDbString::kChangesSinceStr = QString::fromLatin1("changesSince");
const QString JsonDbString::kStateNumberStr = QString::fromLatin1("stateNumber");
const QString JsonDbString::kCollapsedStr = QString::fromLatin1("collapsed");
const QString JsonDbString::kCurrentStateNumberStr = QString::fromLatin1("currentStateNumber");
const QString JsonDbString::kStartingStateNumberStr = QString::fromLatin1("startingStateNumber");
const QString JsonDbString::kTombstoneStr = QString::fromLatin1("Tombstone");
const QString JsonDbString::kPartitionStr = QString::fromLatin1("Partition");

const QString JsonDbString::kSystemPartitionName = QString::fromLatin1("com.nokia.qtjsondb.System");

} } // end namespace QtAddOn::JsonDb
