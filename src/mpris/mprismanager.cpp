#include "mprismanager.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QDebug>

MprisManager::MprisManager(QObject *parent)
    : QObject(parent)
{
    // Connect to NameOwnerChanged to watch for players starting/stopping
    bool connected = QDBusConnection::sessionBus().connect(
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        "org.freedesktop.DBus",
        "NameOwnerChanged",
        this,
        SLOT(onNameOwnerChanged(QString,QString,QString))
    );

    if (!connected) {
        qWarning() << "Failed to connect to NameOwnerChanged signal";
    }

    discoverPlayers();
}

MprisManager::~MprisManager()
{
    QDBusConnection::sessionBus().disconnect(
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        "org.freedesktop.DBus",
        "NameOwnerChanged",
        this,
        SLOT(onNameOwnerChanged(QString,QString,QString))
    );

    qDeleteAll(m_players);
    m_players.clear();
}

QStringList MprisManager::playerNames() const
{
    return m_players.keys();
}

void MprisManager::onNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    if (!name.startsWith("org.mpris.MediaPlayer2.")) {
        return;
    }

    if (newOwner.isEmpty()) {
        // Player closed
        removePlayer(name);
    } else if (oldOwner.isEmpty()) {
        // New player registered
        addPlayer(name);
    } else {
        // Ownership transferred / restarted
        removePlayer(name);
        addPlayer(name);
    }
}

void MprisManager::onPlayerPropertiesChanged()
{
    MprisPlayer *player = qobject_cast<MprisPlayer*>(sender());
    if (player) {
        emit playerPropertiesChanged(player);
    }
}

void MprisManager::discoverPlayers()
{
    // Query D-Bus daemon for active player names
    QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
    if (interface) {
        QDBusReply<QStringList> namesReply = interface->registeredServiceNames();
        if (namesReply.isValid()) {
            for (const QString &name : namesReply.value()) {
                if (name.startsWith("org.mpris.MediaPlayer2.")) {
                    addPlayer(name);
                }
            }
        }
    }
}

void MprisManager::addPlayer(const QString &serviceName)
{
    if (m_players.contains(serviceName)) {
        return;
    }

    MprisPlayer *player = new MprisPlayer(serviceName, this);
    m_players.insert(serviceName, player);

    connect(player, &MprisPlayer::propertiesChanged, this, &MprisManager::onPlayerPropertiesChanged);

    emit playerAdded(player);
}

void MprisManager::removePlayer(const QString &serviceName)
{
    if (!m_players.contains(serviceName)) {
        return;
    }

    MprisPlayer *player = m_players.take(serviceName);
    emit playerRemoved(serviceName);
    player->deleteLater();
}
