#ifndef WEATHERBACKEND_H
#define WEATHERBACKEND_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>

class WeatherBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList countries READ countries NOTIFY countriesLoaded)
    Q_PROPERTY(QString cityName READ cityName NOTIFY weatherUpdated)
    Q_PROPERTY(QString temperature READ temperature NOTIFY weatherUpdated)
    Q_PROPERTY(QString condition READ condition NOTIFY weatherUpdated)
    Q_PROPERTY(QString iconUrl READ iconUrl NOTIFY weatherUpdated)
    Q_PROPERTY(QString windSpeed READ windSpeed NOTIFY weatherUpdated)
    Q_PROPERTY(QString humidity READ humidity NOTIFY weatherUpdated)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)

public:
    explicit WeatherBackend(QObject *parent = nullptr);
    ~WeatherBackend();

    QStringList countries() const;
    QString cityName() const;
    QString temperature() const;
    QString iconUrl() const;
    QString humidity() const;
    QString windSpeed() const;
    QString condition() const;
    bool loading() const;

    QString language() const;
    Q_INVOKABLE void setLanguage(const QString &lang);

public slots:
    void fetchWeather(const QString &location);
    void loadCountries();

signals:
    void countriesLoaded();
    void weatherUpdated();
    void loadingChanged();
    void errorOccurred(const QString &message);
    void languageChanged();


private slots:
    void onWeatherReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    QStringList m_countries;
    QString m_cityName;
    QString m_temperature;
    QString m_iconUrl;
    QString m_humidity;
    QString m_windSpeed;
    QString m_condition;
    bool m_loading;

    QString m_language = QStringLiteral("en");



    void parseWeatherData(const QByteArray &data);
    void resetData();
};

#endif // WEATHERBACKEND_H
