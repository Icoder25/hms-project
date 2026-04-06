#include "hospital_system.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>

bool HospitalManagementSystem::EmergencyComparator::operator()(const EmergencyCase& lhs, const EmergencyCase& rhs) const {
    if (lhs.severity == rhs.severity) {
        return lhs.ticket > rhs.ticket;
    }
    return lhs.severity < rhs.severity;
}

bool HospitalManagementSystem::addPatient(const Patient& patient) {
    if (patient.id.empty() || patient.name.empty() || patient.age <= 0) {
        return false;
    }

    if (patientsById_.find(patient.id) != patientsById_.end()) {
        return false;
    }

    Patient normalized = patient;
    if (normalized.severity < 1) {
        normalized.severity = 1;
    }
    if (normalized.severity > 10) {
        normalized.severity = 10;
    }

    patientsById_.emplace(normalized.id, normalized);

    const std::string nameKey = toLower(normalized.name);
    auto insertPos = std::lower_bound(
        patientNameIndex_.begin(),
        patientNameIndex_.end(),
        std::make_pair(nameKey, normalized.id)
    );
    patientNameIndex_.insert(insertPos, std::make_pair(nameKey, normalized.id));

    return true;
}

bool HospitalManagementSystem::addDoctor(const Doctor& doctor) {
    if (doctor.id.empty() || doctor.name.empty() || doctor.specialization.empty()) {
        return false;
    }

    if (doctorsById_.find(doctor.id) != doctorsById_.end()) {
        return false;
    }

    doctorsById_.emplace(doctor.id, doctor);
    return true;
}

bool HospitalManagementSystem::bookAppointment(const Appointment& appointment) {
    if (appointment.appointmentId.empty() || appointment.patientId.empty() || appointment.doctorId.empty() ||
        appointment.date.empty() || appointment.time.empty()) {
        return false;
    }

    if (appointmentIds_.find(appointment.appointmentId) != appointmentIds_.end()) {
        return false;
    }

    if (!patientExists(appointment.patientId) || !doctorExists(appointment.doctorId)) {
        return false;
    }

    appointmentIds_.insert(appointment.appointmentId);
    appointments_.push_back(appointment);
    return true;
}

bool HospitalManagementSystem::loadFromFiles(const std::string& dataDirectory) {
    clearAllData();

    namespace fs = std::filesystem;
    const fs::path basePath(dataDirectory);
    if (!fs::exists(basePath)) {
        return true;
    }

    bool loadedSuccessfully = true;

    {
        std::ifstream patientFile(basePath / "patients.txt");
        if (patientFile.is_open()) {
            while (true) {
                Patient patient;
                if (!(patientFile >> std::quoted(patient.id) >> std::quoted(patient.name) >> patient.age >>
                      std::quoted(patient.symptoms) >> patient.severity)) {
                    if (patientFile.eof()) {
                        break;
                    }
                    loadedSuccessfully = false;
                    patientFile.clear();
                    patientFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    continue;
                }

                if (!addPatient(patient)) {
                    loadedSuccessfully = false;
                }
            }
        }
    }

    {
        std::ifstream doctorFile(basePath / "doctors.txt");
        if (doctorFile.is_open()) {
            while (true) {
                Doctor doctor;
                if (!(doctorFile >> std::quoted(doctor.id) >> std::quoted(doctor.name) >> std::quoted(doctor.specialization))) {
                    if (doctorFile.eof()) {
                        break;
                    }
                    loadedSuccessfully = false;
                    doctorFile.clear();
                    doctorFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    continue;
                }

                if (!addDoctor(doctor)) {
                    loadedSuccessfully = false;
                }
            }
        }
    }

    {
        std::ifstream appointmentFile(basePath / "appointments.txt");
        if (appointmentFile.is_open()) {
            while (true) {
                Appointment appointment;
                if (!(appointmentFile >> std::quoted(appointment.appointmentId) >> std::quoted(appointment.patientId) >>
                      std::quoted(appointment.doctorId) >> std::quoted(appointment.date) >> std::quoted(appointment.time))) {
                    if (appointmentFile.eof()) {
                        break;
                    }
                    loadedSuccessfully = false;
                    appointmentFile.clear();
                    appointmentFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    continue;
                }

                if (!bookAppointment(appointment)) {
                    loadedSuccessfully = false;
                }
            }
        }
    }

    {
        std::ifstream emergencyFile(basePath / "emergency_queue.txt");
        if (emergencyFile.is_open()) {
            while (true) {
                std::string patientId;
                if (!(emergencyFile >> std::quoted(patientId))) {
                    if (emergencyFile.eof()) {
                        break;
                    }
                    loadedSuccessfully = false;
                    emergencyFile.clear();
                    emergencyFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    continue;
                }

                if (!addEmergencyCase(patientId)) {
                    loadedSuccessfully = false;
                }
            }
        }
    }

    return loadedSuccessfully;
}

