// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtCore/QCommandLineParser>
#include <QDir>
#include "src/effectmanager.h"
#include "src/nodeview.h"
#include "src/propertyhandler.h"
#include "src/fpshelper.h"
#include "src/syntaxhighlighter.h"
#include "src/qsbinspectorhelper.h"

#ifdef _WIN32
#include <Windows.h>
#endif

// QQEM version number which is shown in About dialog and saved into files.
// Note: Use string which can be converted to decimal number.
// So e.g. "0.41" and no "0.41.2", "0.41beta" etc.
#define APP_VERSION_STR "0.41"

int main(int argc, char *argv[])
{
#ifdef _WIN32
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        FILE *out, *err;
        freopen_s(&out, "CONOUT$", "w", stdout);
        freopen_s(&err, "CONOUT$", "w", stderr);
    }
#endif
    QGuiApplication app(argc, argv);
    app.setOrganizationName("The Qt Company");
    app.setOrganizationDomain("qt.io");
    app.setApplicationName("Qt Quick Effect Maker");
    app.setApplicationVersion(QLatin1String(APP_VERSION_STR));

    // Setup command line arguments
    QCommandLineParser cmdLineParser;
    cmdLineParser.setApplicationDescription("Qt Quick Effect Maker - Tool to create custom Qt Quick shader effects.");
    cmdLineParser.addHelpOption();
    cmdLineParser.addVersionOption();

    // Add the cmd options
    cmdLineParser.addPositionalArgument("project", QStringLiteral("Effects project file (*.qep) to open"));
    QCommandLineOption exportPathOption("exportpath",
                                        "Path used for exporting the effect",
                                        "exportpath");
    QCommandLineOption createOption("create",
                                   "Create a new project with the given project file");
    cmdLineParser.addOptions({exportPathOption, createOption});

    cmdLineParser.process(app.arguments());
    const QStringList args = cmdLineParser.positionalArguments();
    // Parsing args
    if (args.count() > 0) {
        QString projectPath = QDir::fromNativeSeparators(args.at(0));
        g_argData.insert("effects_project_path", projectPath);
    }
    if (cmdLineParser.isSet(exportPathOption)) {
        auto val = cmdLineParser.value(exportPathOption);
        QString exportPath = QDir::fromNativeSeparators(val);
        g_argData.insert("export_path", exportPath);
    }
    bool createProject = cmdLineParser.isSet(createOption);
    if (createProject) {
        if (args.isEmpty()) {
            qWarning("Error: Creating a new project requires also the project parameter.");
            exit(1);
        }
        g_argData.insert("create_project", createProject);
    }

    qmlRegisterType<EffectManager>("QQEMLib", 1, 0, "EffectManager");
    qmlRegisterType<NodeView>("QQEMLib", 1, 0, "NodeViewItem");
    qmlRegisterType<FpsHelper>("QQEMLib", 1, 0, "FpsHelper");
    qmlRegisterType<SyntaxHighlighter>("QQEMLib", 1, 0, "SyntaxHighlighter");
    qmlRegisterType<QsbInspectorHelper>("QQEMLib", 1, 0, "QsbInspectorHelper");

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();
    if (context) {
        context->setContextProperty("g_argData", &g_argData);
        context->setContextProperty("g_propertyData", &g_propertyData);
        context->setContextProperty("buildQtVersion", QLatin1String(QT_VERSION_STR));
    } else {
        qWarning("Unable to get access into root QQmlContext!");
    }

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
