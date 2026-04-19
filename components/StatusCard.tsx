// app/components/StatusCard.tsx
"use client";
import React from "react";
// import { SensorData } from "../types/SensorData";
import { SensorData } from "@/app/types/SensorData";
interface StatusCardProps {
  connected: boolean;
  latest?: SensorData;
}

export const StatusCard: React.FC<StatusCardProps> = ({
  connected,
  latest,
}) => {
  return (
    <div className="rounded-xl bg-white/10 backdrop-blur-md p-6 shadow-lg">
      <h3 className="text-xl font-semibold mb-4">ESP32 Connection Status</h3>
      <div className="flex items-center gap-2 mb-4">
        <div
          className={`w-3 h-3 rounded-full ${
            connected ? "bg-green-400 animate-pulse" : "bg-red-500"
          }`}
        />
        <p>{connected ? "Connected ✅" : "Disconnected ❌"}</p>
      </div>
      <div className="space-y-1 text-sm">
        <p>
          <strong>Temperature:</strong> {connected ? latest?.temperature : 0} °C
        </p>
        <p>
          <strong>Humidity:</strong> {connected ? latest?.humidity : 0} %
        </p>
        <p>
          <strong>CO₂:</strong> {connected ? latest?.co2 : 0} ppm
        </p>
        <p>
          <strong>AQI:</strong> {connected ? latest?.aqi : 0}
        </p>
      </div>
    </div>
  );
};
