/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QStack>
#include <QString>

#include "jsondbstrings.h"
#include "jsondbindexquery.h"
#include "jsondbpartition.h"
#include "jsondbquery.h"
#include "jsondbsettings.h"

QT_BEGIN_NAMESPACE_JSONDB_PARTITION

const char* JsonDbQueryTokenizer::sTokens[] = {
"[", "]", "{", "}", "/", "?", ",", ":", "|", "\\"
//operators are ordered by precedence
, "!=", "<=", ">=", "=~", "->", "=", ">", "<"
, ""//end of the token list
};

JsonDbQueryTokenizer::JsonDbQueryTokenizer(QString input)
    : mInput(input), mPos(0)
{
}

QString JsonDbQueryTokenizer::pop()
{
    QString token;
    if (!mNextToken.isEmpty()) {
        token = mNextToken;
        mNextToken.clear();
    } else {
        token = getNextToken();
    }
    return token;
}

QString JsonDbQueryTokenizer::popIdentifier()
{
    QString identifier = pop();
    if (identifier.startsWith('\"')
        && identifier.endsWith('\"'))
        identifier = identifier.mid(1, identifier.size()-2);
    return identifier;
}

QString JsonDbQueryTokenizer::peek()
{
    if (mNextToken.isEmpty()) {
        mNextToken = getNextToken();
    }
    return mNextToken;
}

QString JsonDbQueryTokenizer::getNextToken()
{
    QString result;

    while (mPos < mInput.size()) {
        QChar c = mInput[mPos++];
        if (c == '"') {
            // match string
            result.append(c);
            bool escaped = false;
            QChar sc;
            int size = mInput.size();
            int i;
            //qDebug() << "start of string";
            for (i = mPos; (i < size); i++) {
                sc = mInput[i];
                //qDebug() << i << sc << escaped;
                if (!escaped && (sc == '"'))
                    break;
                escaped = (sc == '\\');
            }
            //qDebug() << "end" << i << sc << escaped;
            //qDebug() << mInput.mid(mPos, i-mPos+1);
            if ((i < size) && (sc == '"')) {
                //qDebug() << mPos << i-mPos << "string is" << mInput.mid(mPos, i-mPos);
                result.append(mInput.mid(mPos, i-mPos+1));
                mPos = i+1;
            } else {
                mPos = i;
                result = QString();
            }
            return result;
        } else if (c.isSpace()) {
            if (result.size())
                return result;
            else
                continue;
        } else if (result.size() && mPos+1 < mInput.size()) {
            //index expression?[n],[*]
            if (c == '[' && mInput[mPos+1] == ']') {
                result.append(mInput.mid(mPos-1,3));
                mPos += 2;
                continue;
            }
        }
        //operators
        int i = 0;
        while (sTokens[i][0] != 0) {
            if (mInput.midRef(mPos - 1,2).startsWith(sTokens[i])) {
                if (!result.isEmpty()) {
                    mPos --;
                    return result;
                }
                result.append(sTokens[i]);
                mPos += strlen(sTokens[i]) - 1;
                return result;
            }
            i++;
        }
        result.append(mInput[mPos-1]);
    }//while
    return QString();
}

QJsonValue QueryTerm::value() const
{
    if (mVariable.size())
        return mQuery->binding(mVariable);
    else
        return mValue;
}

JsonDbQuery::JsonDbQuery(const QList<OrQueryTerm> &qt, const QList<OrderTerm> &ot) :
    queryTerms(qt)
  , orderTerms(ot)
{
}

JsonDbQuery::~JsonDbQuery()
{
    queryTerms.clear();
    orderTerms.clear();
}

