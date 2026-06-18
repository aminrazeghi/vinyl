#ifndef MPRISPLAYER_H
#define MPRISPLAYER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QDBusConnection>

class MprisPlayer : public QObject
{
    Q_OBJECT

public:
    explicit MprisPlayer(const QString &serviceName, QObject *parent = nullptr);
    ~MprisPlayer();

    QString serviceName() const { return m_serviceName; }
    QString identity() const { return m_identity; }
    QString playbackStatus() const { return m_playbackStatus; }
    QVariantMap metadata() const { return m_metadata; }
    qint64 position() const; // Queries current position in microseconds
    double volume() const { return m_volume; }

    bool canGoNext() const { return m_canGoNext; }
    bool canGoPrevious() const { return m_canGoPrevious; }
    bool canPlay() const { return m_canPlay; }
    bool canPause() const { return m_canPause; }
    bool canSeek() const { return m_canSeek; }

    // Media Control Methods
    void play();
    void pause();
    void playPause();
    void next();
    void previous();
    void seek(qint64 positionUs); // Set absolute position in microseconds

signals:
    void propertiesChanged();

private slots:
    void onPropertiesChanged(const QString &interfaceName,
                             const QVariantMap &changedProperties,
                             const QStringList &invalidatedProperties);

private:
    void fetchProperties();

    QString m_serviceName;
    QString m_identity;
    QString m_playbackStatus;
    QVariantMap m_metadata;
    double m_volume = 1.0;
    bool m_canGoNext = false;
    bool m_canGoPrevious = false;
    bool m_canPlay = false;
    bool m_canPause = false;
    bool m_canSeek = false;
};

#endif // MPRISPLAYER_H
