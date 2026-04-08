#include "hospital_system.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin >> std::ws, input);
    return input;
}

int readInt(const std::string& prompt, int minValue, int maxValue) {
    while (true) {
        std::cout << prompt;
        std::string raw;
        std::getline(std::cin >> std::ws, raw);

        std::stringstream parser(raw);
        int value = 0;
        char leftover = '\0';
        if ((parser >> value) && !(parser >> leftover) && value >= minValue && value <= maxValue) {
            return value;
        }

        std::cout << "Invalid input. Enter a number between " << minValue << " and " << maxValue << ".\n";
    }
}

// Print patient details (utility)
void printPatient(const Patient& patient) {
    std::cout << "Patient ID: " << patient.id << "\n";
    std::cout << "Name: " << patient.name << "\n";
    std::cout << "Age: " << patient.age << "\n";
    std::cout << "Symptoms: " << patient.symptoms << "\n";
    std::cout << "Severity (1-10): " << patient.severity << "\n";
}

// Print appointment details (utility)
void printAppointment(const Appointment& appointment) {
    std::cout << "Appointment ID: " << appointment.appointmentId << " | "
              << "Patient ID: " << appointment.patientId << " | "
              << "Doctor ID: " << appointment.doctorId << " | "
              << "Date: " << appointment.date << " | "
              << "Time: " << appointment.time << "\n";
}

void saveImmediately(HospitalManagementSystem& system, const std::string& dataDirectory) {
    if (system.saveToFiles(dataDirectory)) {
        std::cout << "Data files updated immediately.\n";
    } else {
        std::cout << "Warning: change made in memory, but file save failed.\n";
    }
}

// Add patient: uses hash table for ID, sorted vector for name (DAA: Hash Table, Binary Search)
void addPatientFlow(HospitalManagementSystem& system, const std::string& dataDirectory) {
    Patient patient;
    patient.id = readLine("Enter patient ID: ");
    patient.name = readLine("Enter patient name: ");
    patient.age = readInt("Enter patient age (1-120): ", 1, 120);
    patient.symptoms = readLine("Enter symptoms: ");
    patient.severity = readInt("Enter severity (1-10): ", 1, 10);

    if (system.addPatient(patient)) {
        std::cout << "Patient added successfully.\n";
        saveImmediately(system, dataDirectory);
    } else {
        std::cout << "Unable to add patient (duplicate ID or invalid data).\n";
    }
}

// Add doctor: uses hash table for fast lookup (DAA: Hash Table)
void addDoctorFlow(HospitalManagementSystem& system, const std::string& dataDirectory) {
    Doctor doctor;
    doctor.id = readLine("Enter doctor ID: ");
    doctor.name = readLine("Enter doctor name: ");
    doctor.specialization = readLine("Enter specialization: ");

    if (system.addDoctor(doctor)) {
        std::cout << "Doctor added successfully.\n";
        saveImmediately(system, dataDirectory);
    } else {
        std::cout << "Unable to add doctor (duplicate ID or invalid data).\n";
    }
}

// Book appointment: uses list for storage, hash set for unique IDs (DAA: List, Hash Set)
void bookAppointmentFlow(HospitalManagementSystem& system, const std::string& dataDirectory) {
    Appointment appointment;
    appointment.appointmentId = readLine("Enter appointment ID: ");
    appointment.patientId = readLine("Enter patient ID: ");
    appointment.doctorId = readLine("Enter doctor ID: ");
    appointment.date = readLine("Enter date (YYYY-MM-DD): ");
    appointment.time = readLine("Enter time (HH:MM): ");

    if (system.bookAppointment(appointment)) {
        std::cout << "Appointment booked successfully.\n";
        saveImmediately(system, dataDirectory);
    } else {
        std::cout << "Unable to book appointment. Ensure IDs exist and appointment ID is unique.\n";
    }
}

// Search patient by ID: uses hash table lookup (DAA: Hash Table)
void searchPatientByIdFlow(const HospitalManagementSystem& system) {
    const std::string patientId = readLine("Enter patient ID to search: ");
    const std::optional<Patient> patient = system.findPatientById(patientId);

    if (!patient.has_value()) {
        std::cout << "No patient found for ID: " << patientId << "\n";
        return;
    }

    std::cout << "Patient found:\n";
    printPatient(patient.value());
}

// Search patient by name: uses binary search on sorted vector (DAA: Binary Search)
void searchPatientByNameFlow(const HospitalManagementSystem& system) {
    const std::string name = readLine("Enter patient name to search (binary search): ");
    const std::optional<Patient> patient = system.findPatientByNameBinarySearch(name);

    if (!patient.has_value()) {
        std::cout << "No patient found for name: " << name << "\n";
        return;
    }

    std::cout << "Patient found:\n";
    printPatient(patient.value());
}

