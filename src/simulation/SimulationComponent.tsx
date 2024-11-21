import React, { useRef, useEffect } from 'react';
import { createSimulationCanvas } from './createSimulationCanvas';
import { MAIN } from '../../wasm';

interface SimulationComponentProps {
    simulation: MAIN.VisualInterface;
    size?: number;
}

export const SimulationComponent = ({ 
    simulation, 
    size = 400
}: SimulationComponentProps) => {
    const canvasRef = useRef<HTMLCanvasElement>(null);

    useEffect(() => {
        if (!canvasRef.current) return;
        return createSimulationCanvas(simulation, canvasRef.current, size)
    }, [simulation, size]);

    return (
        <canvas 
            ref={canvasRef}
            style={{
                display: 'block',
                width: `${size}px`,
                height: `${size}px`,
                border: '1px solid black'
            }}
        />
    );
}

