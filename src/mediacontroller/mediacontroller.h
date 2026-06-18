#ifndef MEDIACONTROLLER_H
#define MEDIACONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QTimer>
#include "mpris/mprismanager.h"
#include "settings/settingsmanager.h"

class MediaController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString activePlayerName READ activePlayerName NOTIFY activePlayerChanged)
    Q_PROPERTY(QString activePlayerService READ activePlayerService NOTIFY activePlayerChanged)
    Q_PROPERTY(QStringList availablePlayers READ availablePlayers NOTIFY availablePlayersChanged)
    Q_PROPERTY(QStringList availablePlayerIdentities READ availablePlayerIdentities NOTIFY availablePlayersChanged)

    Q_PROPERTY(QString trackTitle READ trackTitle NOTIFY trackMetadataChanged)
    Q_PROPERTY(QString trackArtist READ trackArtist NOTIFY trackMetadataChanged)
    Q_PROPERTY(QString trackAlbum READ trackAlbum NOTIFY trackMetadataChanged)
    Q_PROPERTY(QString coverArtUrl READ coverArtUrl NOTIFY trackMetadataChanged)

    Q_PROPERTY(qint64 positionMs READ positionMs NOTIFY positionMsChanged)
    Q_PROPERTY(qint64 durationMs READ durationMs NOTIFY trackMetadataChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playbackStateChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)

    Q_PROPERTY(bool canGoNext READ canGoNext NOTIFY controlsChanged)
    Q_PROPERTY(bool canGoPrevious READ canGoPrevious NOTIFY controlsChanged)
    Q_PROPERTY(bool canPlay READ canPlay NOTIFY controlsChanged)
    Q_PROPERTY(bool canPause READ canPause NOTIFY controlsChanged)
    Q_PROPERTY(bool canSeek READ canSeek NOTIFY controlsChanged)

public:
    explicit MediaController(MprisManager *mpris, SettingsManager *settings, QObject *parent = nullptr);

    // Getters
    QString activePlayerName() const;
    QString activePlayerService() const;
    QStringList availablePlayers() const;
    QStringList availablePlayerIdentities() const;

    QString trackId() const;
    QString trackTitle() const;
    QString trackArtist() const;
    QString trackAlbum() const;
    QString coverArtUrl() const;

    qint64 positionMs() const;
    qint64 durationMs() const;
    bool isPlaying() const;
    double volume() const;
    void setVolume(double value);

    bool canGoNext() const;
    bool canGoPrevious() const;
    bool canPlay() const;
    bool canPause() const;
    bool canSeek() const;

    // Media Controls
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void playPause();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void seek(qint64 positionMs);
    Q_INVOKABLE void selectPlayer(const QString &serviceName);

signals:
    void activePlayerChanged();
    void availablePlayersChanged();
    void trackMetadataChanged();
    void positionMsChanged();
    void playbackStateChanged();
    void volumeChanged();
    void controlsChanged();

private slots:
    void onPlayerAdded(MprisPlayer *player);
    void onPlayerRemoved(const QString &serviceName);
    void onPlayerPropertiesChanged(MprisPlayer *player);
    void updatePosition();
    void checkPreferredPlayerSetting();

private:
    void updateActivePlayer();
    void connectActivePlayer();
    void disconnectActivePlayer();
    void resyncPosition();

    MprisManager *m_mpris;
    SettingsManager *m_settings;
    MprisPlayer *m_activePlayer = nullptr;

    QTimer *m_positionTimer;
    qint64 m_positionMs = 0;
    qint64 m_lastPositionUpdateEpoch = 0;
    int m_ticksSinceResync = 0;
};

#endif // MEDIACONTROLLER_H
