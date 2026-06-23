#include "drivers/navigationSensors.h"
#include "drivers/NASA_Duo.h"
#include "types/sensorTypes.h"
#include <cstdlib>

void NavigationSensors::setSensorActivations(NavigationSource source, bool setTo)
{
    switch (source)
    {
    case NavigationSource::Compass:
        m_sensorActivation.compass = setTo;
        break;

    case NavigationSource::Gps:
        m_sensorActivation.gps = setTo;
        break;

    case NavigationSource::Wind:
        m_sensorActivation.wind = setTo;
    }
};

void NavigationSensors::setLeadSource(NavigationSource src)
{
    m_leadSource = src;
};
NavigationSource NavigationSensors::getLeadSource() const
{
    return m_leadSource;
}

NavigationSensors::NavigationSnapshot NavigationSensors::createSnapshot()
{
    NavigationSnapshot snapshot;
    snapshot.compass_hdg_dg = m_compass.read();
    snapshot.gps_cog_dg = m_gps.readCOG();
    snapshot.gps_sog_kts = m_gps.readSOG();
    snapshot.wind_angle_dg = m_wind.read();
    snapshot.stw_kts = m_nmea183Bus.readSTW();
    snapshot.LeadSource = getLeadSource();

    return snapshot;
};
