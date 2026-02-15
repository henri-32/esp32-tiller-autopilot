#pragma once

class HeadingSignal {
public:
    HeadingSignal(double startHeading);

    void setTarget(double target);

    void enableDrift(double deg_per_sec);
    void enableOscillation(double amplitude_deg, double period_sec);

    void step(double dt);

    double getHeading() const;

private:
    double normalize(double h);

    double m_heading;
    double m_target;

    // Drift
    double m_driftRate = 0.0;

    // Oscillation
    bool m_oscEnabled = false;
    double m_amp = 0.0;
    double m_period = 1.0;
    double m_time = 0.0;
};

class HeadingDriftSignal {
    public:
    HeadingDriftSignal(double startHeading, double driftDegPerSec) : m_heading(startHeading), m_driftRate(driftDegPerSec){};

    void step(double dt){
        m_heading += m_driftRate * dt; 
        normalize();
    };

    double getHeading() const {
        return m_heading;
    };

    private:
    void normalize() {
        while (m_heading<0) {
        m_heading+=360;
        }
        while (m_heading>=360) {
        m_heading-= 360;
        }
    };

    double m_heading; 
    double m_driftRate; 

};
