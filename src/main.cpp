#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QWindow>
#include <QIcon>
#include <QDebug>
#include <QCoreApplication>

#include "settings/settingsmanager.h"
#include "mpris/mprismanager.h"
#include "mediacontroller/mediacontroller.h"
#include "admuter/admuter.h"

int main(int argc, char *argv[])
{
    // Force XWayland so Qt.WindowStaysOnTopHint and window opacity work.
    // The native Wayland compositor (GNOME) silently ignores these hints.
    qputenv("QT_QPA_PLATFORM", "xcb");

    // QApplication is required since we use QSystemTrayIcon and QMenu (QtWidgets)
    QApplication app(argc, argv);
    app.setApplicationName("VinylVisualizer");
    app.setOrganizationName("VinylApp");

    // Instantiate Modules
    SettingsManager settings;
    MprisManager mprisManager;
    MediaController mediaController(&mprisManager, &settings);
    AdMuter adMuter(&mediaController);
    adMuter.setEnabled(settings.adMuterEnabled());
    QObject::connect(&settings, &SettingsManager::adMuterEnabledChanged,
                     [&]() { adMuter.setEnabled(settings.adMuterEnabled()); });
    QObject::connect(&adMuter, &AdMuter::enabledChanged,
                     [&]() { settings.setAdMuterEnabled(adMuter.enabled()); });

    // Initialize QML Engine
    QQmlApplicationEngine engine;

    // Register C++ Singletons/Instances to QML Context
    engine.rootContext()->setContextProperty("settings", &settings);
    engine.rootContext()->setContextProperty("mediaController", &mediaController);
    engine.rootContext()->setContextProperty("adMuter", &adMuter);

    // Load Main QML File
    const QUrl url(QStringLiteral("qrc:/VinylApp/src/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    QObject *rootObject = engine.rootObjects().first();
    QWindow *mainWindow = qobject_cast<QWindow*>(rootObject);

    // Create System Tray Icon
    QSystemTrayIcon trayIcon(&app);
    
    // Set tray icon (using default cover from resources as tray fallback)
    QIcon appIcon = QIcon::fromTheme("audio-x-generic", QIcon(":/VinylApp/assets/default_cover.png"));
    trayIcon.setIcon(appIcon);
    trayIcon.setToolTip("Vinyl Visualizer");

    // Create Tray Menu
    QMenu trayMenu;

    QAction *toggleVisibleAction = trayMenu.addAction("Show Window");
    QObject::connect(toggleVisibleAction, &QAction::triggered, [mainWindow, toggleVisibleAction]() {
        if (mainWindow) {
            if (mainWindow->isVisible()) {
                mainWindow->hide();
                toggleVisibleAction->setText("Show Window");
            } else {
                mainWindow->show();
                mainWindow->raise();
                mainWindow->requestActivate();
                toggleVisibleAction->setText("Hide Window");
            }
        }
    });

    QAction *alwaysOnTopAction = trayMenu.addAction("Always on Top");
    alwaysOnTopAction->setCheckable(true);
    alwaysOnTopAction->setChecked(settings.alwaysOnTop());
    QObject::connect(alwaysOnTopAction, &QAction::triggered, [&settings](bool checked) {
        settings.setAlwaysOnTop(checked);
    });

    // Keep checkmark in sync if toggled from QML UI, and raise window immediately
    QObject::connect(&settings, &SettingsManager::alwaysOnTopChanged, [&settings, alwaysOnTopAction, mainWindow]() {
        alwaysOnTopAction->setChecked(settings.alwaysOnTop());
        if (mainWindow && settings.alwaysOnTop())
            mainWindow->raise();
    });

    trayMenu.addSeparator();

    // Media Controls in Tray
    QAction *playPauseAction = trayMenu.addAction("Play");
    QObject::connect(playPauseAction, &QAction::triggered, &mediaController, &MediaController::playPause);

    QAction *prevAction = trayMenu.addAction("Previous");
    QObject::connect(prevAction, &QAction::triggered, &mediaController, &MediaController::previous);

    QAction *nextAction = trayMenu.addAction("Next");
    QObject::connect(nextAction, &QAction::triggered, &mediaController, &MediaController::next);

    trayMenu.addSeparator();

    QAction *quitAction = trayMenu.addAction("Quit");
    QObject::connect(quitAction, &QAction::triggered, &app, &QCoreApplication::quit);

    trayIcon.setContextMenu(&trayMenu);
    trayIcon.show();

    // Dynamically update tray actions state
    auto updateTrayMenu = [&]() {
        // Toggle play/pause text
        if (mediaController.isPlaying()) {
            playPauseAction->setText("Pause");
        } else {
            playPauseAction->setText("Play");
        }
        
        // Enable/Disable based on MPRIS capabilities
        playPauseAction->setEnabled(mediaController.isPlaying() ? mediaController.canPause() : mediaController.canPlay());
        prevAction->setEnabled(mediaController.canGoPrevious());
        nextAction->setEnabled(mediaController.canGoNext());

        // Update Tooltip
        QString title = mediaController.trackTitle();
        QString artist = mediaController.trackArtist();
        if (title == "No Media Playing") {
            trayIcon.setToolTip("Vinyl Visualizer");
        } else {
            trayIcon.setToolTip(QString("%1 - %2").arg(title, artist));
        }

        // Update Show/Hide text based on state
        if (mainWindow) {
            if (mainWindow->isVisible()) {
                toggleVisibleAction->setText("Hide Window");
            } else {
                toggleVisibleAction->setText("Show Window");
            }
        }
    };

    // Connect update triggers
    QObject::connect(&mediaController, &MediaController::playbackStateChanged, updateTrayMenu);
    QObject::connect(&mediaController, &MediaController::trackMetadataChanged, updateTrayMenu);
    QObject::connect(&mediaController, &MediaController::controlsChanged, updateTrayMenu);

    // Initial update
    updateTrayMenu();

    // Double-click/Trigger tray icon toggles window visibility
    QObject::connect(&trayIcon, &QSystemTrayIcon::activated, [mainWindow, toggleVisibleAction](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger) {
            if (mainWindow) {
                if (mainWindow->isVisible()) {
                    mainWindow->hide();
                    toggleVisibleAction->setText("Show Window");
                } else {
                    mainWindow->show();
                    mainWindow->raise();
                    mainWindow->requestActivate();
                    toggleVisibleAction->setText("Hide Window");
                }
            }
        }
    });

    return app.exec();
}
