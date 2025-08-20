#include "weatherbackend.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <iostream>

namespace {


inline const QString &httpsPrefix()
{
    static const QString v = QStringLiteral("https:");
    return v;
}

inline const QString &apiBase()
{
    static const QString v = QStringLiteral("https://api.weatherapi.com/v1/current.json");
    return v;
}

inline const QString &apiKey()
{
    static const QString v = qEnvironmentVariable("WEATHER_API_KEY", QStringLiteral("20931f22c7fb468382b85345250408"));
    return v;
}

inline const QString &placeholder()
{
    static const QString v = QStringLiteral("N/A");
    return v;
}

inline const QString &unitTemp()
{
    static const QString v = QStringLiteral("°C");
    return v;
}

inline const QString &unitWind()
{
    static const QString v = QStringLiteral("Km/h");
    return v;
}

inline const QString &unitHumidity()
{
    static const QString v = QStringLiteral("%");
    return v;
}

inline const QStringList &countriesList()
{
    static const QStringList list{
        " ",
        "Afghanistan", "Albania", "Algeria", "Andorra", "Angola", "Argentina", "Armenia",
        "Australia", "Austria", "Azerbaijan", "Bahamas", "Bahrain", "Bangladesh", "Barbados",
        "Belarus", "Belgium", "Belize", "Benin", "Bhutan", "Bolivia", "Bosnia and Herzegovina",
        "Botswana", "Brazil", "Brunei", "Bulgaria", "Burkina Faso", "Burundi", "Cambodia",
        "Cameroon", "Canada", "Cape Verde", "Central African Republic", "Chad", "Chile",
        "China", "Colombia", "Comoros", "Congo", "Costa Rica", "Croatia", "Cuba", "Cyprus",
        "Czech Republic", "Denmark", "Djibouti", "Dominica", "Dominican Republic", "East Timor",
        "Ecuador", "Egypt", "El Salvador", "Equatorial Guinea", "Eritrea", "Estonia", "Eswatini",
        "Ethiopia", "Fiji", "Finland", "France", "Gabon", "Gambia", "Georgia", "Germany",
        "Ghana", "Greece", "Grenada", "Guatemala", "Guinea", "Guinea-Bissau", "Guyana", "Haiti",
        "Honduras", "Hungary", "Iceland", "India", "Indonesia", "Iran", "Iraq", "Ireland",
        "Israel", "Italy", "Ivory Coast", "Jamaica", "Japan", "Jordan", "Kazakhstan", "Kenya",
        "Kiribati", "Korea North", "Korea South", "Kosovo", "Kuwait", "Kyrgyzstan", "Laos",
        "Latvia", "Lebanon", "Lesotho", "Liberia", "Libya", "Liechtenstein", "Lithuania",
        "Luxembourg", "Madagascar", "Malawi", "Malaysia", "Maldives", "Mali", "Malta",
        "Marshall Islands", "Mauritania", "Mauritius", "Mexico", "Micronesia", "Moldova",
        "Monaco", "Mongolia", "Montenegro", "Morocco", "Mozambique", "Myanmar", "Namibia",
        "Nauru", "Nepal", "Netherlands", "New Zealand", "Nicaragua", "Niger", "Nigeria",
        "North Macedonia", "Norway", "Oman", "Pakistan", "Palau", "Panama", "Papua New Guinea",
        "Paraguay", "Peru", "Philippines", "Poland", "Portugal", "Qatar", "Romania", "Russia",
        "Rwanda", "Saint Kitts and Nevis", "Saint Lucia", "Saint Vincent and the Grenadines",
        "Samoa", "San Marino", "Sao Tome and Principe", "Saudi Arabia", "Senegal", "Serbia",
        "Seychelles", "Sierra Leone", "Singapore", "Slovakia", "Slovenia", "Solomon Islands",
        "Somalia", "South Africa", "South Sudan", "Spain", "Sri Lanka", "Sudan", "Suriname",
        "Sweden", "Switzerland", "Syria", "Taiwan", "Tajikistan", "Tanzania", "Thailand",
        "Togo", "Tonga", "Trinidad and Tobago", "Tunisia", "Turkey", "Turkmenistan", "Tuvalu",
        "Uganda", "Ukraine", "United Arab Emirates", "United Kingdom", "United States", "Uruguay",
        "Uzbekistan", "Vanuatu", "Vatican City", "Venezuela", "Vietnam", "Yemen", "Zambia",
        "Zimbabwe"
    };
    return list;
}



}

