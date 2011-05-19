/*
 * This file is part of the API Extractor project.
 *
 * Copyright (C) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include "typedatabase.h"
#include "typesystem.h"
#include "typesystem_p.h"

#include <QFile>
#include <QXmlInputSource>
#include "reporthandler.h"

class TypeDatabase::TypeDatabasePrivate
{
public:
    bool suppressWarnings;
    TypeEntryHash entries;
    SingleTypeEntryHash flagsEntries;
    TemplateEntryHash templates;
    QStringList suppressedWarnings;

    AddedFunctionList globalUserFunctions;
    FunctionModificationList functionMods;

    QStringList requiredTargetImports;

    QStringList typesystemPaths;
    QHash<QString, bool> parsedTypesystemFiles;

    QList<TypeRejection> rejections;
    QStringList rebuildClasses;

    double apiVersion;
    QStringList dropTypeEntries;
};

TypeDatabase::TypeDatabase() : m_d(new TypeDatabasePrivate)
{
    m_d->suppressWarnings = true;
    m_d->apiVersion = 0;
    addType(new VoidTypeEntry());
    addType(new VarargsTypeEntry());
}

TypeDatabase::~TypeDatabase()
{
    delete m_d;
}

TypeDatabase* TypeDatabase::instance(bool newInstance)
{
    static TypeDatabase* db = 0;
    if (!db || newInstance) {
        if (db)
            delete db;
        db = new TypeDatabase;
    }
    return db;
}

QString TypeDatabase::normalizedSignature(const char* signature)
{
    QString normalized = QMetaObject::normalizedSignature(signature);

    if (!instance() || !QString(signature).contains("unsigned"))
        return normalized;

    QStringList types;
    types << "char" << "short" << "int" << "long";
    foreach (const QString& type, types) {
        if (instance()->findType(QString("u%1").arg(type)))
            continue;
        normalized.replace(QRegExp(QString("\\bu%1\\b").arg(type)), QString("unsigned %1").arg(type));
    }

    return normalized;
}

QStringList TypeDatabase::requiredTargetImports() const
{
    return m_d->requiredTargetImports;
}

void TypeDatabase::addRequiredTargetImport(const QString& moduleName)
{
    if (!m_d->requiredTargetImports.contains(moduleName))
        m_d->requiredTargetImports << moduleName;
}

QStringList TypeDatabase::typesystemPaths() const
{
    return m_d->typesystemPaths;
}

void TypeDatabase::addTypesystemPath(const QString& typesystem_paths)
{
    #if defined(Q_OS_WIN32)
    char* path_splitter = const_cast<char*>(";");
    #else
    char* path_splitter = const_cast<char*>(":");
    #endif
    m_d->typesystemPaths += typesystem_paths.split(path_splitter);
}

IncludeList TypeDatabase::extraIncludes(const QString& className) const
{
    ComplexTypeEntry* typeEntry = findComplexType(className);
    if (typeEntry)
        return typeEntry->extraIncludes();
    else
        return IncludeList();
}

ContainerTypeEntry* TypeDatabase::findContainerType(const QString &name) const
{
    QString template_name = name;

    int pos = name.indexOf('<');
    if (pos > 0)
        template_name = name.left(pos);

    TypeEntry* type_entry = findType(template_name);
    if (type_entry && type_entry->isContainer())
        return static_cast<ContainerTypeEntry*>(type_entry);
    return 0;
}

FunctionTypeEntry* TypeDatabase::findFunctionType(const QString& name) const
{
    TypeEntry* entry = findType(name);
    if (entry && entry->type() == TypeEntry::FunctionType)
        return static_cast<FunctionTypeEntry*>(entry);
    return 0;
}


PrimitiveTypeEntry* TypeDatabase::findTargetLangPrimitiveType(const QString& targetLangName) const
{
    foreach (QList<TypeEntry*> entries, m_d->entries.values()) {
        foreach (TypeEntry* e, entries) {
            if (e && e->isPrimitive()) {
                PrimitiveTypeEntry *pe = static_cast<PrimitiveTypeEntry*>(e);
                if (pe->targetLangName() == targetLangName && pe->preferredConversion())
                    return pe;
            }
        }
    }

    return 0;
}

TypeEntry* TypeDatabase::findType(const QString& name) const
{
    QList<TypeEntry *> entries = findTypes(name);
    foreach (TypeEntry *entry, entries) {
        if (entry &&
            (!entry->isPrimitive() || static_cast<PrimitiveTypeEntry *>(entry)->preferredTargetLangType())) {
            return entry;
        }
    }
    return 0;
}

QList<TypeEntry*> TypeDatabase::findTypes(const QString& name) const
{
    return m_d->entries.value(name);
}

TypeEntryHash TypeDatabase::allEntries() const
{
    return m_d->entries;
}

SingleTypeEntryHash TypeDatabase::entries() const
{
    TypeEntryHash entries = allEntries();

    SingleTypeEntryHash returned;
    QList<QString> keys = entries.keys();

    foreach (QString key, keys)
        returned[key] = findType(key);

    return returned;
}

QList<const PrimitiveTypeEntry*> TypeDatabase::primitiveTypes() const
{
    TypeEntryHash entries = allEntries();
    QList<const PrimitiveTypeEntry*> returned;
    foreach(QString key, entries.keys()) {
        foreach(const TypeEntry* typeEntry, entries[key]) {
            if (typeEntry->isPrimitive())
                returned.append((PrimitiveTypeEntry*) typeEntry);
        }
    }
    return returned;
}

QList<const ContainerTypeEntry*> TypeDatabase::containerTypes() const
{
    TypeEntryHash entries = allEntries();
    QList<const ContainerTypeEntry*> returned;
    foreach(QString key, entries.keys()) {
        foreach(const TypeEntry* typeEntry, entries[key]) {
            if (typeEntry->isContainer())
                returned.append((ContainerTypeEntry*) typeEntry);
        }
    }
    return returned;
}
void TypeDatabase::addRejection(const QString& className, const QString& functionName,
                                const QString& fieldName, const QString& enumName)
{
    TypeRejection r;
    r.class_name = className;
    r.function_name = functionName;
    r.field_name = fieldName;
    r.enum_name = enumName;

    m_d->rejections << r;
}

bool TypeDatabase::isClassRejected(const QString& className) const
{
    if (!m_d->rebuildClasses.isEmpty())
        return !m_d->rebuildClasses.contains(className);

    foreach (const TypeRejection& r, m_d->rejections)
    if (r.class_name == className && r.function_name == "*" && r.field_name == "*" && r.enum_name == "*")
        return true;

    return false;
}

bool TypeDatabase::isEnumRejected(const QString& className, const QString& enumName) const
{
    foreach (const TypeRejection& r, m_d->rejections) {
        if (r.enum_name == enumName
            && (r.class_name == className || r.class_name == "*")) {
            return true;
        }
    }

    return false;
}

bool TypeDatabase::isFunctionRejected(const QString& className, const QString& functionName) const
{
    foreach (const TypeRejection& r, m_d->rejections)
    if (r.function_name == functionName &&
        (r.class_name == className || r.class_name == "*"))
        return true;
    return false;
}


bool TypeDatabase::isFieldRejected(const QString& className, const QString& fieldName) const
{
    foreach (const TypeRejection& r, m_d->rejections)
    if (r.field_name == fieldName &&
        (r.class_name == className || r.class_name == "*"))
        return true;
    return false;
}

void TypeDatabase::addType(TypeEntry* e)
{
    m_d->entries[e->qualifiedCppName()].append(e);
}

SingleTypeEntryHash TypeDatabase::flagsEntries() const
{
    return m_d->flagsEntries;
}

TemplateEntry* TypeDatabase::findTemplate(const QString& name) const
{
    return m_d->templates[name];
}

void TypeDatabase::addTemplate(TemplateEntry* t)
{
    m_d->templates[t->name()] = t;
}

AddedFunctionList TypeDatabase::globalUserFunctions() const
{
    return m_d->globalUserFunctions;
}

void TypeDatabase::addGlobalUserFunctions(const AddedFunctionList& functions)
{
    m_d->globalUserFunctions << functions;
}

void TypeDatabase::addGlobalUserFunctionModifications(const FunctionModificationList& functionModifications)
{
    m_d->functionMods << functionModifications;
}

void TypeDatabase::addGlobalUserFunctionModification(const FunctionModification& functionModification)
{
    m_d->functionMods << functionModification;
}
void TypeDatabase::setSuppressWarnings(bool value)
{
    m_d->suppressWarnings = value;
}

void TypeDatabase::addSuppressedWarning(const QString& s)
{
    m_d->suppressedWarnings.append(s);
}

void TypeDatabase::setRebuildClasses(const QStringList& cls)
{
    m_d->rebuildClasses = cls;
}

QString TypeDatabase::filename() const
{
    return "typesystem.txt";
}

void TypeDatabase::addFlagsType(FlagsTypeEntry* fte)
{
    m_d->flagsEntries[fte->originalName()] = fte;
}

FlagsTypeEntry* TypeDatabase::findFlagsType(const QString &name) const
{
    FlagsTypeEntry* fte = (FlagsTypeEntry*) findType(name);
    if (!fte) {
        fte = (FlagsTypeEntry*) m_d->flagsEntries.value(name);
        if (!fte) {
            //last hope, search for flag without scope  inside of flags hash
            foreach(QString key, m_d->flagsEntries.keys()) {
                if (key.endsWith(name)) {
                    fte = (FlagsTypeEntry*) m_d->flagsEntries.value(key);
                    break;
                }
            }
        }
    }
    return fte;
}

AddedFunctionList TypeDatabase::findGlobalUserFunctions(const QString& name) const
{
    AddedFunctionList addedFunctions;
    foreach (AddedFunction func, m_d->globalUserFunctions) {
        if (func.name() == name)
            addedFunctions.append(func);
    }
    return addedFunctions;
}


QString TypeDatabase::globalNamespaceClassName(const TypeEntry * /*entry*/)
{
    return QLatin1String("Global");
}

