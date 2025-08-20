#include "timebackend.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

namespace {

inline const QString &timeApiBase()
{
    static const QString v = QStringLiteral("https://api.timezonedb.com/v2.1/get-time-zone");
    return v;
}

inline const QString &timeApiKey()
{
    static const QString v = qEnvironmentVariable("TIME_API_KEY", QStringLiteral("7893LKWRYN3U"));
    return v;
}

inline int apiSyncMs() { return 5 * 60 * 1000; }  // 5 minutes
inline int localTickMs() { return 1000; }         // 1 second

inline const QString &timeFmt()
{
    static const QString v = QStringLiteral("dd/MM/yyyy hh:mm:ss");
    return v;
}

} // namespace

using namespace std;

TimeBackend::TimeBackend(QObject *parent)
    : QObject(parent)
    , m_updateTimer(new QTimer(this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_timeString("--:--")
    , m_loading(false)
    , m_currentCountry("")
    , m_localTimeTimer(new QTimer(this))
    , m_timezoneOffset(0)
{
    initializeCountryData();

    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &TimeBackend::handleTimeReply);

    // API sync timer (every 5 minutes)
    m_updateTimer->setInterval(apiSyncMs());
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        if (!m_currentCountry.isEmpty())
            fetchTimeData(m_currentCountry);
    });

    // Local time timer (every 1 second)
    m_localTimeTimer->setInterval(localTickMs());
    connect(m_localTimeTimer, &QTimer::timeout, this, &TimeBackend::updateLocalTime);
    m_localTimeTimer->start();
}

