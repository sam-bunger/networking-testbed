import React, { useEffect, useRef, useState } from 'react';
import { MAIN } from '../../wasm';
import { SimulationComponent } from './SimulationComponent';

interface ClientSimulationComponentProps {
    simulation: MAIN.ClientSimulationController;
}

export const ClientSimulationComponent = ({ 
    simulation, 
}: ClientSimulationComponentProps) => {
    const [isFocused, setIsFocused] = useState(false);
    const containerRef = useRef<HTMLDivElement>(null);

    useEffect(() => {
        if (isFocused) {
            const onKeyDown = (event: KeyboardEvent) => {
                const gameInput = simulation.getLatestInput();

                if (event.key === 'd') {
                    gameInput.pressRight();
                } else if (event.key === 'a') {
                    gameInput.pressLeft();
                } else if (event.key === 'w') {
                    gameInput.pressUp();
                } else if (event.key === 's') {
                    gameInput.pressDown();
                }
            }

            const onKeyUp = (event: KeyboardEvent) => {
                const gameInput = simulation.getLatestInput();

                if (event.key === 'd') {
                    gameInput.releaseRight();
                } else if (event.key === 'a') {
                    gameInput.releaseLeft();
                } else if (event.key === 'w') {
                    gameInput.releaseUp();
                } else if (event.key === 's') {
                    gameInput.releaseDown();
                }
            }

            window.addEventListener('keydown', onKeyDown);
            window.addEventListener('keyup', onKeyUp);

            return () => {
                const gameInput = simulation.getLatestInput();
                gameInput.releaseRight();
                gameInput.releaseLeft();
                gameInput.releaseUp();
                gameInput.releaseDown();

                window.removeEventListener('keydown', onKeyDown);
                window.removeEventListener('keyup', onKeyUp);
            }
        }

        return () => {};
    }, [isFocused]);
    
    return (
        <>
            <div 
                ref={containerRef}
                tabIndex={0} // Makes the div focusable
                onFocus={() => setIsFocused(true)}
                onBlur={() => setIsFocused(false)}
            >
                <SimulationComponent simulation={simulation} />
            </div>
        </>
    );
}

