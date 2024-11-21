import { MAIN } from "../../wasm";


export class SimulationHandler {

    private controller: MAIN.SimulationController;
    private destroyed: boolean = false;
    private lastTime: number = 0;
    private readonly frameInterval: number = 1000 / 60; // 60 FPS = ~16.67ms per frame
    private readonly maxDeltaTime: number = 200; // Max 200ms between frames (~5 FPS minimum)

    constructor(controller: MAIN.SimulationController) {
        this.controller = controller;
        requestAnimationFrame(this.tick.bind(this));
    }

    private tick(time: number) {
        let deltaTime = time - this.lastTime;
        
        // Cap maximum delta time to prevent huge time jumps
        deltaTime = Math.min(deltaTime, this.maxDeltaTime);
        
        if (deltaTime >= this.frameInterval) {
            this.lastTime = time - (deltaTime % this.frameInterval);
            this.controller.tick();
        }
        
        if (!this.destroyed) {
            requestAnimationFrame(this.tick.bind(this));
        }
    }

    getController(): MAIN.SimulationController {
        return this.controller;
    }

    addClientSimulation(): MAIN.ClientSimulationController {
        return this.controller.createNewClient();
    }

    dispose() 
    {
        this.destroyed = true;
        MAIN.destroy(this.controller)
    }

}