QJsonValue JsonDbQuery::parseJsonLiteral(const QString &json, QueryTerm *term, const QJsonObject &bindings, bool *ok)
{
    const ushort trueLiteral[] = {'t','r','u','e', 0};
    const ushort falseLiteral[] = {'f','a','l','s','e', 0};
    const ushort *literal = json.utf16();
    Q_ASSERT(ok != NULL);
    *ok = true;
    switch (literal[0]) {
    case '"':
        term->setValue(json.mid(1, json.size()-2));
        break;
    case 't':
        // we will interpret  "true0something" as true is it a real problem ?
        for (int i = 1; i < 5 /* 'true0' length */; ++i) {
            if (trueLiteral[i] != literal[i]) {
                *ok = false;
                return term->value();
            }
        }
        term->setValue(true);
        break;
    case 'f':
        // we will interpret  "false0something" as false is it a real problem ?
        for (int i = 1; i < 6  /* 'false0' length */; ++i) {
            if (falseLiteral[i] != literal[i]) {
                *ok = false;
                return term->value();
            }
        }
        term->setValue(false);
        break;
    case '%':
    {
        const QString name = json.mid(1);
        if (bindings.contains(name))
            term->setValue(bindings.value(name));
        else
            term->setVariable(name);
        break;
    }
    case 0:
        // This can happen if json.length() == 0
        *ok = false;
        return term->value();
    default:
        int result = json.toInt(ok);
        if (*ok) {
            term->setValue(result);
        } else {
            // bad luck, it can be only a double
            term->setValue(json.toDouble(ok));
        }
    }
    return term->value();
}

