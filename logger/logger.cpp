#include <iostream>
#include <string>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <mosquitto.h>
#include <sqlite3.h>
using namespace std;
/**
 * Il programma deve:
    leggere MQTT_HOST e MQTT_PORT dall'ambiente, esattamente come il sensore;
    connettersi al broker;
    iscriversi a casa/+/temperatura;
    stampare topic e payload di ogni messaggio ricevuto.
 */
string stanza_da_topic(const string& topic){
    size_t a = topic.find('/');
    size_t b = topic.find('/',a+1);
    return topic.substr(a+1,b-a-1);

}

bool estrai_temp(const string& json, double& valore){
    size_t pos = json.find("\"temp\":");
    if(pos == string :: npos)return false;
    try{
        valore=stod(json.substr(pos+7));
    }catch(...){
        return false;
    }
    return true;
}

void on_connect(mosquitto* m, void* userdata, int rc) {
    if (rc == 0) {
        cout << "Connesso al broker "<<endl;
        mosquitto_subscribe(m,nullptr,"casa/+/temperatura",0);

    }
}void on_message(mosquitto* m , void* userdata, const mosquitto_message* msg){
    sqlite3* db = static_cast<sqlite3*>(userdata);
    string topic = msg->topic;
    string payload(static_cast<char*> (msg->payload),msg->payloadlen);
    
    string stanza = stanza_da_topic(topic);
   
    double valore;
    if(!estrai_temp(payload,valore)){
        cout<<"messaggio non valido: "<<payload<<endl;
        return;
    }
    //prepara la query con i segna posto
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db,"INSERT INTO letture (sensore,valore)VALUES(?,?)",-1, &stmt,nullptr);
    // collega i valori ai ? , gli indici aprtono da 1
    sqlite3_bind_text(stmt,1,stanza.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt,2,valore);

    //esecuzione
    int rc= sqlite3_step(stmt);
    if(rc!=SQLITE_DONE){
        cout<<"ERRORE SQL:"<<sqlite3_errmsg(db)<<endl;
    }else{
        cout<<"Salvato"<<stanza<<"="<<valore<<endl;
    }
    sqlite3_finalize(stmt);
}

int main(){
    cout << unitbuf;

    const char* db_path= getenv("DB_PATH");
    if(!db_path) db_path="casa.db";

    sqlite3* db =nullptr;
    if(sqlite3_open(db_path,&db)!=SQLITE_OK){
        cout<<"Impossibile aprire il database: "<<sqlite3_errmsg(db)<<endl;
        return 1;
    }
    cout<<"Database aperto:"<<db_path<<endl;

    const char* sql_tabella= 
    "CREATE TABLE IF NOT EXISTS letture("
    "id    INTEGER PRIMARY KEY AUTOINCREMENT,"
    "sensore TEXT NOT NULL,"
    "valore  REAL NOT NULL,"
    "ts DATETIME DEFAULT CURRENT_TIMESTAMP)";
    char* errore = nullptr;
    int rc=sqlite3_exec(db,sql_tabella,nullptr,nullptr, &errore);
    if(rc!= SQLITE_OK){
        cout<<"Errore creazione tabella:"<<errore<<endl;;
        sqlite3_free(errore);
        sqlite3_close(db);
        return 1;
    }
    cout<<"Tabella pronta"<<endl;
    const char* host = getenv("MQTT_HOST");
    if (!host) host = "localhost";
    const char* porta_env = getenv("MQTT_PORT");
    int porta = 1883;
    if (porta_env) porta = atoi(porta_env);

    mosquitto_lib_init();
    mosquitto* m = mosquitto_new("logger", true, db);

   
    mosquitto_connect_callback_set(m, on_connect);
   
    mosquitto_message_callback_set(m,on_message);
    while (mosquitto_connect(m, host, porta, 60) != MOSQ_ERR_SUCCESS) {
        cout << "Broker " << host << ":" << porta << " non raggiungibile, riprovo..." << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }

    mosquitto_loop_forever(m, -1, 1);
    sqlite3_close(db);
   
}