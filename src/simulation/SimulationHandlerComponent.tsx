import React, { useEffect, useState } from 'react';
import { SimulationHandler } from './SimulationHandler';
import { SimulationComponent } from './SimulationComponent';
import { MAIN } from '../../wasm';
import { ClientSimulationComponent } from './ClientSimulationComponent';

interface SimulationHandlerComponentProps {
    controller: MAIN.SimulationController;
}

export const SimulationHandlerComponent: React.FC<SimulationHandlerComponentProps> = ({ controller }) => {
    const [handler] = React.useState(() => new SimulationHandler(controller));
    const [clientSimulations, setClientSimulations] = useState<MAIN.ClientSimulationController[]>([])

    useEffect(() => {

        setClientSimulations([handler.addClientSimulation(), handler.addClientSimulation()]);

        return () => {
            handler.dispose();
        }
    }, [handler])

    return (
        <>
            {/* Render the server simulation first */}
            <SimulationComponent simulation={handler.getController()} />
            
            <div 
                style={{
                    display: 'flex',
                    flexDirection: 'row',
                    gap: '10px'
                }}
            >
                {clientSimulations.map((simulation, index) => (
                    <ClientSimulationComponent 
                        key={index}
                        simulation={simulation}
                    />
                ))}
            </div>
        </>
    );
};
