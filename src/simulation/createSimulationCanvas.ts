import { MAIN } from "../../wasm";

export function createSimulationCanvas(
    simulation: MAIN.VisualInterface,
    canvasRef: HTMLCanvasElement,
    size: number
): () => void {
    const ctx = canvasRef.getContext('2d');
    
    // Set fixed canvas size
    canvasRef.width = size;
    canvasRef.height = size;
    
    let isDestroyed = false;
    
    // Animation loop
    function render() {
        if (isDestroyed) return;
        
        if (!ctx) {
            throw new Error('Failed to get 2D context from canvas');
        }
        
        // Clear canvas
        ctx.clearRect(0, 0, canvasRef.width, canvasRef.height);

        simulation.resetEntityIterator();

        // Get positions from simulation
        let nextEntity = -1;
        while (true) {
            nextEntity = simulation.getNextEntityId();
            if (nextEntity === -1) break;

            const entityType = simulation.getEntityType(nextEntity);
            if (entityType === 0) {
                const player = simulation.getPlayerEntity(nextEntity);

                // Generate a unique color based on entity ID
                const hue = (nextEntity * 137.508) % 360; // Golden angle approximation for good color distribution
                ctx.fillStyle = `hsl(${hue}, 70%, 50%)`;

                // Offset x and y to center of canvas
                const x = player.getX() + canvasRef.width / 2;
                const y = player.getY() + canvasRef.height / 2;

                ctx.beginPath();
                ctx.arc(x, y, player.getRadius(), 0, Math.PI * 2);
                ctx.fill();
                ctx.closePath();
            }
        } 
        
        // Request next frame
        requestAnimationFrame(render);
    }
    
    // Start animation loop
    render();
    
    return () => {
        isDestroyed = true;
    }
}