using namespace std;

WeatherBackend::WeatherBackend(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_loading(false)
{
    resetData();

    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &WeatherBackend::onWeatherReply);

    // Initialize with default countries
    m_countries = countriesList();
}

void WeatherBackend::resetData()
{
    m_cityName = placeholder();
    m_temperature = placeholder();
    m_condition = placeholder();
}

WeatherBackend::~WeatherBackend()
{
    // Clean up network manager
    if (m_networkManager) {
        delete m_networkManager;
        m_networkManager = nullptr;
    }
}

QStringList WeatherBackend::countries() const
{
    return m_countries;
}

QString WeatherBackend::temperature() const
{
    return m_temperature;
}

QString WeatherBackend::iconUrl() const
{
    return m_iconUrl;
}

QString WeatherBackend::humidity() const
{
    return m_humidity;
}

QString WeatherBackend::windSpeed() const
{
    return m_windSpeed;
}

QString WeatherBackend::cityName() const
{
    return m_cityName;
}

QString WeatherBackend::condition() const
{
    return m_condition;
}

bool WeatherBackend::loading() const
{
    return m_loading;
}

QString WeatherBackend::language() const
{
    return m_language;
}

void WeatherBackend::setLanguage(const QString &lang)
{
    m_language = lang;
    emit languageChanged();
}


void WeatherBackend::fetchWeather(const QString &country)
{
    cout << country.toStdString() << std::endl;

    if (apiKey().isEmpty()) {
        emit errorOccurred("Set WEATHER_API_KEY");
        return;
    }

    const QString apiUrl = QStringLiteral("%1?key=%2&q=%3&aqi=no&lang=%4")
                               .arg(apiBase(), apiKey(), country, m_language);

    m_networkManager->get(QNetworkRequest(QUrl(apiUrl)));
}

void WeatherBackend::onWeatherReply(QNetworkReply *reply)
{
    m_loading = false;
    emit loadingChanged();

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    parseWeatherData(reply->readAll());
    reply->deleteLater();
}

void WeatherBackend::parseWeatherData(const QByteArray &data)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred("JSON parse error");
        return;
    }

    QJsonObject json = doc.object();
    bool hasUpdates = false;

    // Extract location
    if (json.contains("location") && json["location"].isObject()) {
        QJsonObject location = json["location"].toObject();
        if (location.contains("name")) {
            m_cityName = location["name"].toString();
            hasUpdates = true;
        }
    }

    // Extract weather data
    if (json.contains("current") && json["current"].isObject()) {
        QJsonObject current = json["current"].toObject();

        if (current.contains("temp_c")) {
            m_temperature = QString::number(current["temp_c"].toDouble()) + unitTemp();
            hasUpdates = true;
        }

        if (current.contains("condition") && json["current"].toObject()["condition"].isObject()) {
            QJsonObject condition = current["condition"].toObject();
            if (condition.contains("text")) {
                m_condition = condition["text"].toString();
                hasUpdates = true;
            }

            if (condition.contains("icon")) {
                m_iconUrl = httpsPrefix() + condition["icon"].toString();
                hasUpdates = true;
            }
        }

        if (current.contains("wind_kph")) {
            m_windSpeed = QString::number(current["wind_kph"].toDouble()) + unitWind();
            hasUpdates = true;
        }

        if (current.contains("humidity")) {
            m_humidity = QString::number(current["humidity"].toInt()) + unitHumidity();
            hasUpdates = true;
        }
    }

    if (hasUpdates)
        emit weatherUpdated();
}

void WeatherBackend::loadCountries()
{
    emit countriesLoaded();
}
