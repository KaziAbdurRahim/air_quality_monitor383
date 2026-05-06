// app/page.tsx
"use client";
import React, { useEffect, useState } from "react";
import { getSensorData } from "./lib/api";
import { SensorData } from "./types/SensorData";
import { ChartCard } from "@/components/ChartCard";
import { ControlPanel } from "@/components/ControlPanel";
import { StatusCard } from "@/components/StatusCard";

export default function Dashboard() {
  const [data, setData] = useState<SensorData[]>([]);
  const [connected, setConnected] = useState(false);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const sensor = await getSensorData();
        setConnected(true);
        setData((prev) => [...prev.slice(-19), sensor]);
      } catch (error) {
        setConnected(false);
      }
    };
    fetchData();
    const interval = setInterval(fetchData, 5000);
    return () => clearInterval(interval);
  }, []);

  const latest = data[data.length - 1];

  return (
    <main className="grid grid-cols-1 md:grid-cols-2 gap-6 p-6">
      <StatusCard connected={connected} latest={latest} />
      {connected && (
        <>
          <ControlPanel />
          <ChartCard data={data} label="Temperature (°C)" field="temperature" />
          <ChartCard data={data} label="Humidity (%)" field="humidity" />
          <ChartCard data={data} label="CO₂ (ppm)" field="co2" />
          <ChartCard data={data} label="AQI" field="aqi" />
        </>
      )}
    </main>
  );
}
