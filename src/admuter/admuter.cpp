#include "admuter.h"
#include "mediacontroller/mediacontroller.h"
#include <QProcess>
#include <QDebug>

AdMuter::AdMuter(MediaController *controller, QObject *parent)
    : QObject(parent)
    , m_controller(controller)
{
    connect(controller, &MediaController::trackMetadataChanged,
            this, &AdMuter::onMetadataChanged);
}

void AdMuter::setEnabled(bool value)
{
    if (m_enabled == value) return;
    m_enabled = value;
    emit enabledChanged();

    if (!m_enabled) {
        if (m_isAd)
            applyMute(false);
    } else {
        onMetadataChanged();
    }
}

void AdMuter::onMetadataChanged()
{
    QString trackId = m_controller->trackId();
    QString title   = m_controller->trackTitle();

    if (trackId == m_lastTrackId) return;
    m_lastTrackId = trackId;

    bool ad = detectAd(trackId, title);
    if (ad != m_isAd) {
        m_isAd = ad;
        emit isAdChanged();
    }

    if (m_enabled)
        applyMute(ad);
}

bool AdMuter::detectAd(const QString &trackId, const QString &title) const
{
    if (trackId.isEmpty()) return false;

    if (trackId.contains(":ad") || trackId.contains("/ad/"))
        return true;

    // Any track ID that isn't a normal track/episode/local file is an ad
    if (!trackId.contains("track") && !trackId.contains("episode") &&
        !trackId.contains("local") && trackId != "mpris:trackid")
        return true;

    if (title == "Advertisement" || title == "Spotify")
        return true;

    return false;
}

QStringList AdMuter::findSpotifySinkInputs() const
{
    QProcess pactl;
    pactl.start("pactl", {"list", "sink-inputs"});
    if (!pactl.waitForFinished(3000))
        return {};

    QStringList sinkIds;
    QString currentSink;
    bool isSpotify = false;

    for (const QString &line : pactl.readAllStandardOutput().split('\n')) {
        if (line.startsWith("Sink Input #")) {
            if (!currentSink.isEmpty() && isSpotify)
                sinkIds.append(currentSink);
            currentSink = line.mid(12).trimmed();
            isSpotify = false;
        } else if (line.contains("application.name = \"Spotify\"") ||
                   line.contains("media.name = \"Spotify\"") ||
                   line.contains("application.process.binary = \"spotify\"")) {
            isSpotify = true;
        }
    }
    if (!currentSink.isEmpty() && isSpotify)
        sinkIds.append(currentSink);

    return sinkIds;
}

void AdMuter::applyMute(bool mute)
{
    const QString muteVal = mute ? "1" : "0";
    for (const QString &sink : findSpotifySinkInputs())
        QProcess::execute("pactl", {"set-sink-input-mute", sink, muteVal});
}
