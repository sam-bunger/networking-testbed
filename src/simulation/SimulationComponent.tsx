import React, { useRef, useEffect } from "react";
import { createSimulationCanvas } from "./createSimulationCanvas";
import { MAIN } from "../../wasm";
import { SimulationHandler } from "./SimulationHandler";

interface SimulationComponentProps {
  simulation: MAIN.VisualInterface;
  handler: SimulationHandler;
  size?: number;
}

export const SimulationComponent = ({
  simulation,
  handler,
  size = 400,
}: SimulationComponentProps) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    if (!canvasRef.current) return;
    return createSimulationCanvas(simulation, canvasRef.current, handler, size);
  }, [simulation, handler, size]);

  return (
    <canvas
      ref={canvasRef}
      style={{
        display: "block",
        width: `${size}px`,
        height: `${size}px`,
        border: "1px solid black",
      }}
    />
  );
};
