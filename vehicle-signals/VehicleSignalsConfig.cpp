/*
 * Copyright (C) 2023,2025 Konsulko Group
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <QDebug>
#include <QUrl>
#include <QFile>
#include <filesystem>
#include <toml.hpp>

#include "VehicleSignalsConfig.h"

/*
VehicleSignalsConfig::VehicleSignalsConfig(const QString &hostname,
					   const unsigned port,
					   const bool useTls,
					   const QString &caCertFileName,
					   const QByteArray &caCert,
					   const QString &tlsServerName,
					   const QString &authToken) :
	m_hostname(hostname),
	m_port(port),
	m_useTls(useTls),
	m_caCertFileName(caCertFileName),
	m_caCert(caCert),
	m_tlsServerName(tlsServerName),
	m_authToken(authToken),
	m_valid(true)
{
	// Potentially could do some certificate validation here...
}
*/

VehicleSignalsConfig::VehicleSignalsConfig(const QString &appname)
{
	// Read global configuration, allowing for override under XDG_CONFIG_HOME
	QString configFile = "";
	char *home = getenv("XDG_CONFIG_HOME");
	if (home) {
		configFile = home;
		configFile += "/AGL/kuksa.toml";

		if (!QFile(configFile).exists())
			configFile.clear();
	}
	if (configFile.isEmpty())
		configFile = "/etc/xdg/AGL/kuksa.toml";
	readConfig(configFile);
	if (!m_valid)
		return;

	// Read application configuration, again allowing for override under XDG_CONFIG_HOME
	configFile = "";
	if (home) {
		configFile = home;
		configFile += "/AGL/";
		configFile += appname;
		configFile += "/kuksa.toml";

		if (!QFile(configFile).exists())
			configFile.clear();
	}
	if (configFile.isEmpty()) {
		configFile = "/etc/xdg/AGL/";
		configFile += appname;
		configFile += "/kuksa.toml";
	}
	readConfig(configFile, true);
}

void VehicleSignalsConfig::readConfig(const QString &configFile, const bool app)
{
	m_valid = false;

	toml::value config;
	qInfo() << "Using KUKSA.val configuration " << configFile;
	try {
		config = toml::parse(configFile.toStdString());
	} catch(const toml::syntax_error& err) {
		qCritical() << "Syntax error in " << configFile;

		// Report all the errors
		qCritical() << err.what();
		return;
	} catch(const std::exception& ex) {
		qCritical() << "Could not read " << configFile;
		return;
	}

	if (config.contains("hostname")) {
		std::string hostname = toml::get_or(config.at("hostname"), "");
		if (hostname.empty()) {
			qCritical() << "Invalid server hostname";
			return;
		}
		m_hostname = QString::fromStdString(hostname);
	}

	if (config.contains("port")) {
		unsigned port = toml::get_or(config.at("port"), 0);
		if (port == 0) {
			qCritical() << "Invalid server port";
			return;
		}
		m_port = port;
	}

	if (config.contains("use-tls") && config.at("use-tls").is_boolean()) {
		m_useTls = toml::get_or(config.at("use-tls"), m_useTls);
	}

	if (config.contains("ca-certificate")) {
		std::string caCertFileName = toml::get_or(config.at("ca-certificate"), "");
		if (!caCertFileName.empty()) {
			m_caCertFileName = QString::fromStdString(caCertFileName);
		} else {
			qCritical() << "Invalid CA certificate filename: " << caCertFileName;
			return;
		}
	}
	if (!m_caCertFileName.isEmpty()) {
		if (m_readCaCertFileName != m_caCertFileName) {
			QFile caCertFile(m_caCertFileName);
			if (!caCertFile.open(QIODevice::ReadOnly)) {
				qCritical() << "Could not open CA certificate file: " << m_caCertFileName;
				return;
			}
			QByteArray caCertData = caCertFile.readAll();
			if (caCertData.isEmpty()) {
				qCritical() << "Invalid CA certificate file: " << m_caCertFileName;
				return;
			}
			m_caCert = caCertData;
			m_readCaCertFileName = m_caCertFileName;
		}
	}

	if (config.contains("tls-server-name")) {
		m_tlsServerName = QString::fromStdString(toml::get_or(config.at("tls-server-name"), ""));
	}

	if (config.contains("authorization")) {
		QString auth = QString::fromStdString(toml::get_or(config.at("authorization"), ""));
		if (!auth.isEmpty()) {
			if (auth.front() == '/') {
				QFile authTokenFile(auth);
				if (!authTokenFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
					qCritical() << "Could not open authorization token file: " << auth;
					return;
				}
				QTextStream in(&authTokenFile);
				QString authToken = in.readLine();
				if (authToken.isEmpty()) {
					qCritical() << "Invalid authorization token file: " << auth;
					return;
				}
				m_authToken = authToken;
			} else {
				// Assume token text has been given
				m_authToken = auth;
			}
		}
	}

	// Verbosity option is application-specific
	if (app) {
		if (config.contains("verbose")) {
			QString verbose = QString::fromStdString(toml::get_or(config.at("verbose"), "invalid"));
			if (!verbose.isEmpty() && verbose != "invalid") {
				if (verbose == "true" || verbose == "1")
					m_verbose = 1;
				if (verbose == "2")
					m_verbose = 2;
			}
		}
	}

	m_valid = true;
}