FunctionModificationList TypeDatabase::functionModifications(const QString& signature) const
{
    FunctionModificationList lst;
    for (int i = 0; i < m_d->functionMods.count(); ++i) {
        const FunctionModification& mod = m_d->functionMods.at(i);
        if (mod.signature == signature)
            lst << mod;
    }

    return lst;
}

bool TypeDatabase::isSuppressedWarning(const QString& s) const
{
    if (!m_d->suppressWarnings)
        return false;

    foreach (const QString &_warning, m_d->suppressedWarnings) {
        QString warning(QString(_warning).replace("\\*", "&place_holder_for_asterisk;"));

        QStringList segs = warning.split("*", QString::SkipEmptyParts);
        if (!segs.size())
            continue;

        int i = 0;
        int pos = s.indexOf(QString(segs.at(i++)).replace("&place_holder_for_asterisk;", "*"));
        //qDebug() << "s == " << s << ", warning == " << segs;
        while (pos != -1) {
            if (i == segs.size())
                return true;
            pos = s.indexOf(QString(segs.at(i++)).replace("&place_holder_for_asterisk;", "*"), pos);
        }
    }

    return false;
}

QString TypeDatabase::modifiedTypesystemFilepath(const QString& tsFile) const
{
    if (!QFile::exists(tsFile)) {
        int idx = tsFile.lastIndexOf('/');
        QString fileName = idx >= 0 ? tsFile.right(tsFile.length() - idx - 1) : tsFile;
        foreach (const QString &path, m_d->typesystemPaths) {
            QString filepath(path + '/' + fileName);
            if (QFile::exists(filepath))
                return filepath;
        }
    }
    return tsFile;
}

