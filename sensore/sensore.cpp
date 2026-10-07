#include <iostream>
#include <string>
#include <sstream>
#include <mosquitto.h>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <random>
#include <thread>
using namespace std;

string crea_json(const string& stanza, double temp, int n) {
    ostringstream json;
    json << fixed << setprecision(1)
         << "{\"sensore\":\"" << stanza << "\",\"temp\":" << temp << ",\"n\":" << n << "}";
    return json.str();
}

string crea_json_umidita(const string& stanza, int umidita, int n) {
    ostringstream json;
    json << "{\"sensore\":\"" << stanza << "\",\"umidita\":" << umidita << ",\"n\":" << n << "}";
    return json.str();
}

bool pubblica(mosquitto* m, const string& topic, const string& payload, int qos, bool retain) {
    int rc = mosquitto_publish(m, nullptr, topic.c_str(), payload.size(), payload.c_str(), qos, retain);
    if (rc != MOSQ_ERR_SUCCESS) {
        cout << "Errore su " << topic << ": " << mosquitto_strerror(rc) << endl;
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    cout << unitbuf;

    string stanza = (argc > 1) ? argv[1] : "soggiorno";
    const char* host = getenv("MQTT_HOST");
    if (!host) host = "localhost";
    const char* porta_env = getenv("MQTT_PORT");
    int porta = 1883;
    if (porta_env) porta = atoi(porta_env);
    int intervallo = (argc > 2) ? stoi(argv[2]) : 5;
    if (intervallo < 1) intervallo = 1;

    mosquitto_lib_init();
    mosquitto* m = mosquitto_new(("sensore-" + stanza).c_str(), true, nullptr);

    while (mosquitto_connect(m, host, porta, 60) != MOSQ_ERR_SUCCESS) {
        cout << "Broker " << host << ":" << porta << " non raggiungibile, riprovo..." << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }
    mosquitto_loop_start(m);

    double temperatura = 21.0;
    int umidita = 50;
    mt19937 gen(random_device{}());
    uniform_real_distribution<> dist_temperatura(-0.3, 0.3);
    uniform_int_distribution<> dist_umidita(-1, 1);

    string topic_temp = "casa/" + stanza + "/temperatura";
    string topic_umid = "casa/" + stanza + "/umidita";
    string topic_allarme = "casa/" + stanza + "/allarme";

    int i = 0;
    bool in_allarme = false;

    while (true) {
        i++;

        temperatura += dist_temperatura(gen);
        if (temperatura < 15) temperatura = 15;
        if (temperatura > 30) temperatura = 30;

        umidita += dist_umidita(gen);
        if (umidita < 30) umidita = 30;
        if (umidita > 70) umidita = 70;

        if (temperatura > 25 && !in_allarme) {
            pubblica(m, topic_allarme, "caldo", 1, true);
            cout << topic_allarme << " -> caldo" << endl;
            in_allarme = true;
        }
        else if (temperatura <= 25 && in_allarme) {
            pubblica(m, topic_allarme, "ok", 1, true);
            cout << topic_allarme << " -> ok" << endl;
            in_allarme = false;
        }

        string payload_temp = crea_json(stanza, temperatura, i);
        string payload_umid = crea_json_umidita(stanza, umidita, i);
        pubblica(m, topic_temp, payload_temp, 0, false);
        pubblica(m, topic_umid, payload_umid, 0, false);

        cout << "Messaggio n. " << i << endl;
        cout << "  " << topic_temp << " -> " << payload_temp << endl;
        cout << "  " << topic_umid << " -> " << payload_umid << endl;

        this_thread::sleep_for(chrono::seconds(intervallo));
    }
}