JsonDbQuery *JsonDbQuery::parse(const QString &query, const QJsonObject &bindings)
{
    JsonDbQuery *parsedQuery = new JsonDbQuery;
    parsedQuery->query = query;

    if (!query.startsWith('['))
        return parsedQuery;

    bool parseError = false;
    JsonDbQueryTokenizer tokenizer(query);
    QString token;
    while (!parseError
           && !(token = tokenizer.pop()).isEmpty()) {
        if (token != "[") {
            qCritical() << "unexpected token" << token;
            break;
        }
        token = tokenizer.pop();
        if (token == "?") {
            OrQueryTerm oqt;
            do {
                QString fieldSpec = tokenizer.popIdentifier();
                if (fieldSpec == "|")
                    fieldSpec = tokenizer.popIdentifier();

                QString opOrJoin = tokenizer.pop();
                QString op;
                QStringList joinFields;
                QString joinField;
                while (opOrJoin == "->") {
                    joinFields.append(fieldSpec);
                    fieldSpec = tokenizer.popIdentifier();
                    opOrJoin = tokenizer.pop();
                }
                if (joinFields.size())
                    joinField = joinFields.join("->");
                op = opOrJoin;


                QueryTerm term(parsedQuery);
                if (!joinField.isEmpty())
                    term.setJoinField(joinField);
                if (fieldSpec.startsWith(QChar('%'))) {
                    const QString name = fieldSpec.mid(1);
                    if (bindings.contains(name)) {
                        QJsonValue val = bindings.value(name);
                        parsedQuery->bind(name, val);
                    }
                    term.setPropertyVariable(name);
                }
                else
                    term.setPropertyName(fieldSpec);
                term.setOp(op);
                if (op == "=~") {
                    QString tvs = tokenizer.pop();
                    int sepPos = 1; // assuming it's a literal "/regexp/modifiers"
                    if (tvs.startsWith(QChar('%'))) {
                        const QString name = tvs.mid(1);
                        if (bindings.contains(name)) {
                            tvs = bindings.value(name).toString();
                            sepPos = 0;
                        }
                    } else if (!tvs.startsWith("\"")) {
                        parsedQuery->queryExplanation.append(QString("Failed to parse query regular expression '%1' in query '%2' %3 op %4")
                                                             .arg(tvs)
                                                             .arg(parsedQuery->query)
                                                             .arg(fieldSpec)
                                                             .arg(op));
                        parseError = true;
                        break;
                    }
                    QChar sep = tvs[sepPos];
                    int eor = sepPos;
                    do {
                        eor = tvs.indexOf(sep, eor+1); // end of regexp;
                        //qDebug() << "tvs" << tvs << "eor" << eor << "tvs[eor-1]" << ((eor > 0) ? tvs[eor-1] : QChar('*'));
                        if (eor <= sepPos) {
                            parseError = true;
                            break;
                        }
                    } while ((eor > 0) && (tvs[eor-1] == '\\'));
                    QString modifiers = tvs.mid(eor+1,tvs.size()-eor-2*sepPos);
                    if (jsondbSettings->debug()) {
                        qDebug() << "modifiers" << modifiers;
                        qDebug() << "regexp" << tvs.mid(sepPos + 1, eor-sepPos-1);
                    }
                    if (modifiers.contains('w'))
                        term.regExp().setPatternSyntax(QRegExp::Wildcard);
                    if (modifiers.contains('i'))
                        term.regExp().setCaseSensitivity(Qt::CaseInsensitive);
                    //qDebug() << "pattern" << tvs.mid(2, eor-2);
                    term.regExp().setPattern(tvs.mid(sepPos + 1, eor-sepPos-1));
                } else if (op == "contains") {
                    bool ok = true;;

                    QString value = tokenizer.pop();
                    if (value == "[" || value == "{") {
                        QStack<QString> tokenStack;
                        tokenStack.push(value);
                        QString tkn = value;

                        while (ok && !tkn.isEmpty()) {
                            tkn = tokenizer.pop();
                            if (tkn == "]" && tokenStack.isEmpty()) {
                                tokenizer.push(tkn);
                                break;
                            } else {
                                value += tkn;
                                if (tkn == "]") {
                                    if (tokenStack.pop() != "[")
                                        ok = false;
                                } else if (tkn == "}") {
                                    if (tokenStack.pop() != "{")
                                        ok = false;
                                }
                            }
                        }

                        if (ok) {
                            QJsonParseError parserError;
                            QJsonDocument parsedValue = QJsonDocument::fromJson(value.toUtf8(), &parserError);
                            if (parserError.error != QJsonParseError::NoError) {
                                ok = false;
                            } else {
                                if (parsedValue.isArray())
                                    term.setValue(parsedValue.array());
                                else
                                    term.setValue(parsedValue.object());
                            }
                        }
                    } else {
                        parseJsonLiteral(value, &term, bindings, &ok);
                    }

                    if (!ok) {
                        parsedQuery->queryExplanation.append(QString("Failed to parse query value '%1' in query '%2' %3 op %4")
                                                             .arg(value)
                                                             .arg(parsedQuery->query)
                                                             .arg(fieldSpec)
                                                             .arg(op));
                        parseError = true;
                        break;
                    }
                } else if ((op != "exists") && (op != "notExists")) {
                    QString value = tokenizer.pop();
                    bool ok = true;;
                    if (value == "[") {
                        QJsonArray values;
                        while (1) {
                            value = tokenizer.pop();
                            if (value == "]")
                                break;
                            parseJsonLiteral(value, &term, bindings, &ok);
                            if (!ok)
                                break;
                            values.append(term.value());
                            if (tokenizer.peek() == ",")
                                tokenizer.pop();
                        }
                        term.setValue(values);
                    } else {
                        parseJsonLiteral(value, &term, bindings, &ok);
                    }
                    if (!ok) {
                        parsedQuery->queryExplanation.append(QString("Failed to parse query value '%1' in query '%2' %3 op %4")
                                                             .arg(value)
                                                             .arg(parsedQuery->query)
                                                             .arg(fieldSpec)
                                                             .arg(op));
                        parseError = true;
                        break;
                    }
                }

                oqt.addTerm(term);
            } while (tokenizer.peek() != "]");
            parsedQuery->queryTerms.append(oqt);
        } else if (token == "=") {
            QString curlyBraceToken = tokenizer.pop();
            if (curlyBraceToken != "{") {
                parsedQuery->queryExplanation.append(QString("Parse error: expecting '{' but got '%1'")
                                                     .arg(curlyBraceToken));
                parseError = true;
                break;
            }
            QString nextToken;
            while (!(nextToken = tokenizer.popIdentifier()).isEmpty()) {
                if (nextToken == "}") {
                    break;
                } else {

                    //qDebug() << "isMapObject" << nextToken << tokenizer.peek();
                    parsedQuery->mapKeyList.append(nextToken);
                    QString colon = tokenizer.pop();
                    if (colon != ":") {
                        parsedQuery->queryExplanation.append(QString("Parse error: expecting ':' but got '%1'").arg(colon));
                        parseError = true;
                        break;
                    }
                    nextToken = tokenizer.popIdentifier();

                    while (tokenizer.peek() == "->") {
                        QString op = tokenizer.pop();
                        nextToken.append(op);
                        nextToken.append(tokenizer.popIdentifier());
                    }
                    parsedQuery->mapExpressionList.append(nextToken);
                    QString maybeComma = tokenizer.pop();
                    if (maybeComma == "}") {
                        tokenizer.push(maybeComma);
                        continue;
                    } else if (maybeComma != ",") {
                        parsedQuery->queryExplanation.append(QString("Parse error: expecting ',', or '}' but got '%1'")
                                                             .arg(maybeComma));
                        parseError = true;
                        break;
                    }
                }
            }
        } else if ((token == "/") || (token == "\\") || (token == ">") || (token == "<")) {
            QString ordering = token;
            OrderTerm term;
            term.propertyName = tokenizer.popIdentifier();
            term.ascending = ((ordering == "/") || (ordering == ">"));
            parsedQuery->orderTerms.append(term);
        } else if (token == "count") {
            parsedQuery->mAggregateOperation = "count";
        } else if (token == "*") {
            // match all objects
        } else {
            qCritical() << QString("Parse error: expecting '?', '/', '\\', or 'count' but got '%1'").arg(token);
            parseError = true;
            break;
        }
        QString closeBracket = tokenizer.pop();
        if (closeBracket != "]") {
            qCritical() << QString("Parse error: expecting ']' but got '%1'").arg(closeBracket);
            parseError = true;
            break;
        }
    }

    if (parseError) {
        QStringList explanation = parsedQuery->queryExplanation;
        delete parsedQuery;
        parsedQuery = new JsonDbQuery;
        parsedQuery->queryExplanation = explanation;
        qCritical() << "Parser error: query" << query << explanation;
        return parsedQuery;
    }

    foreach (const OrQueryTerm &oqt, parsedQuery->queryTerms) {
        foreach (const QueryTerm &term, oqt.terms()) {
            if (term.propertyName() == JsonDbString::kTypeStr) {
                if (term.op() == "=") {
                    parsedQuery->mMatchedTypes.clear();
                    parsedQuery->mMatchedTypes.insert(term.value().toString());
                } else if (term.op() == "!=") {
                    parsedQuery->mMatchedTypes.insert(term.value().toString());
                }
            }
        }
    }

    if (!parsedQuery->queryTerms.size() && !parsedQuery->orderTerms.size()) {
        // match everything -- sort on type
        OrderTerm term;
        term.propertyName = JsonDbString::kTypeStr;
        term.ascending = true;
        parsedQuery->orderTerms.append(term);
    }

    //qDebug() << "queryTerms.size()" << parsedQuery->queryTerms.size();
    //qDebug() << "orderTerms.size()" << parsedQuery->orderTerms.size();
    return parsedQuery;
}

