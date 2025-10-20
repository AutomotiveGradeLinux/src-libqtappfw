/*
 * Copyright (C) 2023,2025 Konsulko Group
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VEHICLE_SIGNALS_CONFIG_H
#define VEHICLE_SIGNALS_CONFIG_H

#include <QObject>

#define KUKSA_CONFIG_DEFAULT_HOSTNAME	"localhost"
#define KUKSA_CONFIG_DEFAULT_PORT	55555
#define KUKSA_CONFIG_DEFAULT_CA_CERT	"/etc/kuksa-val/CA.pem"

// Class to read/hold VSS server configuration

class VehicleSignalsConfig
{
public:
  /*
        explicit VehicleSignalsConfig(const QString &hostname,
				      const unsigned port,
				      const bool useTls,
				      const QString &caCertFileName,
				      const QByteArray &caCert,
				      const QString &tlsServerName,
				      const QString &authToken);
  */
        explicit VehicleSignalsConfig(const QString &appname);
        ~VehicleSignalsConfig() {};

	QString hostname() { return m_hostname; };
	unsigned port() { return m_port; };
	bool useTls() { return m_useTls; };
	QString caCertFileName() { return m_caCertFileName; };
	QByteArray caCert() { return m_caCert; };
	QString tlsServerName() { return m_tlsServerName; };
	QString authToken() { return m_authToken; };
	unsigned verbose() { return m_verbose; };
	bool valid() { return m_valid; };

private:
	QString m_hostname = KUKSA_CONFIG_DEFAULT_HOSTNAME;
	unsigned m_port = KUKSA_CONFIG_DEFAULT_PORT;
	bool m_useTls = false;
	QString m_caCertFileName = KUKSA_CONFIG_DEFAULT_CA_CERT;
	QString m_readCaCertFileName = "";
	QByteArray m_caCert = "";
	QString m_tlsServerName = "";
	QString m_authToken = "";
	unsigned m_verbose = 0;
	bool m_valid = false;

	void readConfig(const QString &configFile, const bool app = false);
};

#endif // VEHICLE_SIGNALS_CONFIG_H
