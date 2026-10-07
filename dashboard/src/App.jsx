import { useEffect, useState } from "react";
import mqtt from "mqtt";

export default function App() {
  const [connesso, setConnesso] = useState(false);
  const [temperature, setTemperature] = useState({});
  const [allarmi, setAllarmi] = useState({});
  const [umidita, setUmidita] = useState({});
  const [stanze, setStanze] = useState({});
  useEffect(() => {
    const client = mqtt.connect("ws://localhost:9001");

    client.on("connect", () => {
      setConnesso(true);
      client.subscribe(["casa/+/temperatura", "casa/+/allarme", "casa/+/umidita"]);
    });
    client.on("close", () => setConnesso(false));

    client.on("message", (topic, msg) => {
      //["casa"=0, "cucina"=1 , "temperatura"=2]
      const parti = topic.split("/") ;
      const stanza = parti[1];
      const tipo = parti[2];
     
      
      
      if(tipo === "temperatura") {
        try {
          const dati = JSON.parse(msg.toString());
          setStanze((prec) => ({
          ... prec,
          [stanza]:{... prec[stanza],temp:dati.temp},
        }));
         // setTemperature((prec) => ({ ...prec, [stanza]: dati.temp }));
        } catch {
          console.log("Messaggio non valido:", msg.toString());
        }
      }else if(tipo === "allarme"){
        setStanze((prec) => ({
          ... prec,
          [stanza]:{... prec[stanza],allarmi:msg.toString()},
        }));
        //setAllarmi((prec) => ({ ...prec, [stanza]: msg.toString() }));
      }else if(tipo === "umidita"){
        try {
          const dati = JSON.parse(msg.toString());
          setStanze((prec) => ({
            ... prec,
            [stanza]:{... prec[stanza],umidita:dati.umidita},
          }));
          // setUmidita((prec) => ({ ...prec, [stanza]: dati.umidita }));
        } catch {
          console.log("Messaggio non valido:", msg.toString());
        }
      }
    });

    return () => client.end();
  }, []);

  return (
    <div style={{ fontFamily: "sans-serif", padding: 24 }}>
      <h1>Casa domotica</h1>
      <p>Broker: {connesso ? "connesso" : "disconnesso"}</p>

      <div style={{ display: "flex", gap: 16 }}>
        {Object.entries(stanze).map(([stanza, info]) => (
          <div key={stanza} style={{ padding: 16, borderRadius: 12, background: info.allarmi==="caldo" ? "#ffd6d6" : "#e3f1ff", minWidth: 140 }}>
            <h3>{stanza}</h3>
            <p>Temperatura:{info.temp} {info.temp ?? "nessun dato"}°C</p>
            <p>Allarme: {info.allarmi ?? "nessun dato"}</p>
            <p>Umidità: {info.umidita ?? "nessun dato"}%</p>
          </div>
        ))}
      </div>
    </div>
  );
}