bool JsonDbQuery::match(const JsonDbObject &object, QHash<QString, JsonDbObject> *objectCache, JsonDbPartition *partition) const
{
    for (int i = 0; i < queryTerms.size(); i++) {
        const OrQueryTerm &orQueryTerm = queryTerms[i];
        bool matches = false;
        foreach (const QueryTerm &term, orQueryTerm.terms()) {
            const QString &joinPropertyName = term.joinField();
            const QString &op = term.op();
            const QJsonValue &termValue = term.value();

            QJsonValue objectFieldValue;
            if (!joinPropertyName.isEmpty()) {
                JsonDbObject joinedObject = object;
                const QVector<QStringList> &joinPaths = term.joinPaths();
                for (int j = 0; j < joinPaths.size(); j++) {
                    if (!joinPaths[j].size()) {
                        if (jsondbSettings->debug())
                            qDebug() << term.joinField() << term.joinPaths();
                    }
                    QString uuidValue = joinedObject.propertyLookup(joinPaths[j]).toString();
                    if (objectCache && objectCache->contains(uuidValue))
                        joinedObject = objectCache->value(uuidValue);
                    else if (partition) {
                        ObjectKey objectKey(uuidValue);
                        partition->getObject(objectKey, joinedObject);
                        if (objectCache) objectCache->insert(uuidValue, joinedObject);
                    }
                }
                objectFieldValue = joinedObject.propertyLookup(term.fieldPath());
            } else {
                if (term.propertyName().isEmpty())
                    objectFieldValue = binding(term.propertyVariable());
                else
                    objectFieldValue = object.propertyLookup(term.fieldPath());
            }
            if ((op == "=") || (op == "==")) {
                if (objectFieldValue == termValue)
                        matches = true;
            } else if ((op == "<>") || (op == "!=")) {
                if (objectFieldValue != termValue)
                    matches = true;
            } else if (op == "=~") {
                if (jsondbSettings->debug())
                    qDebug() << objectFieldValue.toString() << term.regExpConst().exactMatch(objectFieldValue.toString());
                if (term.regExpConst().exactMatch(objectFieldValue.toString()))
                    matches = true;
            } else if (op == "<=") {
                matches = JsonDbIndexQuery::lessThan(objectFieldValue, termValue) || (objectFieldValue == termValue);
            } else if (op == "<") {
                matches = JsonDbIndexQuery::lessThan(objectFieldValue, termValue);
            } else if (op == ">=") {
                matches = JsonDbIndexQuery::greaterThan(objectFieldValue, termValue) || (objectFieldValue == termValue);
            } else if (op == ">") {
                matches = JsonDbIndexQuery::greaterThan(objectFieldValue, termValue);
            } else if (op == "exists") {
                if (objectFieldValue.type() != QJsonValue::Undefined)
                    matches = true;
            } else if (op == "notExists") {
                if (objectFieldValue.type() == QJsonValue::Undefined)
                    matches = true;
            } else if (op == "in") {
                if (0) qDebug() << __FUNCTION__ << __LINE__ << objectFieldValue << "in" << termValue
                                << termValue.toArray().contains(objectFieldValue);
                if (termValue.toArray().contains(objectFieldValue))
                    matches = true;
            } else if (op == "notIn") {
                if (0) qDebug() << __FUNCTION__ << __LINE__ << objectFieldValue << "notIn" << termValue
                                << !termValue.toArray().contains(objectFieldValue);
                if (!termValue.toArray().contains(objectFieldValue))
                    matches = true;
            } else if (op == "contains") {
                if (0) qDebug() << __FUNCTION__ << __LINE__ << objectFieldValue << "contains" << termValue
                                << objectFieldValue.toArray().contains(termValue);
                if (objectFieldValue.toArray().contains(termValue))
                    matches = true;
            } else if (op == "startsWith") {
                if ((objectFieldValue.type() == QJsonValue::String)
                    && objectFieldValue.toString().startsWith(termValue.toString()))
                    matches = true;
            } else {
                qCritical() << "match" << "unhandled term" << term.propertyName() << term.op() << term.value() << term.joinField();
            }
        }
        if (!matches)
            return false;
    }
    return true;
}



