#include "messagehelper.h"

#ifdef QT_DEBUG
#include <QDebug>
#endif

#include <QJsonArray>
#include <QPolygonF>

MessageHelper::MessageHelper(LocationModel *model, QObject *parent) :
    QObject(parent),
    m_locationModel(model)
{

}

bool MessageHelper::parseMessage(const QJsonObject &obj, Message *msg)
{
    if (!msg)
        return false;

    if (obj.isEmpty())
        return false;

    // parse header
    msg->setIdentifier(obj.value(QStringLiteral("identifier")).toString());
    msg->setSent(QDateTime::fromString(obj.value(QStringLiteral("sent")).toString(), Qt::ISODate));

    const QString msgType = obj.value(QStringLiteral("msgType")).toString();
    if (msgType == QLatin1String("Alert")) {
        msg->setMessageType(Message::Alert);
    } else if (msgType == QLatin1String("Cancel")){
        msg->setMessageType(Message::Cancel);
    } else if (msgType == QLatin1String("Update")){
        msg->setMessageType(Message::Update);
    } else {
#ifdef QT_DEBUG
        qDebug() << "Message Type: "  << msgType;
#endif
        msg->setMessageType(Message::Unkown);
    }

    // parse info
    const QJsonObject info = obj.value(QStringLiteral("info")).toArray().first().toObject();

    // categories
    const QJsonArray categories = info.value(QStringLiteral("category")).toArray();

    quint32 flags{0};
    for (const QJsonValue &item : categories) {
        const QString category = item.toString();

        if (category == QLatin1String("CBRNE")) {
            flags |= Message::CategoryCBRNE;
        } else if (category == QLatin1String("Env")){
            flags |= Message::CategoryEnv;
        } else if (category == QLatin1String("Fire")){
            flags |= Message::CategoryFire;
        } else if (category == QLatin1String("Geo")){
            flags |= Message::CategoryGeo;
        } else if (category == QLatin1String("Health")){
            flags |= Message::CategoryHealth;
        } else if (category == QLatin1String("Infra")){
            flags |= Message::CategoryInfra;
        } else if (category == QLatin1String("Met")){
            flags |= Message::CategoryMet;
        } else if (category == QLatin1String("Other")){
            flags |= Message::CategoryOther;
        } else if (category == QLatin1String("Rescue")){
            flags |= Message::CategoryRescue;
        } else if (category == QLatin1String("Safety")){
            flags |= Message::CategorySafety;
        } else if (category == QLatin1String("Security")){
            flags |= Message::CategorySecurity;
        } else {
#ifdef QT_DEBUG
            qDebug() << "Category: "  << category;
#endif
        }
    }
    msg->setCategories(flags);

    // urgency
    const QString urgency = info.value(QStringLiteral("urgency")).toString();
    if (urgency == QLatin1String("Immediate")) {
        msg->setUrgency(Message::UrgencyImmediate);
    } else {
#ifdef QT_DEBUG
        qDebug() << "Urgency: " << urgency;
#endif
        msg->setUrgency(Message::UrgencyUnkown);
    }

    // severity
    const QString severity = info.value(QStringLiteral("severity")).toString();
    if (severity == QLatin1String("Minor")) {
        msg->setSeverity(Message::SeverityMinor);
    } else if (severity == QLatin1String("Moderate")){
        msg->setSeverity(Message::SeverityModerate);
    } else if (severity == QLatin1String("Severe")){
        msg->setSeverity(Message::SeveritySevere);
    } else {
#ifdef QT_DEBUG
        qDebug() << "Severetiy: " << severity;
#endif
        msg->setSeverity(Message::SeverityUndefined);
    }

    // string data
    msg->setEventTitle(info.value(QStringLiteral("event")).toString());
    msg->setHeadline(info.value(QStringLiteral("headline")).toString());
    msg->setDescription(info.value(QStringLiteral("description")).toString());
    msg->setInstruction(info.value(QStringLiteral("instruction")).toString());
    msg->setContact(info.value(QStringLiteral("contact")).toString());

    // senderName
    QString senderName;
    const QJsonArray parameter = info.value(QStringLiteral("parameter")).toArray();

    for (const auto &value : parameter) {
        const QJsonObject obj = value.toObject();

        if (obj.value(QStringLiteral("valueName")).toString() != QLatin1String("sender_langname"))
            continue;

        senderName = obj.value(QStringLiteral("value")).toString();
    }

    if (senderName.isEmpty())
        senderName = info.value(QStringLiteral("senderName")).toString();

    msg->setSenderName(senderName);


    // areas
    const QJsonArray areas = info.value(QStringLiteral("area")).toArray();
    for (const QJsonValue &area : areas) {
        const QJsonArray polygons = area.toObject().value(QStringLiteral("polygon")).toArray();
        for (const QJsonValue &polygon : polygons) {

            QPolygonF poly;

            const QStringList parts = polygon.toString().split(" ");

            for (const QString &part : parts) {
                const QStringList location = part.split(",");


                if (location.count() != 2)
                    continue;

                poly.append(QPointF(location.last().toFloat(),
                                    location.first().toFloat()));
            }

            if (poly.isEmpty())
                continue;

            for (const auto *loc : m_locationModel->locations()) {
                const QPointF point(loc->latitude(), loc->longitude());

                msg->setLocal(poly.containsPoint(point, Qt::OddEvenFill));
            }
        }
    }

    return true;
}
