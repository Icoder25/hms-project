#pragma once

#include <optional>
#include <queue>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// Patient record (simple struct)
struct Patient {
    std::string id;
    std::string name;
    int age;
    std::string symptoms;
    int severity;
};

// Doctor record (simple struct)
struct Doctor {
    std::string id;
    std::string name;
    std::string specialization;
};

// Appointment record (simple struct)
struct Appointment {
    std::string appointmentId;
    std::string patientId;
    std::string doctorId;
    std::string date;
    std::string time;
};

// Main HMS class: uses DAA data structures for efficient operations
class HospitalManagementSystem {
public:
    // Adds patient, maintains sorted index for binary search (DAA: Binary Search, Sorting)
    bool addPatient(const Patient& patient);
    // Adds doctor (DAA: Hash Table)
    bool addDoctor(const Doctor& doctor);
    // Books appointment (DAA: Array/List)
    bool bookAppointment(const Appointment& appointment);

    // Loads all data from files (DAA: File I/O)
    bool loadFromFiles(const std::string& dataDirectory);
    // Saves all data to files (DAA: File I/O)
    bool saveToFiles(const std::string& dataDirectory) const;

    // Finds patient by ID (DAA: Hash Table)
    std::optional<Patient> findPatientById(const std::string& patientId) const;
    // Finds patient by name using binary search (DAA: Binary Search)
    std::optional<Patient> findPatientByNameBinarySearch(const std::string& name) const;

    // Returns patients sorted by name (DAA: Sorting)
    std::vector<Patient> listPatientsSortedByName() const;
    // Returns all doctors (DAA: Hash Table traversal)
    std::vector<Doctor> listDoctors() const;
    // Returns appointments sorted by date/time (DAA: Sorting)
    std::vector<Appointment> listAppointmentsSortedByDateTime() const;

    // Adds emergency case to priority queue (DAA: Priority Queue/Heap)
    bool addEmergencyCase(const std::string& patientId);
    // Treats next emergency case (DAA: Priority Queue/Heap)
    std::optional<Patient> treatNextEmergency();

    // Checks if patient exists (DAA: Hash Table)
    bool patientExists(const std::string& patientId) const;
    // Checks if doctor exists (DAA: Hash Table)
    bool doctorExists(const std::string& doctorId) const;

    // Returns patient count (DAA: Hash Table size)
    std::size_t patientCount() const;
    // Returns doctor count (DAA: Hash Table size)
    std::size_t doctorCount() const;
    // Returns appointment count (DAA: Array/List size)
    std::size_t appointmentCount() const;
    // Returns pending emergency count (DAA: Priority Queue size)
    std::size_t pendingEmergencyCount() const;

    void printSystemStats() const;

private:
    // Emergency case for priority queue (DAA: Heap node)
    struct EmergencyCase {
        int severity;
        long long ticket;
        std::string patientId;
    };

    // Comparator for emergency priority queue (DAA: Heap comparator)
    struct EmergencyComparator {
        bool operator()(const EmergencyCase& lhs, const EmergencyCase& rhs) const;
    };

    // Hash table for fast patient lookup (DAA: Hash Table)
    std::unordered_map<std::string, Patient> patientsById_;
    // Hash table for fast doctor lookup (DAA: Hash Table)
    std::unordered_map<std::string, Doctor> doctorsById_;
    // Hash set for unique appointment IDs (DAA: Hash Set)
    std::unordered_set<std::string> appointmentIds_;

    // List of appointments (DAA: Array/List)
    std::vector<Appointment> appointments_;
    // Sorted vector for patient name index (DAA: Sorted Array for Binary Search)
    std::vector<std::pair<std::string, std::string>> patientNameIndex_;

    // Priority queue for emergencies (DAA: Heap/Priority Queue)
    std::priority_queue<EmergencyCase, std::vector<EmergencyCase>, EmergencyComparator> emergencyQueue_;
    // Monotonic counter for emergency queue tie-breaking
    long long emergencyTicketCounter_ = 0;

    // Utility: convert string to lowercase
    static std::string toLower(std::string text);
    // Utility: compare appointments by date/time
    static bool compareAppointmentDateTime(const Appointment& lhs, const Appointment& rhs);
    // Utility: clear all data structures
    void clearAllData();
};
