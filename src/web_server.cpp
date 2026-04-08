
// HMS Web Server: Exposes REST API and serves frontend
#include "hospital_system.h"

#include <httplib.h> // REST API server (cpp-httplib)
#include <nlohmann/json.hpp> // JSON serialization/deserialization

#include <filesystem> // File and directory operations
#include <iostream>
#include <mutex> // Thread safety for shared HMS instance
#include <string>
#include <vector>

using json = nlohmann::json; // Alias for convenience

// Utility functions for JSON conversion and error handling
namespace {
// Convert Patient struct to JSON
json patientToJson(const Patient& patient) {
    return json{
        {"id", patient.id},
        {"name", patient.name},
        {"age", patient.age},
        {"symptoms", patient.symptoms},
        {"severity", patient.severity}
    };
}

// Convert Doctor struct to JSON
json doctorToJson(const Doctor& doctor) {
    return json{
        {"id", doctor.id},
        {"name", doctor.name},
        {"specialization", doctor.specialization}
    };
}

// Convert Appointment struct to JSON
json appointmentToJson(const Appointment& appointment) {
    return json{
        {"appointmentId", appointment.appointmentId},
        {"patientId", appointment.patientId},
        {"doctorId", appointment.doctorId},
        {"date", appointment.date},
        {"time", appointment.time}
    };
}

// Set JSON response with status code
void setJson(httplib::Response& response, const json& body, int status = 200) {
    response.status = status;
    response.set_content(body.dump(), "application/json");
}

// Set error response as JSON
void setError(httplib::Response& response, int status, const std::string& message) {
    setJson(response, json{{"error", message}}, status);
}

// Parse JSON body from HTTP request
bool parseJsonBody(const httplib::Request& request, httplib::Response& response, json& body) {
    try {
        body = json::parse(request.body);
        return true;
    } catch (...) {
        setError(response, 400, "Invalid JSON body.");
        return false;
    }
}
}  // namespace