void TimeBackend::initializeCountryData()
{
    m_countryCoordinates.clear();

    m_countryCoordinates.insert("Afghanistan", {"34.5553", "69.2075"});
    m_countryCoordinates.insert("Albania", {"41.3275", "19.8187"});
    m_countryCoordinates.insert("Algeria", {"36.7538", "3.0588"});
    m_countryCoordinates.insert("Andorra", {"42.5063", "1.5218"});
    m_countryCoordinates.insert("Angola", {"-8.8383", "13.2344"});
    m_countryCoordinates.insert("Argentina", {"-34.6037", "-58.3816"});
    m_countryCoordinates.insert("Armenia", {"40.1792", "44.4991"});
    m_countryCoordinates.insert("Australia", {"-35.2809", "149.1300"});
    m_countryCoordinates.insert("Austria", {"48.2082", "16.3738"});
    m_countryCoordinates.insert("Azerbaijan", {"40.4093", "49.8671"});
    m_countryCoordinates.insert("Bahamas", {"25.0343", "-77.3963"});
    m_countryCoordinates.insert("Bahrain", {"26.2235", "50.5876"});
    m_countryCoordinates.insert("Bangladesh", {"23.8103", "90.4125"});
    m_countryCoordinates.insert("Barbados", {"13.1132", "-59.5988"});
    m_countryCoordinates.insert("Belarus", {"53.9045", "27.5615"});
    m_countryCoordinates.insert("Belgium", {"50.8503", "4.3517"});
    m_countryCoordinates.insert("Belize", {"17.1899", "-88.4976"});
    m_countryCoordinates.insert("Benin", {"6.4969", "2.6289"});
    m_countryCoordinates.insert("Bhutan", {"27.4728", "89.6390"});
    m_countryCoordinates.insert("Bolivia", {"-16.4897", "-68.1193"});
    m_countryCoordinates.insert("Bosnia and Herzegovina", {"43.8563", "18.4131"});
    m_countryCoordinates.insert("Botswana", {"-24.6282", "25.9231"});
    m_countryCoordinates.insert("Brazil", {"-15.8267", "-47.9218"});
    m_countryCoordinates.insert("Brunei", {"4.9031", "114.9398"});
    m_countryCoordinates.insert("Bulgaria", {"42.6977", "23.3219"});
    m_countryCoordinates.insert("Burkina Faso", {"12.3714", "-1.5197"});
    m_countryCoordinates.insert("Burundi", {"-3.3614", "29.3599"});
    m_countryCoordinates.insert("Cambodia", {"11.5564", "104.9282"});
    m_countryCoordinates.insert("Cameroon", {"3.8480", "11.5021"});
    m_countryCoordinates.insert("Canada", {"45.4215", "-75.6972"});
    m_countryCoordinates.insert("Cape Verde", {"14.9330", "-23.5133"});
    m_countryCoordinates.insert("Central African Republic", {"4.3947", "18.5582"});
    m_countryCoordinates.insert("Chad", {"12.1348", "15.0557"});
    m_countryCoordinates.insert("Chile", {"-33.4489", "-70.6693"});
    m_countryCoordinates.insert("China", {"39.9042", "116.4074"});
    m_countryCoordinates.insert("Colombia", {"4.7110", "-74.0721"});
    m_countryCoordinates.insert("Comoros", {"-11.7172", "43.2473"});
    m_countryCoordinates.insert("Congo", {"-4.2634", "15.2429"});
    m_countryCoordinates.insert("Costa Rica", {"9.9281", "-84.0907"});
    m_countryCoordinates.insert("Croatia", {"45.8150", "15.9819"});
    m_countryCoordinates.insert("Cuba", {"23.1136", "-82.3666"});
    m_countryCoordinates.insert("Cyprus", {"35.1856", "33.3823"});
    m_countryCoordinates.insert("Czech Republic", {"50.0755", "14.4378"});
    m_countryCoordinates.insert("Denmark", {"55.6761", "12.5683"});
    m_countryCoordinates.insert("Djibouti", {"11.5721", "43.1456"});
    m_countryCoordinates.insert("Dominica", {"15.3092", "-61.3794"});
    m_countryCoordinates.insert("Dominican Republic", {"18.4861", "-69.9312"});
    m_countryCoordinates.insert("East Timor", {"-8.5569", "125.5603"});
    m_countryCoordinates.insert("Ecuador", {"-0.1807", "-78.4678"});
    m_countryCoordinates.insert("Egypt", {"30.0444", "31.2357"});
    m_countryCoordinates.insert("El Salvador", {"13.6929", "-89.2182"});
    m_countryCoordinates.insert("Equatorial Guinea", {"3.7504", "8.7371"});
    m_countryCoordinates.insert("Eritrea", {"15.3229", "38.9251"});
    m_countryCoordinates.insert("Estonia", {"59.4370", "24.7536"});
    m_countryCoordinates.insert("Eswatini", {"-26.3051", "31.1367"});
    m_countryCoordinates.insert("Ethiopia", {"9.0084", "38.7648"});
    m_countryCoordinates.insert("Fiji", {"-18.1248", "178.4501"});
    m_countryCoordinates.insert("Finland", {"60.1699", "24.9384"});
    m_countryCoordinates.insert("France", {"48.8566", "2.3522"});
    m_countryCoordinates.insert("Gabon", {"0.4162", "9.4673"});
    m_countryCoordinates.insert("Gambia", {"13.4549", "-16.5790"});
    m_countryCoordinates.insert("Georgia", {"41.7151", "44.8271"});
    m_countryCoordinates.insert("Germany", {"52.5200", "13.4050"});
    m_countryCoordinates.insert("Ghana", {"5.6037", "-0.1870"});
    m_countryCoordinates.insert("Greece", {"37.9838", "23.7275"});
    m_countryCoordinates.insert("Grenada", {"12.0561", "-61.7486"});
    m_countryCoordinates.insert("Guatemala", {"14.6349", "-90.5069"});
    m_countryCoordinates.insert("Guinea", {"9.6412", "-13.5784"});
    m_countryCoordinates.insert("Guinea-Bissau", {"11.8636", "-15.5846"});
    m_countryCoordinates.insert("Guyana", {"6.8013", "-58.1551"});
    m_countryCoordinates.insert("Haiti", {"18.5944", "-72.3074"});
    m_countryCoordinates.insert("Honduras", {"14.0723", "-87.1921"});
    m_countryCoordinates.insert("Hungary", {"47.4979", "19.0402"});
    m_countryCoordinates.insert("Iceland", {"64.1466", "-21.9426"});
    m_countryCoordinates.insert("India", {"28.6139", "77.2090"});
    m_countryCoordinates.insert("Indonesia", {"-6.2088", "106.8456"});
    m_countryCoordinates.insert("Iran", {"35.6892", "51.3890"});
    m_countryCoordinates.insert("Iraq", {"33.3152", "44.3661"});
    m_countryCoordinates.insert("Ireland", {"53.3498", "-6.2603"});
    m_countryCoordinates.insert("Israel", {"31.7683", "35.2137"});
    m_countryCoordinates.insert("Italy", {"41.9028", "12.4964"});
    m_countryCoordinates.insert("Ivory Coast", {"5.3599", "-4.0083"});
    m_countryCoordinates.insert("Jamaica", {"18.0179", "-76.8099"});
    m_countryCoordinates.insert("Japan", {"35.6762", "139.6503"});
    m_countryCoordinates.insert("Jordan", {"31.9454", "35.9284"});
    m_countryCoordinates.insert("Kazakhstan", {"51.1605", "71.4704"});
    m_countryCoordinates.insert("Kenya", {"-1.2864", "36.8172"});
    m_countryCoordinates.insert("Kiribati", {"1.4518", "172.9717"});
    m_countryCoordinates.insert("Korea North", {"39.0392", "125.7625"});
    m_countryCoordinates.insert("Korea South", {"37.5665", "126.9780"});
    m_countryCoordinates.insert("Kosovo", {"42.6629", "21.1655"});
    m_countryCoordinates.insert("Kuwait", {"29.3759", "47.9774"});
    m_countryCoordinates.insert("Kyrgyzstan", {"42.8746", "74.5698"});
    m_countryCoordinates.insert("Laos", {"17.9757", "102.6331"});
    m_countryCoordinates.insert("Latvia", {"56.9496", "24.1052"});
    m_countryCoordinates.insert("Lebanon", {"33.8938", "35.5018"});
    m_countryCoordinates.insert("Lesotho", {"-29.3101", "27.4786"});
    m_countryCoordinates.insert("Liberia", {"6.2907", "-10.7605"});
    m_countryCoordinates.insert("Libya", {"32.8872", "13.1913"});
    m_countryCoordinates.insert("Liechtenstein", {"47.1410", "9.5209"});
    m_countryCoordinates.insert("Lithuania", {"54.6872", "25.2797"});
    m_countryCoordinates.insert("Luxembourg", {"49.6116", "6.1319"});
    m_countryCoordinates.insert("Madagascar", {"-18.8792", "47.5079"});
    m_countryCoordinates.insert("Malawi", {"-13.9626", "33.7741"});
    m_countryCoordinates.insert("Malaysia", {"3.1390", "101.6869"});
    m_countryCoordinates.insert("Maldives", {"4.1755", "73.5093"});
    m_countryCoordinates.insert("Mali", {"12.6392", "-8.0029"});
    m_countryCoordinates.insert("Malta", {"35.8989", "14.5146"});
    m_countryCoordinates.insert("Marshall Islands", {"7.0897", "171.3803"});
    m_countryCoordinates.insert("Mauritania", {"18.0731", "-15.9582"});
    m_countryCoordinates.insert("Mauritius", {"-20.1609", "57.5012"});
    m_countryCoordinates.insert("Mexico", {"19.4326", "-99.1332"});
    m_countryCoordinates.insert("Micronesia", {"6.9147", "158.1610"});
    m_countryCoordinates.insert("Moldova", {"47.0105", "28.8638"});
    m_countryCoordinates.insert("Monaco", {"43.7384", "7.4246"});
    m_countryCoordinates.insert("Mongolia", {"47.8864", "106.9057"});
    m_countryCoordinates.insert("Montenegro", {"42.4304", "19.2594"});
    m_countryCoordinates.insert("Morocco", {"34.0209", "-6.8416"});
    m_countryCoordinates.insert("Mozambique", {"-25.9692", "32.5732"});
    m_countryCoordinates.insert("Myanmar", {"16.8409", "96.1735"});
    m_countryCoordinates.insert("Namibia", {"-22.5609", "17.0658"});
    m_countryCoordinates.insert("Nauru", {"-0.5228", "166.9315"});
    m_countryCoordinates.insert("Nepal", {"27.7172", "85.3240"});
    m_countryCoordinates.insert("Netherlands", {"52.3676", "4.9041"});
    m_countryCoordinates.insert("New Zealand", {"-41.2865", "174.7762"});
    m_countryCoordinates.insert("Nicaragua", {"12.1364", "-86.2514"});
    m_countryCoordinates.insert("Niger", {"13.5127", "2.1126"});
    m_countryCoordinates.insert("Nigeria", {"9.0579", "7.4951"});
    m_countryCoordinates.insert("North Macedonia", {"42.0080", "21.4294"});
    m_countryCoordinates.insert("Norway", {"59.9139", "10.7522"});
    m_countryCoordinates.insert("Oman", {"23.5859", "58.4059"});
    m_countryCoordinates.insert("Pakistan", {"33.6844", "73.0479"});
    m_countryCoordinates.insert("Palau", {"7.5149", "134.5825"});
    m_countryCoordinates.insert("Panama", {"8.9824", "-79.5199"});
    m_countryCoordinates.insert("Papua New Guinea", {"-9.4438", "147.1803"});
    m_countryCoordinates.insert("Paraguay", {"-25.2637", "-57.5759"});
    m_countryCoordinates.insert("Peru", {"-12.0464", "-77.0428"});
    m_countryCoordinates.insert("Philippines", {"14.5995", "120.9842"});
    m_countryCoordinates.insert("Poland", {"52.2297", "21.0122"});
    m_countryCoordinates.insert("Portugal", {"38.7223", "-9.1393"});
    m_countryCoordinates.insert("Qatar", {"25.2769", "51.5200"});
    m_countryCoordinates.insert("Romania", {"44.4268", "26.1025"});
    m_countryCoordinates.insert("Russia", {"55.7558", "37.6173"});
    m_countryCoordinates.insert("Rwanda", {"-1.9501", "30.0588"});
    m_countryCoordinates.insert("Saint Kitts and Nevis", {"17.3578", "-62.7830"});
    m_countryCoordinates.insert("Saint Lucia", {"14.0101", "-60.9875"});
    m_countryCoordinates.insert("Saint Vincent and the Grenadines", {"13.1600", "-61.2248"});
    m_countryCoordinates.insert("Samoa", {"-13.7590", "-172.1046"});
    m_countryCoordinates.insert("San Marino", {"43.9424", "12.4578"});
    m_countryCoordinates.insert("Sao Tome and Principe", {"0.3302", "6.7333"});
    m_countryCoordinates.insert("Saudi Arabia", {"24.7136", "46.6753"});
    m_countryCoordinates.insert("Senegal", {"14.7167", "-17.4677"});
    m_countryCoordinates.insert("Serbia", {"44.7866", "20.4489"});
    m_countryCoordinates.insert("Seychelles", {"-4.6796", "55.4920"});
    m_countryCoordinates.insert("Sierra Leone", {"8.4840", "-13.2297"});
    m_countryCoordinates.insert("Singapore", {"1.3521", "103.8198"});
    m_countryCoordinates.insert("Slovakia", {"48.1486", "17.1077"});
    m_countryCoordinates.insert("Slovenia", {"46.0569", "14.5058"});
    m_countryCoordinates.insert("Solomon Islands", {"-9.4456", "159.9729"});
    m_countryCoordinates.insert("Somalia", {"2.0469", "45.3182"});
    m_countryCoordinates.insert("South Africa", {"-25.7479", "28.2293"});
    m_countryCoordinates.insert("South Sudan", {"4.8594", "31.5713"});
    m_countryCoordinates.insert("Spain", {"40.4168", "-3.7038"});
    m_countryCoordinates.insert("Sri Lanka", {"6.9271", "79.8612"});
    m_countryCoordinates.insert("Sudan", {"15.5007", "32.5599"});
    m_countryCoordinates.insert("Suriname", {"5.8520", "-55.2038"});
    m_countryCoordinates.insert("Sweden", {"59.3293", "18.0686"});
    m_countryCoordinates.insert("Switzerland", {"46.9480", "7.4474"});
    m_countryCoordinates.insert("Syria", {"33.5138", "36.2765"});
    m_countryCoordinates.insert("Taiwan", {"25.0330", "121.5654"});
    m_countryCoordinates.insert("Tajikistan", {"38.5598", "68.7870"});
    m_countryCoordinates.insert("Tanzania", {"-6.1630", "35.7516"});
    m_countryCoordinates.insert("Thailand", {"13.7563", "100.5018"});
    m_countryCoordinates.insert("Togo", {"6.1725", "1.2314"});
    m_countryCoordinates.insert("Tonga", {"-21.1393", "-175.2049"});
    m_countryCoordinates.insert("Trinidad and Tobago", {"10.6918", "-61.2225"});
    m_countryCoordinates.insert("Tunisia", {"36.8065", "10.1815"});
    m_countryCoordinates.insert("Turkey", {"39.9334", "32.8597"});
    m_countryCoordinates.insert("Turkmenistan", {"37.9601", "58.3261"});
    m_countryCoordinates.insert("Tuvalu", {"-8.5167", "179.2166"});
    m_countryCoordinates.insert("Uganda", {"0.3136", "32.5811"});
    m_countryCoordinates.insert("Ukraine", {"50.4501", "30.5234"});
    m_countryCoordinates.insert("United Arab Emirates", {"24.4539", "54.3773"});
    m_countryCoordinates.insert("United Kingdom", {"51.5074", "-0.1278"});
    m_countryCoordinates.insert("United States", {"38.9072", "-77.0369"});
    m_countryCoordinates.insert("Uruguay", {"-34.9011", "-56.1645"});
    m_countryCoordinates.insert("Uzbekistan", {"41.2995", "69.2401"});
    m_countryCoordinates.insert("Vanuatu", {"-17.7333", "168.3273"});
    m_countryCoordinates.insert("Vatican City", {"41.9029", "12.4534"});
    m_countryCoordinates.insert("Venezuela", {"10.4806", "-66.9036"});
    m_countryCoordinates.insert("Vietnam", {"21.0278", "105.8342"});
    m_countryCoordinates.insert("Yemen", {"15.3694", "44.1910"});
    m_countryCoordinates.insert("Zambia", {"-15.3875", "28.3228"});
    m_countryCoordinates.insert("Zimbabwe", {"-17.8292", "31.0522"});
}

