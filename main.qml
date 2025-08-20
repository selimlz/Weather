import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root
    width: 400
    height: 600
    visible: true
    title: "Weather App"

    property string appLang: "en"
    property var i18n: ({
        "en": { title: "Weather App", current: "Current Weather", humidity: "Humidity:", wind: "Wind Speed:", btn: "Get Weather", lang: "Language" },
        "fr": { title: "Application Météo", current: "Météo actuelle", humidity: "Humidité :", wind: "Vitesse du vent :", btn: "Obtenir la météo", lang: "Langue" },
        "de": { title: "Wetter-App", current: "Aktuelles Wetter", humidity: "Luftfeuchtigkeit:", wind: "Windgeschwindigkeit:", btn: "Wetter abrufen", lang: "Sprache" },
        "ar": { title: "تطبيق الطقس", current: "الطقس الحالي", humidity: "الرطوبة:", wind: "سرعة الرياح:", btn: "احصل على الطقس", lang: "اللغة" },
        "es": { title: "Aplicación del tiempo", current: "Tiempo actual", humidity: "Humedad:", wind: "Velocidad del viento:", btn: "Obtener el tiempo", lang: "Idioma" }
    })

    function t(k) { return (i18n[appLang] && i18n[appLang][k]) || k }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#2B2B2B" }
            GradientStop { position: 8.0; color: "#E08282" }
        }
    }

    ListModel {
        id: langModel
        ListElement { code: "en"; label: "English";  flag: "qrc:/flag/uk.png" }
        ListElement { code: "fr"; label: "Français"; flag: "qrc:/flag/fr.png" }
        ListElement { code: "de"; label: "Deutsch";  flag: "qrc:/flag/Flag_of_Germany.png" }
        ListElement { code: "es"; label: "Español";  flag: "qrc:/flag/esp.png" }
        ListElement { code: "ar"; label: "العربية";  flag: "qrc:/flag/sa.png" }
    }

    ComboBox {
        id: langCombo
        model: langModel
        width: 100
        height: 30
        x: 280
        y: 30

        delegate: ItemDelegate {
            width: langCombo.width
            contentItem: Row {
                spacing: 8
                Image { source: flag; width: 30; height: 30; fillMode: Image.PreserveAspectFit }
                Text { text: label; verticalAlignment: Text.AlignVCenter }
            }
        }

        displayText: langModel.get(currentIndex).label

        onActivated: {
            const entry = langModel.get(index)
            appLang = entry.code
            weatherBackend.setLanguage(entry.code)
            weatherBackend.fetchWeather(countryCombo.currentText)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Text {
            text: t("title")
            font.pixelSize: 24
            font.bold: true
            color: "white"
        }

        ComboBox {
            id: countryCombo
            Layout.fillWidth: true
            model: weatherBackend.countries
            currentIndex: 0
            font.pixelSize: 16

            onActivated: {
                weatherBackend.fetchWeather(currentText)
                timeBackend.fetchTimeData(currentText)
            }

            delegate: ItemDelegate {
                width: countryCombo.width
                contentItem: Text {
                    text: modelData
                    font: countryCombo.font
                    color: "black"
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 10
                }
                background: Rectangle { color: "white" }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 250
            radius: 10
            color: "#f5f7fa"
            border.color: "#d3d3d3"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                Text {
                    text: t("current")
                    font.pixelSize: 18
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: weatherBackend.cityName
                    font.pixelSize: 20
                    visible: !weatherBackend.loading
                }

                Text {
                    text: timeBackend.timeString
                    font.pixelSize: 20
                    visible: !weatherBackend.loading
                }

                RowLayout {
                    id: weatherRow
                    spacing: 50

                    Text {
                        text: weatherBackend.temperature
                        font.pixelSize: 36
                        visible: !weatherBackend.loading
                    }

                    Image {
                        id: weatherIcon
                        source: weatherBackend.iconUrl
                        width: 48
                        height: 48
                        fillMode: Image.PreserveAspectFit
                    }
                }

                Text {
                    text: weatherBackend.condition
                    font.pixelSize: 16
                    visible: !weatherBackend.loading
                }

                BusyIndicator {
                    Layout.alignment: Qt.AlignHCenter
                    running: weatherBackend.loading
                    visible: running
                }
            }
        }

        Row {
            spacing: 5
            Text { text: t("humidity"); font.bold: true; font.pixelSize: 18; color: "white" }
            Text { text: weatherBackend.humidity; font.pixelSize: 18; color: "white" }
        }

        Row {
            spacing: 5
            Text { text: t("wind"); font.bold: true; font.pixelSize: 18; color: "white" }
            Text { text: weatherBackend.windSpeed; font.pixelSize: 18; color: "white" }
        }

        Button {
            text: t("btn")
            Layout.fillWidth: true
            Layout.preferredHeight: 45
            font.pixelSize: 16
            enabled: !weatherBackend.loading

            background: Rectangle {
                radius: 5
                color: parent.down ? "#3498db" : "#2980b9"
            }

            contentItem: Text {
                text: parent.text
                font: parent.font
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                weatherBackend.fetchWeather(countryCombo.currentText)
                timeBackend.fetchTimeData(countryCombo.currentText)
            }
        }
    }

    Connections {
        target: weatherBackend
        onErrorOccurred: console.error("API Error:", message)
    }

    Connections {
        target: timeBackend
        onErrorOccurred: console.error("Time Error:", message)
    }

    Component.onCompleted: {
        timeBackend.startAutoUpdate(300) // Sync every 5 minutes
    }
}

