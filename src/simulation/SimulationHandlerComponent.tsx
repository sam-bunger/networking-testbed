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

        setClientSimulations([handler.addClientSimulation(),handler.addClientSimulation(),handler.addClientSimulation(),handler.addClientSimulation(),handler.addClientSimulation()]);

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
                backgroundColor: "#f0f0f0",
                width: '100%',
                height: '100%',
                display: 'flex',
                flexDirection: 'column',
                alignItems: 'center',
                justifyContent: 'center',
            }}
        >
            <div 
                style={{
                    display: 'flex',
                    flexDirection: 'row',
                    gap: '10px',
                    alignItems: 'flex-end',
                    backgroundColor: "#f0f0f0",
                    position: 'absolute',
                    left: 0,
                    right: 0,
                    overflowX: 'scroll',
                    paddingLeft: '20px',
                    paddingRight: '20px',
                }}
            >
                <div
                    style={{
                        padding: '5px',
                        alignSelf: 'flex-end',
                        flexShrink: 0,
                    }}
                >
                    <h3>Server View</h3>
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
        </div>
            
        </>
    );
};
