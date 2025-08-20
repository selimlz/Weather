#ifndef TIMEBACKEND_H
#define TIMEBACKEND_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>
#include <QPair>
#include <QTimer>

class TimeBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString timeString READ timeString NOTIFY timeUpdated)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    explicit TimeBackend(QObject *parent = nullptr);
    ~TimeBackend();

    QString timeString() const;
    bool loading() const;
    Q_INVOKABLE QVariantMap getCoordinates(const QString &country);

public slots:
    void fetchTimeData(const QString &country);
    void startAutoUpdate(int intervalSeconds = 60);
    void stopAutoUpdate();
    void updateLocalTime();

signals:
    void timeUpdated();
    void loadingChanged();
    void errorOccurred(const QString &message);

private slots:
    void handleTimeReply(QNetworkReply *reply);

private:
    QTimer *m_updateTimer;
    QNetworkAccessManager *m_networkManager;
    QString m_timeString;
    bool m_loading;
    QMap<QString, QPair<QString, QString>> m_countryCoordinates;
    QString m_currentCountry;
    int m_timezoneOffsetSec = 0;
    QTimer *m_localTimeTimer;
    QDateTime m_lastSyncedTime;
    QString m_timezone;

    int m_timezoneOffset = 0;
    QString m_timezoneName;
    QDateTime m_lastApiTime;

    void initializeCountryData();
    void parseTimeResponse(const QByteArray &data);
};

#endif // TIMEBACKEND_H
