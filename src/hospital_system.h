#pragma once

#include <optional>
#include <queue>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

struct Patient {
    std::string id;
    std::string name;
    int age;
    std::string symptoms;
    int severity;
};

struct Doctor {
    std::string id;
    std::string name;
    std::string specialization;
};

struct Appointment {
    std::string appointmentId;
    std::string patientId;
    std::string doctorId;
    std::string date;
    std::string time;
};

class HospitalManagementSystem {
public:
    bool addPatient(const Patient& patient);
    bool addDoctor(const Doctor& doctor);
    bool bookAppointment(const Appointment& appointment);

    bool loadFromFiles(const std::string& dataDirectory);
    bool saveToFiles(const std::string& dataDirectory) const;

    std::optional<Patient> findPatientById(const std::string& patientId) const;
    std::optional<Patient> findPatientByNameBinarySearch(const std::string& name) const;

    std::vector<Patient> listPatientsSortedByName() const;
    std::vector<Doctor> listDoctors() const;
    std::vector<Appointment> listAppointmentsSortedByDateTime() const;

    bool addEmergencyCase(const std::string& patientId);
    std::optional<Patient> treatNextEmergency();

    bool patientExists(const std::string& patientId) const;
    bool doctorExists(const std::string& doctorId) const;

    std::size_t patientCount() const;
    std::size_t doctorCount() const;
    std::size_t appointmentCount() const;
    std::size_t pendingEmergencyCount() const;

    void printSystemStats() const;

private:
    struct EmergencyCase {
        int severity;
        long long ticket;
        std::string patientId;
    };

    struct EmergencyComparator {
        bool operator()(const EmergencyCase& lhs, const EmergencyCase& rhs) const;
    };

    std::unordered_map<std::string, Patient> patientsById_;
    std::unordered_map<std::string, Doctor> doctorsById_;
    std::unordered_set<std::string> appointmentIds_;

    std::vector<Appointment> appointments_;
    std::vector<std::pair<std::string, std::string>> patientNameIndex_;

    std::priority_queue<EmergencyCase, std::vector<EmergencyCase>, EmergencyComparator> emergencyQueue_;
    long long emergencyTicketCounter_ = 0;

    static std::string toLower(std::string text);
    static bool compareAppointmentDateTime(const Appointment& lhs, const Appointment& rhs);
    void clearAllData();
};