bool HospitalManagementSystem::saveToFiles(const std::string& dataDirectory) const {
    namespace fs = std::filesystem;
    const fs::path basePath(dataDirectory);

    try {
        fs::create_directories(basePath);
    } catch (...) {
        return false;
    }

    {
        std::ofstream patientFile(basePath / "patients.txt", std::ios::trunc);
        if (!patientFile.is_open()) {
            return false;
        }

        const std::vector<Patient> patients = listPatientsSortedByName();
        for (const Patient& patient : patients) {
            patientFile << std::quoted(patient.id) << ' '
                        << std::quoted(patient.name) << ' '
                        << patient.age << ' '
                        << std::quoted(patient.symptoms) << ' '
                        << patient.severity << '\n';
        }
    }

    {
        std::ofstream doctorFile(basePath / "doctors.txt", std::ios::trunc);
        if (!doctorFile.is_open()) {
            return false;
        }

        const std::vector<Doctor> doctors = listDoctors();
        for (const Doctor& doctor : doctors) {
            doctorFile << std::quoted(doctor.id) << ' '
                       << std::quoted(doctor.name) << ' '
                       << std::quoted(doctor.specialization) << '\n';
        }
    }

    {
        std::ofstream appointmentFile(basePath / "appointments.txt", std::ios::trunc);
        if (!appointmentFile.is_open()) {
            return false;
        }

        const std::vector<Appointment> appointments = listAppointmentsSortedByDateTime();
        for (const Appointment& appointment : appointments) {
            appointmentFile << std::quoted(appointment.appointmentId) << ' '
                            << std::quoted(appointment.patientId) << ' '
                            << std::quoted(appointment.doctorId) << ' '
                            << std::quoted(appointment.date) << ' '
                            << std::quoted(appointment.time) << '\n';
        }
    }

    {
        std::ofstream emergencyFile(basePath / "emergency_queue.txt", std::ios::trunc);
        if (!emergencyFile.is_open()) {
            return false;
        }

        auto queueCopy = emergencyQueue_;
        while (!queueCopy.empty()) {
            const auto nextCase = queueCopy.top();
            queueCopy.pop();
            emergencyFile << std::quoted(nextCase.patientId) << '\n';
        }
    }

    return true;
}

std::optional<Patient> HospitalManagementSystem::findPatientById(const std::string& patientId) const {
    const auto it = patientsById_.find(patientId);
    if (it == patientsById_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::optional<Patient> HospitalManagementSystem::findPatientByNameBinarySearch(const std::string& name) const {
    if (name.empty()) {
        return std::nullopt;
    }

    const std::string nameKey = toLower(name);
    const auto it = std::lower_bound(
        patientNameIndex_.begin(),
        patientNameIndex_.end(),
        std::make_pair(nameKey, std::string())
    );

    if (it == patientNameIndex_.end() || it->first != nameKey) {
        return std::nullopt;
    }

    const auto patientIt = patientsById_.find(it->second);
    if (patientIt == patientsById_.end()) {
        return std::nullopt;
    }

    return patientIt->second;
}

std::vector<Patient> HospitalManagementSystem::listPatientsSortedByName() const {
    std::vector<Patient> result;
    result.reserve(patientNameIndex_.size());

    for (const auto& entry : patientNameIndex_) {
        const auto patientIt = patientsById_.find(entry.second);
        if (patientIt != patientsById_.end()) {
            result.push_back(patientIt->second);
        }
    }

    return result;
}

std::vector<Doctor> HospitalManagementSystem::listDoctors() const {
    std::vector<Doctor> result;
    result.reserve(doctorsById_.size());

    for (const auto& item : doctorsById_) {
        result.push_back(item.second);
    }

    std::sort(result.begin(), result.end(), [](const Doctor& lhs, const Doctor& rhs) {
        return lhs.id < rhs.id;
    });

    return result;
}

std::vector<Appointment> HospitalManagementSystem::listAppointmentsSortedByDateTime() const {
    std::vector<Appointment> sorted = appointments_;
    std::sort(sorted.begin(), sorted.end(), compareAppointmentDateTime);
    return sorted;
}

bool HospitalManagementSystem::addEmergencyCase(const std::string& patientId) {
    const auto it = patientsById_.find(patientId);
    if (it == patientsById_.end()) {
        return false;
    }

    emergencyQueue_.push(EmergencyCase{it->second.severity, emergencyTicketCounter_++, patientId});
    return true;
}

std::optional<Patient> HospitalManagementSystem::treatNextEmergency() {
    if (emergencyQueue_.empty()) {
        return std::nullopt;
    }

    const EmergencyCase nextCase = emergencyQueue_.top();
    emergencyQueue_.pop();

    const auto it = patientsById_.find(nextCase.patientId);
    if (it == patientsById_.end()) {
        return std::nullopt;
    }

    return it->second;
}

bool HospitalManagementSystem::patientExists(const std::string& patientId) const {
    return patientsById_.find(patientId) != patientsById_.end();
}

bool HospitalManagementSystem::doctorExists(const std::string& doctorId) const {
    return doctorsById_.find(doctorId) != doctorsById_.end();
}

std::size_t HospitalManagementSystem::patientCount() const {
    return patientsById_.size();
}

std::size_t HospitalManagementSystem::doctorCount() const {
    return doctorsById_.size();
}

std::size_t HospitalManagementSystem::appointmentCount() const {
    return appointments_.size();
}

std::size_t HospitalManagementSystem::pendingEmergencyCount() const {
    return emergencyQueue_.size();
}

void HospitalManagementSystem::clearAllData() {
    patientsById_.clear();
    doctorsById_.clear();
    appointmentIds_.clear();
    appointments_.clear();
    patientNameIndex_.clear();

    while (!emergencyQueue_.empty()) {
        emergencyQueue_.pop();
    }

    emergencyTicketCounter_ = 0;
}

void HospitalManagementSystem::printSystemStats() const {
    std::cout << "\n--- System Stats ---\n";
    std::cout << "Patients: " << patientsById_.size() << "\n";
    std::cout << "Doctors: " << doctorsById_.size() << "\n";
    std::cout << "Appointments: " << appointments_.size() << "\n";
    std::cout << "Pending emergency cases: " << emergencyQueue_.size() << "\n";
}

std::string HospitalManagementSystem::toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

bool HospitalManagementSystem::compareAppointmentDateTime(const Appointment& lhs, const Appointment& rhs) {
    if (lhs.date == rhs.date) {
        return lhs.time < rhs.time;
    }
    return lhs.date < rhs.date;
}
