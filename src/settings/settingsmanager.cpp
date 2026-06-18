#include "settingsmanager.h"

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings("GroovrApp", "Groovr")
{
}

bool SettingsManager::alwaysOnTop() const
{
    return m_settings.value("alwaysOnTop", false).toBool();
}

void SettingsManager::setAlwaysOnTop(bool value)
{
    if (alwaysOnTop() != value) {
        m_settings.setValue("alwaysOnTop", value);
        emit alwaysOnTopChanged();
    }
}

QString SettingsManager::preferredPlayer() const
{
    return m_settings.value("preferredPlayer", "").toString();
}

void SettingsManager::setPreferredPlayer(const QString &value)
{
    if (preferredPlayer() != value) {
        m_settings.setValue("preferredPlayer", value);
        emit preferredPlayerChanged();
    }
}

int SettingsManager::windowX() const
{
    return m_settings.value("windowX", -1).toInt();
}

void SettingsManager::setWindowX(int value)
{
    if (windowX() != value) {
        m_settings.setValue("windowX", value);
        emit windowXChanged();
    }
}

int SettingsManager::windowY() const
{
    return m_settings.value("windowY", -1).toInt();
}

void SettingsManager::setWindowY(int value)
{
    if (windowY() != value) {
        m_settings.setValue("windowY", value);
        emit windowYChanged();
    }
}

int SettingsManager::windowWidth() const
{
    return m_settings.value("windowWidth", 400).toInt();
}

void SettingsManager::setWindowWidth(int value)
{
    if (windowWidth() != value) {
        m_settings.setValue("windowWidth", value);
        emit windowWidthChanged();
    }
}

int SettingsManager::windowHeight() const
{
    return m_settings.value("windowHeight", 550).toInt();
}

void SettingsManager::setWindowHeight(int value)
{
    if (windowHeight() != value) {
        m_settings.setValue("windowHeight", value);
        emit windowHeightChanged();
    }
}

double SettingsManager::windowOpacity() const
{
    return m_settings.value("windowOpacity", 1.0).toDouble();
}

void SettingsManager::setWindowOpacity(double value)
{
    if (windowOpacity() != value) {
        m_settings.setValue("windowOpacity", value);
        emit windowOpacityChanged();
    }
}

bool SettingsManager::adMuterEnabled() const
{
    return m_settings.value("adMuterEnabled", false).toBool();
}

void SettingsManager::setAdMuterEnabled(bool value)
{
    if (adMuterEnabled() != value) {
        m_settings.setValue("adMuterEnabled", value);
        emit adMuterEnabledChanged();
    }
}
