#include "mprisplayer.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusArgument>
#include <QDebug>

MprisPlayer::MprisPlayer(const QString &serviceName, QObject *parent)
    : QObject(parent)
    , m_serviceName(serviceName)
{
    // Connect to properties changes
    bool connected = QDBusConnection::sessionBus().connect(
        m_serviceName,
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        this,
        SLOT(onPropertiesChanged(QString,QVariantMap,QStringList))
    );

    if (!connected) {
        qWarning() << "Failed to connect to PropertiesChanged signal for" << m_serviceName;
    }

    fetchProperties();
}

MprisPlayer::~MprisPlayer()
{
    QDBusConnection::sessionBus().disconnect(
        m_serviceName,
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        this,
        SLOT(onPropertiesChanged(QString,QVariantMap,QStringList))
    );
}

qint64 MprisPlayer::position() const
{
    // Query Position on demand since it is not updated via PropertiesChanged signals
    QDBusInterface playerInterface(m_serviceName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", QDBusConnection::sessionBus());
    if (playerInterface.isValid()) {
        QVariant pos = playerInterface.property("Position");
        if (pos.isValid()) {
            return pos.toLongLong();
        }
    }
    return 0;
}

void MprisPlayer::play()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(m_serviceName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Play");
    QDBusConnection::sessionBus().send(msg);
}

void MprisPlayer::pause()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(m_serviceName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Pause");
    QDBusConnection::sessionBus().send(msg);
}

void MprisPlayer::playPause()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(m_serviceName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "PlayPause");
    QDBusConnection::sessionBus().send(msg);
}

void MprisPlayer::next()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(m_serviceName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Next");
    QDBusConnection::sessionBus().send(msg);
}

void MprisPlayer::previous()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(m_serviceName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Previous");
    QDBusConnection::sessionBus().send(msg);
}

void MprisPlayer::seek(qint64 positionUs)
{
    QVariant trackIdVar = m_metadata.value("mpris:trackid");
    QDBusObjectPath trackId("/");
    if (trackIdVar.isValid()) {
        if (trackIdVar.canConvert<QDBusObjectPath>()) {
            trackId = qvariant_cast<QDBusObjectPath>(trackIdVar);
        } else {
            trackId = QDBusObjectPath(trackIdVar.toString());
        }
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(m_serviceName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "SetPosition");
    msg << QVariant::fromValue(trackId) << positionUs;
    QDBusConnection::sessionBus().send(msg);
}

void MprisPlayer::onPropertiesChanged(const QString &interfaceName,
                                     const QVariantMap &changedProperties,
                                     const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties)

    if (interfaceName != "org.mpris.MediaPlayer2.Player") {
        return;
    }

    bool updated = false;

    if (changedProperties.contains("PlaybackStatus")) {
        m_playbackStatus = changedProperties.value("PlaybackStatus").toString();
        updated = true;
    }
    if (changedProperties.contains("Metadata")) {
        m_metadata = qdbus_cast<QVariantMap>(changedProperties.value("Metadata"));
        updated = true;
    }
    if (changedProperties.contains("Volume")) {
        m_volume = changedProperties.value("Volume").toDouble();
        updated = true;
    }
    if (changedProperties.contains("CanGoNext")) {
        m_canGoNext = changedProperties.value("CanGoNext").toBool();
        updated = true;
    }
    if (changedProperties.contains("CanGoPrevious")) {
        m_canGoPrevious = changedProperties.value("CanGoPrevious").toBool();
        updated = true;
    }
    if (changedProperties.contains("CanPlay")) {
        m_canPlay = changedProperties.value("CanPlay").toBool();
        updated = true;
    }
    if (changedProperties.contains("CanPause")) {
        m_canPause = changedProperties.value("CanPause").toBool();
        updated = true;
    }
    if (changedProperties.contains("CanSeek")) {
        m_canSeek = changedProperties.value("CanSeek").toBool();
        updated = true;
    }

    if (updated) {
        emit propertiesChanged();
    }
}

void MprisPlayer::fetchProperties()
{
    // Fetch Identity
    QDBusInterface rootInterface(m_serviceName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", QDBusConnection::sessionBus());
    if (rootInterface.isValid()) {
        QDBusReply<QVariant> identityReply = rootInterface.call("Get", "org.mpris.MediaPlayer2", "Identity");
        if (identityReply.isValid()) {
            m_identity = identityReply.value().toString();
        }
    }
    if (m_identity.isEmpty()) {
        m_identity = m_serviceName.mid(QString("org.mpris.MediaPlayer2.").length());
    }

    // Fetch Player Properties
    QDBusInterface playerInterface(m_serviceName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", QDBusConnection::sessionBus());
    if (playerInterface.isValid()) {
        QDBusReply<QVariantMap> allProperties = playerInterface.call("GetAll", "org.mpris.MediaPlayer2.Player");
        if (allProperties.isValid()) {
            QVariantMap props = allProperties.value();
            m_playbackStatus = props.value("PlaybackStatus", "Stopped").toString();
            m_volume = props.value("Volume", 1.0).toDouble();
            m_canGoNext = props.value("CanGoNext", false).toBool();
            m_canGoPrevious = props.value("CanGoPrevious", false).toBool();
            m_canPlay = props.value("CanPlay", false).toBool();
            m_canPause = props.value("CanPause", false).toBool();
            m_canSeek = props.value("CanSeek", false).toBool();
            if (props.contains("Metadata")) {
                m_metadata = qdbus_cast<QVariantMap>(props.value("Metadata"));
            }
        }
    }
}