// Main entry: launches REST API and serves frontend
// Uses mutex for thread safety (multiple requests)
// Delegates all data operations to HospitalManagementSystem (DAA: Hash Table, Priority Queue, Binary Search, etc.)
int main() {
    namespace fs = std::filesystem;
    const std::string dataDirectory = "data";
    HospitalManagementSystem system;
    std::mutex dataMutex;

    // Load persistent data (DAA: File I/O)
    if (!system.loadFromFiles(dataDirectory)) {
        std::cerr << "Warning: some records could not be loaded from data files.\n";
    }

    // REST API server
    httplib::Server server;

    // CORS headers for browser access
    server.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Headers", "Content-Type"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"}
    });

    // Handle CORS preflight requests
    server.Options(R"(/api/.*)", [](const httplib::Request&, httplib::Response& response) {
        response.status = 204;
    });

    // Try to mount the frontend directory (serves static files)
    const std::vector<std::string> frontendCandidates = {
        "./frontend",
        "../frontend",
        "../../frontend"
    };

    bool mounted = false;
    std::string mountedFrontendPath;
    for (const auto& candidate : frontendCandidates) {
        const fs::path path(candidate);
        if (fs::exists(path / "index.html") && server.set_mount_point("/", candidate.c_str())) {
            mounted = true;
            mountedFrontendPath = candidate;
            break;
        }
    }

    if (!mounted) {
        std::cerr << "Failed to mount frontend directory. Ensure frontend/index.html exists.\n";
        return 1;
    }

    // Helper: persist data after mutating API calls
    auto persistOrError = [&](httplib::Response& response) -> bool {
        if (!system.saveToFiles(dataDirectory)) {
            setError(response, 500, "Failed to save data files.");
            return false;
        }
        return true;
    };

    // Health check endpoint
    server.Get("/api/health", [](const httplib::Request&, httplib::Response& response) {
        setJson(response, json{{"status", "ok"}});
    });

    // System stats endpoint (DAA: Hash Table size, Priority Queue size)
    server.Get("/api/stats", [&](const httplib::Request&, httplib::Response& response) {
        std::lock_guard<std::mutex> lock(dataMutex);
        setJson(response, json{
            {"patients", system.patientCount()},
            {"doctors", system.doctorCount()},
            {"appointments", system.appointmentCount()},
            {"pendingEmergency", system.pendingEmergencyCount()}
        });
    });

    // List all patients (DAA: Sorted Vector traversal)
    server.Get("/api/patients", [&](const httplib::Request&, httplib::Response& response) {
        std::lock_guard<std::mutex> lock(dataMutex);
        const std::vector<Patient> patients = system.listPatientsSortedByName();
        json body = json::array();
        for (const auto& patient : patients) {
            body.push_back(patientToJson(patient));
        }
        setJson(response, body);
    });

    // Add patient (DAA: Hash Table for ID, Sorted Vector for name)
    server.Post("/api/patients", [&](const httplib::Request& request, httplib::Response& response) {
        std::lock_guard<std::mutex> lock(dataMutex);
        json body;
        if (!parseJsonBody(request, response, body)) {
            return;
        }

        Patient patient;
        patient.id = body.value("id", "");
        patient.name = body.value("name", "");
        patient.age = body.value("age", 0);
        patient.symptoms = body.value("symptoms", "");
        patient.severity = body.value("severity", 1);

        if (!system.addPatient(patient)) {
            setError(response, 400, "Could not add patient. Check duplicate ID or invalid fields.");
            return;
        }

        if (!persistOrError(response)) {
            return;
        }

        setJson(response, patientToJson(patient), 201);
    });

    // List all doctors (DAA: Hash Table traversal)
    server.Get("/api/doctors", [&](const httplib::Request&, httplib::Response& response) {
        std::lock_guard<std::mutex> lock(dataMutex);
        const std::vector<Doctor> doctors = system.listDoctors();
        json body = json::array();
        for (const auto& doctor : doctors) {
            body.push_back(doctorToJson(doctor));
        }
        setJson(response, body);
    });

    // Add doctor (DAA: Hash Table)
    server.Post("/api/doctors", [&](const httplib::Request& request, httplib::Response& response) {
        std::lock_guard<std::mutex> lock(dataMutex);
        json body;
        if (!parseJsonBody(request, response, body)) {
            return;
        }

        Doctor doctor;
        doctor.id = body.value("id", "");
        doctor.name = body.value("name", "");
        doctor.specialization = body.value("specialization", "");

        if (!system.addDoctor(doctor)) {
            setError(response, 400, "Could not add doctor. Check duplicate ID or invalid fields.");
            return;
        }

        if (!persistOrError(response)) {
            return;
        }

        setJson(response, doctorToJson(doctor), 201);
    });

    // List all appointments (DAA: List, Sorting by date/time)
    server.Get("/api/appointments", [&](const httplib::Request&, httplib::Response& response) {
        std::lock_guard<std::mutex> lock(dataMutex);
        const std::vector<Appointment> appointments = system.listAppointmentsSortedByDateTime();
        json body = json::array();
        for (const auto& appointment : appointments) {
            body.push_back(appointmentToJson(appointment));
        }
        setJson(response, body);
    });

    // Book appointment (DAA: List, Hash Set for unique IDs)
    server.Post("/api/appointments", [&](const httplib::Request& request, httplib::Response& response) {
        std::lock_guard<std::mutex> lock(dataMutex);
        json body;
        if (!parseJsonBody(request, response, body)) {
            return;
        }

        Appointment appointment;
        appointment.appointmentId = body.value("appointmentId", "");
        appointment.patientId = body.value("patientId", "");
        appointment.doctorId = body.value("doctorId", "");
        appointment.date = body.value("date", "");
        appointment.time = body.value("time", "");

        if (!system.bookAppointment(appointment)) {
            setError(response, 400, "Could not book appointment. Check IDs and unique appointment ID.");
            return;
        }

        if (!persistOrError(response)) {
            return;
        }

        setJson(response, appointmentToJson(appointment), 201);
    });

    // Add emergency case (DAA: Priority Queue/Heap)
    server.Post("/api/emergency", [&](const httplib::Request& request, httplib::Response& response) {
        std::lock_guard<std::mutex> lock(dataMutex);
        json body;
        if (!parseJsonBody(request, response, body)) {
            return;
        }

        const std::string patientId = body.value("patientId", "");
        if (!system.addEmergencyCase(patientId)) {
            setError(response, 400, "Could not enqueue emergency case. Patient ID not found.");
            return;
        }

        if (!persistOrError(response)) {
            return;
        }

        setJson(response, json{{"queuedPatientId", patientId}}, 201);
    });

    // Treat next emergency (DAA: Priority Queue/Heap)
    server.Post("/api/emergency/next", [&](const httplib::Request&, httplib::Response& response) {
        std::lock_guard<std::mutex> lock(dataMutex);
        const std::optional<Patient> treated = system.treatNextEmergency();
        if (!treated.has_value()) {
            setError(response, 404, "No emergency cases in queue.");
            return;
        }

        if (!persistOrError(response)) {
            return;
        }

        setJson(response, patientToJson(treated.value()));
    });

    // Startup message
    std::cout << "HMS web server running at http://localhost:8080\n";
    std::cout << "Serving frontend from " << mountedFrontendPath << " and API from /api/*\n";

    if (!server.listen("0.0.0.0", 8080)) {
        std::cerr << "Failed to start web server on port 8080.\n";
        return 1;
    }

    return 0;
}