QueryTerm::QueryTerm(const JsonDbQuery *query)
    : mQuery(query), mJoinPaths()
{
}

QueryTerm::~QueryTerm()
{
    mValue = QJsonValue();
    mJoinPaths.clear();
}

OrQueryTerm::OrQueryTerm()
{
}

OrQueryTerm::OrQueryTerm(const QueryTerm &term)
{
    mTerms.append(term);
}

OrQueryTerm::~OrQueryTerm()
{
}

QList<QString> OrQueryTerm::propertyNames() const
{
    QList<QString> propertyNames;
    foreach (const QueryTerm &term, mTerms) {
        QString propertyName = term.propertyName();
        if (!propertyNames.contains(propertyName))
            propertyNames.append(propertyName);
    }
    return propertyNames;
}

QList<QString> OrQueryTerm::findUnindexablePropertyNames() const
{
  QList<QString> unindexablePropertyNames;
    foreach (const QueryTerm &term, mTerms) {
        const QString propertyName = term.propertyName();
        const QString op = term.op();
        if (op == QLatin1String("notExists") && !unindexablePropertyNames.contains(propertyName))
            unindexablePropertyNames.append(propertyName);
    }
    return unindexablePropertyNames;
}

OrderTerm::OrderTerm()
{
}

OrderTerm::~OrderTerm()
{
}

QVariantMap JsonDbQueryResult::toVariantMap() const
{
    QJsonObject resultmap, errormap;
    QJsonArray variantList;
    for (int i = 0; i < data.size(); i++)
        variantList.append(data.at(i));
    resultmap.insert(JsonDbString::kDataStr, variantList);
    resultmap.insert(JsonDbString::kLengthStr, data.size());
    resultmap.insert(JsonDbString::kOffsetStr, offset);
    resultmap.insert(JsonDbString::kExplanationStr, explanation);
    resultmap.insert(QString("sortKeys"), sortKeys);
    if (error.isObject())
        errormap = error.toObject();
    return JsonDbPartition::makeResponse(resultmap, errormap).toVariantMap();
}

JsonDbQueryResult JsonDbQueryResult::makeErrorResponse(JsonDbError::ErrorCode code, const QString &message, bool silent)
{
    JsonDbQueryResult result;
    QJsonObject errormap;
    errormap.insert(JsonDbString::kCodeStr, code);
    errormap.insert(JsonDbString::kMessageStr, message);
    result.error = errormap;
    if (jsondbSettings->verbose() && !silent && !errormap.isEmpty())
        qCritical() << errormap;
    return result;
}

QT_END_NAMESPACE_JSONDB_PARTITION