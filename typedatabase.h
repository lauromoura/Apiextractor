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

#ifndef TYPEDATABASE_H
#define TYPEDATABASE_H

#include <QStringList>
#include "typesystem.h"

class ContainerTypeEntry;
class PrimitiveTypeEntry;
class APIEXTRACTOR_API TypeDatabase
{
    TypeDatabase();
    TypeDatabase(const TypeDatabase&);
    TypeDatabase& operator=(const TypeDatabase&);
public:

    ~TypeDatabase();

    /**
    * Return the type system instance.
    * \param newInstance This parameter is useful just for unit testing, because singletons causes
    *                    too many side effects on unit testing.
    */
    static TypeDatabase* instance(bool newInstance = false);

    static QString normalizedSignature(const char* signature);

    QStringList requiredTargetImports() const;

    void addRequiredTargetImport(const QString& moduleName);

    QStringList typesystemPaths() const;

    void addTypesystemPath(const QString& typesystem_paths);

    IncludeList extraIncludes(const QString& className) const;

    PrimitiveTypeEntry* findPrimitiveType(const QString& name) const;
    ComplexTypeEntry* findComplexType(const QString& name) const;
    ObjectTypeEntry* findObjectType(const QString& name) const;
    NamespaceTypeEntry* findNamespaceType(const QString& name) const;
    ContainerTypeEntry* findContainerType(const QString& name) const;
    FunctionTypeEntry* findFunctionType(const QString& name) const;

    TypeEntry* findType(const QString& name) const;

    QList<TypeEntry*> findTypes(const QString &name) const;

    TypeEntryHash allEntries() const;

    SingleTypeEntryHash entries() const;

    PrimitiveTypeEntry* findTargetLangPrimitiveType(const QString& targetLangName) const;

    QList<const PrimitiveTypeEntry*> primitiveTypes() const;

    QList<const ContainerTypeEntry*> containerTypes() const;

    void addRejection(const QString& className, const QString& functionName,
                        const QString& fieldName, const QString& enumName);
    bool isClassRejected(const QString& className) const;
    bool isFunctionRejected(const QString& className, const QString& functionName) const;
    bool isFieldRejected(const QString& className, const QString& fieldName) const;
    bool isEnumRejected(const QString& className, const QString& enumName) const;

    void addType(TypeEntry* e);

    SingleTypeEntryHash flagsEntries() const;

    FlagsTypeEntry* findFlagsType(const QString& name) const;

    void addFlagsType(FlagsTypeEntry* fte);

    TemplateEntry* findTemplate(const QString& name) const;

    void addTemplate(TemplateEntry* t);

    AddedFunctionList globalUserFunctions() const;

    void addGlobalUserFunctions(const AddedFunctionList& functions);

    AddedFunctionList findGlobalUserFunctions(const QString& name) const;

    void addGlobalUserFunctionModifications(const FunctionModificationList& functionModifications);

    void addGlobalUserFunctionModification(const FunctionModification& functionModification);

    FunctionModificationList functionModifications(const QString& signature) const;

    void setSuppressWarnings(bool value);

    void addSuppressedWarning(const QString& s);

    bool isSuppressedWarning(const QString& s) const;

    void setRebuildClasses(const QStringList &cls);

    static QString globalNamespaceClassName(const TypeEntry *te);

    QString filename() const;

    QString modifiedTypesystemFilepath(const QString& tsFile) const;
    bool parseFile(const QString &filename, bool generate = true);
    bool parseFile(QIODevice* device, bool generate = true);

    void setApiVersion(const QString& package, const QByteArray& version);

    bool checkApiVersion(const QString& package, const QByteArray& version) const;

    const QStringList& dropTypeEntries() const;

    bool hasDroppedTypeEntries() const;

    bool shouldDropTypeEntry(const QString& fullTypeName) const;

    void setDropTypeEntries(QStringList dropTypeEntries);

private:

    class TypeDatabasePrivate;
    TypeDatabasePrivate* m_d;
};

#endif
