#include "mediacontroller.h"
#include <QDateTime>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusObjectPath>

MediaController::MediaController(MprisManager *mpris, SettingsManager *settings, QObject *parent)
    : QObject(parent)
    , m_mpris(mpris)
    , m_settings(settings)
{
    // Position tracking timer (250ms interval)
    m_positionTimer = new QTimer(this);
    m_positionTimer->setInterval(250);
    connect(m_positionTimer, &QTimer::timeout, this, &MediaController::updatePosition);
    m_positionTimer->start();

    // Connect manager signals
    connect(m_mpris, &MprisManager::playerAdded, this, &MediaController::onPlayerAdded);
    connect(m_mpris, &MprisManager::playerRemoved, this, &MediaController::onPlayerRemoved);
    connect(m_mpris, &MprisManager::playerPropertiesChanged, this, &MediaController::onPlayerPropertiesChanged);

    // Watch settings changes
    connect(m_settings, &SettingsManager::preferredPlayerChanged, this, &MediaController::checkPreferredPlayerSetting);

    updateActivePlayer();
}

QString MediaController::activePlayerName() const
{
    if (m_activePlayer) {
        return m_activePlayer->identity();
    }
    return QStringLiteral("No Player");
}

QString MediaController::activePlayerService() const
{
    if (m_activePlayer) {
        return m_activePlayer->serviceName();
    }
    return QString();
}

QStringList MediaController::availablePlayers() const
{
    return m_mpris->playerNames();
}

QStringList MediaController::availablePlayerIdentities() const
{
    QStringList identities;
    for (MprisPlayer *player : m_mpris->players()) {
        identities.append(player->identity());
    }
    return identities;
}

QString MediaController::trackId() const
{
    if (!m_activePlayer) return QString();
    QVariant v = m_activePlayer->metadata().value("mpris:trackid");
    if (!v.isValid()) return QString();
    if (v.canConvert<QDBusObjectPath>())
        return qvariant_cast<QDBusObjectPath>(v).path();
    return v.toString();
}

QString MediaController::trackTitle() const
{
    if (m_activePlayer) {
        QVariantMap metadata = m_activePlayer->metadata();
        if (metadata.contains("xesam:title")) {
            return metadata.value("xesam:title").toString();
        }
    }
    return QStringLiteral("No Media Playing");
}

QString MediaController::trackArtist() const
{
    if (m_activePlayer) {
        QVariantMap metadata = m_activePlayer->metadata();
        if (metadata.contains("xesam:artist")) {
            QVariant artistVar = metadata.value("xesam:artist");
            if (artistVar.canConvert<QStringList>()) {
                return artistVar.toStringList().join(QStringLiteral(", "));
            } else {
                return artistVar.toString();
            }
        }
    }
    return QStringLiteral("Unknown Artist");
}

QString MediaController::trackAlbum() const
{
    if (m_activePlayer) {
        QVariantMap metadata = m_activePlayer->metadata();
        if (metadata.contains("xesam:album")) {
            return metadata.value("xesam:album").toString();
        }
    }
    return QString();
}

QString MediaController::coverArtUrl() const
{
    if (m_activePlayer) {
        QVariantMap metadata = m_activePlayer->metadata();
        if (metadata.contains("mpris:artUrl")) {
            return metadata.value("mpris:artUrl").toString();
        }
    }
    return QString();
}

qint64 MediaController::positionMs() const
{
    return m_positionMs;
}

qint64 MediaController::durationMs() const
{
    if (m_activePlayer) {
        QVariantMap metadata = m_activePlayer->metadata();
        if (metadata.contains("mpris:length")) {
            return metadata.value("mpris:length").toLongLong() / 1000; // Microseconds to Milliseconds
        }
    }
    return 0;
}

bool MediaController::isPlaying() const
{
    if (m_activePlayer) {
        return m_activePlayer->playbackStatus() == "Playing";
    }
    return false;
}

double MediaController::volume() const
{
    if (m_activePlayer) {
        return m_activePlayer->volume();
    }
    return 1.0;
}

void MediaController::setVolume(double value)
{
    if (m_activePlayer) {
        // Volume property setting in MPRIS is via org.freedesktop.DBus.Properties interface
        // We can write it via QDBusInterface
        QDBusInterface playerInterface(m_activePlayer->serviceName(), "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", QDBusConnection::sessionBus());
        if (playerInterface.isValid()) {
            playerInterface.call("Set", "org.mpris.MediaPlayer2.Player", "Volume", QVariant::fromValue(value));
        }
    }
}

bool MediaController::canGoNext() const
{
    return m_activePlayer ? m_activePlayer->canGoNext() : false;
}

bool MediaController::canGoPrevious() const
{
    return m_activePlayer ? m_activePlayer->canGoPrevious() : false;
}

