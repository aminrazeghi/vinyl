#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>

class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool alwaysOnTop READ alwaysOnTop WRITE setAlwaysOnTop NOTIFY alwaysOnTopChanged)
    Q_PROPERTY(QString preferredPlayer READ preferredPlayer WRITE setPreferredPlayer NOTIFY preferredPlayerChanged)
    Q_PROPERTY(int windowX READ windowX WRITE setWindowX NOTIFY windowXChanged)
    Q_PROPERTY(int windowY READ windowY WRITE setWindowY NOTIFY windowYChanged)
    Q_PROPERTY(int windowWidth READ windowWidth WRITE setWindowWidth NOTIFY windowWidthChanged)
    Q_PROPERTY(int windowHeight READ windowHeight WRITE setWindowHeight NOTIFY windowHeightChanged)
    Q_PROPERTY(double windowOpacity READ windowOpacity WRITE setWindowOpacity NOTIFY windowOpacityChanged)
    Q_PROPERTY(bool adMuterEnabled READ adMuterEnabled WRITE setAdMuterEnabled NOTIFY adMuterEnabledChanged)

public:
    explicit SettingsManager(QObject *parent = nullptr);

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool value);

    QString preferredPlayer() const;
    void setPreferredPlayer(const QString &value);

    int windowX() const;
    void setWindowX(int value);

    int windowY() const;
    void setWindowY(int value);

    int windowWidth() const;
    void setWindowWidth(int value);

    int windowHeight() const;
    void setWindowHeight(int value);

    double windowOpacity() const;
    void setWindowOpacity(double value);

    bool adMuterEnabled() const;
    void setAdMuterEnabled(bool value);

signals:
    void alwaysOnTopChanged();
    void preferredPlayerChanged();
    void windowXChanged();
    void windowYChanged();
    void windowWidthChanged();
    void windowHeightChanged();
    void windowOpacityChanged();
    void adMuterEnabledChanged();

private:
    QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
