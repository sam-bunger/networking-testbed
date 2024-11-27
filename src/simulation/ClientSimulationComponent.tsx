import React, { useEffect, useRef, useState } from 'react';
import { MAIN } from '../../wasm';
import { SimulationComponent } from './SimulationComponent';
import { ClientSimulationStats } from './ClientSimulationStats';
import { SimulationHandler } from './SimulationHandler';

interface ClientSimulationComponentProps {
    simulation: MAIN.ClientSimulationController;
    handler: SimulationHandler;
}

export const ClientSimulationComponent = ({ 
    simulation, 
    handler,
}: ClientSimulationComponentProps) => {
    const [isFocused, setIsFocused] = useState(false);
    
    const [packetDelay, setPacketDelay] = useState(50);
    const [packetDropRate, setPacketDropRate] = useState(0);
    const [packetJitter, setPacketJitter] = useState(0);
    const [throughputRate, setThroughputRate] = useState(100);

    const containerRef = useRef<HTMLDivElement>(null);

    useEffect(() => {
        if (isFocused) {
            let isHoldingLeft = false;
            let isHoldingRight = false;
            let isHoldingUp = false;
            let isHoldingDown = false
            let isHoldingSpace = false;

            const manageInputs = () => {
                const gameInput = simulation.getLatestInput();

                if (isHoldingLeft && !isHoldingRight) {
                    gameInput.setLeftRight(-1);
                } else if (isHoldingRight && !isHoldingLeft) {
                    gameInput.setLeftRight(1);
                } else {
                    gameInput.setLeftRight(0);
                }

                if (isHoldingUp && !isHoldingDown) {
                    gameInput.setUpDown(-1);
                } else if (isHoldingDown && !isHoldingUp) {
                    gameInput.setUpDown(1);
                } else {
                    gameInput.setUpDown(0);
                }

                gameInput.setFire(isHoldingSpace);
            }

            const onKeyDown = (event: KeyboardEvent) => {
                if (event.key === 'd') {
                    isHoldingRight = true;
                } else if (event.key === 'a') {
                    isHoldingLeft = true;
                } else if (event.key === 'w') {
                    isHoldingUp = true;
                } else if (event.key === 's') {
                    isHoldingDown = true;
                } else if (event.key === ' ') {
                    isHoldingSpace = true;
                }
                manageInputs();
            }

            const onKeyUp = (event: KeyboardEvent) => {
                if (event.key === 'd') {
                    isHoldingRight = false;
                } else if (event.key === 'a') {
                    isHoldingLeft = false;
                } else if (event.key === 'w') {
                    isHoldingUp = false;
                } else if (event.key === 's') {
                    isHoldingDown = false;
                } else if (event.key === ' ') {
                    isHoldingSpace = false;
                }
                manageInputs();
            }

            window.addEventListener('keydown', onKeyDown);
            window.addEventListener('keyup', onKeyUp);

            return () => {
                const gameInput = simulation.getLatestInput();
                gameInput.setUpDown(0);
                gameInput.setLeftRight(0);

                window.removeEventListener('keydown', onKeyDown);
                window.removeEventListener('keyup', onKeyUp);
            }
        }

        return () => {};
    }, [isFocused]);

    useEffect(() => {
        simulation.getNetwork().setPacketDelay(packetDelay);
    }, [packetDelay]);

    useEffect(() => {
        simulation.getNetwork().setPacketDropRate(packetDropRate);
    }, [packetDropRate]);

    useEffect(() => {
        simulation.getNetwork().setPacketJitter(packetJitter);
    }, [packetJitter]);

    useEffect(() => {
        simulation.getNetwork().setThroughputRate(throughputRate);
    }, [throughputRate]);

  
    
    return (
        <>
            <div 
                ref={containerRef}
                tabIndex={0}
                onFocus={() => setIsFocused(true)}
                onBlur={() => setIsFocused(false)}
                style={{
                    padding: '5px',
                }}
            >
                <ClientSimulationStats simulation={simulation} />
                <div className="network-controls" style={{
                    display: 'flex',
                    flexDirection: 'column',
                    gap: '5px',
                    border: '1px solid black',
                    marginBottom: '10px',     
                    marginTop: '10px',  
                    padding: '10px',
                }}>
                    <label style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                        Packet Delay (ms):
                        <input
                            type="number"
                            value={packetDelay}
                            onChange={(e) => setPacketDelay(Number(e.target.value))}
                            min={0}
                            style={{ width: '180px', marginLeft: '10px' }}
                        />
                    </label>
                    <label style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                        Packet Drop Rate:
                        <input
                            type="number"
                            value={packetDropRate}
                            onChange={(e) => setPacketDropRate(Number(e.target.value))}
                            min={0}
                            max={1}
                            step={0.1}
                            style={{ width: '180px', marginLeft: '10px' }}
                        />
                    </label>
                    <label style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                        Packet Jitter (ms):
                        <input
                            type="number"
                            value={packetJitter}
                            onChange={(e) => setPacketJitter(Number(e.target.value))}
                            min={0}
                            style={{ width: '180px', marginLeft: '10px' }}
                        />
                    </label>
                    <label style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                        Throughput (kbps):
                        <input
                            type="number"
                            value={throughputRate}
                            onChange={(e) => setThroughputRate(Number(e.target.value))}
                            min={0}
                            max={10_000}
                            step={2}
                            style={{ width: '180px', marginLeft: '10px' }}
                        />
                    </label>
                </div>
                <SimulationComponent simulation={simulation} handler={handler} />
            </div>
        </>
    );
}