// List patients: traverses sorted vector (DAA: Sorting, Traversal)
void listPatientsFlow(const HospitalManagementSystem& system) {
    const std::vector<Patient> patients = system.listPatientsSortedByName();
    if (patients.empty()) {
        std::cout << "No patients in the system.\n";
        return;
    }

    std::cout << "\nPatients (sorted by name):\n";
    for (const Patient& patient : patients) {
        printPatient(patient);
        std::cout << "--------------------------\n";
    }
}

// List doctors: traverses hash table (DAA: Hash Table Traversal)
void listDoctorsFlow(const HospitalManagementSystem& system) {
    const std::vector<Doctor> doctors = system.listDoctors();
    if (doctors.empty()) {
        std::cout << "No doctors in the system.\n";
        return;
    }

    std::cout << "\nDoctors:\n";
    for (const Doctor& doctor : doctors) {
        std::cout << "Doctor ID: " << doctor.id << " | Name: " << doctor.name
                  << " | Specialization: " << doctor.specialization << "\n";
    }
}

// List appointments: sorts by date/time (DAA: Sorting)
void listAppointmentsFlow(const HospitalManagementSystem& system) {
    const std::vector<Appointment> appointments = system.listAppointmentsSortedByDateTime();
    if (appointments.empty()) {
        std::cout << "No appointments in the system.\n";
        return;
    }

    std::cout << "\nAppointments (sorted by date/time):\n";
    for (const Appointment& appointment : appointments) {
        printAppointment(appointment);
    }
}

// Add emergency: uses priority queue (heap) for emergencies (DAA: Priority Queue/Heap)
void addEmergencyFlow(HospitalManagementSystem& system, const std::string& dataDirectory) {
    const std::string patientId = readLine("Enter patient ID for emergency queue: ");
    if (system.addEmergencyCase(patientId)) {
        std::cout << "Emergency case added to queue.\n";
        saveImmediately(system, dataDirectory);
    } else {
        std::cout << "Unable to add emergency case (patient ID not found).\n";
    }
}

// Treat emergency: pops from priority queue (heap) (DAA: Priority Queue/Heap)
void treatEmergencyFlow(HospitalManagementSystem& system, const std::string& dataDirectory) {
    const std::optional<Patient> patient = system.treatNextEmergency();
    if (!patient.has_value()) {
        std::cout << "No emergency cases to treat.\n";
        return;
    }

    std::cout << "Treating next emergency patient:\n";
    printPatient(patient.value());
    saveImmediately(system, dataDirectory);
}

// Print menu (utility)
void printMenu() {
    std::cout << "\n===== Hospital Management System =====\n";
    std::cout << "1. Add patient\n";
    std::cout << "2. Add doctor\n";
    std::cout << "3. Book appointment\n";
    std::cout << "4. Search patient by ID\n";
    std::cout << "5. Search patient by name (Binary search)\n";
    std::cout << "6. List patients sorted by name\n";
    std::cout << "7. List appointments sorted by date/time\n";
    std::cout << "8. Add emergency case (Priority queue)\n";
    std::cout << "9. Treat next emergency\n";
    std::cout << "10. List doctors\n";
    std::cout << "11. Show system stats\n";
    std::cout << "0. Exit\n";
}

// Main function: CLI for HMS, each menu option uses a specific DAA approach
int main() {
    HospitalManagementSystem system;
    const std::string dataDirectory = "data";

    if (system.loadFromFiles(dataDirectory)) {
        std::cout << "Data loaded from .\\" << dataDirectory << " successfully.\n";
    } else {
        std::cout << "Data loaded with some issues. Check files in .\\" << dataDirectory << ".\n";
    }

    bool running = true;
    while (running) {
        printMenu();
        const int choice = readInt("Enter your choice: ", 0, 11);

        switch (choice) {
            case 1:
                addPatientFlow(system, dataDirectory);
                break;
            case 2:
                addDoctorFlow(system, dataDirectory);
                break;
            case 3:
                bookAppointmentFlow(system, dataDirectory);
                break;
            case 4:
                searchPatientByIdFlow(system);
                break;
            case 5:
                searchPatientByNameFlow(system);
                break;
            case 6:
                listPatientsFlow(system);
                break;
            case 7:
                listAppointmentsFlow(system);
                break;
            case 8:
                addEmergencyFlow(system, dataDirectory);
                break;
            case 9:
                treatEmergencyFlow(system, dataDirectory);
                break;
            case 10:
                listDoctorsFlow(system);
                break;
            case 11:
                system.printSystemStats();
                break;
            case 0:
                running = false;
                break;
            default:
                std::cout << "Unknown choice.\n";
                break;
        }
    }

    if (system.saveToFiles(dataDirectory)) {
        std::cout << "Data saved to .\\" << dataDirectory << " successfully.\n";
    } else {
        std::cout << "Failed to save data in .\\" << dataDirectory << ".\n";
    }

    std::cout << "Exiting Hospital Management System.\n";
    return 0;
}