bool MediaController::canPlay() const
{
    return m_activePlayer ? m_activePlayer->canPlay() : false;
}

bool MediaController::canPause() const
{
    return m_activePlayer ? m_activePlayer->canPause() : false;
}

bool MediaController::canSeek() const
{
    return m_activePlayer ? m_activePlayer->canSeek() : false;
}

void MediaController::play()
{
    if (m_activePlayer) {
        m_activePlayer->play();
    }
}

void MediaController::pause()
{
    if (m_activePlayer) {
        m_activePlayer->pause();
    }
}

void MediaController::playPause()
{
    if (m_activePlayer) {
        m_activePlayer->playPause();
    }
}

void MediaController::next()
{
    if (m_activePlayer) {
        m_activePlayer->next();
    }
}

void MediaController::previous()
{
    if (m_activePlayer) {
        m_activePlayer->previous();
    }
}

void MediaController::seek(qint64 positionMs)
{
    if (m_activePlayer) {
        m_activePlayer->seek(positionMs * 1000); // Milliseconds to Microseconds
        m_positionMs = positionMs;
        m_lastPositionUpdateEpoch = QDateTime::currentMSecsSinceEpoch();
        emit positionMsChanged();
    }
}

void MediaController::selectPlayer(const QString &serviceName)
{
    // Save to settings
    m_settings->setPreferredPlayer(serviceName); // empty string means Automatic
    updateActivePlayer();
}

void MediaController::onPlayerAdded(MprisPlayer *player)
{
    Q_UNUSED(player)
    updateActivePlayer();
    emit availablePlayersChanged();
}

void MediaController::onPlayerRemoved(const QString &serviceName)
{
    Q_UNUSED(serviceName)
    updateActivePlayer();
    emit availablePlayersChanged();
}

void MediaController::onPlayerPropertiesChanged(MprisPlayer *player)
{
    if (player == m_activePlayer) {
        emit playbackStateChanged();
        emit trackMetadataChanged();
        emit volumeChanged();
        emit controlsChanged();
        resyncPosition();
    }

    // If automatic switching is active, a player starting playback should trigger a switch
    if (m_settings->preferredPlayer().isEmpty() && player->playbackStatus() == "Playing" && player != m_activePlayer) {
        updateActivePlayer();
    }
}

void MediaController::updatePosition()
{
    if (!m_activePlayer) {
        if (m_positionMs != 0) {
            m_positionMs = 0;
            emit positionMsChanged();
        }
        return;
    }

    if (m_activePlayer->playbackStatus() == "Playing") {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        qint64 elapsed = now - m_lastPositionUpdateEpoch;
        m_positionMs += elapsed;
        m_lastPositionUpdateEpoch = now;

        // Sync with actual player position every 2 seconds to correct local drift
        m_ticksSinceResync++;
        if (m_ticksSinceResync >= 8) { // 8 * 250ms = 2000ms
            resyncPosition();
        } else {
            emit positionMsChanged();
        }
    } else {
        // Paused/Stopped: sync to stay exact
        resyncPosition();
    }
}

void MediaController::checkPreferredPlayerSetting()
{
    updateActivePlayer();
}

void MediaController::updateActivePlayer()
{
    MprisPlayer *newActive = nullptr;
    QString preferred = m_settings->preferredPlayer();

    // 1. Try to use user-preferred player
    if (!preferred.isEmpty()) {
        newActive = m_mpris->player(preferred);
    }

    // 2. If no preferred player or preferred player not found, auto-switch
    if (!newActive) {
        // Rule A: Find the first player currently playing
        for (MprisPlayer *player : m_mpris->players()) {
            if (player->playbackStatus() == QStringLiteral("Playing")) {
                newActive = player;
                break;
            }
        }

        // Rule B: Find the first player currently paused
        if (!newActive) {
            for (MprisPlayer *player : m_mpris->players()) {
                if (player->playbackStatus() == QStringLiteral("Paused")) {
                    newActive = player;
                    break;
                }
            }
        }

        // Rule C: Fall back to the first available player
        if (!newActive && !m_mpris->players().isEmpty()) {
            newActive = m_mpris->players().first();
        }
    }

    if (m_activePlayer != newActive) {
        m_activePlayer = newActive;
        resyncPosition();
        emit activePlayerChanged();
        emit playbackStateChanged();
        emit trackMetadataChanged();
        emit volumeChanged();
        emit controlsChanged();
    }
}

void MediaController::resyncPosition()
{
    if (m_activePlayer) {
        m_positionMs = m_activePlayer->position() / 1000;
        m_lastPositionUpdateEpoch = QDateTime::currentMSecsSinceEpoch();
        m_ticksSinceResync = 0;
        emit positionMsChanged();
    }
}
