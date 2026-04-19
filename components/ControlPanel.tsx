// app/components/ControlPanel.tsx
"use client";
import React, { useState } from "react";
// import { sendControlCommand } from "../lib/api";
import { sendControlCommand } from "@/app/lib/api";

export const ControlPanel: React.FC = () => {
  const [exhaustOn, setExhaustOn] = useState(false);

  const handleToggle = async () => {
    const newState = !exhaustOn;
    setExhaustOn(newState);
    await sendControlCommand(newState ? "ON" : "OFF", "SLOW");
  };

  const handleSpeed = async (speed: string) => {
    if (exhaustOn) await sendControlCommand("ON", speed);
  };

  return (
    <div className="rounded-xl bg-white/10 backdrop-blur-md p-6 shadow-lg">
      <h3 className="text-xl font-semibold mb-4">Exhaust Control</h3>
      <button
        onClick={handleToggle}
        className="px-4 py-2 rounded-lg bg-cyan-400 text-black font-semibold hover:bg-cyan-300 transition"
      >
        {exhaustOn ? "Turn OFF" : "Turn ON"}
      </button>
      <div className="mt-4 flex gap-2">
        {["SLOW", "MID", "FAST"].map((speed) => (
          <button
            key={speed}
            onClick={() => handleSpeed(speed)}
            className="px-4 py-2 rounded-lg bg-gradient-to-r from-green-400 to-blue-500 text-black font-semibold hover:opacity-80 transition"
          >
            {speed}
          </button>
        ))}
      </div>
    </div>
  );
};
