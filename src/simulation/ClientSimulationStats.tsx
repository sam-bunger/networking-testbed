import React, { useEffect, useState } from 'react';
import { MAIN } from '../../wasm';
import { LineChart, Line, YAxis, ResponsiveContainer } from 'recharts';

interface SimulationComponentProps {
    simulation: MAIN.ClientSimulationController;
}

export const ClientSimulationStats = ({ 
    simulation, 
}: SimulationComponentProps) => {
    const [predictedFrameCountData, setPredictedFrameCountData] = useState<number[]>([]);
    
    useEffect(() => {
        const interval = setInterval(() => {
            const stat = simulation.getLastPredictedFrameCount();
            setPredictedFrameCountData(prev => {
                const newData = [...prev, stat];
                // Keep last 60 frames (1 second of data)
                return newData.slice(-60);
            });
        }, 1000 / 60);

        return () => {
            clearInterval(interval);
        }
    }, [simulation]);

    // Transform data for Recharts
    const chartData = predictedFrameCountData.map((value, index) => ({
        value: value
    }));

    return (
        <div style={{ width: "100%" }}>
            <h3>Predicted Frame Count</h3>
            <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                
                <div style={{ height: '100px', flex: 1 }}>
                    <ResponsiveContainer width="100%" height="100%">
                        <LineChart data={chartData}>
                            <YAxis 
                                domain={[0, 50]} 
                                tick={{ fontSize: 12 }}
                                tickCount={10}  
                                width={20}
                            />
                            <Line
                                type="monotone"
                                dataKey="value"
                                stroke="#4CAF50"
                                strokeWidth={2}
                                dot={false}
                                isAnimationActive={false}
                            />
                        </LineChart>
                    </ResponsiveContainer>
                </div>
                <div style={{ fontSize: '14px', minWidth: '60px' }}>
                    {predictedFrameCountData[predictedFrameCountData.length - 1] || 0}
                </div>
            </div>
        </div>
    );
}

