#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "weatherbackend.h"
#include "timebackend.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    WeatherBackend weatherBackend;
    TimeBackend timeBackend;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("weatherBackend", &weatherBackend);
    engine.rootContext()->setContextProperty("timeBackend", &timeBackend);

    const QUrl url(QStringLiteral("qrc:/weatherApp/main.qml"));
    engine.load(url);

    return app.exec();
}
