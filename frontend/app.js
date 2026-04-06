function showToast(message, isError = false) {
    const toast = document.getElementById("toast");
    toast.textContent = message;
    toast.style.background = isError ? "#8f2f1f" : "#122d40";
    toast.classList.add("show");
    window.setTimeout(() => {
        toast.classList.remove("show");
    }, 2200);
}

function getApiBase() {
    const params = new URLSearchParams(window.location.search);
    const apiFromQuery = params.get("api");
    if (apiFromQuery) {
        return apiFromQuery.replace(/\/$/, "");
    }

    if (window.location.port === "8080") {
        return "";
    }

    return "http://127.0.0.1:8080";
}

const API_BASE = getApiBase();

function setConnectionStatus(isOnline) {
    const badge = document.getElementById("connectionBadge");
    if (!badge) {
        return;
    }

    if (isOnline) {
        badge.textContent = "API Online";
        badge.classList.remove("status-offline");
        badge.classList.add("status-online");
    } else {
        badge.textContent = "API Offline";
        badge.classList.remove("status-online");
        badge.classList.add("status-offline");
    }
}

function escapeHtml(value) {
    return String(value)
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");
}

async function api(path, options = {}) {
    const requestOptions = { ...options };
    if (requestOptions.body && !requestOptions.headers) {
        requestOptions.headers = { "Content-Type": "application/json" };
    }

    const url = `${API_BASE}${path}`;
    let response;
    try {
        response = await fetch(url, requestOptions);
    } catch {
        throw new Error("Cannot reach C++ API server. Start hms_web on port 8080.");
    }

    const contentType = response.headers.get("content-type") || "";
    const payload = contentType.includes("application/json") ? await response.json() : await response.text();

    if (!response.ok) {
        let errorMessage = "Request failed";
        if (typeof payload === "string") {
            const normalized = payload.trim().toLowerCase();
            if (normalized.startsWith("<!doctype") || normalized.startsWith("<html")) {
                errorMessage = `API endpoint not found at ${url}. Check server route.`;
            } else {
                errorMessage = payload;
            }
        } else {
            errorMessage = payload.error || "Request failed";
        }

        throw new Error(errorMessage);
    }

    return payload;
}

function renderPatients(items) {
    const body = document.getElementById("patientsBody");
    if (!items.length) {
        body.innerHTML = '<tr><td colspan="5" class="empty-row">No patients available</td></tr>';
        return;
    }

    body.innerHTML = items
        .map((item) => `
            <tr>
                <td>${escapeHtml(item.id)}</td>
                <td>${escapeHtml(item.name)}</td>
                <td>${escapeHtml(item.age)}</td>
                <td>${escapeHtml(item.symptoms)}</td>
                <td>${escapeHtml(item.severity)}</td>
            </tr>
        `)
        .join("");
}

function renderDoctors(items) {
    const body = document.getElementById("doctorsBody");
    if (!items.length) {
        body.innerHTML = '<tr><td colspan="3" class="empty-row">No doctors available</td></tr>';
        return;
    }

    body.innerHTML = items
        .map((item) => `
            <tr>
                <td>${escapeHtml(item.id)}</td>
                <td>${escapeHtml(item.name)}</td>
                <td>${escapeHtml(item.specialization)}</td>
            </tr>
        `)
        .join("");
}

function renderAppointments(items) {
    const body = document.getElementById("appointmentsBody");
    if (!items.length) {
        body.innerHTML = '<tr><td colspan="5" class="empty-row">No appointments available</td></tr>';
        return;
    }

    body.innerHTML = items
        .map((item) => `
            <tr>
                <td>${escapeHtml(item.appointmentId)}</td>
                <td>${escapeHtml(item.patientId)}</td>
                <td>${escapeHtml(item.doctorId)}</td>
                <td>${escapeHtml(item.date)}</td>
                <td>${escapeHtml(item.time)}</td>
            </tr>
        `)
        .join("");
}

async function refreshAll() {
    try {
        const [stats, patients, doctors, appointments] = await Promise.all([
            api("/api/stats"),
            api("/api/patients"),
            api("/api/doctors"),
            api("/api/appointments")
        ]);

        document.getElementById("statPatients").textContent = stats.patients;
        document.getElementById("statDoctors").textContent = stats.doctors;
        document.getElementById("statAppointments").textContent = stats.appointments;
        document.getElementById("statEmergency").textContent = stats.pendingEmergency;

        renderPatients(patients);
        renderDoctors(doctors);
        renderAppointments(appointments);
        setConnectionStatus(true);
    } catch (error) {
        setConnectionStatus(false);
        showToast(error.message, true);
    }
}

document.getElementById("refreshBtn").addEventListener("click", refreshAll);

document.getElementById("patientForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    const payload = {
        id: document.getElementById("pId").value.trim(),
        name: document.getElementById("pName").value.trim(),
        age: Number(document.getElementById("pAge").value),
        symptoms: document.getElementById("pSymptoms").value.trim(),
        severity: Number(document.getElementById("pSeverity").value)
    };

    try {
        await api("/api/patients", { method: "POST", body: JSON.stringify(payload) });
        event.target.reset();
        showToast("Patient added");
        refreshAll();
    } catch (error) {
        showToast(error.message, true);
    }
});

document.getElementById("doctorForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    const payload = {
        id: document.getElementById("dId").value.trim(),
        name: document.getElementById("dName").value.trim(),
        specialization: document.getElementById("dSpec").value.trim()
    };

    try {
        await api("/api/doctors", { method: "POST", body: JSON.stringify(payload) });
        event.target.reset();
        showToast("Doctor added");
        refreshAll();
    } catch (error) {
        showToast(error.message, true);
    }
});

document.getElementById("appointmentForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    const payload = {
        appointmentId: document.getElementById("aId").value.trim(),
        patientId: document.getElementById("aPatientId").value.trim(),
        doctorId: document.getElementById("aDoctorId").value.trim(),
        date: document.getElementById("aDate").value,
        time: document.getElementById("aTime").value
    };

    try {
        await api("/api/appointments", { method: "POST", body: JSON.stringify(payload) });
        event.target.reset();
        showToast("Appointment booked");
        refreshAll();
    } catch (error) {
        showToast(error.message, true);
    }
});

document.getElementById("emergencyForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    const payload = { patientId: document.getElementById("ePatientId").value.trim() };

    try {
        await api("/api/emergency", { method: "POST", body: JSON.stringify(payload) });
        event.target.reset();
        showToast("Emergency case queued");
        refreshAll();
    } catch (error) {
        showToast(error.message, true);
    }
});

document.getElementById("treatBtn").addEventListener("click", async () => {
    try {
        const treated = await api("/api/emergency/next", { method: "POST" });
        document.getElementById("treatedPatient").textContent = `Treated: ${treated.name} (${treated.id})`;
        showToast("Emergency case treated");
        refreshAll();
    } catch (error) {
        document.getElementById("treatedPatient").textContent = "";
        showToast(error.message, true);
    }
});

refreshAll();