bool TypeDatabase::parseFile(const QString &filename, bool generate)
{
    QString filepath = modifiedTypesystemFilepath(filename);
    if (m_d->parsedTypesystemFiles.contains(filepath))
        return m_d->parsedTypesystemFiles[filepath];

    QFile file(filepath);
    if (!file.exists()) {
        ReportHandler::warning("Can't find " + filename+", typesystem paths: "+m_d->typesystemPaths.join(", "));
        return false;
    }

    int count = m_d->entries.size();
    bool ok = parseFile(&file, generate);
    m_d->parsedTypesystemFiles[filepath] = ok;
    int newCount = m_d->entries.size();

    ReportHandler::debugSparse(QString::fromLatin1("Parsed: '%1', %2 new entries")
    .arg(filename)
    .arg(newCount - count));
    return ok;
}

bool TypeDatabase::parseFile(QIODevice* device, bool generate)
{
    QXmlInputSource source(device);
    QXmlSimpleReader reader;
    Handler handler(this, generate);

    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);

    return reader.parse(&source, false);
}

PrimitiveTypeEntry *TypeDatabase::findPrimitiveType(const QString& name) const
{
    QList<TypeEntry*> entries = findTypes(name);

    foreach (TypeEntry* entry, entries) {
        if (entry && entry->isPrimitive() && static_cast<PrimitiveTypeEntry*>(entry)->preferredTargetLangType())
            return static_cast<PrimitiveTypeEntry*>(entry);
    }

    return 0;
}

ComplexTypeEntry* TypeDatabase::findComplexType(const QString& name) const
{
    TypeEntry* entry = findType(name);
    if (entry && entry->isComplex())
        return static_cast<ComplexTypeEntry*>(entry);
    else
        return 0;
}

ObjectTypeEntry* TypeDatabase::findObjectType(const QString& name) const
{
    TypeEntry* entry = findType(name);
    if (entry && entry->isObject())
        return static_cast<ObjectTypeEntry*>(entry);
    else
        return 0;
}

NamespaceTypeEntry* TypeDatabase::findNamespaceType(const QString& name) const
{
    TypeEntry* entry = findType(name);
    if (entry && entry->isNamespace())
        return static_cast<NamespaceTypeEntry*>(entry);
    else
        return 0;
}

double TypeDatabase::apiVersion() const
{
    return m_d->apiVersion;
}

void TypeDatabase::setApiVersion(double version)
{
    m_d->apiVersion = version;
}

bool TypeDatabase::supportedApiVersion(double version) const
{
    return version <= m_d->apiVersion;
}

const QStringList& TypeDatabase::dropTypeEntries() const
{
    return m_d->dropTypeEntries;
}

bool TypeDatabase::hasDroppedTypeEntries() const
{
    return !m_d->dropTypeEntries.isEmpty();
}

bool TypeDatabase::shouldDropTypeEntry(const QString& fullTypeName) const
{
    return m_d->dropTypeEntries.contains(fullTypeName);
}

void TypeDatabase::setDropTypeEntries(QStringList dropTypeEntries)
{
    m_d->dropTypeEntries = dropTypeEntries;
    m_d->dropTypeEntries.sort();
}

