"use client";
import React from "react";
import { Line } from "react-chartjs-2";
// import { SensorData } from "@/types/SensorData";
import { SensorData } from "@/app/types/SensorData";
import {
  Chart as ChartJS,
  LineElement,
  CategoryScale,
  LinearScale,
  PointElement,
  Tooltip,
  Legend,
} from "chart.js";

ChartJS.register(
  LineElement,
  CategoryScale,
  LinearScale,
  PointElement,
  Tooltip,
  Legend,
);

interface ChartCardProps {
  data: SensorData[];
  label: string;
  field: keyof SensorData;
}

export const ChartCard: React.FC<ChartCardProps> = ({ data, label, field }) => {
  // If no data, show a default zero dataset
  const safeData =
    data.length > 0
      ? data
      : [
          {
            temperature: 0,
            humidity: 0,
            co2: 0,
            aqi: 0,
            timestamp: new Date().toISOString(),
          },
        ];

  const chartData = {
    labels: safeData.map((d) => new Date(d.timestamp).toLocaleTimeString()),
    datasets: [
      {
        label,
        data: safeData.map((d) => d[field] as number),
        borderColor: "#00ffcc",
        backgroundColor: "rgba(0, 255, 204, 0.2)",
        fill: true,
      },
    ],
  };

  const options = {
    responsive: true,
    maintainAspectRatio: false,
    scales: {
      y: {
        beginAtZero: true,
      },
    },
  };

  return (
    <div className="rounded-xl bg-white/10 backdrop-blur-md p-6 shadow-lg h-64">
      <h3 className="text-lg font-semibold mb-2">{label}</h3>
      <Line data={chartData} options={options} />
    </div>
  );
};
