#include "settings.h"

#include <QDebug>

Settings* Settings::s_pInstance = NULL;

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_settings()
{
    qDebug() << this << "Creating";
}

Settings::Settings(QString &path, QObject *parent)
    : QObject(parent)
    , m_settings(path, QSettings::IniFormat)
{
    qDebug() << this << "Create from file" << path;
}

Settings::~Settings()
{
    qDebug() << this << "Destroying";
}

QVariant Settings::setting(QString key, QVariant value)
{
    if( ! value.isNull() ) {
        m_settings.setValue(key, value);
        emit settingChanged(key);
    }
    if( m_settings.value(key).isNull() ) {
        qWarning() << this << "Unable to find predefined setting" << key;
    }

    return m_settings.value(key);
}

bool Settings::isNull(QString key)
{
    return m_settings.value(key).isNull();
}

void Settings::setDefault(QString key, QVariant value)
{
    if( m_settings.value(key).isNull() ) {
        qDebug() << this << "Set default value for" << key << "=" << value;
        m_settings.setValue(key, value);
        emit settingChanged(key);
    }
}
