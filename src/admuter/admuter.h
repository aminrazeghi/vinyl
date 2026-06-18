#ifndef ADMUTER_H
#define ADMUTER_H

#include <QObject>
#include <QString>
#include <QStringList>

class MediaController;

class AdMuter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool isAd READ isAd NOTIFY isAdChanged)

public:
    explicit AdMuter(MediaController *controller, QObject *parent = nullptr);

    bool enabled() const { return m_enabled; }
    void setEnabled(bool value);

    bool isAd() const { return m_isAd; }

signals:
    void enabledChanged();
    void isAdChanged();

private slots:
    void onMetadataChanged();

private:
    bool detectAd(const QString &trackId, const QString &title) const;
    QStringList findSpotifySinkInputs() const;
    void applyMute(bool mute);

    MediaController *m_controller;
    bool m_enabled = false;
    bool m_isAd = false;
    QString m_lastTrackId;
};

#endif // ADMUTER_H
