#ifndef MPRISMANAGER_H
#define MPRISMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include "mprisplayer.h"

class MprisManager : public QObject
{
    Q_OBJECT

public:
    explicit MprisManager(QObject *parent = nullptr);
    ~MprisManager();

    QList<MprisPlayer*> players() const { return m_players.values(); }
    MprisPlayer* player(const QString &serviceName) const { return m_players.value(serviceName, nullptr); }
    QStringList playerNames() const;

signals:
    void playerAdded(MprisPlayer *player);
    void playerRemoved(const QString &serviceName);
    void playerPropertiesChanged(MprisPlayer *player);

private slots:
    void onNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
    void onPlayerPropertiesChanged();

private:
    void discoverPlayers();
    void addPlayer(const QString &serviceName);
    void removePlayer(const QString &serviceName);

    QMap<QString, MprisPlayer*> m_players;
};

#endif // MPRISMANAGER_H