QVariantMap TimeBackend::getCoordinates(const QString &country)
{
    QVariantMap result;

    if (!m_countryCoordinates.contains(country)) {
        result["error"] = "Country not found";
        return result;
    }

    if (m_countryCoordinates.contains(country)) {
        auto coords = m_countryCoordinates[country];
        result["lat"] = coords.first;
        result["lng"] = coords.second;
        result["success"] = true;
    } else {
        result["success"] = false;
        result["error"] = "Country coordinates not available";
    }

    return result;
}

void TimeBackend::stopAutoUpdate()
{
    m_updateTimer->stop();
}

void TimeBackend::updateLocalTime()
{
    if (!m_lastApiTime.isValid())
        return;

    // Calculate current time in target timezone
    QDateTime now = QDateTime::currentDateTimeUtc().addSecs(m_timezoneOffset);
    m_timeString = now.toString(timeFmt());
    emit timeUpdated();
}

void TimeBackend::fetchTimeData(const QString &country)
{
    m_currentCountry = country;

    if (timeApiKey().isEmpty()) {
        emit errorOccurred("Set TIME_API_KEY");
        m_loading = false;
        emit loadingChanged();
        return;
    }

    QVariantMap coords = getCoordinates(country);
    if (!coords["success"].toBool()) {
        emit errorOccurred(coords["error"].toString());
        return;
    }

    m_loading = true;
    emit loadingChanged();

    const QString url = QStringLiteral("%1?key=%2&format=json&by=position&lat=%3&lng=%4")
                            .arg(timeApiBase(), timeApiKey(),
                                 coords["lat"].toString(),
                                 coords["lng"].toString());

    m_networkManager->get(QNetworkRequest(QUrl(url)));
}

