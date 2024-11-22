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

        setClientSimulations([handler.addClientSimulation(),handler.addClientSimulation(),handler.addClientSimulation()]);

        return () => {
            handler.dispose();
        }
    }, [handler])

    // useEffect(() => {
    //     const interval = setInterval(() => {
    //         setClientSimulations((simulations) => {
    //             return [
    //                 ...simulations,
    //                 handler.addClientSimulation()
    //             ]
    //         })
    //     }, 5000);

    //     return () => {
    //         clearInterval(interval);
    //     }
    // }, [clientSimulations])

    return (
        <>
            <div 
                style={{
                    display: 'flex',
                    flexDirection: 'row',
                    gap: '10px',
                    alignItems: 'flex-end',
                    justifyContent: 'center',
                }}
            >
                <div
                    style={{
                        padding: '5px',
                    }}
                >
                    <label>Server View</label>
                    <SimulationComponent simulation={handler.getController()} handler={handler} />
                </div>
                {clientSimulations.map((simulation, index) => (
                    <ClientSimulationComponent 
                        key={index}
                        simulation={simulation}
                        handler={handler}
                    />
                ))}
            </div>
        </>
    );
};
