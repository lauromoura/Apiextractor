/*
* This file is part of the API Extractor project.
*
* Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "testnaming.h"
#include "abstractmetabuilder.h"
#include <QtTest/QTest>
#include "testutil.h"

/** Basic rules of naming
 *
 *  AbstractMeta*->name() -> Target language name, WITHOUT scope information
 *  TypeEntry->name() -> C++ name, WITHOUT scope information
 *  TypeEntry->qualifiedCppName() -> Self explanatory
 *  TypeEntry->targetLangName() -> Target language name, WITH scope but WITHOUT package
 *  TypeEntry->qualifiedTargetLangName() -> Target language name, WITH scope and WITH package
 */

void TestNaming::testClass()
{
    const char* cppCode ="class ClassName {};";
    const char* xmlCode = "<typesystem package=\"Foo\"><value-type name=\"ClassName\"/></typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    QCOMPARE(classes[0]->name(), QString("ClassName"));

    TypeEntry* type = classes[0]->typeEntry();
    QVERIFY(type);
    QCOMPARE(type->name(), QString("ClassName"));
    QCOMPARE(type->qualifiedCppName(), QString("ClassName"));
    QCOMPARE(type->targetLangName(), QString("ClassName"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.ClassName"));
}

void TestNaming::testClassInNamespace()
{
    const char* cppCode ="namespace Nsp { class ClassName {}; }";
    const char* xmlCode = "<typesystem package=\"Foo\"><namespace-type name=\"Nsp\">\
                           <value-type name=\"ClassName\"/></namespace-type></typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* a = classes.findClass("ClassName");
    QVERIFY(a);
    QCOMPARE(a->name(), QString("ClassName"));

    TypeEntry* type = a->typeEntry();
    QVERIFY(type);
    QCOMPARE(type->name(), QString("ClassName"));
    QCOMPARE(type->qualifiedCppName(), QString("Nsp::ClassName"));
    QCOMPARE(type->targetLangName(), QString("Nsp.ClassName"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.Nsp.ClassName"));

    AbstractMetaClass* b = classes.findClass("Nsp");
    QVERIFY(b);
    QCOMPARE(b->name(), QString("Nsp"));

    type = b->typeEntry();
    QVERIFY(type);
    QCOMPARE(type->name(), QString("Nsp"));
    QCOMPARE(type->qualifiedCppName(), QString("Nsp"));
    QCOMPARE(type->targetLangName(), QString("Nsp"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.Nsp"));
}

void TestNaming::testDeepClassNesting()
{
    const char* cppCode ="class A { class B { class C { class D {}; }; }; };";
    const char* xmlCode = "<typesystem package=\"Foo\">\
                                <value-type name=\"A\">\
                                    <value-type name=\"B\">\
                                        <value-type name=\"C\">\
                                            <value-type name=\"D\"/>\
                                        </value-type>\
                                    </value-type>\
                                </value-type>\
                           </typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();

    // A
    AbstractMetaClass* a = classes.findClass("A");
    QVERIFY(a);
    QCOMPARE(a->name(), QString("A"));

    TypeEntry* type = a->typeEntry();
    QVERIFY(type);
    QCOMPARE(type->name(), QString("A"));
    QCOMPARE(type->qualifiedCppName(), QString("A"));
    QCOMPARE(type->targetLangName(), QString("A"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.A"));

    // B
    a = classes.findClass("B");
    QVERIFY(a);
    QCOMPARE(a->name(), QString("B"));

    type = a->typeEntry();
    QVERIFY(type);
    QCOMPARE(type->name(), QString("B"));
    QCOMPARE(type->qualifiedCppName(), QString("A::B"));
    QCOMPARE(type->targetLangName(), QString("A.B"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.A.B"));

    // C
    a = classes.findClass("C");
    QVERIFY(a);
    QCOMPARE(a->name(), QString("C"));

    type = a->typeEntry();
    QVERIFY(type);
    QCOMPARE(type->name(), QString("C"));
    QCOMPARE(type->qualifiedCppName(), QString("A::B::C"));
    QCOMPARE(type->targetLangName(), QString("A.B.C"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.A.B.C"));

    // D
    a = classes.findClass("D");
    QVERIFY(a);
    QCOMPARE(a->name(), QString("D"));

    type = a->typeEntry();
    QVERIFY(type);
    QCOMPARE(type->name(), QString("D"));
    QCOMPARE(type->qualifiedCppName(), QString("A::B::C::D"));
    QCOMPARE(type->targetLangName(), QString("A.B.C.D"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.A.B.C.D"));
}

void TestNaming::testDeepClassNestingWithNamespace()
{
    const char* cppCode ="class A { class B { class C { class D {}; }; }; };";
    const char* xmlCode = "<typesystem package=\"Foo\">\
                                <value-type name=\"A\">\
                                    <value-type name=\"B\">\
                                        <value-type name=\"C\">\
                                            <value-type name=\"D\"/>\
                                        </value-type>\
                                    </value-type>\
                                </value-type>\
                           </typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();

    // A
    AbstractMetaClass* a = classes.findClass("A");
    QVERIFY(a);
    QCOMPARE(a->name(), QString("A"));

    TypeEntry* type = a->typeEntry();
    QVERIFY(type);
    QCOMPARE(type->name(), QString("A"));
    QCOMPARE(type->qualifiedCppName(), QString("A"));
    QCOMPARE(type->targetLangName(), QString("A"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.A"));

    // B
    a = classes.findClass("B");
    QVERIFY(a);
    QCOMPARE(a->name(), QString("B"));

    type = a->typeEntry();
    QVERIFY(type);
    QCOMPARE(type->name(), QString("B"));
    QCOMPARE(type->qualifiedCppName(), QString("A::B"));
    QCOMPARE(type->targetLangName(), QString("A.B"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.A.B"));

    // C
    a = classes.findClass("C");
    QVERIFY(a);
    QCOMPARE(a->name(), QString("C"));

    type = a->typeEntry();
    QVERIFY(type);
    QCOMPARE(type->name(), QString("C"));
    QCOMPARE(type->qualifiedCppName(), QString("A::B::C"));
    QCOMPARE(type->targetLangName(), QString("A.B.C"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.A.B.C"));

    // D
    a = classes.findClass("D");
    QVERIFY(a);
    QCOMPARE(a->name(), QString("D"));

    type = a->typeEntry();
    QVERIFY(type);
    QCOMPARE(type->name(), QString("D"));
    QCOMPARE(type->qualifiedCppName(), QString("A::B::C::D"));
    QCOMPARE(type->targetLangName(), QString("A.B.C.D"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.A.B.C.D"));
}

void TestNaming::testClassInHiddenNamespace()
{
    const char* cppCode ="namespace Nsp { class ClassName {}; }";
    const char* xmlCode = "<typesystem package=\"Foo\"><namespace-type name=\"Nsp\" generate=\"no\">\
                           <value-type name=\"ClassName\"/></namespace-type></typesystem>";
    TestUtil t(cppCode, xmlCode);
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* a = classes.findClass("ClassName");
    QVERIFY(a);
    QCOMPARE(a->name(), QString("ClassName"));

    TypeEntry* type = a->typeEntry();
    QVERIFY(type);
    QCOMPARE(type->name(), QString("ClassName"));
    QCOMPARE(type->qualifiedCppName(), QString("Nsp::ClassName"));
    QCOMPARE(type->targetLangName(), QString("ClassName"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.ClassName"));

    // Test name of non-generated namespace
    AbstractMetaClass* b = classes.findClass("Nsp");
    QVERIFY(b);
    QCOMPARE(b->name(), QString(""));

    type = b->typeEntry();
    QVERIFY(type);
    QVERIFY(!type->generateCode());

    QCOMPARE(type->name(), QString("Nsp"));
    QCOMPARE(type->qualifiedCppName(), QString("Nsp"));
    QCOMPARE(type->targetLangName(), QString(""));
    QCOMPARE(type->qualifiedTargetLangName(), QString(""));
}

void TestNaming::testEnum()
{
    const char* cppCode ="enum GlobalEnum { A, B};\
                          struct A {\
                              enum ClassEnum { A, B };\
                              void method(ClassEnum);\
                          };\
                          void func(A::ClassEnum);";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <enum-type name='GlobalEnum' />\
        <value-type name='A'> \
            <enum-type name='ClassEnum' />\
        </value-type> \
        <function signature='func(A::ClassEnum)' />\
    </typesystem>";

    TestUtil t(cppCode, xmlCode);

    // Global enum
    AbstractMetaEnumList enums = t.builder()->globalEnums();
    AbstractMetaEnum* e = enums.first();
    QCOMPARE(e->name(), QString("GlobalEnum"));

    TypeEntry* type = e->typeEntry();
    QCOMPARE(type->name(), QString("GlobalEnum"));
    QCOMPARE(type->qualifiedCppName(), QString("GlobalEnum"));
    QCOMPARE(type->targetLangName(), QString("GlobalEnum"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.GlobalEnum"));

    // Class Enum
    AbstractMetaClassList classes = t.builder()->classes();
    e = classes.first()->enums().first();
    QCOMPARE(e->name(), QString("ClassEnum"));

    type = e->typeEntry();
    QCOMPARE(type->name(), QString("ClassEnum"));
    QCOMPARE(type->qualifiedCppName(), QString("A::ClassEnum"));
    QCOMPARE(type->targetLangName(), QString("A.ClassEnum"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.A.ClassEnum"));

    // TODO test the AbstractMetaType
}

void TestNaming::testEnumInNamespace()
{
    const char* cppCode ="namespace Nsp {\
                            enum GlobalEnum { a, b};\
                            struct A {\
                                enum ClassEnum { A, B };\
                                void method(ClassEnum);\
                            };\
                            void func(A::ClassEnum);\
                          }";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <namespace-type name='Nsp'>\
            <enum-type name='GlobalEnum' />\
            <value-type name='A'> \
                <enum-type name='ClassEnum' />\
            </value-type> \
            <function signature='func(A::ClassEnum)' />\
        </namespace-type>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode);

    // Global enum
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* nsp = classes.findClass("Nsp");
    AbstractMetaEnum* e = nsp->enums().first();
    QCOMPARE(e->name(), QString("GlobalEnum"));

    TypeEntry* type = e->typeEntry();
    QCOMPARE(type->name(), QString("GlobalEnum"));
    QCOMPARE(type->qualifiedCppName(), QString("Nsp::GlobalEnum"));
    QCOMPARE(type->targetLangName(), QString("Nsp.GlobalEnum"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.Nsp.GlobalEnum"));

    // Class Enum
    AbstractMetaClass* a = classes.findClass("Nsp::A");
    QVERIFY(a);
    e = a->enums().first();
    QCOMPARE(e->name(), QString("ClassEnum"));

    type = e->typeEntry();
    QCOMPARE(type->name(), QString("ClassEnum"));
    QCOMPARE(type->qualifiedCppName(), QString("Nsp::A::ClassEnum"));
    QCOMPARE(type->targetLangName(), QString("Nsp.A.ClassEnum"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.Nsp.A.ClassEnum"));

    // TODO test the AbstractMetaType
}

void TestNaming::testEnumInHiddenNamespace()
{
    const char* cppCode ="namespace Nsp {\
                            enum GlobalEnum { a, b};\
                            struct A {\
                                enum ClassEnum { A, B };\
                                void method(ClassEnum);\
                            };\
                            void func(A::ClassEnum);\
                          }";
    const char* xmlCode = "\
    <typesystem package=\"Foo\"> \
        <namespace-type name='Nsp'>\
            <enum-type name='GlobalEnum' />\
            <value-type name='A'> \
                <enum-type name='ClassEnum' />\
            </value-type> \
            <function signature='func(A::ClassEnum)' />\
        </namespace-type>\
    </typesystem>";

    TestUtil t(cppCode, xmlCode);

    // Global enum
    AbstractMetaClassList classes = t.builder()->classes();
    AbstractMetaClass* nsp = classes.findClass("Nsp");
    AbstractMetaEnum* e = nsp->enums().first();
    QCOMPARE(e->name(), QString("GlobalEnum"));

    TypeEntry* type = e->typeEntry();
    QCOMPARE(type->name(), QString("GlobalEnum"));
    QCOMPARE(type->qualifiedCppName(), QString("Nsp::GlobalEnum"));
    QCOMPARE(type->targetLangName(), QString("Nsp.GlobalEnum"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.Nsp.GlobalEnum"));

    // Class Enum
    AbstractMetaClass* a = classes.findClass("Nsp::A");
    QVERIFY(a);
    e = a->enums().first();
    QCOMPARE(e->name(), QString("ClassEnum"));

    type = e->typeEntry();
    QCOMPARE(type->name(), QString("ClassEnum"));
    QCOMPARE(type->qualifiedCppName(), QString("Nsp::A::ClassEnum"));
    QCOMPARE(type->targetLangName(), QString("Nsp.A.ClassEnum"));
    QCOMPARE(type->qualifiedTargetLangName(), QString("Foo.Nsp.A.ClassEnum"));

    // TODO test the AbstractMetaType
}

QTEST_APPLESS_MAIN(TestNaming)

#include "testnaming.moc"