void TimeBackend::startAutoUpdate(int intervalSeconds)
{
    m_updateTimer->setInterval(intervalSeconds * 1000);
    m_updateTimer->start();

    if (!m_currentCountry.isEmpty())
        fetchTimeData(m_currentCountry); // Fetch immediately
}

void TimeBackend::handleTimeReply(QNetworkReply *reply)
{
    m_loading = false;
    emit loadingChanged();

    if (!reply) {
        emit errorOccurred("Null reply received");
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Time API Error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    parseTimeResponse(reply->readAll());
    reply->deleteLater();
}

void TimeBackend::parseTimeResponse(const QByteArray &data)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred("Failed to parse time data");
        return;
    }

    QJsonObject json = doc.object();
    if (json.contains("formatted") && json.contains("gmtOffset") && json.contains("zoneName")) {
        QString timeString = json["formatted"].toString();
        m_timezoneOffset = json["gmtOffset"].toInt();
        m_timezoneName = json["zoneName"].toString();

        // Parse the API time and store it
        m_lastApiTime = QDateTime::fromString(timeString, Qt::ISODate);
        if (m_lastApiTime.isValid()) {
            m_timeString = m_lastApiTime.toString(timeFmt());
            emit timeUpdated();
        }
    }
}

QString TimeBackend::timeString() const
{
    return m_timeString;
}

bool TimeBackend::loading() const
{
    return m_loading;
}

TimeBackend::~TimeBackend()
{
    delete m_networkManager;
}
